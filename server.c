#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

#include <SDL3/SDL.h>

#include "game.h"

struct Data{
    char data_1[512];
    char data_2[512];
};

void CreateTileMap(struct GameState *gameState){
    
    gameState->tileMap.tilePxX = 40;
    gameState->tileMap.tilePxY = 40;

    gameState->tileMap.tilesAcross = 15;
    gameState->tileMap.tilesDown = 13;

    uint32_t grid[195] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    memcpy(gameState->tileMap.tileType, grid, sizeof(gameState->tileMap.tileType));

}

void CreateUnitMap(struct GameState *gameState){

    gameState->unitMap.tilePxX = 40;
    gameState->unitMap.tilePxY = 40;

    gameState->unitMap.tilesAcross = 15;
    gameState->unitMap.tilesDown = 13;

    uint32_t grid[195] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    int counter = 0;
    for (int i=0; i<195; i++){
        if (counter < UNIT_COUNT){
            if (grid[i] == 2){
                gameState->units[counter].posOnGrid = i;
                gameState->units[counter].unitType = 2;
                if(counter%2 == 0){
                    gameState->units[counter].ownerID = 2;
                }else{
                    gameState->units[counter].ownerID = 1;
                }
                counter++;
            }
        }else {
            break;
        }
    }

    memcpy(gameState->unitMap.tileType, grid, sizeof(gameState->unitMap.tileType));

}

void SetUpGameState(struct GameState *gameState){
    gameState->BG_red = 40;
    gameState->BG_blue = 40;

    CreateTileMap(gameState);
    CreateUnitMap(gameState);
}

int GetClientGameStateUpdate(int client, struct GameState *gameState){

    struct gamestate gamestatebuf;
    printf("wait for client %i\n", client);

    ssize_t bytes = 0;
    uint8_t *p = (uint8_t*)&gamestatebuf;

    while (bytes < sizeof(gamestatebuf)) {
        ssize_t r = recv(client, p + bytes, sizeof(gamestatebuf) - bytes, 0);
        if (r <= 0) break;
        bytes += r;
    }

    

}

int main(){

    struct GameState gameState;

    int server;
    int clients[2];

    struct Data Data;

    socklen_t addrlen_1;
    socklen_t addrlen_2;
    struct sockaddr_in srv;

    addrlen_1 = 0;
    addrlen_2 = 0;
    memset(&srv, 0, sizeof(srv));
    memset(&clients[0], 0, sizeof(clients[0]));
    memset(&clients[1], 0, sizeof(clients[1]));

    server  = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0){
        printf("socket()\n");
        close(server);
        return -1;
    }

    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = 0;
    srv.sin_port = htons(PORT);

    if(bind(server, (struct sockaddr *)&srv, sizeof(srv))){
        printf("bind()\n");
        close(server);
        return -1;
    }

    SetUpGameState(&gameState);

    if(listen(server, 2)){
        printf("listen()\n");
        close(server);
        return -1;
    }

    printf("Listening for clients on: %d\n", PORT);

    uint8_t clientID = 1;

    clients[0] = accept(server, (struct sockaddr *)&srv, &addrlen_1);
    if(clients[0] < 0){
        printf("accept() 1\n");
        close(server);
        return -1;
    }
    send(clients[0], &clientID, sizeof(clientID), 0);
    send(clients[0], &gameState, sizeof(gameState), 0);

    printf("Client 1 connected\n");
    printf("Listening for second client on: %d\n", PORT);

    clients[1] = accept(server, (struct sockaddr *)&srv, &addrlen_2);
    if(clients[1] < 0){
        printf("accept() 2\n");
        close(clients[0]);
        close(server);
        return -1;
    }
    clientID = 2;
    send(clients[1], &clientID, sizeof(clientID), 0);
    send(clients[1], &gameState, sizeof(gameState), 0);

    printf("Client 2 connected\n");

    for(;;){

        struct GameState newGameState;

        int len;
        printf("Reciving form client 1\n");
        len = recv(clients[0], Data.data_2, sizeof(Data.data_2)-1, 0);
        if (len <= 0) break;
        Data.data_2[len] = 0;
        printf("Received from client 1: %s\n", Data.data_2);

        printf("Reciving form client 2\n");
        len = recv(clients[1], Data.data_1, sizeof(Data.data_1)-1, 0);
        if (len <= 0) break;
        Data.data_1[len] = 0;
        printf("Received from client 2: %s\n", Data.data_1);

        newGameState.BG_red = 100;
        newGameState.BG_blue = 100;
        CreateTileMap(&newGameState);

        printf("Sending data to clients\n");

        send(clients[0], &newGameState, sizeof(gameState), 0);
        send(clients[1], &newGameState, sizeof(gameState), 0);
    }

    printf("Closing server\n");

    close(clients[0]);
    close(clients[1]);
    close(server);
    return 0;

}
