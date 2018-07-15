#pragma once

#include <vector>
#include <string>

#include <SDL.h>
#include "imgui/imgui.h"

namespace TurnipEmu{
	// TODO: Move into separate file
	class Emulator {
	public:
		virtual void tick() = 0;
		virtual void handleInput(SDL_Event event);
	};
	
	class Display {
	public:
		class CustomWindow {
		public:
			const unsigned int width;
			const unsigned int height;
			const std::string title;

			CustomWindow(std::string title, const unsigned int width, const unsigned int height);
			virtual void draw() = 0;
		};
		
		Display(std::string title, unsigned int width, unsigned int height);
		~Display();
		void registerEmulator(Emulator* emulator);
		void registerCustomWindow(CustomWindow* customWindow);

		void loop();
	protected:
		const unsigned int width;
		const unsigned int height;
		std::vector<Emulator*> emulators;
		std::vector<CustomWindow*> customWindows;

		SDL_Window* sdlWindow = nullptr;
		SDL_GLContext openGlContext;
		ImGuiIO* io;
		
		void render();
		
		void LogAvailableError();
		
		const char* const logTag = "DISPLAY";
	};
}
