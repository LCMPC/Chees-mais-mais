#ifndef UI_THEME_H
#define UI_THEME_H

#include "raylib.h"
#include <stdbool.h>

extern const Color UI_BLACK;
extern const Color UI_BG;
extern const Color UI_WHITE;
extern const Color UI_RED;
extern const Color UI_MUTED;

typedef enum UiPieceType {
    UI_PAWN = 0, UI_ROOK, UI_BISHOP, UI_KNIGHT, UI_KING, UI_QUEEN
} UiPieceType;

void UiInit(void);
void UiUnload(void);
void UiDrawText(const char *text, float x, float y, float fontSize, Color color);
void UiDrawTextCentered(const char *text, float centerX, float y, float fontSize, Color color);
void UiDrawPanel(Rectangle bounds);
void UiDrawButton(Rectangle bounds, const char *label, bool hovered);
void UiDrawPiece(int type, bool white, Rectangle bounds, float alpha);
void UiDrawArrow(Rectangle bounds, bool right, Color color);

#endif
