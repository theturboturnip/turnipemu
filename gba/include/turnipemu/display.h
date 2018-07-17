#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include <SDL.h>
#include "imgui/imgui.h"

namespace TurnipEmu{
	class Emulator;
	class Display {
	public:
		class CustomWindow {
		public:
			CustomWindow(std::string title, unsigned int width, unsigned int height);
			virtual void drawCustomWindowContents() = 0;

			const std::string& getTitle(){
				return title;
			}
			ImVec2 getSize(){
				return ImVec2(width, height);
			}
		protected:
			std::string title;
			unsigned int width;
			unsigned int height;
		};
		
		Display(std::string title, unsigned int width, unsigned int height);
		~Display();
		void registerEmulator(Emulator* emulator);
		void registerCustomWindow(Emulator* parent, CustomWindow* customWindow);

		void loop();
	protected:
		const unsigned int width;
		const unsigned int height;
		std::unordered_map<Emulator*, std::vector<CustomWindow*>> emulators;

		SDL_Window* sdlWindow = nullptr;
		SDL_GLContext openGlContext;
		ImGuiIO* io;
		
		void render();
		
		void LogAvailableError();
		
		const char* const logTag = "DISPLAY";
	};
}
