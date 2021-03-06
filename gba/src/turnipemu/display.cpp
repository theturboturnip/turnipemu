#include "turnipemu/display.h"

#include "turnipemu/emulator.h"
#include "turnipemu/log.h"
#include "turnipemu/gl.h"

#define TURNIPEMU_INCLUDE_IMGUI_DISPLAY_EXTENSIONS
#include "turnipemu/imgui.h"

namespace TurnipEmu{
	Display::Display(std::string title, unsigned int width, unsigned int height) : width(width), height(height){
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
			LogAvailableError();
			return;
		}
    
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
    
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		LogAvailableError();
    
		auto flags = SDL_WINDOW_OPENGL;// | SDL_WINDOW_RESIZABLE;
		sdlWindow = SDL_CreateWindow(title.c_str(),
									 SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
									 width, height,
									 flags);
		if (sdlWindow == nullptr){
			LogAvailableError();
			return;
		}

		openGlContext = SDL_GL_CreateContext(sdlWindow);
		if (!openGlContext){
			LogAvailableError();
			return;
		}
    
		int major, minor;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
		LogLine(logTag, "Using OpenGL version %d.%d", major, minor);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		glClearColor(0.0f,0.0f,0.4f,0.0f);
		LogAvailableError();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		io = &ImGui::GetIO();
		ImGui_ImplSDL2_InitForOpenGL(sdlWindow, openGlContext);
		ImGui_ImplOpenGL3_Init("#version 150");
		ImGui::StyleColorsDark();
	}
	Display::~Display(){
		if (sdlWindow){
			if (openGlContext){
				ImGui_ImplOpenGL3_Shutdown();
				ImGui::DestroyContext();
				
				SDL_GL_DeleteContext(openGlContext);
			}
			SDL_DestroyWindow(sdlWindow);
			SDL_Quit();
		}
	}
	void Display::LogAvailableError(){
		const char* sdlError = SDL_GetError();
		if (*sdlError != '\0')
			LogLine(logTag, "SDL Error: %s", sdlError);
		GLuint glError = glGetError();
		if (glError)
			LogLine(logTag, "OpenGL Error: %d", glError);
	}
	void Display::registerEmulator(Emulator* emulator){
		emulators[emulator].clear();
		emulators[emulator].push_back(emulator);
	}
	void Display::registerCustomWindow(Emulator* parentEmulator, CustomWindow* customWindow){
		emulators[parentEmulator].push_back(customWindow);
	}

	void Display::loop(){
		bool done = false;
		while (!done)
		{
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				ImGui_ImplSDL2_ProcessEvent(&event);
				if (event.type == SDL_QUIT)
					done = true;
				if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(sdlWindow))
					done = true;
			}
			for (auto& emulatorWindowPair : emulators){
				auto emulator = emulatorWindowPair.first;
				if (!emulator->isStopped())
					emulator->tickUntilVSyncOrStopped();
			}
			render();
		}
	}
	void Display::render(){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(sdlWindow);
		ImGui::NewFrame();

		int windowGroupId = 0;
		for (auto& emulatorWindowPair : emulators){
			ImGui::PushID(windowGroupId);
			for (CustomWindow* customWindow : emulatorWindowPair.second){
				auto windowSize = customWindow->getSize();
				ImGui::SetNextWindowSize(windowSize);
				if (ImGui::Begin(customWindow->getTitle().c_str(), nullptr, ImGuiWindowFlags_NoResize)){
					customWindow->drawCustomWindowContents();
				}
				ImGui::End();
			}
			ImGui::PopID();
			windowGroupId++;
		}
		ImGui::ShowDemoWindow(nullptr);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_MakeCurrent(sdlWindow, openGlContext);
        glViewport(0, 0, (int)io->DisplaySize.x, (int)io->DisplaySize.y);
		SDL_GL_SwapWindow(sdlWindow);
	}

	Display::CustomWindow::CustomWindow(std::string title, unsigned int width, unsigned int height)
		: title(title), width(width), height(height) {}
}
