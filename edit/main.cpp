#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cmath>

#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <filesystem>

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"

#include "Widgets.hpp"

class Timer{
	public:
		using Clock = std::chrono::high_resolution_clock;
		using Seconds = std::chrono::duration<float>;

		Timer() noexcept: startT(Clock::now()), loopT(startT){}

		float elapsed() const noexcept{ return Seconds(Clock::now() - startT).count(); }

		float tickElapsed(const bool doReset = true) noexcept{
			auto loopEndT = Clock::now();
			Seconds loopDt = loopEndT - loopT;
			if(doReset) loopT = loopEndT;
			return loopDt.count();
		}

		void reset() noexcept{
			startT = Clock::now();
			loopT = startT;
		}

	private:
		Clock::time_point startT;
		Clock::time_point loopT;
};

int main(int argc, char *argv[]){
	for(int i = 1; i < argc; i++){
		auto arg = argv[i];
		(void)arg;
	}

	if(SDL_Init(SDL_INIT_VIDEO) != 0){
		edit::bail("Error in SDL_Init: %s\n", SDL_GetError());
	}
	
	std::atexit(SDL_Quit);

	if(TTF_Init() != 0){
		edit::bail("Error in TTF_Init: %s\n", TTF_GetError());
	}

	std::atexit(TTF_Quit);

	const auto formats = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP;

	if((IMG_Init(formats) & ~formats) != 0){
		edit::bail("Error in IMG_Init: %s\n", IMG_GetError());
	}

	FT_Library ft;
	if(FT_Init_FreeType(&ft) != 0){
		edit::bail("Error in FT_Init_FreeType");
	}

	auto monoidFont = edit::Font(ft, "monoid-font/Monoid-Regular.ttf", 10);
	auto monoidFace = monoidFont.faces();

	auto window = edit::EditWindow(1000, 600, monoidFace);

	SDL_Event ev;

	Timer loopTime;

	bool running = true;

	while(running){
		const float dt = loopTime.tickElapsed();

		while(SDL_PollEvent(&ev)){
			switch(ev.type){
				case SDL_QUIT:{
					running = false;
					break;
				}

				case SDL_TEXTINPUT:{
					std::string_view text = ev.text.text;
					window.onTextEntry(text);
					break;
				}

				case SDL_KEYDOWN:
				case SDL_KEYUP:{
					bool pressed = ev.type == SDL_KEYDOWN;
					if(!pressed) break;

					if(ev.key.windowID != SDL_GetWindowID(window.sdl2Handle()))
						break;

					if(ev.key.keysym.sym == SDLK_BACKSPACE){
						window.onBackspace();
					}

					break;
				}

				case SDL_MOUSEMOTION:{
					window.doMotion(ev.motion.x, ev.motion.y);

					if(ev.motion.windowID != SDL_GetWindowID(window.sdl2Handle()))
						break;

					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:{
					bool pressed = ev.type == SDL_MOUSEBUTTONDOWN;

					if(ev.button.windowID != SDL_GetWindowID(window.sdl2Handle()))
						break;

					if(pressed)
						window.doPress(ev.button.button, ev.button.x, ev.button.y);
					else
						window.doRelease(ev.button.button, ev.button.x, ev.button.y);

					break;
				}

				default:
					break;
			}
		}

		window.update(nullptr, dt);
		window.present();
		std::this_thread::yield();
	}

	FT_Done_FreeType(ft);

	return 0;
}
