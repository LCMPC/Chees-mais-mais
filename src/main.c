#include "raylib.h"
#include "ui/screens.h"
#include "ui/theme.h"

int main(void) {
    GameState state = { SCREEN_START, 0, 1, 0.0f };

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1440, 900, "Chess++");
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
