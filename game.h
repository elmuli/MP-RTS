#ifndef GAME_H
#define GAME_H

#include <SDL3_image/SDL_image.h>
#include <stdlib.h>
#include <stdint.h>

#define PORT 50505
#define IP "0.0.0.0"

#define WIDHT 800
#define HIGHT 600
#define UNIT_COUNT 4
#define TURN_MAX_COMMANDS 10

#pragma pack(push, 1)

struct TileMap {
  uint32_t tilePxX, tilePxY;
  uint32_t tilesAcross, tilesDown;
  uint32_t tileType[195];
};

struct Unit{
    uint32_t posOnGrid;
    uint8_t unitType, ownerID;
    uint16_t health;
    uint16_t damage;
};

struct __attribute__((packed))GameState{
    uint8_t BG_red;
    uint8_t BG_blue;
    struct TileMap tileMap;
    struct TileMap unitMap;
    uint8_t isReady;
    uint8_t waitForServer;
    struct Unit units[UNIT_COUNT];
    struct Unit *selectedUnit;
};

enum CommandType{
    CMD_MOVE_UNIT,
    CMD_END_TURN
};

typedef struct{
    enum CommandType type;
    union{
        struct{ uint8_t unitType; uint32_t oldPosOnGrid; uint32_t newPosOnGrid;} move;
    } data;
} Command;

#pragma pack(pop)

#endif
