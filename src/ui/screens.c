#include "screens.h"
#include "theme.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *PIECE_NAMES[] = { "PEAO", "TORRE", "BISPO", "CAVALO", "REI", "RAINHA" };
static const char PIECE_TYPES[] = "PTBCRD";
static const char PROMOCAO_TIPOS[] = "DTBC";

static Rectangle rect(float x, float y, float w, float h) {
    return (Rectangle){ x, y, w, h };
}

static bool hovered(Rectangle r) {
    return CheckCollisionPointRec(GetMousePosition(), r);
}

static bool clicked(Rectangle r) {
    return hovered(r) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static float minimum(float a, float b) { return a < b ? a : b; }

static void frame_title(const char *section) {
    float w = (float)GetScreenWidth();
    UiDrawText("CHESS++", 36, 23, 25, UI_WHITE);
    DrawLineEx((Vector2){ 36, 62 }, (Vector2){ w - 36, 62 }, 2, UI_WHITE);
    DrawRectangle(36, 60, 104, 4, UI_RED);
    UiDrawText(section, w - 250, 29, 17, UI_MUTED);
}

static Rectangle start_play(void) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    return rect(w * .39f, h * .63f, w * .22f, 51);
}

static Rectangle start_exit(void) {
    Rectangle a = start_play();
    return rect(a.x, a.y + 64, a.width, a.height);
}

static Rectangle select_left(void) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    return rect(w * .34f, h * .70f, 54, 50);
}

static Rectangle select_right(void) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    return rect(w * .66f - 54, h * .70f, 54, 50);
}

static Rectangle select_confirm(void) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    return rect(w * .40f, h * .82f, w * .20f, 47);
}

static Rectangle defeat_back(void) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    return rect(w * .39f, h * .68f, w * .22f, 52);
}

static void iniciar_run(GameState *state);
static void update_game(GameState *state);

void UpdateCurrentScreen(GameState *state) {
    if (!state) return;
    switch (state->screen) {
        case SCREEN_START:
            if (clicked(start_play()) || IsKeyPressed(KEY_ENTER)) state->screen = SCREEN_SELECT;
            else if (clicked(start_exit())) state->screen = SCREEN_EXIT;
            break;
        case SCREEN_SELECT:
            if (clicked(select_left()) || IsKeyPressed(KEY_LEFT)) state->selected_piece = (state->selected_piece + 4) % 5;
            if (clicked(select_right()) || IsKeyPressed(KEY_RIGHT)) state->selected_piece = (state->selected_piece + 1) % 5;
            if (clicked(select_confirm()) || IsKeyPressed(KEY_ENTER)) iniciar_run(state);
            if (IsKeyPressed(KEY_ESCAPE)) state->screen = SCREEN_START;
            break;
        case SCREEN_GAME:
            update_game(state);
            break;
        case SCREEN_DEFEAT:
            if (clicked(defeat_back()) || IsKeyPressed(KEY_ENTER)) state->screen = SCREEN_START;
            break;
        case SCREEN_EXIT: break;
    }
}

static void draw_start(const GameState *state) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    float wave = sinf(state->time * 1.8f) * 5;
    frame_title("MENU PRINCIPAL");
    DrawLineEx((Vector2){w*.5f, h*.18f}, (Vector2){w*.5f,h*.25f}, 2, UI_RED);
    UiDrawTextCentered("CHESS++", w*.5f, h*.34f + wave, minimum(w*.065f, 77), UI_WHITE);
    UiDrawTextCentered("UM ROGUELIKE DE XADREZ", w*.5f, h*.46f + wave, 18, UI_RED);
    UiDrawButton(start_play(), "JOGAR", hovered(start_play()));
    UiDrawButton(start_exit(), "SAIR", hovered(start_exit()));

    Rectangle info = rect(36, h*.34f, w*.19f, h*.34f);
    UiDrawPanel(info);
    UiDrawText("A RUN", info.x+18, info.y+17, 17, UI_RED);
    UiDrawText("Escolha uma peca.", info.x+18, info.y+64, 15, UI_WHITE);
    UiDrawText("Venca cada rodada.", info.x+18, info.y+96, 15, UI_WHITE);
    UiDrawText("Evolua seu exercito.", info.x+18, info.y+128, 15, UI_WHITE);
    DrawLineEx((Vector2){info.x+18,info.y+163},(Vector2){info.x+info.width-18,info.y+163},1,UI_RED);
    UiDrawText("Cada casa importa.", info.x+18, info.y+179, 13, UI_MUTED);

    Rectangle podium = rect(w*.78f, h*.34f, w*.19f, h*.34f);
    UiDrawPanel(podium);
    UiDrawText("PODIO", podium.x+18, podium.y+17, 17, UI_RED);
    for (int i=0; i<3; ++i) {
        char line[48];
        snprintf(line, sizeof line, "%d  ---  RODADA --", i+1);
        UiDrawText(line, podium.x+18, podium.y+67+i*44, 15, UI_WHITE);
    }
    UiDrawText("Em breve", podium.x+18, podium.y+podium.height-39, 13, UI_MUTED);
    UiDrawTextCentered("ENTER PARA JOGAR", w*.5f, h-35, 13, UI_MUTED);
}

static void draw_selection(const GameState *state) {
    float w=(float)GetScreenWidth(), h=(float)GetScreenHeight();
    frame_title("ESCOLHA SUA PECA");
    UiDrawTextCentered("SEU PRIMEIRO MOVIMENTO",w*.5f,h*.14f,20,UI_RED);
    UiDrawTextCentered("Uma peca para comecar. Uma run para construir.",w*.5f,h*.20f,16,UI_MUTED);
    float cx=w*.5f, py=h*.30f;
    float step=minimum(w*.16f,190);
    for(int offset=-2;offset<=2;++offset) {
        int type=(state->selected_piece+offset+5)%5;
        float size=offset==0?minimum(h*.28f,205):minimum(h*.20f,145);
        float alpha=offset==0?1.0f:(abs(offset)==1?.48f:.19f);
        Rectangle box=rect(cx+offset*step-size*.5f,py+(offset==0?0:34),size,size);
        if(offset==0) {
            DrawRectangleLinesEx(rect(box.x-10,box.y-10,box.width+20,box.height+20),2,UI_RED);
            DrawLineEx((Vector2){cx-32,box.y+box.height+22},(Vector2){cx+32,box.y+box.height+22},3,UI_RED);
        }
        UiDrawPiece(type,true,box,alpha);
    }
    UiDrawArrow(select_left(),false,hovered(select_left())?UI_RED:UI_WHITE);
    UiDrawArrow(select_right(),true,hovered(select_right())?UI_RED:UI_WHITE);
    UiDrawTextCentered(PIECE_NAMES[state->selected_piece],cx,h*.71f,26,UI_WHITE);
    UiDrawButton(select_confirm(),"CONFIRMAR",hovered(select_confirm()));
    UiDrawTextCentered("SETAS PARA TROCAR  |  ENTER PARA CONFIRMAR",cx,h*.95f,12,UI_MUTED);
}

typedef struct {
    float x, y, tile, largura, altura, lateral, margem;
} LayoutTabuleiro;

static LayoutTabuleiro layout_tabuleiro(const GameState *state) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    LayoutTabuleiro layout;
    layout.tile = minimum((h-300)/5, (w*.50f)/state->combate.largura);
    layout.largura = layout.tile * state->combate.largura;
    layout.altura = layout.tile * 5;
    layout.x = (w-layout.largura)*.5f;
    layout.y = (h-layout.altura)*.5f+10;
    layout.margem = 24;
    layout.lateral = minimum(250, layout.x-layout.margem-18);
    return layout;
}

static int tipo_ui(char tipo) {
    for (int i = 0; i < TIPOS_PECA; ++i) {
        if (PIECE_TYPES[i] == tipo) return i;
    }
    return -1;
}

static const char *nome_tipo(char tipo) {
    int i = tipo_ui(tipo);
    return i < 0 ? "PECA" : PIECE_NAMES[i];
}

static Rectangle casa_rect(LayoutTabuleiro layout, Casa p) {
    return rect(layout.x + p.c*layout.tile, layout.y + p.l*layout.tile,
                layout.tile, layout.tile);
}

static Rectangle painel_lateral(LayoutTabuleiro layout, bool enemy) {
    float x = enemy ? GetScreenWidth()-layout.margem-layout.lateral : layout.margem;
    return rect(x,layout.y,layout.lateral,layout.altura);
}

static Casa casa_mouse(const GameState *state) {
    LayoutTabuleiro layout = layout_tabuleiro(state);
    Vector2 mouse = GetMousePosition();
    if (mouse.x < layout.x || mouse.y < layout.y ||
        mouse.x >= layout.x + layout.largura || mouse.y >= layout.y + layout.altura) {
        return (Casa){-1, -1};
    }
    return (Casa){(int)((mouse.y-layout.y)/layout.tile),
                  (int)((mouse.x-layout.x)/layout.tile)};
}

static void limpar_selecao(GameState *state) {
    state->selecionada = -1;
    state->arrastando = false;
    state->ja_selecionada = false;
    state->acoes.n = 0;
    state->quantidade_opcoes = 0;
    state->carta_opcoes = -1;
    state->primeira_opcao = 0;
}

static void escolher_peca(GameState *state, int unidade) {
    if (unidade >= 0 && unidade != state->selecionada)
        state->scroll_melhorias[state->combate.unidades[unidade].lado] = 0;
    limpar_selecao(state);
    state->selecionada = unidade;
    if (unidade >= 0 && state->combate.unidades[unidade].vivo)
        gerarAcoes(&state->combate, unidade, &state->acoes);
}

static bool ha_casa_recruta(const Combate *j) {
    for (int linha = 3; linha < 5; ++linha) {
        for (int coluna = 0; coluna < j->largura; ++coluna) {
            if (vazio(j, (Casa){linha, coluna})) return true;
        }
    }
    return false;
}

static void comecar_combate(GameState *state) {
    state->fase = FASE_COMBATE;
    state->passes = 0;
    state->inspecionada = -1;
    state->scroll_melhorias[0] = 0;
    state->scroll_melhorias[1] = 0;
    limpar_selecao(state);
}

static void preparar_recompensa(GameState *state) {
    Combate *j = &state->combate;
    limpar_selecao(state);
    state->round = j->rodada;
    state->oferecer_recrutamento = j->rodada % 2 == 0 && ha_casa_recruta(j);
    int limite = j->rodada % 5 == 0 ? 5 : 3;
    if (state->oferecer_recrutamento) --limite;
    state->quantidade_recompensas = sortearCartas(j, state->recompensas, limite);
    state->fase = FASE_RECOMPENSA;
    if (state->quantidade_recompensas == 0 && !state->oferecer_recrutamento)
        comecar_combate(state);
}

static void iniciar_run(GameState *state) {
    iniciarCombate(&state->combate, PIECE_TYPES[state->selected_piece]);
    state->round = state->combate.rodada;
    state->screen = SCREEN_GAME;
    state->fase = FASE_POSICAO_INICIAL;
    state->inspecionada = -1;
    state->scroll_melhorias[0] = 0;
    state->scroll_melhorias[1] = 0;
    limpar_selecao(state);
}

static Rectangle recompensa_rect(int i, int n) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    float width = minimum(255, (w-100-(n-1)*16)/n);
    float x = (w-(n*width+(n-1)*16))*.5f + i*(width+16);
    return rect(x, h*.27f, width, h*.56f);
}

static Rectangle promocao_painel(void) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    float largura = minimum(w-80, 900);
    return rect((w-largura)*.5f, (h-280)*.5f, largura, 280);
}

static Rectangle promocao_rect(int i) {
    Rectangle p = promocao_painel();
    float largura = (p.width-36-3*12)/4;
    return rect(p.x+18+i*(largura+12), p.y+92, largura, 143);
}

static void update_promocao(GameState *state, int unidade) {
    Combate *j = &state->combate;
    limpar_selecao(state);
    for (int i = 0; i < 4; ++i) {
        if (!clicked(promocao_rect(i)) && !IsKeyPressed(KEY_ONE+i)) continue;
        if (escolherPromocao(j,unidade,PROMOCAO_TIPOS[i]) &&
            promocaoPendente(j) < 0 && j->bonus >= 0)
            escolher_peca(state,j->bonus);
        return;
    }
}

static void iniciar_recrutamento(GameState *state) {
    char tipos[] = "PTBCR";
    for (int i = 0; i < 3; ++i) {
        int escolhido = i + rand() % (5-i);
        char tmp = tipos[i];
        tipos[i] = tipos[escolhido];
        tipos[escolhido] = tmp;
        state->opcoes_recrutamento[i] = tipos[i];
    }
    state->fase = FASE_RECRUTAR;
}

static Rectangle passar_rect(void) {
    return rect(GetScreenWidth()-205, GetScreenHeight()-57, 181, 36);
}

static Rectangle opcoes_painel(void) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    float width = minimum(w-80, 650);
    return rect((w-width)*.5f, 92, width, h-158);
}

static int quantidade_visivel(void) {
    Rectangle p = opcoes_painel();
    return (int)((p.height-175)/54);
}

static Rectangle opcao_rect(int i) {
    Rectangle p = opcoes_painel();
    return rect(p.x+18, p.y+87+i*54, p.width-36, 47);
}

static void confirmar_acao(GameState *state, Acao acao) {
    int ator = acao.ator;
    if (executarAcao(&state->combate, acao)) {
        state->passes = 0;
        limpar_selecao(state);
        if (promocaoPendente(&state->combate) < 0 &&
            state->combate.bonus == ator && state->combate.unidades[ator].vivo)
            escolher_peca(state, ator);
    } else {
        limpar_selecao(state);
    }
}

static int contar_variantes(const GameState *state, int carta) {
    int total = 0;
    for (int i = 0; i < state->acoes.n; ++i) {
        Acao a = state->acoes.itens[i];
        if (a.carta == carta && igualCasa(a.destino,state->destino_acao))
            ++total;
    }
    return total;
}

static void listar_opcoes(GameState *state, Casa destino, int carta) {
    state->quantidade_opcoes = 0;
    state->carta_opcoes = carta;
    state->primeira_opcao = 0;
    state->destino_acao = destino;
    for (int i = 0; i < state->acoes.n; ++i) {
        Acao a = state->acoes.itens[i];
        if (!igualCasa(a.destino,destino) || (carta >= 0 && a.carta != carta))
            continue;
        if (carta < 0) {
            bool repetida = false;
            for (int k = 0; k < state->quantidade_opcoes; ++k) {
                if (state->acoes.itens[state->opcoes_acao[k]].carta == a.carta) {
                    repetida = true;
                    break;
                }
            }
            if (repetida) continue;
        }
        state->opcoes_acao[state->quantidade_opcoes++] = i;
    }
}

static bool selecionar_destino(GameState *state, Casa destino) {
    listar_opcoes(state,destino,-1);
    bool encontrou = state->quantidade_opcoes > 0;
    if (state->quantidade_opcoes == 1) {
        Acao a = state->acoes.itens[state->opcoes_acao[0]];
        if (a.carta == 0 && contar_variantes(state,0) == 1)
            confirmar_acao(state, a);
    }
    return encontrou;
}

static void confirmar_opcao(GameState *state, int indice) {
    Acao a = state->acoes.itens[state->opcoes_acao[indice]];
    if (state->carta_opcoes < 0 && contar_variantes(state,a.carta) > 1)
        listar_opcoes(state,state->destino_acao,a.carta);
    else
        confirmar_acao(state,a);
}

static void update_opcoes(GameState *state) {
    if (IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        if (state->carta_opcoes >= 0)
            listar_opcoes(state,state->destino_acao,-1);
        else
            state->quantidade_opcoes = 0;
        return;
    }
    int visiveis = quantidade_visivel();
    float wheel = GetMouseWheelMove();
    if (wheel > 0 || IsKeyPressed(KEY_UP)) --state->primeira_opcao;
    if (wheel < 0 || IsKeyPressed(KEY_DOWN)) ++state->primeira_opcao;
    int maximo = state->quantidade_opcoes - visiveis;
    if (maximo < 0) maximo = 0;
    if (state->primeira_opcao < 0) state->primeira_opcao = 0;
    if (state->primeira_opcao > maximo) state->primeira_opcao = maximo;
    for (int i = 0; i < visiveis && state->primeira_opcao+i < state->quantidade_opcoes; ++i) {
        if (clicked(opcao_rect(i)) || (i < 9 && IsKeyPressed(KEY_ONE+i))) {
            confirmar_opcao(state,state->primeira_opcao+i);
            return;
        }
    }
    if (IsKeyPressed(KEY_ENTER)) {
        confirmar_opcao(state,state->primeira_opcao);
    }
}

static bool tem_acao_no_turno(const Combate *j) {
    ListaAcoes lista;
    for (int u = 0; u < 40; ++u) {
        if (!j->unidades[u].vivo || j->unidades[u].lado != j->lado) continue;
        lista.n = 0;
        gerarAcoes(j, u, &lista);
        if (lista.n > 0) return true;
    }
    return false;
}

static int unidade_painel(const GameState *state, bool enemy) {
    const Combate *j = &state->combate;
    int u = state->selecionada;
    if (u >= 0 && j->unidades[u].vivo && j->unidades[u].lado == (int)enemy) return u;
    u = enemy ? state->inspecionada : -1;
    if (u >= 0 && j->unidades[u].vivo && j->unidades[u].lado == (int)enemy) return u;
    for (int i = 0; i < 40; ++i)
        if (j->unidades[i].vivo && j->unidades[i].lado == (int)enemy) return i;
    return -1;
}

static float inicio_cartas_painel(const GameState *state, Rectangle painel, bool enemy) {
    const Combate *j = &state->combate;
    bool presentes[6] = {false};
    int tipos = 0;
    for (int i = 0; i < 40; ++i) {
        if (!j->unidades[i].vivo || j->unidades[i].lado != (int)enemy) continue;
        int tipo = tipo_ui(j->unidades[i].tipo);
        if (tipo >= 0 && !presentes[tipo]) {
            presentes[tipo] = true;
            ++tipos;
        }
    }
    return painel.y + 88 + tipos*22;
}

static int cartas_painel(const GameState *state, int u, const Carta **saida) {
    if (u < 0) return 0;
    const Combate *j = &state->combate;
    const Unidade *p = &j->unidades[u];
    int tipo = indiceTipo(p->tipo);
    if (tipo < 0) return 0;
    unsigned adquiridas = p->lado ? p->cartas : j->adquiridas[tipo];
    int n = 0;
    for (int i = 0; i < CARTAS_TIPO; ++i) {
        const Carta *c = &catalogo[tipo][i];
        if (adquiridas & (1u << c->id)) saida[n++] = c;
    }
    return n;
}

static Rectangle carta_painel_rect(Rectangle painel, float y, int indice) {
    return rect(painel.x+9,y+indice*18,painel.width-18,17);
}

static int cartas_visiveis(Rectangle painel, float y) {
    int n = (int)((painel.y+painel.height-y-18)/18);
    return n > 0 ? n : 0;
}

static int limitar_scroll(int scroll, int total, int visiveis) {
    if (scroll > total-visiveis) scroll = total-visiveis;
    return scroll > 0 ? scroll : 0;
}

static bool update_painel(GameState *state, bool enemy) {
    Rectangle painel = painel_lateral(layout_tabuleiro(state),enemy);
    if (!hovered(painel)) return false;
    int u = unidade_painel(state,enemy);
    const Carta *cartas[CARTAS_TIPO];
    int total = cartas_painel(state,u,cartas);
    float y = inicio_cartas_painel(state,painel,enemy);
    int visiveis = cartas_visiveis(painel,y);
    int scroll = state->scroll_melhorias[enemy];
    float wheel = GetMouseWheelMove();
    if (wheel > 0) --scroll;
    if (wheel < 0) ++scroll;
    scroll = limitar_scroll(scroll,total,visiveis);
    state->scroll_melhorias[enemy] = scroll;
    if (!enemy) {
        for (int i = 0; i < visiveis && i+scroll < total; ++i) {
            const Carta *c = cartas[i+scroll];
            if (clicked(carta_painel_rect(painel,y,i)) && cartaReativa(c->tipo,c->id)) {
                state->combate.reacoesDesativadas[indiceTipo(c->tipo)] ^= 1u << c->id;
                if (state->selecionada >= 0)
                    gerarAcoes(&state->combate,state->selecionada,&state->acoes);
                break;
            }
        }
    }
    return true;
}

static void update_game(GameState *state) {
    Combate *j = &state->combate;
    int promovendo = promocaoPendente(j);
    if (state->fase == FASE_COMBATE && promovendo >= 0) {
        update_promocao(state,promovendo);
        return;
    }
    if (state->quantidade_opcoes > 0) {
        update_opcoes(state);
        return;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (state->fase == FASE_COMBATE && state->selecionada >= 0) limpar_selecao(state);
        else state->screen = SCREEN_START;
        return;
    }
    if (state->fase == FASE_RECOMPENSA) {
        int n = state->quantidade_recompensas + state->oferecer_recrutamento;
        for (int i = 0; i < n; ++i) {
            if (!clicked(recompensa_rect(i, n)) && !IsKeyPressed(KEY_ONE+i)) continue;
            if (i == state->quantidade_recompensas) iniciar_recrutamento(state);
            else if (adquirirCarta(j, state->recompensas[i])) comecar_combate(state);
            return;
        }
        return;
    }
    if (state->fase == FASE_RECRUTAR) {
        for (int i = 0; i < 3; ++i) {
            if (!clicked(recompensa_rect(i, 3)) && !IsKeyPressed(KEY_ONE+i)) continue;
            state->recruta = state->opcoes_recrutamento[i];
            state->fase = FASE_POSICIONAR;
            return;
        }
        return;
    }
    if (state->fase == FASE_POSICAO_INICIAL || state->fase == FASE_POSICIONAR) {
        Casa p = casa_mouse(state);
        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || !dentro(j, p) || p.l < 3) return;
        if (state->fase == FASE_POSICAO_INICIAL) {
            for (int u = 0; u < 40; ++u) {
                if (!j->unidades[u].vivo || j->unidades[u].lado != 0) continue;
                if (!igualCasa(posicao(j,u), p) && !deslocar(j,u,p)) return;
                j->unidades[u].origem = p;
                j->unidades[u].moveu = 0;
                preparar_recompensa(state);
                return;
            }
        } else if (vazio(j,p) && recrutar(j, state->recruta, p)) {
            comecar_combate(state);
        }
        return;
    }
    if (contarLado(j, 0) == 0) {
        state->screen = SCREEN_DEFEAT;
        return;
    }
    if (contarLado(j, 1) == 0) {
        limpar_selecao(state);
        if (prepararRodada(j)) preparar_recompensa(state);
        else state->screen = SCREEN_DEFEAT;
        return;
    }
    int inspeccionada = ocupante(j,casa_mouse(state));
    if (inspeccionada >= 0 && j->unidades[inspeccionada].lado == 1 &&
        inspeccionada != state->inspecionada) {
        state->inspecionada = inspeccionada;
        state->scroll_melhorias[1] = 0;
    }
    if (update_painel(state,false) || update_painel(state,true)) {
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) state->arrastando = false;
        return;
    }
    if (clicked(passar_rect())) {
        if (!tem_acao_no_turno(j)) {
            limpar_selecao(state);
            if (++state->passes >= 2) state->screen = SCREEN_DEFEAT;
            else encerrarAcao(j,-1);
        }
        return;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        limpar_selecao(state);
        return;
    }
    Casa p = casa_mouse(state);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && dentro(j,p)) {
        if (state->selecionada >= 0) {
            Casa atual = posicao(j, state->selecionada);
            if (!igualCasa(atual, p)) {
                if (selecionar_destino(state, p)) return;
            }
        }
        int u = ocupante(j,p);
        if (u >= 0 && j->unidades[u].lado == j->lado &&
            (j->bonus < 0 || j->bonus == u)) {
            bool mesma = u == state->selecionada;
            escolher_peca(state, u);
            state->ja_selecionada = mesma;
            state->arrastando = true;
        } else limpar_selecao(state);
    }
    if (state->arrastando && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        state->arrastando = false;
        if (dentro(j,p) && state->selecionada >= 0 &&
            (!igualCasa(p,posicao(j,state->selecionada)) || state->ja_selecionada))
            selecionar_destino(state,p);
    }
}

static float texto_wrap(const char *text, float x, float y, float width, int size, Color cor, bool desenhar) {
    float inicio = y;
    char line[256] = "";
    char word[128];
    while (*text) {
        while (*text == ' ') ++text;
        if (!*text) break;
        int n = 0;
        while (*text && *text != ' ' && n < (int)sizeof word-1) word[n++] = *text++;
        word[n] = '\0';
        char candidate[256];
        snprintf(candidate, sizeof candidate, "%s%s%s", line, line[0] ? " " : "", word);
        if (line[0] && MeasureText(candidate, size+2) > width) {
            if (desenhar) UiDrawText(line,x,y,(float)size,cor);
            y += size + 5;
            snprintf(line,sizeof line,"%s",word);
        } else {
            snprintf(line,sizeof line,"%s",candidate);
        }
    }
    if (*line) {
        if (desenhar) UiDrawText(line,x,y,(float)size,cor);
        y += size + 5;
    }
    return y-inicio;
}

static float texto_ajustado(const char *text, float x, float y, float width, float height,
                            int tamanho, Color cor) {
    while (tamanho > 10 && texto_wrap(text,x,y,width,tamanho,cor,false) > height)
        --tamanho;
    return texto_wrap(text,x,y,width,tamanho,cor,true);
}

static void draw_recompensa(const GameState *state) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    bool recrutando = state->fase == FASE_RECRUTAR;
    int n = recrutando ? 3 : state->quantidade_recompensas + state->oferecer_recrutamento;
    frame_title(recrutando ? "RECRUTAMENTO" : "MELHORIAS");
    UiDrawTextCentered(recrutando ? "ESCOLHA UMA NOVA PECA" : "ESCOLHA SUA RECOMPENSA",
                       w*.5f,h*.15f,26,UI_WHITE);
    char linha[64];
    snprintf(linha,sizeof linha,"RODADA %02d",state->round);
    UiDrawTextCentered(linha,w*.5f,h*.22f,16,UI_RED);
    for (int i = 0; i < n; ++i) {
        Rectangle r = recompensa_rect(i,n);
        const Carta *carta = !recrutando && i < state->quantidade_recompensas ?
                            state->recompensas[i] : NULL;
        char tipo = recrutando ? state->opcoes_recrutamento[i] : (carta ? carta->tipo : 0);
        UiDrawPanel(r);
        if (hovered(r)) DrawRectangleLinesEx(r, 2, UI_RED);
        float icon = minimum(minimum(64,r.width*.4f),r.height*.16f);
        if (tipo) UiDrawPiece(tipo_ui(tipo),true,rect(r.x+(r.width-icon)*.5f,r.y+12,icon,icon),1);
        else UiDrawTextCentered("+",r.x+r.width*.5f,r.y+15,52,UI_RED);
        UiDrawTextCentered(tipo ? nome_tipo(tipo) : "NOVA PECA",
                           r.x+r.width*.5f,r.y+icon+20,14,UI_RED);
        float text_y = r.y+icon+46;
        text_y += texto_ajustado(carta ? carta->nome : recrutando ? nome_tipo(tipo) : "Recrutar",
                                 r.x+14,text_y,r.width-28,46,16,UI_WHITE);
        const char *descricao = carta ? carta->descricao : recrutando ?
            "Adicione esta peca ao exercito. Escolha uma casa livre nas duas primeiras fileiras." :
            "Escolha um entre tres tipos de peca para aumentar seu exercito.";
        text_y += 7;
        texto_ajustado(descricao,r.x+14,text_y,r.width-28,r.y+r.height-35-text_y,13,UI_MUTED);
        char key[20];
        snprintf(key,sizeof key,"%d",i+1);
        UiDrawTextCentered(key,r.x+r.width*.5f,r.y+r.height-24,13,UI_RED);
    }
    UiDrawTextCentered("CLIQUE EM UMA CARTA OU PRESSIONE SEU NUMERO",w*.5f,h*.93f,12,UI_MUTED);
}

static void draw_side(Rectangle bounds, const GameState *state, bool enemy) {
    const Combate *j = &state->combate;
    UiDrawPanel(bounds);
    UiDrawText(enemy ? "INIMIGOS" : "ALIADOS",bounds.x+12,bounds.y+14,15,enemy?UI_RED:UI_WHITE);
    int counts[6] = {0};
    for (int u = 0; u < 40; ++u) {
        const Unidade *p = &j->unidades[u];
        if (!p->vivo || p->lado != (int)enemy) continue;
        int tipo = tipo_ui(p->tipo);
        if (tipo >= 0) counts[tipo]++;
    }
    float y = bounds.y+36;
    for (int i = 0; i < 6; ++i) {
        if (!counts[i]) continue;
        UiDrawPiece(i,!enemy,rect(bounds.x+10,y,22,22),1);
        char linha[32];
        snprintf(linha,sizeof linha,"%s  x%d",PIECE_NAMES[i],counts[i]);
        UiDrawText(linha,bounds.x+38,y+5,12,UI_WHITE);
        y += 22;
    }
    int selected = unidade_painel(state,enemy);
    y = inicio_cartas_painel(state,bounds,enemy);
    if (selected < 0 || y > bounds.y+bounds.height) return;
    const Unidade *p = &j->unidades[selected];
    UiDrawText("MELHORIAS",bounds.x+12,y-36,12,UI_RED);
    UiDrawText(nome_tipo(p->tipo),bounds.x+12,y-18,11,UI_WHITE);

    const Carta *cartas[CARTAS_TIPO];
    int total = cartas_painel(state,selected,cartas);
    int visiveis = cartas_visiveis(bounds,y);
    int scroll = limitar_scroll(state->scroll_melhorias[enemy],total,visiveis);
    const Carta *inspecionada = NULL;
    for (int i = 0; i < visiveis && i+scroll < total; ++i) {
        const Carta *carta = cartas[i+scroll];
        Rectangle r = carta_painel_rect(bounds,y,i);
        bool reativa = !enemy && cartaReativa(carta->tipo,carta->id);
        bool desligada = reativa &&
            (j->reacoesDesativadas[indiceTipo(carta->tipo)] & (1u << carta->id));
        bool indisponivel = desligada || p->silencio || usouCarta(j,selected,carta->id);
        if (hovered(r)) {
            inspecionada = carta;
            DrawRectangleRec(r,(Color){50,33,37,255});
        }
        UiDrawText(carta->nome,r.x+3,r.y+3,10,indisponivel?UI_MUTED:UI_WHITE);
        if (reativa)
            UiDrawText(desligada?"[OFF]":"[ON]",r.x+r.width-42,r.y+3,10,desligada?UI_MUTED:UI_RED);
    }
    if (!total && visiveis > 0)
        UiDrawText("Nenhuma",bounds.x+12,y+3,11,UI_MUTED);
    if (visiveis < total) {
        char linha[48];
        snprintf(linha,sizeof linha,"%d-%d / %d  |  role",scroll+1,scroll+visiveis,total);
        UiDrawText(linha,bounds.x+12,bounds.y+bounds.height-16,10,UI_MUTED);
    }
    if (inspecionada) {
        Rectangle tip = rect(enemy ? bounds.x-320 : bounds.x+bounds.width+8,
                             bounds.y+bounds.height-101,312,99);
        UiDrawPanel(tip);
        UiDrawText(inspecionada->nome,tip.x+10,tip.y+8,13,UI_RED);
        texto_ajustado(inspecionada->descricao,tip.x+10,tip.y+30,tip.width-20,
                       tip.height-38,12,UI_WHITE);
    }
}

static void draw_captures(float x, float y, float width, const char *label, const Combate *j, int lado) {
    UiDrawText(label,x,y,11,UI_MUTED);
    DrawRectangleLinesEx(rect(x,y+19,width,36),1,UI_MUTED);
    float px = x+7;
    for (int i = 0; i < 6; ++i) {
        int tipo = indiceTipo(PIECE_TYPES[i]);
        int n = j->capturas[lado][tipo];
        if (!n) continue;
        UiDrawPiece(i,lado==0,rect(px,y+23,26,26),1);
        char qtd[12];
        snprintf(qtd,sizeof qtd,"x%d",n);
        UiDrawText(qtd,px+27,y+30,11,UI_WHITE);
        px += 53;
    }
}

static const char *rotulo_extra(char tipo, int carta) {
    if (tipo == 'B') {
        if (carta == 2) return "Reflexao";
        if (carta == 5) return "Canto de chegada";
        if (carta == 7) return "Recuo";
    }
    if (tipo == 'T') {
        if (carta == 4 || carta == 5) return "Destino do aliado";
        if (carta == 7) return "Recuo";
        if (carta == 9) return "Destino final";
        if (carta == 14 || carta == 18) return "Aliado";
        if (carta == 16) return "Ponto de curva";
    }
    if (tipo == 'C') {
        if (carta == 7) return "Destino do resgatado";
        if (carta == 10) return "Cavalo aliado";
        if (carta == 16) return "Destino final";
        if (carta == 19) return "Peao aliado";
    }
    if (tipo == 'R') {
        if (carta == 2) return "Destino do aliado";
        if (carta == 15) return "Aliado tocado";
    }
    if (tipo == 'D') {
        if (carta == 6) return "Destino do peao";
        if (carta == 19) return "Segundo aliado";
    }
    return "Alvo adicional";
}

static void detalhe_variante(const Combate *j, Acao a, char *buf, size_t tamanho) {
    char tipo = j->unidades[a.ator].tipo;
    if (dentro(j,a.extra)) {
        snprintf(buf,tamanho,"%s: %c%d",rotulo_extra(tipo,a.carta),
                 'A'+a.extra.c,5-a.extra.l);
    } else if (tipo == 'C' && a.carta == 7) {
        snprintf(buf,tamanho,"Sem movimento adicional");
    } else {
        snprintf(buf,tamanho,"Destino: %c%d",'A'+a.destino.c,5-a.destino.l);
    }
    if (tipo == 'D' && a.carta == 6) {
        size_t n = strlen(buf);
        snprintf(buf+n,tamanho-n," | Rainha %s",
                 dentro(j,a.extra2) ? "acompanha" : "fica");
    } else if (dentro(j,a.extra2)) {
        size_t n = strlen(buf);
        snprintf(buf+n,tamanho-n," | Destino do aliado: %c%d",
                 'A'+a.extra2.c,5-a.extra2.l);
    }
}

static const char *nome_acao(const Combate *j, Acao a) {
    const Carta *carta = buscarCarta(j->unidades[a.ator].tipo,a.carta);
    if (carta) return carta->nome;
    return aliado(j,a.ator,ocupante(j,a.destino)) ? "Toque do rei" : "Movimento base";
}

static void draw_opcoes(const GameState *state) {
    const Combate *j = &state->combate;
    Rectangle p = opcoes_painel();
    DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),(Color){0,0,0,170});
    UiDrawPanel(p);
    bool variantes = state->carta_opcoes >= 0;
    Acao primeira = state->acoes.itens[state->opcoes_acao[0]];
    UiDrawText(variantes ? nome_acao(j,primeira) : "ESCOLHA A ACAO",
               p.x+18,p.y+16,20,UI_WHITE);
    UiDrawText(variantes ? "Escolha a variante de movimento." :
               "Confirme o movimento ou selecione uma melhoria.",
               p.x+18,p.y+48,13,UI_MUTED);
    int visiveis = quantidade_visivel();
    const char *descricao = NULL;
    for (int i = 0; i < visiveis && state->primeira_opcao+i < state->quantidade_opcoes; ++i) {
        Acao a = state->acoes.itens[state->opcoes_acao[state->primeira_opcao+i]];
        const Carta *c = buscarCarta(j->unidades[a.ator].tipo,a.carta);
        if (i == 0 || hovered(opcao_rect(i))) {
            int alvo = ocupante(j,a.destino);
            descricao = c ? c->descricao : (alvo >= 0 && aliado(j,a.ator,alvo) ?
                "Concede a duas ações consecutivas ao aliado selecionado." :
                "Movimento normal da peça. Se houver um adversário, captura-o.");
        }
        Rectangle r = opcao_rect(i);
        DrawRectangleRec(r,hovered(r)?(Color){70,27,30,255}:(Color){32,32,37,255});
        DrawRectangleLinesEx(r,1,hovered(r)?UI_RED:UI_MUTED);
        char label[120], variante[110];
        detalhe_variante(j,a,variante,sizeof variante);
        snprintf(label,sizeof label,"%d. %s",i+1,
                 variantes ? variante : nome_acao(j,a));
        UiDrawText(label,r.x+10,r.y+5,15,UI_WHITE);
        char detalhes[120];
        int total = contar_variantes(state,a.carta);
        if (!variantes && total > 1)
            snprintf(detalhes,sizeof detalhes,"%d variantes disponiveis",total);
        else if (variantes || !dentro(j,a.extra))
            snprintf(detalhes,sizeof detalhes,"%s: %c%d",
                     j->unidades[a.ator].tipo == 'B' && a.carta == 5 ?
                     "Canto intermediario" : "Casa selecionada",
                     'A'+a.destino.c,5-a.destino.l);
        else
            snprintf(detalhes,sizeof detalhes,"%s",variante);
        UiDrawText(detalhes,r.x+10,r.y+27,12,UI_MUTED);
    }
    if (descricao)
        texto_ajustado(descricao,p.x+18,p.y+p.height-75,p.width-36,46,12,UI_WHITE);
    char info[100];
    snprintf(info,sizeof info,"%d %s  |  RODA OU SETAS PARA NAVEGAR  |  ESC PARA VOLTAR",
             state->quantidade_opcoes,variantes ? "variantes" : "opcoes");
    UiDrawText(info,p.x+18,p.y+p.height-23,11,UI_MUTED);
}

static void draw_promocao(const GameState *state, int unidade) {
    const Combate *j = &state->combate;
    const Unidade *peao = &j->unidades[unidade];
    Casa casa = posicao(j,unidade);
    Rectangle p = promocao_painel();
    DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),(Color){0,0,0,185});
    UiDrawPanel(p);
    UiDrawTextCentered("PROMOCAO TEMPORARIA",p.x+p.width*.5f,p.y+15,24,UI_WHITE);
    char info[80];
    snprintf(info,sizeof info,"PEAO %s EM %c%d",peao->lado ? "INIMIGO" : "ALIADO",
             'A'+casa.c,5-casa.l);
    UiDrawTextCentered(info,p.x+p.width*.5f,p.y+49,15,UI_RED);
    UiDrawTextCentered("Escolha uma peca ate o fim desta rodada.",p.x+p.width*.5f,
                       p.y+69,13,UI_MUTED);
    for (int i = 0; i < 4; ++i) {
        Rectangle r = promocao_rect(i);
        UiDrawPanel(r);
        if (hovered(r)) DrawRectangleLinesEx(r,2,UI_RED);
        float icon = minimum(r.width*.45f,75);
        char tipo = PROMOCAO_TIPOS[i];
        UiDrawPiece(tipo_ui(tipo),peao->lado == 0,
                    rect(r.x+(r.width-icon)*.5f,r.y+9,icon,icon),1);
        UiDrawTextCentered(nome_tipo(tipo),r.x+r.width*.5f,r.y+92,15,UI_WHITE);
        char tecla[2] = {(char)('1'+i),'\0'};
        UiDrawTextCentered(tecla,r.x+r.width*.5f,r.y+117,13,UI_RED);
    }
    UiDrawTextCentered("CLIQUE NA PECA OU PRESSIONE 1-4",p.x+p.width*.5f,
                       p.y+p.height-24,12,UI_MUTED);
}

static void draw_game(const GameState *state) {
    if (state->fase == FASE_RECOMPENSA || state->fase == FASE_RECRUTAR) {
        draw_recompensa(state);
        return;
    }
    const Combate *j = &state->combate;
    LayoutTabuleiro layout = layout_tabuleiro(state);
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    char titulo[48];
    snprintf(titulo,sizeof titulo,"RODADA %02d",state->round);
    frame_title(titulo);
    bool posicionando = state->fase == FASE_POSICAO_INICIAL || state->fase == FASE_POSICIONAR;
    int promovendo = promocaoPendente(j);
    const char *status = posicionando ? "ESCOLHA UMA CASA NAS DUAS FILEIRAS INFERIORES" :
        promovendo >= 0 ? "PROMOCAO DE PEAO" :
        j->lado ? "TURNO INIMIGO - CONTROLE MANUAL" : "TURNO ALIADO";
    UiDrawTextCentered(status,w*.5f,78,15,UI_RED);
    draw_captures(layout.x,layout.y-62,layout.largura,"ALIADOS CAPTURADOS",j,0);
    draw_captures(layout.x,layout.y+layout.altura+7,layout.largura,"INIMIGOS CAPTURADOS",j,1);
    Casa origem = state->selecionada >= 0 ? posicao(j,state->selecionada) : (Casa){-1,-1};
    for (int l = 0; l < 5; ++l) {
        for (int c = 0; c < j->largura; ++c) {
            Casa p = {l,c};
            Rectangle sq = casa_rect(layout,p);
            Color base = (l+c)%2==0 ? (Color){235,234,231,255} : (Color){45,45,49,255};
            DrawRectangleRec(sq,base);
            bool legal = false;
            if (posicionando) {
                legal = l >= 3 && (vazio(j,p) || (state->fase == FASE_POSICAO_INICIAL &&
                        ocupante(j,p) >= 0 && j->unidades[ocupante(j,p)].lado == 0));
            } else {
                for (int i = 0; i < state->acoes.n; ++i) {
                    if (igualCasa(state->acoes.itens[i].destino,p)) {
                        legal = true;
                        break;
                    }
                }
            }
            if (legal) DrawRectangleRec(sq,(Color){206,41,49,90});
            if (j->selada[l][c]) DrawRectangleLinesEx(sq,3,UI_RED);
            int u = ocupante(j,p);
            if (u >= 0) {
                const Unidade *unidade = &j->unidades[u];
                if (!state->arrastando || u != state->selecionada)
                    UiDrawPiece(tipo_ui(unidade->tipo),unidade->lado==0,
                                rect(sq.x+layout.tile*.08f,sq.y+layout.tile*.08f,
                                     layout.tile*.84f,layout.tile*.84f),1);
                if (unidade->chefe || unidade->duplo || unidade->guarda || unidade->gambito ||
                    unidade->silencio || unidade->marca || unidade->herdeira) {
                    const char *badge = unidade->chefe ? "CHEFE" : unidade->duplo ? "2X" :
                        unidade->guarda ? "GUARDA" : unidade->gambito ? "GAMBITO" :
                        unidade->silencio ? "SILENCIO" : unidade->marca ? "MARCA" : "HERDEIRA";
                    DrawRectangleRec(rect(sq.x,sq.y+sq.height-15,sq.width,15),UI_BLACK);
                    UiDrawTextCentered(badge,sq.x+sq.width*.5f,sq.y+sq.height-13,9,UI_RED);
                }
            }
            if (legal || igualCasa(origem,p)) DrawRectangleLinesEx(sq,igualCasa(origem,p)?3:1.5f,UI_RED);
            if (hovered(sq)) DrawRectangleLinesEx(sq,1,UI_WHITE);
            if (l == 4) {
                char label[] = {(char)('A'+c),'\0'};
                UiDrawText(label,sq.x+3,sq.y+sq.height-13,10,UI_MUTED);
            }
            if (c == 0) {
                char label[] = {(char)('5'-l),'\0'};
                UiDrawText(label,sq.x+3,sq.y+2,10,UI_MUTED);
            }
        }
    }
    DrawRectangleLinesEx(rect(layout.x-3,layout.y-3,layout.largura+6,layout.altura+6),3,UI_RED);
    draw_side(painel_lateral(layout,false),state,false);
    draw_side(painel_lateral(layout,true),state,true);
    if (state->arrastando && state->selecionada >= 0) {
        Vector2 mouse = GetMousePosition();
        float size = layout.tile*.84f;
        UiDrawPiece(tipo_ui(j->unidades[state->selecionada].tipo),
                    j->unidades[state->selecionada].lado == 0,
                    rect(mouse.x-size*.5f,mouse.y-size*.5f,size,size),.9f);
    }
    if (posicionando)
        UiDrawTextCentered("CLIQUE NA CASA DE ORIGEM",w*.5f,h-31,12,UI_MUTED);
    else {
        UiDrawText("ARRASTE OU SELECIONE E CLIQUE. CLIQUE DE NOVO NA PECA PARA PODERES ESTACIONARIOS.",
                   24,h-40,11,UI_MUTED);
        UiDrawText("CLIQUE NAS REACOES [ON]/[OFF] PARA ATIVAR OU DESATIVAR. ROLE O PAINEL PARA VER MAIS.",
                   24,h-23,10,UI_MUTED);
        if (promovendo < 0 && !tem_acao_no_turno(j))
            UiDrawButton(passar_rect(),"PASSAR",hovered(passar_rect()));
    }
    if (promovendo >= 0) draw_promocao(state,promovendo);
    else if (state->quantidade_opcoes > 0) draw_opcoes(state);
}

static void draw_defeat(const GameState *state) {
    float w=(float)GetScreenWidth(), h=(float)GetScreenHeight();
    frame_title("FIM DA RUN");
    DrawLineEx((Vector2){w*.5f,h*.25f},(Vector2){w*.5f,h*.34f},3,UI_RED);
    UiDrawTextCentered("DERROTA",w*.5f,h*.38f,minimum(w*.077f,92),UI_WHITE);
    char round_text[64];
    snprintf(round_text,sizeof round_text,"VOCE CHEGOU A RODADA %02d",state->round);
    UiDrawTextCentered(round_text,w*.5f,h*.57f,20,UI_RED);
    UiDrawButton(defeat_back(),"VOLTAR AO INICIO",hovered(defeat_back()));
    UiDrawTextCentered("ENTER PARA VOLTAR",w*.5f,h*.92f,13,UI_MUTED);
}

void DrawCurrentScreen(const GameState *state) {
    if (!state) return;
    ClearBackground(UI_BG);
    switch (state->screen) {
        case SCREEN_START: draw_start(state); break;
        case SCREEN_SELECT: draw_selection(state); break;
        case SCREEN_GAME: draw_game(state); break;
        case SCREEN_DEFEAT: draw_defeat(state); break;
        case SCREEN_EXIT: break;
    }
}
