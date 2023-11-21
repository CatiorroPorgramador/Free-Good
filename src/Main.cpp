#include "Game.hpp"

int main(int argc, char** argv) {
	Engine::Init();
	Engine::MainRow = Engine::ScriptLoadLua("data/scripts/Engine.lua");

	std::string window_name;
	int window_width, window_height;
    uint8_t window_bg_color[3];
	
	if (Engine::ScriptCheckLua()) {
		lua_getglobal(Engine::LuaState, "Engine");
		if (lua_istable(Engine::LuaState, -1)) {
			lua_pushstring(Engine::LuaState, "WindowName");
			lua_gettable(Engine::LuaState, -2);
			window_name = lua_tostring(Engine::LuaState, -1);
			lua_pop(Engine::LuaState, 1);

			lua_pushstring(Engine::LuaState, "WindowWidth");
			lua_gettable(Engine::LuaState, -2);
			window_width = lua_tointeger(Engine::LuaState, -1);
			lua_pop(Engine::LuaState, 1);

			lua_pushstring(Engine::LuaState, "WindowHeight");
			lua_gettable(Engine::LuaState, -2);
			window_height = lua_tointeger(Engine::LuaState, -1);
			lua_pop(Engine::LuaState, 1);

            lua_pushstring(Engine::LuaState, "WindowBackgroundColor");
			lua_gettable(Engine::LuaState, -2);
            if (lua_istable(Engine::LuaState, -1)) {
                lua_rawgeti(Engine::LuaState, -1, 1);
                window_bg_color[0] = lua_tointeger(Engine::LuaState, -1);
                lua_pop(Engine::LuaState, 1);

                lua_rawgeti(Engine::LuaState, -1, 2);
                window_bg_color[1] = lua_tointeger(Engine::LuaState, -1);
                lua_pop(Engine::LuaState, 1);

                lua_rawgeti(Engine::LuaState, -1, 3);
                window_bg_color[2] = lua_tointeger(Engine::LuaState, -1);
                lua_pop(Engine::LuaState, 1);
            }
            lua_pop(Engine::LuaState, 1);
		} else {return 1;}
	}

	SDL_Window* window = SDL_CreateWindow(window_name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	Engine::Scene *default_scene = new Engine::GamePlay();
    default_scene->Init(renderer);

	bool running = true;
    while(running){
        // Events
        SDL_Event event;

        while(SDL_PollEvent(&event)) {
            switch(event.type){
                case SDL_QUIT:
                    running = false;
                    break;
                
                case SDL_KEYDOWN:
                    default_scene->InputDown(event.key.keysym.sym);
                    break;
                case SDL_KEYUP:
                    default_scene->InputUp(event.key.keysym.sym);
                    break;

                case SDL_MOUSEMOTION:
                    default_scene->MouseMotion(event.button);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    default_scene->MouseDown(event.button);
                    break;
                case SDL_MOUSEBUTTONUP:
                    default_scene->MouseUp(event.button);
                    break;
                default:
                    break;
            }
        }

        /* Update */
        default_scene->Update();

        SDL_SetRenderDrawColor(renderer, window_bg_color[0], window_bg_color[1], window_bg_color[2], 255);
        SDL_RenderClear(renderer);
        default_scene->Render();

        // Debbuug
        SDL_RenderPresent(renderer);
        SDL_Delay(16.7);
    }

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	Engine::End();

	return 0;
}