#ifndef CHESS_SCREENS_H
#define CHESS_SCREENS_H

#include "raylib.h"

typedef enum {
    SCREEN_START,
    SCREEN_SELECT,
    SCREEN_GAME,
    SCREEN_DEFEAT,
    SCREEN_EXIT
} GameScreen;

typedef struct {
    GameScreen screen;
    int selected_piece;
    int round;
    float time;
} GameState;

void UpdateCurrentScreen(GameState *state);
void DrawCurrentScreen(const GameState *state);

#endif
