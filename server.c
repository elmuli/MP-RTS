#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

#include <SDL3/SDL.h>

#include "game.h"

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
                gameState->units[counter].health = 15;
                gameState->units[counter].damage = 8;
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
    gameState->selectedUnit = NULL;

    CreateTileMap(gameState);
    CreateUnitMap(gameState);
}

void ResetUnit(Unit *unit){
    unit->posOnGrid = -1;
    unit->unitType = -1;
    unit->ownerID = -1;
    unit->health = -1;
    unit->damage = -1;
}

int GetClientGameStateUpdate(int client, struct GameState *gameState){

    for (int i=0;i<TURN_MAX_COMMANDS;i++){
        Command cmd;
        printf("wait for client %i\n", client);

        ssize_t bytes = 0;
        uint8_t *p = (uint8_t*)&cmd;

        while (bytes < sizeof(cmd)) {
            ssize_t r = recv(client, p + bytes, sizeof(cmd) - bytes, 0);
            if (r <= 0) return -1;
            bytes += r;
        }

        switch (cmd.type) {
            case CMD_END_TURN:
                i = TURN_MAX_COMMANDS;
                printf("Ended turn\n");
                break;
            case CMD_ATTACK:
                for (int k=0;k<UNIT_COUNT;k++){
                    if (gameState->units[k].posOnGrid == cmd.data.attack.posOnGrid && gameState->units[k].ownerID == cmd.data.attack.ownerID){
                        gameState->units[k].health -= cmd.data.attack.dealtDamage;
                        printf("Attacked unit in %i, having %i hp\n",gameState->units[k].posOnGrid, gameState->units[k].health);
                        if(gameState->units[k].health >= 30000){
                            gameState->unitMap.tileType[gameState->units[k].posOnGrid] = 0;
                            ResetUnit(&gameState->units[k]);
                        }
                    }
                }
                break;
            case CMD_MOVE_UNIT:
                if (cmd.data.move.newPosOnGrid > 193){
                    break;
                }else{
                    printf("Moving unit to %i\n", cmd.data.move.newPosOnGrid);
                    printf("Got data, oldPos: %i, newPos: %i, Type: %i\n", cmd.data.move.oldPosOnGrid, cmd.data.move.newPosOnGrid, cmd.data.move.unitType);
                    gameState->unitMap.tileType[cmd.data.move.oldPosOnGrid] = 0;
                    gameState->unitMap.tileType[cmd.data.move.newPosOnGrid] = 2;
                    for (int k=0;k<UNIT_COUNT;k++){
                        if (gameState->units[k].posOnGrid == cmd.data.move.oldPosOnGrid){
                            gameState->units[k].posOnGrid = cmd.data.move.newPosOnGrid;
                        }
                    }
                    break;
                }
        }
    }

    return 1;
}

int main(){

    struct GameState gameState;

    int server;
    int clients[2];

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

        printf("Getting client data\n");

        int clientReturn_1 = GetClientGameStateUpdate(clients[0], &gameState);
        int clientReturn_2 = GetClientGameStateUpdate(clients[1], &gameState);

        if (clientReturn_1 < 0 || clientReturn_2 < 0) {
            break;
        }

        printf("Sending data to clients\n");

        send(clients[0], &gameState, sizeof(gameState), 0);
        send(clients[1], &gameState, sizeof(gameState), 0);
    }

    printf("Closing server\n");

    close(clients[0]);
    close(clients[1]);
    close(server);
    return 0;

}
