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

uint8_t ClientID;
Command commands[TURN_MAX_COMMANDS];
int commandCount;

SDL_Texture *tileTextures[4];
SDL_FRect tileRect;


void LoadTexture(SDL_Renderer *renderer, char *name, int index){
    SDL_Surface *Surface = SDL_LoadBMP(name); 
    if (!Surface) {
        printf("Could not load %s: %s\n", name, SDL_GetError());
        return;
    }
    tileTextures[index] = SDL_CreateTextureFromSurface(renderer, Surface);
    if(!tileTextures[index]){
        printf("Did not create %s texture: %s\n", name, SDL_GetError());
    }
    printf("Loaded %s\n", name);
    SDL_DestroySurface(Surface);
}

void LoadMapTextures(SDL_Renderer *renderer){ 
    LoadTexture(renderer, "assets/GrassTile.bmp", 0);
    LoadTexture(renderer, "assets/CastleTile.bmp", 1);
    LoadTexture(renderer, "assets/PlatoonTile.bmp", 2);
    LoadTexture(renderer, "assets/PlatoonTile2.bmp", 3);
}

int DistanceIndexIndex(int index_1, int index_2, struct GameState *gameState){
    int indexPointY_1 = index_1/gameState->tileMap.tilesAcross*gameState->tileMap.tilePxY;
    int indexPointX_1 = index_1%gameState->tileMap.tilesAcross*gameState->tileMap.tilePxY;

    int indexPointY_2 = index_2/gameState->tileMap.tilesAcross*gameState->tileMap.tilePxY;
    int indexPointX_2 = index_2%gameState->tileMap.tilesAcross*gameState->tileMap.tilePxY;

    int x = abs(indexPointX_1-indexPointX_2);
    int y = abs(indexPointY_1-indexPointY_2);

    return x*x+y*y;
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
    for (int i=0;i<UNIT_COUNT;i++){
        if(gameState->units[i].posOnGrid == -1) continue;

        float TileY = gameState->units[i].posOnGrid/gameState->unitMap.tilesAcross*(float)gameState->unitMap.tilePxY;
        float TileX = gameState->units[i].posOnGrid%gameState->unitMap.tilesAcross*(float)gameState->unitMap.tilePxX;

        tileRect.x = TileX;
        tileRect.y = TileY;

        if(!SDL_RenderTexture(renderer, tileTextures[gameState->units[i].ownerID+1], NULL, &tileRect)){
            printf("%s\n", SDL_GetError());
            break;
        }
    }
}

int UnitActions(struct GameState *gameState){

    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    if(mouseX > 600 || mouseY > 520){
        printf("Clicked out of bounds\n");
        return 0;
    }

    printf("Calculate index\n");

    int tileY = mouseY / gameState->tileMap.tilePxY;
    int tileX = mouseX / gameState->tileMap.tilePxX;

    int tileIndex = tileY * gameState->tileMap.tilesAcross + tileX;

    for (int k=0;k<UNIT_COUNT;k++){
        if(gameState->units[k].posOnGrid == tileIndex && gameState->units[k].ownerID == ClientID){
            gameState->selectedUnit = &gameState->units[k];
            printf("Selected a unit %i in %i\n", k, tileIndex);
            return 1;
        }else if(gameState->units[k].posOnGrid == tileIndex 
                && gameState->units[k].ownerID != ClientID 
                && gameState->selectedUnit != NULL
        ){
            int dist = DistanceIndexIndex(gameState->selectedUnit->posOnGrid, gameState->units[k].posOnGrid, gameState);
            if (dist > 3200){
                printf("Not in range\n");
                return 0;
            }
            Command cmd = {.type = CMD_ATTACK,
                    .data.attack = {.ownerID = gameState->units[k].ownerID, 
                                  .posOnGrid = gameState->units[k].posOnGrid,
                                  .dealtDamage = gameState->selectedUnit->damage}};
            printf("Attacked a unit in %i\n", tileIndex);
            commands[commandCount] = cmd;
            commandCount++;
            return 1;
        }
    }

    if(tileIndex > 194){
        printf("Too big index: %i\n", tileIndex);
        return 0;
    }

    if(gameState->selectedUnit == NULL){
        return 0;
    }

    int dist = DistanceIndexIndex(gameState->selectedUnit->posOnGrid, tileIndex, gameState);
    if(dist < 12800){
        int oldIndex = gameState->selectedUnit->posOnGrid;
        gameState->unitMap.tileType[gameState->selectedUnit->posOnGrid] = 0;
        gameState->unitMap.tileType[tileIndex] = gameState->selectedUnit->unitType;
        gameState->selectedUnit->posOnGrid = tileIndex;
        printf("Moved a unit to %i\n", tileIndex);

        Command cmd = {.type = CMD_MOVE_UNIT,
                .data.move = {.unitType = gameState->selectedUnit->unitType, .oldPosOnGrid = oldIndex, .newPosOnGrid = tileIndex}};
        commands[commandCount] = cmd;
        commandCount++;

        return 1;
    }else{
        printf("Too far\n");
        return 0;
    }
}

void GetPlayerInput(SDL_Event *event, struct GameState *gameState, const bool *keys){
    SDL_PumpEvents();

    if (keys[SDL_SCANCODE_SPACE]) {
        gameState->isReady = 1;
    }
}

int DrawTextBox(SDL_Renderer *renderer, int posX, int posY, int width, int hight, const char *text){
    SDL_FRect rect;

    rect.x = posX;
    rect.y = posY;
    rect.w = width;
    rect.h = hight;

    SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    float scale = (hight-10)/7;
    SDL_SetRenderScale(renderer, scale, scale);
    SDL_RenderDebugText(renderer, (posX+5)/scale, (posY+5)/scale, text);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
    return 1;
}

int DrawUI(SDL_Renderer *renderer, struct GameState *gameState){
    DrawTextBox(renderer, 10, 530, 250, 50, "$: 500");
    DrawTextBox(renderer, 300, 530, 250, 50, "P: 50");
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

    ssize_t idR = recv(s, &ClientID, sizeof(ClientID), 0);
    if(idR < 0){
        printf("Didn't recieve ClientID\n");
        close(s);
        return -1;
    }
    printf("ClientID recieved %i\n", ClientID);

    commandCount = 0;
    int isRunning = 1;
    gameState.isReady = 1;
    gameState.waitForServer = 1;

    tileRect.h = 40.0f;
    tileRect.w = 40.0f;

    LoadMapTextures(renderer);

    while(isRunning) {

        if (gameState.isReady){
            if (gameState.waitForServer){
                struct GameState gamestatebuf;
                printf("wait for server\n");

                ssize_t bytes = 0;
                uint8_t *p = (uint8_t*)&gamestatebuf;

                while (bytes < sizeof(gamestatebuf)) {
                    ssize_t r = recv(s, p + bytes, sizeof(gamestatebuf) - bytes, 0);
                    if (r <= 0) break;
                    bytes += r;
                }
                memcpy(&gameState, &gamestatebuf, sizeof(gameState));
                gameState.waitForServer = 0;
                gameState.isReady = 0;
                printf("Response from server\n");
            }
            else {
                for(int i=0;i<commandCount;i++){
                    send(s, &commands[i], sizeof(Command), 0);
                }
                Command endCmd = {.type = CMD_END_TURN };
                send(s, &endCmd, sizeof(Command), 0);
                printf("Sent client data\n");
                commandCount = 0;
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
                        printf("Command count %i\n", commandCount);
                        if (commandCount >= TURN_MAX_COMMANDS){
                            continue;
                        }else{
                            UnitActions(&gameState);
                        }
                }
            }

            GetPlayerInput(&windowEvent, &gameState, keys);

            SDL_SetRenderScale(renderer, 1.0f, 1.0f);
            SDL_SetRenderDrawColor(renderer, gameState.BG_red, 40, gameState.BG_blue, 255);
            SDL_RenderClear(renderer);

            DrawUI(renderer, &gameState);

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
