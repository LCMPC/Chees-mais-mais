#include "raylib.h"
#include "ui/screens.h"
#include "ui/theme.h"
#include <stdlib.h>
#include <time.h>

int main(void) {
    GameState state = { .screen = SCREEN_START, .round = 1 };
    srand((unsigned)time(NULL));

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1440, 900, "Chess++");
    SetExitKey(KEY_NULL);
    SetWindowMinSize(1024, 640);
    SetTargetFPS(60);
    UiInit();

    while (!WindowShouldClose() && state.screen != SCREEN_EXIT) {
        state.time += GetFrameTime();
        UpdateCurrentScreen(&state);

        BeginDrawing();
        DrawCurrentScreen(&state);
        EndDrawing();
    }

    UiUnload();
    CloseWindow();
    return 0;
}
