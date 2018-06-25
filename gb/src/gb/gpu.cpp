// Copyright Samuel Stark 2017

#include "gb/gpu.h"
#include "gb/cpu.h"

#include <cstring>
#include <chrono>
#include <thread>

namespace GB{
	GPU::Palette::Palette(uint8_t register_value){
		for (int i = 0; i < 4; ++i){
			palette[i] = static_cast<Pixel>((register_value >> (i * 2)) & 0b11);
		}
	}

	GPU::GPU(CPU& cpu) : cpu(cpu){}

	void GPU::reset(){
		memset(vram, 0, sizeof(vram)); 
		memset(spriteinfo, 0, sizeof(spriteinfo));
		memset(framebuffer, static_cast<int>(Pixel::Black), sizeof(framebuffer));

		mode = Mode::HBlank;
		mode_clock = 0;
		line_counter = 0;

		set_lcdc_status(0);
	}

	void GPU::step(){
		if (cpu.mmu.dma_timer >= 0) return;
		
		mode_clock += cpu.clock_cycles_this_step;

		switch(mode){
		case Mode::OAMRead:
			if (mode_clock >= 80){
				mode_clock -= 80;
				mode = Mode::VRAMRead;
			}
			break;
		case Mode::VRAMRead:
			if (mode_clock >= 172){
				mode_clock -= 172;

				render_scanline();
				mode = Mode::HBlank;
				if (current_lcdc_status.enable_hblank_interrupt){
					cpu.interrupts.trigger(Interrupt::LcdStat);
				}
			}
			break;
		case Mode::HBlank:
			if (mode_clock >= 204){
				mode_clock -= 204;
				set_scanline(line_counter + 1);

				if (line_counter >= SCREEN_HEIGHT){
					render_to_framebuffer(); // This does both VBlank interrupts
					set_scanline(0);
					mode = Mode::VBlank;
				}else{
					mode = Mode::OAMRead;
					if (current_lcdc_status.enable_oam_interrupt){
						cpu.interrupts.trigger(Interrupt::LcdStat);
					}
				}
			}
			break;
		case Mode::VBlank:
			if (mode_clock >= 456){
				mode_clock -= 456;
				set_scanline(line_counter + 1);

				if (line_counter >= 153){
					mode = Mode::OAMRead;
					if (current_lcdc_status.enable_oam_interrupt){
						cpu.interrupts.trigger(Interrupt::LcdStat);
					}
					set_scanline(0);
				}
			}
		}
	}

	void GPU::set_scanline(int new_scanline){
		line_counter = new_scanline;
		cpu.mmu.write_byte(MMU::GPU_SCANLINE_ADDRESS, static_cast<uint8_t>(line_counter + 1));
		if (line_counter+1 == cpu.mmu.read_byte(MMU::GPU_SCANLINE_INTERRUPT_VALUE_ADDRESS)){
			current_lcdc_status.scanline_is_eq_to_interrupt_val = true;
			if (current_lcdc_status.enable_scanline_interrupt){
				cpu.interrupts.trigger(Interrupt::LcdStat);
			}
		}else{
			current_lcdc_status.scanline_is_eq_to_interrupt_val = false;
		}
	}

	void GPU::render_scanline(){
		uint8_t gpu_control_byte = cpu.mmu.read_byte(MMU::GPU_CONTROL_ADDRESS);
		Control gpu_control = *((Control*)&gpu_control_byte);

		if (!gpu_control.enable_lcd){
			Pixel* current_framebuffer_pixel = framebuffer + line_counter * SCREEN_WIDTH;
			for (int i = 0; i < SCREEN_WIDTH; ++i){
				(*current_framebuffer_pixel++) = Pixel::White;
			}
			return;
		}	  

		int scanline_color_map[SCREEN_WIDTH] = {0};

		if (gpu_control.enable_bg){
			Palette background_palette(cpu.mmu.read_byte(MMU::GPU_BG_PALETTE_ADDRESS));

			const uint8_t scroll_x = cpu.mmu.read_byte(MMU::GPU_SCROLL_X_ADDRESS);
			const uint8_t scroll_y = cpu.mmu.read_byte(MMU::GPU_SCROLL_Y_ADDRESS);

			const uint16_t bg_tile_map_start = gpu_control.bg_tile_map ? 0x1c00 : 0x1800;
			const uint16_t bg_tile_line_start_offset = (((line_counter + scroll_y) & 0xff) / 8);
			const uint16_t bg_tile_line_start = bg_tile_map_start + bg_tile_line_start_offset * 32;
			uint16_t bg_tile_line_offset = scroll_x / 8;

			uint8_t sprite_x = scroll_x % 8;
			const uint8_t sprite_y = (line_counter + scroll_y) % 8;

			uint16_t current_tile_index;
			uint8_t current_row_byte_1;
			uint8_t current_row_byte_2;

			auto update_current_tile_data = [&]{
				// From 0 to 383
				current_tile_index = vram[bg_tile_line_start + bg_tile_line_offset];
				if (gpu_control.tile_data == 0){
					current_tile_index = 256 + ((int8_t)(uint8_t)current_tile_index);
				}

				// Each tile = 16 bytes, each row takes up 2 bytes
				current_row_byte_1 = vram[current_tile_index * 16 + sprite_y * 2 + 0];
				current_row_byte_2 = vram[current_tile_index * 16 + sprite_y * 2 + 1];
			};
			update_current_tile_data();

			Pixel* current_framebuffer_pixel = framebuffer + line_counter * SCREEN_WIDTH;
		
			for (int i = 0; i < SCREEN_WIDTH; ++i){
				uint8_t bit1 = (current_row_byte_1 >> (7 - sprite_x)) & 1;
				uint8_t bit2 = (current_row_byte_2 >> (7 - sprite_x)) & 1;
				uint8_t palette_index = bit1 + // Take the first bit from the first row
					bit2 * 2; // Take the second bit from the second row
				// If Color 0 is used then sprites can display underneath, so store the color index.
				scanline_color_map[i] = palette_index;
			
				*(current_framebuffer_pixel++) = background_palette[palette_index];

				sprite_x++;
				if (sprite_x == 8){
					sprite_x = 0;
					bg_tile_line_offset = (bg_tile_line_offset + 1) % TILE_MAP_WIDTH;
				
					update_current_tile_data();
				}
			}
		}

		if (gpu_control.enable_window && cpu.mmu.read_byte(0xFF4A) <= 143 && cpu.mmu.read_byte(0xFF4B) > 6){
			//fprintf(stdout, "Window X: %d, Window Y: %d\n", cpu.mmu.read_byte(0xFF4B), cpu.mmu.read_byte(0xFF4A));
			Palette window_palette(cpu.mmu.read_byte(MMU::GPU_BG_PALETTE_ADDRESS));

			const uint8_t window_x = cpu.mmu.read_byte(MMU::GPU_WINDOW_X_ADDRESS) - 7;
			const uint8_t window_y = cpu.mmu.read_byte(MMU::GPU_WINDOW_Y_ADDRESS);

			if (window_x <= 166 && window_y <= 143 && line_counter >= window_y){
				const uint16_t window_tile_map_start = gpu_control.window_tile_map ? 0x1c00 : 0x1800;
				const uint16_t window_tile_line_start_offset = (((line_counter - window_y) & 0xff) / 8);
				const uint16_t window_tile_line_start = window_tile_map_start + window_tile_line_start_offset * 32;
				uint16_t window_tile_line_offset = window_x / 8;

				uint8_t sprite_x = 0;//window_x % 8;
				const uint8_t sprite_y = (line_counter - window_y) % 8;

				uint16_t current_tile_index;
				uint8_t current_row_byte_1;
				uint8_t current_row_byte_2;

				auto update_current_tile_data = [&]{
					// From 0 to 383
					current_tile_index = vram[window_tile_line_start + window_tile_line_offset];
					if (gpu_control.tile_data == 0){
						current_tile_index = 256 + ((int8_t)(uint8_t)current_tile_index);
					}

					// Each tile = 16 bytes, each row takes up 2 bytes
					current_row_byte_1 = vram[current_tile_index * 16 + sprite_y * 2 + 0];
					current_row_byte_2 = vram[current_tile_index * 16 + sprite_y * 2 + 1];
				};
				update_current_tile_data();

				Pixel* current_framebuffer_pixel = framebuffer + line_counter * SCREEN_WIDTH;
		
				for (int i = window_x; i < SCREEN_WIDTH; ++i){
					uint8_t bit1 = (current_row_byte_1 >> (7 - sprite_x)) & 1;
					uint8_t bit2 = (current_row_byte_2 >> (7 - sprite_x)) & 1;
					uint8_t palette_index = bit1 + // Take the first bit from the first row
						bit2 * 2; // Take the second bit from the second row
					// If Color 0 is used then sprites can display underneath, so store the color index.
					scanline_color_map[i] = palette_index;
			
					*(current_framebuffer_pixel++) = window_palette[palette_index];

					sprite_x++;
					if (sprite_x == 8){
						sprite_x = 0;
						window_tile_line_offset = (window_tile_line_offset + 1) % TILE_MAP_WIDTH;
				
						update_current_tile_data();
					}
				}
			}
		}
		
		if (gpu_control.enable_sprites){
			assert(gpu_control.sprite_size == 0); // TODO: 8x16 sprite support
		
			Palette sprite_palettes[2] = {
				Palette(cpu.mmu.read_byte(MMU::GPU_SPRITE_PALETTE_0_ADDRESS)),
				Palette(cpu.mmu.read_byte(MMU::GPU_SPRITE_PALETTE_1_ADDRESS))
			};

			uint8_t sprite_index = 0;
			constexpr uint8_t SPRITE_COUNT = 40;
			uint8_t sprites_encountered = 0;
			constexpr uint8_t MAX_SPRITES_PER_LINE = 10;

			// TODO: The ordering isn't accurate.
			// If >10 sprites are on the same line then they should be displayed in order of x (lowest X value first),
			// until 10 have been displayed. This will instead display them in the order provided until it hits 10 sprites.
		
			while (sprite_index < SPRITE_COUNT && sprites_encountered < MAX_SPRITES_PER_LINE){
				const Sprite sprite = spriteinfo_as_sprites[sprite_index];
				const int sprite_x_pos = sprite.x - 8;
				const int sprite_y_pos = sprite.y - 16;

			
				if (sprite_x_pos >= 0 && sprite_x_pos < SCREEN_WIDTH){
					if (sprite_y_pos <= line_counter && (sprite_y_pos + 8) > line_counter){
						Palette& sprite_palette = sprite_palettes[sprite.palette];

						const uint8_t sprite_y = sprite.flip_vertical ? (7 - (line_counter - sprite_y_pos)) : (line_counter - sprite_y_pos);

						// Sprites always use tile_data = 0
						uint8_t current_tile_index = sprite.tile;

						// Each tile = 16 bytes, each row takes up 2 bytes
						uint8_t current_row_byte_1 = vram[current_tile_index * 16 + sprite_y * 2 + 0];
						uint8_t current_row_byte_2 = vram[current_tile_index * 16 + sprite_y * 2 + 1];

						Pixel* start_framebuffer_pixel = framebuffer + line_counter * SCREEN_WIDTH + sprite_x_pos;
				
						for (uint8_t x = 0; x < 8; x++){
							if (sprite_x_pos + x >= SCREEN_WIDTH) continue;
							if (sprite_x_pos + x < 0) continue;

							uint8_t sprite_x = sprite.flip_horizontal ? (7 - x) : x;
					
							uint8_t bit1 = (current_row_byte_1 >> (7 - sprite_x)) & 1;
							uint8_t bit2 = (current_row_byte_2 >> (7 - sprite_x)) & 1;
							uint8_t palette_index = bit1 + // Take the first bit from the first row
								bit2 * 2; // Take the second bit from the second row

							if (palette_index == 0) continue;

							if (!sprite.priority || (scanline_color_map[sprite_x_pos + x] == 0)){
								// Finally draw the sprite if it takes priority over the BG || if the BG was transparent here.
								*(start_framebuffer_pixel + x) = sprite_palette[palette_index];
								scanline_color_map[sprite_x_pos + x] = 1;
							}
						}
						sprites_encountered++;
					}
   				}
		  
				sprite_index++;
			}
		}
	}
	void GPU::render_to_framebuffer(){
		cpu.interrupts.trigger(Interrupt::VBlank);
		cpu.on_vblank(cpu);
		if (current_lcdc_status.enable_vblank_interrupt) cpu.interrupts.trigger(Interrupt::LcdStat);
		if (CPU::limit_fps) std::this_thread::sleep_for(std::chrono::milliseconds(17));
	}

	void GPU::set_lcdc_status(uint8_t from_byte){
		LCDCStatus new_status = *(LCDCStatus*)(&from_byte);

		current_lcdc_status.enable_hblank_interrupt = new_status.enable_hblank_interrupt;
		current_lcdc_status.enable_vblank_interrupt = new_status.enable_vblank_interrupt;
		current_lcdc_status.enable_oam_interrupt = new_status.enable_oam_interrupt;
		current_lcdc_status.enable_scanline_interrupt = new_status.enable_scanline_interrupt;

		/*
		fprintf(stdout, "Set LCDCStatus to 0x%02x\n\tHblank %d\n\tVblank: %d\n\tOam: %d\n\tscanline: %d\n",
		  from_byte,
		  new_status.enable_hblank_interrupt,
		  new_status.enable_vblank_interrupt,
		  new_status.enable_oam_interrupt,
		  new_status.enable_scanline_interrupt);
		//*/
	}
	uint8_t GPU::get_lcdc_status(){
		current_lcdc_status.mode = static_cast<uint8_t>(mode);

		uint8_t status_as_byte = *(uint8_t*)(&current_lcdc_status);
		return status_as_byte;
	}
}
