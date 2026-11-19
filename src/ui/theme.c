#include "theme.h"
#include <stdio.h>

const Color UI_BLACK = {8, 8, 11, 255};
const Color UI_BG = {13, 13, 17, 255};
const Color UI_WHITE = {238, 237, 231, 255};
const Color UI_RED = {206, 41, 49, 255};
const Color UI_MUTED = {129, 129, 135, 255};

static Font font;
static bool customFont;
static Texture2D sprites[2][6];
static bool spriteReady[2][6];
static Rectangle spriteSources[2][6];
static const char *const pieceNames[6] = {"pawn", "rook", "bishop", "knight", "king", "queen"};
static const char *const spritePaths[2][6] = {
    {
        [UI_PAWN] = "sprites/PeaoPreto Background Removed.png",
        [UI_ROOK] = "sprites/TorrePreta Background Removed.png",
        [UI_BISHOP] = "sprites/BispoPreto Background Removed.png",
        [UI_KNIGHT] = "sprites/CavaloPreto Background Removed.png",
        [UI_KING] = "sprites/ReiPreto Background Removed.png",
        [UI_QUEEN] = "sprites/RainhaPreta Background Removed.png",
    },
    {
        [UI_PAWN] = "sprites/PeaoBranco Background Removed.png",
        [UI_ROOK] = "sprites/TorreBranca Background Removed.png",
        [UI_BISHOP] = "sprites/BispoBranco Background Removed.png",
        [UI_KNIGHT] = "sprites/CavaloBranco Background Removed.png",
        [UI_KING] = "sprites/ReiBranco Background Removed.png",
        [UI_QUEEN] = "sprites/RainhaBranca Background Removed.png",
    },
};

static Color faded(Color color, float alpha) {
    if (alpha < 0) alpha = 0;
    if (alpha > 1) alpha = 1;
    color.a = (unsigned char)(color.a * alpha);
    return color;
}

static bool loadSprite(const char *path, Texture2D *sprite) {
    if (!FileExists(path)) return false;
    Texture2D texture = LoadTexture(path);
    if (texture.id == 0 || texture.width <= 0 || texture.height <= 0) {
        if (texture.id != 0) UnloadTexture(texture);
        return false;
    }
    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
    *sprite = texture;
    return true;
}

void UiInit(void) {
    customFont = FileExists("assets/fonts/ui.ttf");
    if (customFont) {
        font = LoadFontEx("assets/fonts/ui.ttf", 72, 0, 0);
        if (font.texture.id == 0) customFont = false;
    }
    if (!customFont) font = GetFontDefault();
    for (int side = 0; side < 2; side++) {
        for (int type = 0; type < 6; type++) {
            spriteReady[side][type] = loadSprite(spritePaths[side][type], &sprites[side][type]);
            bool primary = spriteReady[side][type];
            if (!spriteReady[side][type]) {
                char path[128];
                snprintf(path, sizeof path, "assets/pieces/%s_%s.png", side ? "white" : "black", pieceNames[type]);
                spriteReady[side][type] = loadSprite(path, &sprites[side][type]);
            }
            if (spriteReady[side][type]) {
                Texture2D texture = sprites[side][type];
                // A mesma margem para os PNGs de 512px preserva o alinhamento das bases.
                if (primary && texture.width == 512 && texture.height == 512)
                    spriteSources[side][type] = (Rectangle){32, 32, 448, 448};
                else
                    spriteSources[side][type] = (Rectangle){0, 0, (float)texture.width, (float)texture.height};
            }
        }
    }
}

void UiUnload(void) {
    for (int side = 0; side < 2; side++)
        for (int type = 0; type < 6; type++) {
            if (spriteReady[side][type]) UnloadTexture(sprites[side][type]);
            sprites[side][type] = (Texture2D){0};
            spriteReady[side][type] = false;
        }
    if (customFont) UnloadFont(font);
}

void UiDrawText(const char *text, float x, float y, float fontSize, Color color) {
    DrawTextEx(font, text, (Vector2){x, y}, fontSize, customFont ? 0.3f : 1.5f, color);
}

void UiDrawTextCentered(const char *text, float centerX, float y, float fontSize, Color color) {
    Vector2 size = MeasureTextEx(font, text, fontSize, customFont ? 0.3f : 1.5f);
    UiDrawText(text, centerX - size.x / 2, y, fontSize, color);
}

void UiDrawPanel(Rectangle bounds) {
    DrawRectangleRounded(bounds, 0.06f, 8, (Color){20, 20, 25, 255});
    DrawRectangleRoundedLinesEx(bounds, 0.06f, 8, 1.5f, (Color){90, 90, 96, 255});
}

void UiDrawButton(Rectangle bounds, const char *label, bool hovered) {
    Color fill = hovered ? UI_RED : (Color){28, 28, 33, 255};
    DrawRectangleRounded(bounds, 0.12f, 8, fill);
    DrawRectangleRoundedLinesEx(bounds, 0.12f, 8, 2, hovered ? UI_RED : UI_WHITE);
    Vector2 size = MeasureTextEx(font, label, 23, customFont ? 0.3f : 1.5f);
    UiDrawText(label, bounds.x + (bounds.width - size.x) / 2, bounds.y + (bounds.height - size.y) / 2, 23, UI_WHITE);
}

void UiDrawArrow(Rectangle bounds, bool right, Color color) {
    float cx = bounds.x + bounds.width / 2, cy = bounds.y + bounds.height / 2;
    float half = bounds.width < bounds.height ? bounds.width * 0.25f : bounds.height * 0.25f;
    float dir = right ? 1.0f : -1.0f;
    DrawLineEx((Vector2){cx - dir * half, cy - half}, (Vector2){cx + dir * half, cy}, 3, color);
    DrawLineEx((Vector2){cx + dir * half, cy}, (Vector2){cx - dir * half, cy + half}, 3, color);
}

void UiDrawPiece(int type, bool white, Rectangle bounds, float alpha) {
    if (type < 0 || type > 5) return;
    if (spriteReady[white ? 1 : 0][type]) {
        Texture2D texture = sprites[white ? 1 : 0][type];
        Rectangle source = spriteSources[white ? 1 : 0][type];
        float scaleX = bounds.width / source.width;
        float scaleY = bounds.height / source.height;
        float scale = scaleX < scaleY ? scaleX : scaleY;
        float width = source.width * scale, height = source.height * scale;
        Rectangle dest = {bounds.x + (bounds.width - width) / 2,
                          bounds.y + (bounds.height - height) / 2, width, height};
        DrawTexturePro(texture, source, dest, (Vector2){0, 0}, 0, faded(WHITE, alpha));
        return;
    }
    float cx = bounds.x + bounds.width / 2;
    float cy = bounds.y + bounds.height / 2;
    float s = (bounds.width < bounds.height ? bounds.width : bounds.height) / 100.0f;
    Color base = faded(white ? UI_WHITE : (Color){43, 43, 49, 255}, alpha);
    Color edge = faded(white ? UI_BLACK : UI_WHITE, alpha);
    Color accent = faded(UI_RED, alpha);
    DrawEllipse((int)cx, (int)(cy + 34*s), 31*s, 7*s, faded(UI_BLACK, alpha * 0.55f));
    DrawRectangleRounded((Rectangle){cx-27*s,cy+21*s,54*s,12*s},0.25f,6,base);
    DrawRectangleRoundedLinesEx((Rectangle){cx-27*s,cy+21*s,54*s,12*s},0.25f,6,1.5f*s,edge);
    if (type == UI_PAWN) {
        DrawCircleV((Vector2){cx,cy-14*s},13*s,base);
        DrawTriangle((Vector2){cx-13*s,cy+19*s},(Vector2){cx,cy-13*s},(Vector2){cx+13*s,cy+19*s},base);
        DrawCircleLines((int)cx,(int)(cy-14*s),13*s,edge);
    } else if (type == UI_ROOK) {
        DrawRectangleRec((Rectangle){cx-18*s,cy-17*s,36*s,39*s},base);
        DrawRectangleLinesEx((Rectangle){cx-18*s,cy-17*s,36*s,39*s},1.5f*s,edge);
        for(int i=-1;i<=1;i++) DrawRectangleRec((Rectangle){cx+(i*14-6)*s,cy-28*s,12*s,13*s},base);
    } else if (type == UI_BISHOP) {
        DrawEllipse((int)cx,(int)(cy-9*s),17*s,25*s,base);
        DrawEllipseLines((int)cx,(int)(cy-9*s),17*s,25*s,edge);
        DrawLineEx((Vector2){cx-7*s,cy-28*s},(Vector2){cx+8*s,cy+7*s},2*s,accent);
    } else if (type == UI_KNIGHT) {
        DrawTriangle((Vector2){cx-19*s,cy+21*s},(Vector2){cx+18*s,cy+21*s},(Vector2){cx+10*s,cy-18*s},base);
        DrawTriangle((Vector2){cx-19*s,cy+2*s},(Vector2){cx+10*s,cy-31*s},(Vector2){cx+11*s,cy+2*s},base);
        DrawCircleV((Vector2){cx+3*s,cy-14*s},2.7f*s,accent);
    } else if (type == UI_KING || type == UI_QUEEN) {
        DrawTriangle((Vector2){cx-22*s,cy+21*s},(Vector2){cx,cy-24*s},(Vector2){cx+22*s,cy+21*s},base);
        if(type == UI_KING) {
            DrawLineEx((Vector2){cx,cy-36*s},(Vector2){cx,cy-15*s},3*s,accent);
            DrawLineEx((Vector2){cx-9*s,cy-29*s},(Vector2){cx+9*s,cy-29*s},3*s,accent);
        } else {
            for(int i=-1;i<=1;i++) DrawCircleV((Vector2){cx+i*19*s,cy-(i==0?28:18)*s},5*s,accent);
        }
    }
}
