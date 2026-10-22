#include "raylib.h"
#include <stdio.h>
#include <string.h>

#define COLS 54
#define LINHAS 29
#define CEL_W 16
#define CEL_H 24
#define TAM_FONTE 22
#define MARGEM_X 48
#define MARGEM_Y 40
#define NUM_OPCOES 4

static const char *FONTE_TITULO[6][5] = {
    {"#   #", " # # ", "  #  ", " # # ", "#   #"},
    {" ### ", "#   #", "#####", "#   #", "#   #"},
    {"#### ", "#   #", "#   #", "#   #", "#### "},
    {"#### ", "#   #", "#### ", "#  # ", "#   #"},
    {"#####", "#    ", "#### ", "#    ", "#####"},
    {"#####", "   # ", "  #  ", " #   ", "#####"}
};

static const char *OPCOES[NUM_OPCOES] = {
    "NOVO JOGO", "CONTINUAR", "OPCOES", "SAIR"
};

static char tela[LINHAS][COLS + 1];
static int inv_ini[LINHAS];
static int inv_fim[LINHAS];

static void limpar_buffer(void)
{
    int r, c;
    for (r = 0; r < LINHAS; r++) {
        for (c = 0; c < COLS; c++) tela[r][c] = ' ';
        tela[r][COLS] = '\0';
        tela[r][0] = '|';
        tela[r][COLS - 1] = '|';
        inv_ini[r] = -1;
        inv_fim[r] = -1;
    }
}

static void borda(int r)
{
    int c;
    for (c = 0; c < COLS; c++) tela[r][c] = '-';
    tela[r][0] = '+';
    tela[r][COLS - 1] = '+';
}

static void tracejado(int r)
{
    const char *txt = "- - - - - - - - - - - - -";
    int len = (int)strlen(txt);
    int ini = (COLS - len) / 2;
    memcpy(&tela[r][ini], txt, len);
}

static void tabuleiro(int r, int deslocamento)
{
    int c;
    for (c = 1; c < COLS - 1; c++)
        tela[r][c] = ((c + deslocamento) % 2 == 0) ? '#' : '.';
}

static int centro(int r, const char *txt)
{
    int len = (int)strlen(txt);
    int ini = (COLS - len) / 2;
    memcpy(&tela[r][ini], txt, len);
    return ini;
}

static void titulo(int r0)
{
    char buf[64];
    int r, l;
    for (r = 0; r < 5; r++) {
        buf[0] = '\0';
        for (l = 0; l < 6; l++) {
            strcat(buf, FONTE_TITULO[l][r]);
            if (l < 5) strcat(buf, "  ");
        }
        centro(r0 + r, buf);
    }
}

static void montar(int sel)
{
    char buf[64];
    int i, r, ini;

    limpar_buffer();
    borda(0);
    tabuleiro(1, 0);
    tabuleiro(2, 1);
    titulo(4);
    centro(10, "R O G U E L I K E");
    tracejado(12);
    for (i = 0; i < NUM_OPCOES; i++) {
        r = 14 + i * 2;
        if (i == sel) {
            snprintf(buf, sizeof(buf), "  %s  ", OPCOES[i]);
            ini = centro(r, buf);
            inv_ini[r] = ini;
            inv_fim[r] = ini + (int)strlen(buf);
        } else {
            centro(r, OPCOES[i]);
        }
    }
    tracejado(22);
    centro(24, "W/S: mover    ENTER: selecionar");
    tabuleiro(26, 1);
    tabuleiro(27, 0);
    borda(28);
}

static void desenhar(Font fonte)
{
    int r, c, x, y;
    char s[2] = {0, 0};
    Vector2 m;
    Color cor;

    for (r = 0; r < LINHAS; r++) {
        for (c = 0; c < COLS; c++) {
            x = MARGEM_X + c * CEL_W;
            y = MARGEM_Y + r * CEL_H;
            cor = WHITE;
            if (c >= inv_ini[r] && c < inv_fim[r]) {
                DrawRectangle(x, y, CEL_W, CEL_H, WHITE);
                cor = BLACK;
            }
            if (tela[r][c] == ' ') continue;
            s[0] = tela[r][c];
            m = MeasureTextEx(fonte, s, TAM_FONTE, 0);
            DrawTextEx(fonte, s,
                       (Vector2){x + (CEL_W - m.x) / 2.0f, y + (CEL_H - m.y) / 2.0f},
                       TAM_FONTE, 0, cor);
        }
    }
}

int main(void)
{
    int sel = 0;
    int sair = 0;
    Font fonte;

    InitWindow(COLS * CEL_W + MARGEM_X * 2, LINHAS * CEL_H + MARGEM_Y * 2, "Xadrez Roguelike");
    SetTargetFPS(60);
    fonte = GetFontDefault();
    SetTextureFilter(fonte.texture, TEXTURE_FILTER_POINT);

    while (!WindowShouldClose() && !sair) {
        if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
            sel = (sel + NUM_OPCOES - 1) % NUM_OPCOES;
        if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
            sel = (sel + 1) % NUM_OPCOES;
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            switch (sel) {
            case 0: TraceLog(LOG_INFO, "Novo jogo"); break;
            case 1: TraceLog(LOG_INFO, "Continuar"); break;
            case 2: TraceLog(LOG_INFO, "Opcoes"); break;
            default: sair = 1; break;
            }
        }

        montar(sel);

        BeginDrawing();
        ClearBackground(BLACK);
        desenhar(fonte);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}