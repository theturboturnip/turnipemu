// Copyright Samuel Stark 2017

#include "gb/cpu.h"
#include "gb/gpu.h"

#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>

#include "SDL.h"
#include "timer.h"

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;

int sdl_setup(int width, int height){
	/* Initialize SDL. */
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		return 1;

	/* Create the window where we will draw. */
	window = SDL_CreateWindow("GameBoy",
							  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							  width, height,
							  0);
	if(window == nullptr){
		fprintf(stderr, "Failed to create window!\n");
		return 1;
	}
		
	/* We must call SDL_CreateRenderer in order for draw calls to affect this window. */
	renderer = SDL_CreateRenderer(window, -1, 0);
	if(renderer == nullptr){
		fprintf(stderr, "Failed to create renderer!\n");
		return 1;
	}

	/* Select the color for drawing. It is set to red here. */
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

	/* Clear the entire screen to our selected color. */
	SDL_RenderClear(renderer);

	/* Up until now everything was drawn behind the scenes.
	   This will show the new, red contents of the window. */
	SDL_RenderPresent(renderer);

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (texture == nullptr){
		fprintf(stderr, "Failed to create texture\n");
		return 1;
	}

	return 0;
}
void sdl_cleanup(){
	SDL_DestroyTexture(texture);
	texture = nullptr;
	SDL_DestroyRenderer(renderer);
	renderer = nullptr;
	SDL_DestroyWindow(window);
	window = nullptr;
}

static bool wants_quit = false;
void test_input(GB::CPU& cpu){
	SDL_Event keyevent;    //The SDL event that we will poll to get events.
	while (SDL_PollEvent(&keyevent)){
		if (keyevent.type == SDL_QUIT)
			wants_quit = true;
		else if (keyevent.type == SDL_KEYDOWN){
			switch(keyevent.key.keysym.sym){
			case SDLK_UP:
				cpu.input.on_direction_down(GB::Input::Direction::Up);
				break;
			case SDLK_DOWN:
				cpu.input.on_direction_down(GB::Input::Direction::Down);
				break;
			case SDLK_LEFT:
				cpu.input.on_direction_down(GB::Input::Direction::Left);
				break;
			case SDLK_RIGHT:
				cpu.input.on_direction_down(GB::Input::Direction::Right);
				break;
			case SDLK_RETURN:
				cpu.input.on_button_down(GB::Input::Button::Start);
				break;
			case SDLK_RSHIFT:
				cpu.input.on_button_down(GB::Input::Button::Select);
				break;
			case SDLK_a:
				cpu.input.on_button_down(GB::Input::Button::A);
				break;
			case SDLK_s:
			case SDLK_b:
				cpu.input.on_button_down(GB::Input::Button::B);
				break;
			default:
				break;
			}
		}else if (keyevent.type == SDL_KEYUP){
			switch(keyevent.key.keysym.sym){
			case SDLK_UP:
				cpu.input.on_direction_up(GB::Input::Direction::Up);
				break;
			case SDLK_DOWN:
				cpu.input.on_direction_up(GB::Input::Direction::Down);
				break;
			case SDLK_LEFT:
				cpu.input.on_direction_up(GB::Input::Direction::Left);
				break;
			case SDLK_RIGHT:
				cpu.input.on_direction_up(GB::Input::Direction::Right);
				break;
			case SDLK_RETURN:
				cpu.input.on_button_up(GB::Input::Button::Start);
				break;
			case SDLK_RSHIFT:
				cpu.input.on_button_up(GB::Input::Button::Select);
				break;
			case SDLK_a:
				cpu.input.on_button_up(GB::Input::Button::A);
				break;
			case SDLK_s:
			case SDLK_b:
				cpu.input.on_button_up(GB::Input::Button::B);
				break;
			default:
				break;
			}
		}
	}
}

void sdl_update_window(GB::CPU& cpu){
	
	test_input(cpu);

	GB::GPU& gpu = cpu.gpu;
	
	uint32_t* pixels = nullptr;
	int pitch = 0;
	uint32_t format;

	/*static t_sl_timer sdl_timer;
	static t_sl_timer sdl_frame_timer;
	static int sdl_timer_count;
	if (sdl_timer_count==0) {
		SL_TIMER_EXIT(sdl_frame_timer);
		fprintf(stderr,"Each of 100 ticks took %fms\n", SL_TIMER_VALUE_US(sdl_timer) / 1000.f / 100);
		fprintf(stderr,"100 Frames each took %fms\n", SL_TIMER_VALUE_US(sdl_frame_timer) / 1000.f / 100);
		SL_TIMER_INIT(sdl_timer);
		SL_TIMER_INIT(sdl_frame_timer);
		SL_TIMER_ENTRY(sdl_frame_timer);
	}
	sdl_timer_count = (sdl_timer_count+1) % 100;
	SL_TIMER_ENTRY(sdl_timer);*/
	SDL_QueryTexture(texture, &format, nullptr, nullptr, nullptr);
	if (SDL_LockTexture(texture, nullptr, (void**)&pixels, &pitch))
	{
		fprintf(stderr, "Failed to lock texture: %s\n", SDL_GetError());
		return;
	}
	
	SDL_PixelFormat* pixelFormat = SDL_AllocFormat(format);

	for (int pixelPosition = 0; pixelPosition < GB::GPU::SCREEN_WIDTH * GB::GPU::SCREEN_HEIGHT; pixelPosition++){
		uint8_t color = 255 * (static_cast<int>(gpu.framebuffer[pixelPosition]) * 1.0f/3);
		color = 255 - color;
		pixels[pixelPosition] = SDL_MapRGBA(pixelFormat, color, color, color, 0);
	}

	SDL_UnlockTexture(texture);
	
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
	//SL_TIMER_EXIT(sdl_timer);
}

int main(int argc, char* argv[]){
	/* Setup the BIOS and ROM */
	assert(argc >= 3);
	
	const char* bios_path = argv[1];
	std::array<uint8_t, GB::MMU::BIOS_SIZE> bios;
	std::ifstream bios_file(bios_path, std::ios::binary);
	bios_file >> std::noskipws;
	bios_file.read(reinterpret_cast<char*>(bios.data()), bios.size());
	
	const char* rom_path = argv[2];
	std::vector<uint8_t> rom;
	std::ifstream rom_file(rom_path, std::ios::binary);
	rom_file >> std::noskipws;
	rom_file.seekg(0, std::ios::end);
	rom.reserve(rom_file.tellg());
	rom_file.seekg(0, std::ios::beg);
	rom.insert(rom.begin(),
			   std::istreambuf_iterator<char>(rom_file),
               std::istreambuf_iterator<char>());

	if (argc >= 4 && strcmp(argv[3], "--no-limit") == 0)
		GB::CPU::limit_fps = false;
	
	/* Create the CPU */
	GB::CPU cpu(std::move(bios), std::move(rom), sdl_update_window);
	cpu.reset();
	//cpu.check_instructions();
	//return 0;
	//cpu.manual_step_requested = true;
	cpu.exit_bios();
	cpu.registers.pc = 0x100;
	
	/* Setup the Graphics */
	if (sdl_setup(GB::GPU::SCREEN_WIDTH, GB::GPU::SCREEN_HEIGHT) == 1)
		return 1;
	
	while(!cpu.stopped && !wants_quit){
		if (cpu.manual_step_requested){
			std::string input;
			std::getline(std::cin, input);
			if (input == "go"){
				cpu.manual_step_requested = false;
			}else if (input == "ret"){
				cpu.waiting_for_ret = true;
				cpu.manual_step_requested = false;
			}
		}
		cpu.step();
	}


	SDL_Event keyevent;    //The SDL event that we will poll to get events.
	while (!wants_quit && SDL_WaitEvent(&keyevent))   //Poll our SDL key event for any keystrokes.
	{
		if (keyevent.type == SDL_QUIT)
			wants_quit = true;
	}

	sdl_cleanup();
	
	return 0;
}
