#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>

#include "game.h"


struct GameState gameState;

SDL_Texture *tileTextures[3];
SDL_FRect tileRect;


void LoadMapTextures(SDL_Renderer *renderer){ 
    SDL_Surface *Surface = SDL_LoadBMP("assets/GrassTile.bmp"); 
    if (!Surface) {
        printf("Could not load GrassTile img: %s\n", SDL_GetError());
        return;
    }
    tileTextures[0] = SDL_CreateTextureFromSurface(renderer, Surface);
    if(!tileTextures[0]){
        printf("Did not create GrassTile texture: %s\n", SDL_GetError());
    }
    printf("Loaded Grass tile\n");
    Surface = SDL_LoadBMP("assets/CastleTile.bmp");
    if (!Surface) {
        printf("Could not load CastleTile img: %s\n", SDL_GetError());
        return;
    }
    tileTextures[1] = SDL_CreateTextureFromSurface(renderer, Surface);
    if(!tileTextures[1]){
        printf("Did not create CaslteTile texture: %s\n", SDL_GetError());
    }
    printf("Loaded Castle tile\n");
    Surface = SDL_LoadBMP("assets/PlatoonTile.bmp");
    if (!Surface) {
        printf("Could not load PlatoonTile img: %s\n", SDL_GetError());
        return;
    }
    tileTextures[2] = SDL_CreateTextureFromSurface(renderer, Surface);
    if(!tileTextures[2]){
        printf("Did not create PlatoonTile texture: %s\n", SDL_GetError());
    }
    printf("Loaded Platoon tile\n");
    SDL_DestroySurface(Surface);
}

void DrawTileMap(struct GameState *gameState, SDL_Renderer *renderer){
    for (int i = 0; i < 195;) {
        float TileY = i/gameState->tileMap.tilesAcross*(float)gameState->tileMap.tilePxY;
        float TileX = i%gameState->tileMap.tilesAcross*(float)gameState->tileMap.tilePxX;

        tileRect.x = TileX;
        tileRect.y = TileY;

        int tileIndex = 0;
        if(gameState->tileMap.tileType[i] == 1){
            tileIndex = 1;
        }

        if(!SDL_RenderTexture(renderer, tileTextures[tileIndex], NULL, &tileRect)){
            printf("%s\n", SDL_GetError());
            break;
        }
        i++;
    }
}

void DrawUnits(struct GameState *gameState, SDL_Renderer *renderer){
    for (int i=0;i<195;i++){
        if(gameState->unitMap.tileType[i] == 2){

            float TileY = i/gameState->tileMap.tilesAcross*(float)gameState->tileMap.tilePxY;
            float TileX = i%gameState->tileMap.tilesAcross*(float)gameState->tileMap.tilePxX;

            tileRect.x = TileX;
            tileRect.y = TileY;

            if(!SDL_RenderTexture(renderer, tileTextures[2], NULL, &tileRect)){
                printf("%s\n", SDL_GetError());
                break;
            }
        }
    }
}


int SelectUnit(struct GameState *gameState){
    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    for (int i=0;i<195;i++){
        if(gameState->unitMap.tileType[i] > 0){
            float tileY = i/gameState->tileMap.tilesAcross*(float)gameState->tileMap.tilePxY;
            float tileX = i%gameState->tileMap.tilesAcross*(float)gameState->tileMap.tilePxX;

            if(
                mouseX > tileX 
                && mouseX < tileX+gameState->tileMap.tilePxX 
                && mouseY > tileY 
                && mouseY < tileY+gameState->tileMap.tilePxY)
            {
                for (int k=0;k<3;k++){
                    if(gameState->units[k].posOnGrid == i){
                        gameState->selectedUnit = &gameState->units[k];
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int MoveUnit(struct GameState *gameState){
    if (gameState->selectedUnit == nullptr) {
    }
    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    //int tileX = mouseX / gameState->tileMap.tilePxX;
    int tileY = mouseY / gameState->tileMap.tilePxY;

    int newIndex = tileY * gameState->tileMap.tilesAcross + gameState->tileMap.tilePxX;
    gameState->unitMap.tileType[gameState->selectedUnit->posOnGrid] = 0;
    gameState->unitMap.tileType[newIndex] = gameState->selectedUnit->unitType;
    gameState->selectedUnit->posOnGrid = newIndex;
    return 1;
}

void GetPlayerInput(SDL_Event *event, struct GameState *gameState, const bool *keys){
    SDL_PumpEvents();

    if (keys[SDL_SCANCODE_SPACE]) {
        gameState->isReady = 1;
    }
}

int main(int argc, char *argv[]) {
    int s;
    struct sockaddr_in sock;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s < 0){
        printf("socket()\n");
        return -1;
    }

    sock.sin_addr.s_addr = inet_addr(IP);
    sock.sin_family = AF_INET;
    sock.sin_port = htons(PORT);


    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("RTS", WIDHT, HIGHT, 0);
    if(!window){
        printf("CreateWindow()\n");
        return -1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if(!renderer){
        printf("CreateRenderer()\n");
        return -1;
    }

    SDL_Event windowEvent;
    const bool *keys = SDL_GetKeyboardState(NULL);

    printf("SDL initialized\n");

    if(connect(s, (struct sockaddr*)&sock, sizeof(sock))){
        printf("connect()\n");
        close(s);
        return -1;
    }
    printf("Connected to server.\n");


    int isRunning = 1;
    gameState.isReady = 1;
    gameState.waitForServer = 1;
    char buf[512];

    tileRect.h = 40.0f;
    tileRect.w = 40.0f;

    LoadMapTextures(renderer);

    while(isRunning) {

        if (gameState.isReady){
            if (gameState.waitForServer){
                struct GameState gameStateBuf;
                printf("Wait for server\n");

                ssize_t bytes = 0;
                uint8_t *p = (uint8_t*)&gameStateBuf;

                while (bytes < sizeof(gameStateBuf)) {
                    ssize_t r = recv(s, p + bytes, sizeof(gameStateBuf) - bytes, 0);
                    if (r <= 0) break;
                    bytes += r;
                }
                memcpy(&gameState, &gameStateBuf, sizeof(gameState));
                gameState.waitForServer = 0;
                gameState.isReady = 0;
                printf("Response from server\n");
            }
            else {
                printf("> ");
                fgets(buf, sizeof(buf), stdin);
                send(s, buf, strlen(buf), 0);
                gameState.waitForServer = 1;
            }
        }
        else {

            if (SDL_PollEvent(&windowEvent)) {
                switch (windowEvent.type) {
                    case SDL_EVENT_QUIT:
                        isRunning = 0;
                        break;
                    case SDL_EVENT_MOUSE_BUTTON_UP:
                        if (!SelectUnit(&gameState)) {
                            MoveUnit(&gameState);
                        }
                }
            }

            GetPlayerInput(&windowEvent, &gameState, keys);

            SDL_SetRenderDrawColor(renderer, gameState.BG_red, 40, gameState.BG_blue, 255);
            SDL_RenderClear(renderer);

            DrawTileMap(&gameState, renderer);
            DrawUnits(&gameState, renderer);

            SDL_RenderPresent(renderer);
        }
    }

    close(s);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
