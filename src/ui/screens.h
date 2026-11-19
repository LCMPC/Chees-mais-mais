#ifndef CHESS_SCREENS_H
#define CHESS_SCREENS_H

#include "raylib.h"
#include "../campo/combate.h"
#include "../ranking.h"

typedef enum {
    SCREEN_START,
    SCREEN_SELECT,
    SCREEN_GAME,
    SCREEN_DEFEAT,
    SCREEN_EXIT
} GameScreen;

typedef enum {
    FASE_POSICAO_INICIAL,
    FASE_RECOMPENSA,
    FASE_RECRUTAR,
    FASE_POSICIONAR,
    FASE_COMBATE
} FaseInterface;

typedef struct {
    GameScreen screen;
    int selected_piece;
    int round;
    float time;
    Ranking ranking;
    char nick[RANK_NICK_MAX+1];
    bool nick_focus;
    float nick_error;
    int ranking_scroll;
    size_t run_rank;
    Combate combate;
    FaseInterface fase;
    const Carta *recompensas[5];
    int quantidade_recompensas;
    bool oferecer_recrutamento;
    char opcoes_recrutamento[3];
    char recruta;
    int selecionada;
    int inspecionada;
    int scroll_melhorias[2];
    bool arrastando;
    bool ja_selecionada;
    ListaAcoes acoes;
    Casa destino_acao;
    int opcoes_acao[MAX_ACOES];
    int quantidade_opcoes;
    int carta_opcoes; /* -1: escolha da carta; >=0: variantes da carta */
    int primeira_opcao;
    int passes;
    float animacao_tempo, animacao_duracao;
    Combate antes_animacao;
    int ator_arrastado;
    Vector2 origem_arrastada; /* coordenadas fracionárias em casas */
    bool recompensa_apos_animacao;
} GameState;

void UpdateCurrentScreen(GameState *state);
void DrawCurrentScreen(const GameState *state);

#endif
