#pragma once

#include "display.h"

namespace TurnipEmu{
	namespace Memory{
		class Map;
	}
		
	class Emulator : public Display::CustomWindow {
		friend class Memory::Map;
		
	public:
		Emulator(Display& display, std::string title)
			: Display::CustomWindow(title, 0, 0){
			display.registerEmulator(this);
		}
		
		virtual void tick() = 0;
		void tickUntilVSyncOrStopped(){
			while(!paused && !stopped && !vsyncReady){
				this->tick();
			}
		}
		virtual void handleInput(SDL_Event event){}

		inline bool isStopped(){
			return stopped;
		}
		virtual void reset(){
			paused = false;
			stopped = false;
			vsyncReady = false;
		}

		void drawCustomWindowContents() override;
	protected:
		bool paused = false;
		bool stopped = false;
		bool vsyncReady = false;
		std::string stopMessage;
	};
	
}
