#pragma once

#include "Engine/Engine.h"

namespace Engine {
    enum COLLISIONS {
        DOWN = 1,
        UP = 0,
        RIGHT = 1,
        LEFT = 0,
    };

    float direction[2];
    float pos[2];

    class TileMap {
        SDL_Texture *texture;

        SDL_Rect *tmp_rect;
        SDL_Rect *tmp_src;

        uint8_t tile_size = 16*4;

    public:
        uint8_t coordinates[10][20];
        size_t length = 20;

        TileMap() {
            tmp_rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
            tmp_src = (SDL_Rect*)malloc(sizeof(SDL_Rect));

            tmp_rect->x = 0;
            tmp_rect->y = 0;
            tmp_rect->w = tile_size;
            tmp_rect->h = tile_size;

            tmp_rect->x = 0;
            tmp_src->y = 0;
            tmp_src->w = 16;
            tmp_src->h = 16;
        }

        ~TileMap() {
            SDL_DestroyTexture(texture);

            free(tmp_rect);
            free(tmp_src);
        }

        void Init(SDL_Renderer* renderer) {
            SDL_Surface* surface = IMG_Load("data/tilemap-sheet.png");
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);

            for (int i{0}; i < 20; ++i) {
                if (i != 4)
                    coordinates[8][i] = 1;
            }
        }

        void Render(SDL_Renderer* renderer) {
            for (int y=0; y < length; y++) {
                for (int x=0; x < length; x++) {
                    if (coordinates[y][x] != 0) {
                        tmp_rect->x = ((x)*tile_size);
                        tmp_rect->y = (y)*tile_size;

                        tmp_src->x = (coordinates[y][x]*16);
                        SDL_RenderCopy(renderer, texture, tmp_src, tmp_rect);
                    }
                }
            }
        }

        void SetCell(int x, int y, int id) {
            coordinates[y][x] = id;
        }

        void CheckCollision(SDL_Rect *rect, int* collisions, int dx, int dy) {
            SDL_Rect *next_move = (SDL_Rect*)malloc(sizeof(SDL_Rect));
            SDL_Rect *tmp_rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));

            int ret[2];

            next_move->x = rect->x;
            next_move->y = rect->y;
            next_move->w = rect->w;
            next_move->h = rect->h;

            tmp_rect->w = 16*4;
            tmp_rect->h = 16*4;

            for (int y=0; y < length; y++) {
                for (int x=0; x < length; x++) {
                    tmp_rect->x = x*tile_size;
                    tmp_rect->y = y*tile_size;

                    if (coordinates[y][x] == 0) {
                        break;
                    }

                    // Down
                    next_move->y = rect->y + 1;
                    if (SDL_HasIntersection(next_move, tmp_rect)) {
                        collisions[1] = 1;
                    }

                    // Up
                    next_move->y = rect->y - 1;
                    if (SDL_HasIntersection(next_move, tmp_rect)) {
                        collisions[1] = -1;
                    }

                    next_move->x = rect->x;
                    next_move->y = rect->y;

                    // Right 
                    next_move->x = rect->x + 1;
                    if (SDL_HasIntersection(next_move, tmp_rect)) {
                        collisions[0] = 1;
                        printf("RIGHT\n");
                    }

                    // Left
                    next_move->x = rect->x - 1;
                    if (SDL_HasIntersection(next_move, tmp_rect)) {
                        collisions[0] = -1;
                    }

                    next_move->x = rect->x;
                    next_move->y = rect->y;
                }
            }

            free(next_move);
            free(tmp_rect);
        }
    };

    class Player {
        SDL_Texture* texture;

        SDL_Rect *src;

        float speed;

        // Lua Script
        lua_State *LuaState;
        int row;
    
    public:
        SDL_Rect *rect;

        float dx, dy, jump_force;

        bool is_on_floor = false, just_jump = false;

        Player() {
            rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
            src = (SDL_Rect*)malloc(sizeof(SDL_Rect));

            dx, dy = 0;
            speed = 2;
        }

        ~Player() {
            SDL_DestroyTexture(texture);

            free(rect);
            free(src);

            lua_close(LuaState);
        }

        void Init(SDL_Renderer* renderer) {
            LuaState = luaL_newstate();
            luaL_openlibs(LuaState);

            row = luaL_dofile(LuaState, "data/scripts/player.lua");
            std::string script_texture_path;
            int script_rect[4];
            int script_src[4];

            float script_speed;
            float script_jump_force;

            if (row != LUA_OK) {
                std::cout << lua_tostring(LuaState, -1);
            }
            else {
                lua_getglobal(LuaState, "Player");
                if (lua_istable(LuaState, -1)) {
                    lua_pushstring(LuaState, "TexturePath");
                    lua_gettable(LuaState, -2);
                    script_texture_path = lua_tostring(LuaState, -1);
                    lua_pop(LuaState, 1);

                    lua_pushstring(LuaState, "Speed");
                    lua_gettable(LuaState, -2);
                    script_speed = lua_tonumber(LuaState, -1);
                    lua_pop(LuaState, 1);

                    lua_pushstring(LuaState, "JumpForce");
                    lua_gettable(LuaState, -2);
                    script_jump_force = lua_tonumber(LuaState, -1);
                    lua_pop(LuaState, 1);

                    lua_pushstring(LuaState, "Rect");
			        lua_gettable(LuaState, -2);
                    if (lua_istable(LuaState, -1)) {
                        lua_rawgeti(LuaState, -1, 1);
                        script_rect[0] = lua_tointeger(LuaState, -1);
                        lua_pop(LuaState, 1);

                        lua_rawgeti(LuaState, -1, 2);
                        script_rect[1] = lua_tointeger(LuaState, -1);
                        lua_pop(LuaState, 1);

                        lua_rawgeti(LuaState, -1, 3);
                        script_rect[2] = lua_tointeger(LuaState, -1);
                        lua_pop(LuaState, 1);

                        lua_rawgeti(LuaState, -1, 4);
                        script_rect[3] = lua_tointeger(LuaState, -1);
                        lua_pop(LuaState, 1);
                    }
                    lua_pop(LuaState, 1);

                    lua_pushstring(LuaState, "SourceRect");
			        lua_gettable(LuaState, -2);
                    if (lua_istable(LuaState, -1)) {
                        lua_rawgeti(LuaState, -1, 1);
                        script_src[0] = lua_tointeger(LuaState, -1);
                        lua_pop(LuaState, 1);

                        lua_rawgeti(LuaState, -1, 2);
                        script_src[1] = lua_tointeger(LuaState, -1);
                        lua_pop(LuaState, 1);

                        lua_rawgeti(LuaState, -1, 3);
                        script_src[2] = lua_tointeger(LuaState, -1);
                        lua_pop(LuaState, 1);

                        lua_rawgeti(LuaState, -1, 4);
                        script_src[3] = lua_tointeger(LuaState, -1);
                        lua_pop(LuaState, 1);
                    }
                    lua_pop(LuaState, 1);
                }
                lua_pop(LuaState, 1);
            }

            SDL_Surface* surface = IMG_Load(script_texture_path.c_str());
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);

            rect->x = script_rect[0];
            rect->y = script_rect[1];
            rect->w = script_rect[2];
            rect->h = script_rect[3];

            src->x = script_src[0];
            src->y = script_src[1];
            src->w = script_src[2];
            src->h = script_src[3];

            speed = script_speed;
            jump_force = script_jump_force;
        }

        void InputDown(SDL_Keycode key) {
            switch (key)
            {
            case SDLK_a:
                dx = -speed;
                break;
            case SDLK_d:
                dx = speed;
                break;
            case SDLK_SPACE:
                is_on_floor = false;
                dy = -jump_force;
                break;
            default:
                break;
            }
        }
        
        void InputUp(SDL_Keycode key) {
            switch (key)
            {
            case SDLK_a:
                dx = 0;
                break;
            case SDLK_d:
                dx = 0;
                break;
            default:
                break;
            }
        }

        void Update() {
            // Physics
            if (!is_on_floor)
                dy += 0.3;
            else
                dy = 0;
            
            rect->x += dx;
            rect->y += dy;
        }

        void Render(SDL_Renderer* renderer) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, rect);

            SDL_RenderCopy(renderer, texture, src, rect);
        }
    };

    class GamePlay : public Scene {
        TileMap tilemap;

        Player player;

        SDL_Rect pointer;
        
    public:
        ~GamePlay(){};

        bool pause;

        SDL_Renderer* renderer;
        int mouse_x, mouse_y;

        void Init(SDL_Renderer* renderer) {
            this->renderer = renderer;

            player.Init(renderer);

            tilemap.Init(renderer);

            pointer.w = 16*4;
            pointer.h = 16*4;
        }
        
        void InputDown(SDL_Keycode key) {
            player.InputDown(key);
        }
        
        void InputUp(SDL_Keycode key) {
            player.InputUp(key);
        }

        void MouseMotion(SDL_MouseButtonEvent mouse) {
        }
        
        void MouseDown(SDL_MouseButtonEvent click) {
            int cx = ((click.x)/(16*4));
            int cy = (click.y/(16*4));

            if (click.button == SDL_BUTTON_LEFT) {
                tilemap.SetCell(cx, cy, 1);
            } else if (click.button == SDL_BUTTON_RIGHT) {
                tilemap.SetCell(cx, cy, 0);
            }
        }
        
        void MouseUp(SDL_MouseButtonEvent) {}
        
        void Update() {
            player.Update();

            // Player Physics
            int player_to_map[2];
            for (int y=0; y < tilemap.length; y++) {
                for (int x=0; x < tilemap.length; x++) {
                    if (tilemap.coordinates[y][x] != 0) {
                        SDL_Rect tmp_rect;

                        tmp_rect.x = ((x*(16*4))+4);
                        tmp_rect.y = (y*(16*4));
                        tmp_rect.w = (16*4)-8;
                        tmp_rect.h = 1;

                        // Down
                        if (SDL_HasIntersection(player.rect, &tmp_rect)) {
                            player.is_on_floor = true;
                            player.rect->y = tmp_rect.y - (16*4);
                            player.dy = 0;
                        } else {
                            player.is_on_floor = false;
                        }
                        
                        // Up
                        tmp_rect.y = (y*(16*4))+(16*4);
                        if (SDL_HasIntersection(player.rect, &tmp_rect) && player.dy <= 0) {
                            player.rect->y = tmp_rect.y;
                            player.dy = -1-(player.dy/4);
                        }

                        // Right
                        tmp_rect.x = (x*(16*4));
                        tmp_rect.y = (y*(16*4));
                        tmp_rect.w = 1;
                        tmp_rect.h = (16*4);

                        if (SDL_HasIntersection(player.rect, &tmp_rect)) {
                            player.rect->x = ((x*(16*4))-64);
                            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                            SDL_RenderDrawRect(renderer, &tmp_rect);
                        }

                        // Left
                        tmp_rect.x = ((x*(16*4))+64);

                        if (SDL_HasIntersection(player.rect, &tmp_rect)) {
                            player.rect->x = ((x*(16*4))+64);
                            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                            SDL_RenderDrawRect(renderer, &tmp_rect);
                        }

                        
                    }
                }
            }
        }
        
        void Render() {
            player.Render(renderer);
            tilemap.Render(renderer);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &pointer);
        }
    };
}