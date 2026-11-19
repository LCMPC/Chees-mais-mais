#include "start_menu.h"
#include "theme.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static const char *const piece_names[TIPOS_PECA] = {
    "PEAO", "TORRE", "BISPO", "CAVALO", "REI", "RAINHA"
};

static const float row_height = 46.0f;
static const float error_time = .48f;

typedef struct {
    Rectangle info, nick, play, exit, ranking, list;
    float center;
} MenuLayout;

static float minimum(float a, float b) { return a < b ? a : b; }
static float maximum(float a, float b) { return a > b ? a : b; }

static MenuLayout layout(void) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    float info_width = minimum(w * .205f, 290);
    float rank_width = minimum(w * .28f, 390);
    float rank_x = w - 36 - rank_width;
    float center = (36 + info_width + rank_x) / 2;
    float button_width = minimum(w * .29f, 330);
    float top = h * .24f;
    float panel_height = h * .62f;
    MenuLayout l = {
        .info = {36, h * .34f, info_width, maximum(218, h * .34f)},
        .nick = {center - button_width/2, h * .49f, button_width, 44},
        .play = {center - button_width/2, h * .64f, button_width, 51},
        .exit = {center - button_width/2, h * .64f + 63, button_width, 51},
        .ranking = {rank_x, top, rank_width, panel_height},
        .list = {rank_x + 12, top + 55, rank_width - 24, panel_height - 87},
        .center = center,
    };
    return l;
}

static bool hovered(Rectangle r) {
    return CheckCollisionPointRec(GetMousePosition(), r);
}

static bool clicked(Rectangle r) {
    return hovered(r) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static size_t visible_rows(MenuLayout l) {
    return (size_t)(l.list.height / row_height);
}

static size_t scroll_max(const Ranking *ranking, MenuLayout l) {
    size_t rows = visible_rows(l);
    return ranking->count > rows ? ranking->count - rows : 0;
}

/* Keep the whole UTF-8 character, never truncate a multibyte nickname. */
static bool append_char(char *nick, int codepoint) {
    if (codepoint < 32 || codepoint == 127) return false;
    int bytes = 0;
    const char *encoded = CodepointToUTF8(codepoint, &bytes);
    size_t len = strlen(nick);
    if (bytes <= 0 || len + (size_t)bytes > RANK_NICK_MAX) return false;
    memcpy(nick + len, encoded, (size_t)bytes);
    nick[len + (size_t)bytes] = '\0';
    return true;
}

static bool erase_char(char *nick) {
    size_t len = strlen(nick);
    if (!len) return false;
    do { --len; } while (len && ((unsigned char)nick[len] & 0xc0) == 0x80);
    nick[len] = '\0';
    return true;
}

static void trim_nick(char *nick) {
    char *start = nick;
    while (*start == ' ') ++start;
    size_t len = strlen(start);
    while (len && start[len - 1] == ' ') --len;
    memmove(nick, start, len);
    nick[len] = '\0';
}

void UpdateStartMenu(GameState *state) {
    MenuLayout l = layout();
    size_t max = scroll_max(&state->ranking, l);
    if (state->ranking_scroll < 0) state->ranking_scroll = 0;
    if ((size_t)state->ranking_scroll > max) state->ranking_scroll = (int)max;
    if (hovered(l.ranking)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            int step = (int)ceilf(fabsf(wheel));
            state->ranking_scroll += wheel < 0 ? step : -step;
            if (state->ranking_scroll < 0) state->ranking_scroll = 0;
            if ((size_t)state->ranking_scroll > max) state->ranking_scroll = (int)max;
        }
    }

    /* Preserve the warning until editing resumes, while the shake decays. */
    if (state->nick_error > .001f)
        state->nick_error = maximum(.001f, state->nick_error - GetFrameTime());

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) state->nick_focus = hovered(l.nick);
    if (IsKeyPressed(KEY_TAB)) state->nick_focus = !state->nick_focus;
    bool changed = false;
    for (int key = GetCharPressed(); key > 0; key = GetCharPressed()) {
        if (state->nick_focus) changed |= append_char(state->nick, key);
    }
    if (state->nick_focus && (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)))
        changed |= erase_char(state->nick);
    if (changed) state->nick_error = 0;

    if (clicked(l.play) || IsKeyPressed(KEY_ENTER)) {
        trim_nick(state->nick);
        if (state->nick[0] && RankingNickExists(&state->ranking, state->nick)) {
            state->nick_error = error_time;
            state->nick_focus = true;
        } else {
            if (!state->nick[0]) RankingRandomNick(&state->ranking, state->nick);
            state->nick_error = 0;
            state->nick_focus = false;
            state->screen = SCREEN_SELECT;
        }
    } else if (clicked(l.exit)) {
        state->screen = SCREEN_EXIT;
    }
}

static void draw_title(float w) {
    UiDrawText("CHESS++", 36, 23, 25, UI_WHITE);
    DrawLineEx((Vector2){36, 62}, (Vector2){w - 36, 62}, 2, UI_WHITE);
    DrawRectangle(36, 60, 104, 4, UI_RED);
    UiDrawText("MENU PRINCIPAL", w - 250, 29, 17, UI_MUTED);
}

static void draw_info(Rectangle info) {
    UiDrawPanel(info);
    UiDrawText("A RUN", info.x + 18, info.y + 17, 17, UI_RED);
    UiDrawText("Escolha uma peca.", info.x + 18, info.y + 64, 15, UI_WHITE);
    UiDrawText("Venca cada rodada.", info.x + 18, info.y + 96, 15, UI_WHITE);
    UiDrawText("Evolua seu exercito.", info.x + 18, info.y + 128, 15, UI_WHITE);
    DrawLineEx((Vector2){info.x + 18, info.y + 163},
               (Vector2){info.x + info.width - 18, info.y + 163}, 1, UI_RED);
    UiDrawText("Cada casa importa.", info.x + 18, info.y + 179, 13, UI_MUTED);
}

static void draw_nick(const GameState *state, MenuLayout l) {
    Rectangle box = l.nick;
    if (state->nick_error > .001f) {
        float amount = state->nick_error / error_time;
        box.x += sinf(state->nick_error * 76) * 8 * amount;
    }
    UiDrawText("SEU NICK (OPCIONAL)", l.nick.x, l.nick.y - 21, 13, UI_MUTED);
    DrawRectangleRounded(box, .12f, 8, (Color){28, 28, 33, 255});
    Color border = state->nick_error > 0 ? UI_RED : state->nick_focus ? UI_WHITE : UI_MUTED;
    DrawRectangleRoundedLinesEx(box, .12f, 8, 2, border);
    BeginScissorMode((int)box.x + 8, (int)box.y + 3, (int)box.width - 16, (int)box.height - 6);
    if (state->nick[0]) {
        char line[RANK_NICK_MAX + 2];
        snprintf(line, sizeof line, "%s%s", state->nick,
                 state->nick_focus && ((int)(state->time * 2) % 2 == 0) ? "|" : "");
        float size = 18;
        while (size > 12 && MeasureText(line, (int)size + 2) > box.width - 22) size--;
        UiDrawText(line, box.x + 11, box.y + 12, size, UI_WHITE);
    } else {
        UiDrawText("Vazio = nick aleatorio", box.x + 11, box.y + 13, 15, UI_MUTED);
    }
    EndScissorMode();
    if (state->nick_error > 0)
        UiDrawText("Esse nick ja existe!", l.nick.x, l.nick.y + l.nick.height + 7, 14, UI_RED);
}

/* The detail card uses all its available surface instead of clipping upgrades.
   Cards with many names adapt into a grid with one column per piece type. */
static void draw_details(const RankingEntry *entry, MenuLayout l) {
    float h = (float)GetScreenHeight();
    Rectangle panel = {36, 91, l.ranking.x - 52, h - 143};
    DrawRectangleRec(panel, UI_BLACK);
    UiDrawPanel(panel);
    DrawRectangleLinesEx(panel, 2, UI_RED);
    UiDrawText("DETALHES DA RUN", panel.x + 18, panel.y + 14, 17, UI_RED);
    UiDrawText(entry->nick, panel.x + 18, panel.y + 37, 22, UI_WHITE);
    char round[40];
    snprintf(round, sizeof round, "RODADA %d", entry->round);
    UiDrawText(round, panel.x + 18, panel.y + 63, 13, UI_MUTED);

    int types[TIPOS_PECA], n = 0;
    for (int t = 0; t < TIPOS_PECA; ++t)
        if (entry->counts[t] || entry->upgrades[t]) types[n++] = t;
    if (!n) {
        UiDrawText("Sem pecas registradas.", panel.x + 18, panel.y + 100, 15, UI_MUTED);
        return;
    }
    int columns = n > 3 ? 3 : n;
    int rows = (n + columns - 1) / columns;
    float padding = 10;
    float top = panel.y + 87;
    float cw = (panel.width - 2 * padding) / columns;
    float ch = (panel.y + panel.height - top - padding) / rows;
    for (int i = 0; i < n; ++i) {
        int t = types[i];
        Rectangle card = {panel.x + padding + cw * (i % columns), top + ch * (i / columns), cw - 6, ch - 6};
        DrawRectangleRec(card, (Color){28, 28, 33, 255});
        DrawRectangleLinesEx(card, 1, UI_MUTED);
        char label[40];
        snprintf(label, sizeof label, "%s x%d", piece_names[t], entry->counts[t]);
        UiDrawPiece(t, true, (Rectangle){card.x + 6, card.y + 4, 30, 30}, 1);
        UiDrawText(label, card.x + 39, card.y + 10, 14, UI_WHITE);

        int upgrades = 0;
        for (int c = 0; c < CARTAS_TIPO; ++c)
            if (entry->upgrades[t] & (1u << catalogo[t][c].id)) ++upgrades;
        float y = card.y + 37;
        if (!upgrades) {
            UiDrawText("Sem melhorias", card.x + 8, y, 13, UI_MUTED);
            continue;
        }
        float step = minimum(17, (card.height - 40) / upgrades);
        for (int c = 0; c < CARTAS_TIPO; ++c) {
            const Carta *upgrade = &catalogo[t][c];
            if (!(entry->upgrades[t] & (1u << upgrade->id))) continue;
            float size = minimum(14, step - 2);
            while (size > 8 && MeasureText(upgrade->nome, (int)size + 1) > card.width - 19) --size;
            DrawRectangle((int)card.x + 7, (int)(y + size / 2), 3, 3, UI_RED);
            UiDrawText(upgrade->nome, card.x + 15, y, size, UI_WHITE);
            y += step;
        }
    }
}

static const RankingEntry *draw_ranking(const GameState *state, MenuLayout l) {
    UiDrawPanel(l.ranking);
    UiDrawText("RANKING", l.ranking.x + 18, l.ranking.y + 17, 17, UI_RED);
    DrawLineEx((Vector2){l.ranking.x + 16, l.list.y - 6},
               (Vector2){l.ranking.x + l.ranking.width - 16, l.list.y - 6}, 1, UI_MUTED);
    const RankingEntry *hovered_entry = NULL;
    if (!state->ranking.count) {
        UiDrawText("Nenhuma run ainda.", l.list.x + 6, l.list.y + 16, 15, UI_MUTED);
        UiDrawText("Seja o primeiro!", l.list.x + 6, l.list.y + 41, 14, UI_WHITE);
        return NULL;
    }
    size_t scroll = state->ranking_scroll < 0 ? 0 : (size_t)state->ranking_scroll;
    size_t max = scroll_max(&state->ranking, l);
    if (scroll > max) scroll = max;
    size_t rows = visible_rows(l);
    if (rows > state->ranking.count - scroll) rows = state->ranking.count - scroll;
    for (size_t i = 0; i < rows; ++i) {
        size_t index = scroll + i;
        const RankingEntry *entry = &state->ranking.entries[index];
        Rectangle row = {l.list.x, l.list.y + i * row_height, l.list.width - 7, row_height - 3};
        if (hovered(row)) {
            DrawRectangleRounded(row, .08f, 4, (Color){44, 35, 39, 255});
            hovered_entry = entry;
        }
        char place[32], round[40];
        snprintf(place, sizeof place, "#%zu", index + 1);
        snprintf(round, sizeof round, "RODADA %02d", entry->round);
        Color accent = (index < 3 || hovered(row)) ? UI_RED : UI_MUTED;
        UiDrawText(place, row.x + 5, row.y + 12, 16, accent);
        float nick_x = row.x + 54;
        float size = 16;
        while (size > 11 && MeasureText(entry->nick, (int)size + 2) > row.width - 63) --size;
        BeginScissorMode((int)nick_x, (int)row.y, (int)(row.width - 56), (int)row.height);
        UiDrawText(entry->nick, nick_x, row.y + 4, size, UI_WHITE);
        UiDrawText(round, nick_x, row.y + 24, 11, UI_MUTED);
        EndScissorMode();
        DrawLineEx((Vector2){row.x + 5, row.y + row.height},
                   (Vector2){row.x + row.width - 3, row.y + row.height}, 1, (Color){53, 53, 58, 255});
    }
    if (max) {
        float rail_height = rows * row_height;
        float thumb_height = maximum(16, rail_height * ((float)rows / state->ranking.count));
        float x = l.list.x + l.list.width - 5;
        float y = l.list.y + (rail_height - thumb_height) * ((float)scroll / max);
        DrawRectangleRec((Rectangle){x, l.list.y, 3, rail_height}, UI_MUTED);
        DrawRectangleRec((Rectangle){x, y, 3, thumb_height}, UI_RED);
    }
    UiDrawText(max ? "Role para ver mais" : "Passe o mouse para detalhes",
               l.ranking.x + 18, l.ranking.y + l.ranking.height - 24, 12, UI_MUTED);
    return hovered_entry;
}

void DrawStartMenu(const GameState *state) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    MenuLayout l = layout();
    float wave = sinf(state->time * 1.8f) * 5;
    draw_title(w);
    DrawLineEx((Vector2){l.center, h * .15f}, (Vector2){l.center, h * .20f}, 2, UI_RED);
    UiDrawTextCentered("CHESS++", l.center, h * .24f + wave,
                       minimum(minimum(w * .065f, 77), h * .10f), UI_WHITE);
    UiDrawTextCentered("UM ROGUELIKE DE XADREZ", l.center, h * .37f + wave, 18, UI_RED);
    draw_info(l.info);
    draw_nick(state, l);
    UiDrawButton(l.play, "JOGAR", hovered(l.play));
    UiDrawButton(l.exit, "SAIR", hovered(l.exit));
    const RankingEntry *entry = draw_ranking(state, l);
    UiDrawTextCentered("ENTER PARA JOGAR", l.center, h - 35, 13, UI_MUTED);
    if (entry) draw_details(entry, l);
}
