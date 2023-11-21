@echo off

g++ -Iinclude -Llibs -o bin/Main src/Main.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -llua54