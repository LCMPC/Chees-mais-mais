#include "screens.h"
#include "theme.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static const char *PIECE_NAMES[] = { "PEAO", "TORRE", "BISPO", "CAVALO", "REI" };

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

static Rectangle game_demo(void) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    return rect(w - 209, h - 58, 173, 35);
}

static Rectangle defeat_back(void) {
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    return rect(w * .39f, h * .68f, w * .22f, 52);
}

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
            if (clicked(select_confirm()) || IsKeyPressed(KEY_ENTER)) { state->round = 1; state->screen = SCREEN_GAME; }
            if (IsKeyPressed(KEY_ESCAPE)) state->screen = SCREEN_START;
            break;
        case SCREEN_GAME:
            if (clicked(game_demo()) || IsKeyPressed(KEY_D)) state->screen = SCREEN_DEFEAT;
            if (IsKeyPressed(KEY_ESCAPE)) state->screen = SCREEN_START;
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

static void draw_side(Rectangle bounds, const char *title, bool enemy, int selected) {
    UiDrawPanel(bounds);
    UiDrawText(title,bounds.x+15,bounds.y+14,16,enemy?UI_RED:UI_WHITE);
    DrawLineEx((Vector2){bounds.x+15,bounds.y+46},(Vector2){bounds.x+bounds.width-15,bounds.y+46},1,UI_MUTED);
    float icon=minimum(bounds.width*.35f,58);
    float row_step=minimum(80,(bounds.height-160)/3);
    for(int i=0;i<3;++i) {
        float y=bounds.y+62+i*row_step;
        UiDrawPiece((i==0?selected:i),!enemy,rect(bounds.x+15,y,icon,icon),1);
        char label[32];
        snprintf(label,sizeof label,"%s  x%d",PIECE_NAMES[(i==0?selected:i)],i==0?1:2);
        UiDrawText(label,bounds.x+icon+26,y+8,13,UI_WHITE);
        UiDrawText("SEM MELHORIA",bounds.x+icon+26,y+31,11,UI_MUTED);
    }
    UiDrawText("MELHORIAS",bounds.x+15,bounds.y+bounds.height-92,14,UI_RED);
    UiDrawText("Nenhuma nesta demo",bounds.x+15,bounds.y+bounds.height-62,12,UI_MUTED);
}

static void draw_captures(float x,float y,float width,const char *label,bool white) {
    UiDrawText(label,x,y,11,UI_MUTED);
    DrawRectangleLinesEx(rect(x,y+22,width,37),1,UI_MUTED);
    for(int i=0;i<3;++i) UiDrawPiece(i,white,rect(x+7+i*40,y+25,31,31),.75f);
}

static void draw_game(const GameState *state) {
    float w=(float)GetScreenWidth(), h=(float)GetScreenHeight();
    frame_title("RODADA 01");
    float tile=minimum((h-290)/5,(w*.46f)/5);
    float board=tile*5;
    float bx=(w-board)*.5f, by=(h-board)*.5f+8;
    float sidew=minimum(w*.20f,250);
    float sx=minimum(38,bx-sidew-22);
    if(sx<12) sx=12;
    draw_side(rect(sx,by,sidew,board),"ALIADOS",false,state->selected_piece);
    draw_side(rect(w-sx-sidew,by,sidew,board),"INIMIGOS",true,UI_PAWN);
    draw_captures(bx,by-82,board,"ALIADOS CAPTURADOS PELO INIMIGO",true);
    draw_captures(bx,by+board+15,board,"INIMIGOS CAPTURADOS PELO JOGADOR",false);
    for(int row=0;row<5;++row) for(int col=0;col<5;++col) {
        Rectangle sq=rect(bx+col*tile,by+row*tile,tile,tile);
        Color base=((row+col)%2==0)?(Color){235,234,231,255}:(Color){45,45,49,255};
        DrawRectangleRec(sq,base);
        if(row==4&&col==2) UiDrawPiece(state->selected_piece,true,rect(sq.x+tile*.08f,sq.y+tile*.08f,tile*.84f,tile*.84f),1);
        if(row==0&&(col==1||col==3)) UiDrawPiece(col==1?UI_ROOK:UI_KNIGHT,false,rect(sq.x+tile*.08f,sq.y+tile*.08f,tile*.84f,tile*.84f),1);
        if(row==1&&col==2) UiDrawPiece(UI_PAWN,false,rect(sq.x+tile*.08f,sq.y+tile*.08f,tile*.84f,tile*.84f),1);
    }
    DrawRectangleLinesEx(rect(bx-3,by-3,board+6,board+6),3,UI_RED);
    UiDrawText("PREVIA DE INTERFACE",38,h-49,12,UI_MUTED);
    UiDrawButton(game_demo(),"SIMULAR DERROTA",hovered(game_demo()));
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
