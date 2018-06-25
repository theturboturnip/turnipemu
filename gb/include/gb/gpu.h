// Copyright Samuel Stark 2017

#pragma once

#include <cstdint>
#include <array>
#include "mmu.h"

namespace GB{
	class CPU;
	
	class GPU{
	public:
		enum class Pixel{
			Black = 0,
			DarkGrey = 1,
			LightGrey = 2,
			White = 3
		};
	protected:
		class Palette{
		public:
			Palette(uint8_t register_value);
			inline Pixel operator[] (int value){
				return palette[value];
			}
		private:
			Pixel palette[4];
		};

		struct Sprite{
			uint8_t y;
			uint8_t x;
			uint8_t tile;
			struct {		
				uint8_t padding : 4;
				
				uint8_t palette : 1;
				bool flip_horizontal : 1;
				bool flip_vertical : 1;
				uint8_t priority : 1; // The :1 means it takes up one bit
				};
		};

		enum class Mode{
			OAMRead = 2,
			VRAMRead = 3,
			HBlank = 0,
			VBlank = 1
		};

		struct Control{
			bool enable_bg : 1; // Bit 0
			bool enable_sprites : 1;
			uint8_t sprite_size : 1; // 0 for 8x8, 1 for 8x16
			uint8_t bg_tile_map : 1;
			uint8_t tile_data : 1;
			bool enable_window : 1;
			uint8_t window_tile_map : 1;
			bool enable_lcd : 1; // Bit 7
		};
		static_assert(sizeof(Control) == 1, "GPUControl should be exactly 1 byte in size");

		struct LCDCStatus{
			uint8_t mode : 2;
			bool scanline_is_eq_to_interrupt_val : 1;

			bool enable_hblank_interrupt : 1;
			bool enable_vblank_interrupt : 1;
			bool enable_oam_interrupt : 1;
			bool enable_scanline_interrupt : 1;

			uint8_t padding : 1;
		};
		static_assert(sizeof(LCDCStatus) == 1, "LCDCStatus should be exactly 1 byte in size");

		
	public:
		constexpr static uint8_t SCREEN_WIDTH = 160;
		constexpr static uint8_t SCREEN_HEIGHT = 144;

		constexpr static uint8_t TILE_MAP_WIDTH = 32;
		constexpr static uint8_t TILE_MAP_HEIGHT = 32;
	
		GPU(GB::CPU& cpu);

		uint8_t vram[0x2000];
		union{
			uint8_t spriteinfo[0x100];
			Sprite spriteinfo_as_sprites[40];
		};
		static_assert(sizeof(spriteinfo_as_sprites) <= sizeof(spriteinfo), "Reinterpreted Sprite Info is larger than actual!");
	
		Pixel framebuffer[SCREEN_WIDTH*SCREEN_HEIGHT];
	
		void step();
		void reset();

		void set_lcdc_status(uint8_t from_byte);
		uint8_t get_lcdc_status(void);

		// TODO: Use LCDCStatus.mode instead?
		Mode mode = Mode::HBlank;
	
	protected:
		void set_scanline(int);
		void render_scanline();
		void render_to_framebuffer();
	
		CPU& cpu;
		int line_counter = 0;
		int mode_clock = 0;

		LCDCStatus current_lcdc_status;
	};
}
