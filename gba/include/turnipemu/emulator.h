#pragma once

#include "display.h"

namespace TurnipEmu{
	class MemoryMap;
	
	class Emulator {
		friend class MemoryMap;
		
	public:
		Emulator(Display& display){
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
	protected:
		bool paused = false;
		bool stopped = false;
		bool vsyncReady = false;
	};
	
}
