#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#ifdef _WIN32
#pragma comment(lib, "libs/lua54.lib")
#endif

#define frq 22050
#define cnl 1
#define buffer 4096
#define frm AUDIO_S16SYS

namespace Engine {
    float offset[2] = {0, 0};

    lua_State *LuaState;
    int MainRow;

    void Init() {
        SDL_Init(SDL_INIT_EVERYTHING);
        Mix_OpenAudio(frq, frm, cnl, buffer);
        TTF_Init();

        LuaState = luaL_newstate();
        luaL_openlibs(LuaState);
    }

    void End() {
        lua_close(LuaState);
        Mix_CloseAudio();
        TTF_Quit();
    }

    //
    //
    // SELF ENGINE
    //
    //

    class Window {
        SDL_Window* w;
        SDL_Renderer* r;
    
    public:
        Window(const char* title, int width, int height) {
            w = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
            r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);
        }

        ~Window() {
            SDL_DestroyWindow(w);
            SDL_DestroyRenderer(r);
        }

        void Begin() {
            SDL_SetRenderDrawColor(r, 20, 20, 20, 255);
            SDL_RenderClear(r);
        }

        void End(){
            SDL_RenderPresent(r);
            SDL_Delay(16.7);
        }

        // ----- //
        void DrawTexture(SDL_Texture* texture, SDL_Rect* rect, SDL_Rect* src) {
            SDL_RenderCopy(this->r, texture, rect, src);
        }

        void DrawRect(SDL_Rect* rect, bool fill=false) {
            if (fill)
                SDL_RenderDrawRect(this->r, rect);
            else
                SDL_RenderFillRect(this->r, rect);
        }
    };

    class Scene {
    public:
        virtual ~Scene(){};

        bool pause;

        SDL_Renderer* renderer;
        int mouse_x, mouse_y;

        virtual void Init(SDL_Renderer*) {}
        virtual void InputDown(SDL_Keycode) {}
        virtual void InputUp(SDL_Keycode) {}
        virtual void MouseMotion(SDL_MouseButtonEvent) {}
        virtual void MouseDown(SDL_MouseButtonEvent) {}
        virtual void MouseUp(SDL_MouseButtonEvent) {}
        virtual void Update() {}
        virtual void Render() {}
    protected:
    };

    //
    //
    // LUA
    //
    //

    bool ScriptCheckLua() {
        if (MainRow != LUA_OK) {
            std::cout << lua_tostring(LuaState, -1);
            return false;
        }

        return true;
    }

    int ScriptLoadLua(const char* filename) {
        return luaL_dofile(LuaState, filename);
    }
}


#endif