#include <stdlib.h>
#include "../../campo/combate.h"

static const Casa nenhuma = {-1, -1};

static int sinal(int n) {
    return (n > 0) - (n < 0);
}

static int distancia(Casa a, Casa b) {
    int dl = abs(a.l - b.l), dc = abs(a.c - b.c);
    return dl > dc ? dl : dc;
}

static int alinhada(Casa a, Casa b) {
    return !igualCasa(a,b) && (ortogonal(a,b) || diagonal(a,b));
}

static int visivel(const Combate *j, Casa a, Casa b) {
    return alinhada(a,b) && caminho(j,a,b);
}

static int pousoPermitido(const Combate *j, int u, Casa p) {
    return dentro(j,p) && j->selada[p.l][p.c] != j->unidades[u].lado+1;
}

static int destinoLivre(const Combate *j, int u, Casa p) {
    return pousoPermitido(j,u,p) && vazio(j,p);
}

static int disponivel(const Combate *j, int u, int id) {
    return temCarta(j,u,id) && !usouCarta(j,u,id);
}

static int podeTocar(const Combate *j, int u, int v) {
    return aliado(j,u,v) && u != v && j->unidades[v].tipo != 'R'
        && !j->unidades[v].duplo;
}

static int casaCapturavel(const Combate *j, int u, Casa p) {
    int v = ocupante(j,p);
    return pousoPermitido(j,u,p)
        && (vazio(j,p) || (inimigo(j,u,v) && podeCapturar(j,u,v)));
}

static int capturaRei(const Combate *j, int u, Casa a, Casa b) {
    int v = ocupante(j,b);
    if (!pousoPermitido(j,u,b) || !inimigo(j,u,v) || !podeCapturar(j,u,v)) return 0;
    if (adjacente(a,b)) return 1;
    if (distancia(a,b) != 2 || !visivel(j,a,b)) return 0;
    return (temCarta(j,u,5) && contarLado(j,j->unidades[u].lado) == 1)
        || (temCarta(j,u,20) && j->unidades[v].chefe);
}

static void gerarRei(const Combate *j, int u, ListaAcoes *lista) {
    Casa a = posicao(j,u);
    for (int l = 0; l < 5; l++) {
        for (int c = 0; c < j->largura; c++) {
            Casa b = {l,c};
            int v = ocupante(j,b);
            if (igualCasa(a,b)) continue;
            if (podeTocar(j,u,v)) {
                if (temCarta(j,u,1) && ortogonal(a,b) && distancia(a,b) == 2 && caminho(j,a,b))
                    adicionar(lista,u,1,b,nenhuma,nenhuma);
                if (disponivel(j,u,7))
                    adicionar(lista,u,7,b,nenhuma,nenhuma);
                if (temCarta(j,u,13) && igualCasa(a,j->unidades[u].origem)
                    && diagonal(a,b) && caminho(j,a,b))
                    adicionar(lista,u,13,b,nenhuma,nenhuma);
            }
            if (temCarta(j,u,17) && aliado(j,u,v) && adjacente(a,b) && !j->unidades[v].guarda)
                adicionar(lista,u,17,b,nenhuma,nenhuma);
            if (temCarta(j,u,2) && aliado(j,u,v)
                && ortogonal(a,b) && caminho(j,a,b)) {
                for (int dl = -1; dl <= 1; dl++) {
                    for (int dc = -1; dc <= 1; dc++) {
                        Casa p = {a.l+dl,a.c+dc};
                        if (destinoLivre(j,v,p)) adicionar(lista,u,2,b,p,nenhuma);
                    }
                }
            }
            if (temCarta(j,u,5) && contarLado(j,j->unidades[u].lado) == 1
                && distancia(a,b) == 2 && visivel(j,a,b) && casaCapturavel(j,u,b))
                adicionar(lista,u,5,b,nenhuma,nenhuma);
            if (temCarta(j,u,11) && inimigo(j,u,v) && adjacente(a,b) && !j->unidades[v].chefe)
                adicionar(lista,u,11,b,nenhuma,nenhuma);
            if (temCarta(j,u,20) && inimigo(j,u,v) && j->unidades[v].chefe
                && distancia(a,b) == 2 && visivel(j,a,b) && casaCapturavel(j,u,b))
                adicionar(lista,u,20,b,nenhuma,nenhuma);
            if (disponivel(j,u,15) && capturaRei(j,u,a,b)) {
                for (int dl = -1; dl <= 1; dl++) {
                    for (int dc = -1; dc <= 1; dc++) {
                        Casa p = {b.l+dl,b.c+dc};
                        int alvo = ocupante(j,p);
                        if (podeTocar(j,u,alvo)) adicionar(lista,u,15,b,p,nenhuma);
                    }
                }
            }
        }
    }
}

static int movimentoPeaoBase(const Combate *j, int u, Casa b) {
    Casa a = posicao(j,u);
    int v = ocupante(j,b);
    int direcao = j->unidades[u].lado ? 1 : -1;
    int dl = b.l-a.l, dc = abs(b.c-a.c);
    if (!pousoPermitido(j,u,b)) return 0;
    if (inimigo(j,u,v))
        return dl == direcao && dc == 1 && podeCapturar(j,u,v);
    if (!vazio(j,b) || dc != 0) return 0;
    return dl == direcao || (dl == 2*direcao && !j->unidades[u].moveu
        && vazio(j,(Casa){a.l+direcao,a.c}));
}

static void gerarRegencia(const Combate *j, int u, int peao, ListaAcoes *lista) {
    Casa a = posicao(j,u), b = posicao(j,peao);
    for (int l = 0; l < 5; l++) {
        for (int c = 0; c < j->largura; c++) {
            Casa p = {l,c};
            if (!movimentoPeaoBase(j,peao,p)) continue;
            adicionar(lista,u,6,b,p,nenhuma);
            /* O peão deixa livre o último quadrado; se pousar no trajeto,
             * a rainha não pode ocupar a casa anterior. */
            int dl = sinal(b.l-a.l), dc = sinal(b.c-a.c);
            int bloqueio = 0;
            for (Casa q={a.l+dl,a.c+dc}; !igualCasa(q,b);
                 q=(Casa){q.l+dl,q.c+dc}) {
                if (igualCasa(q,p)) bloqueio = 1;
            }
            if (!bloqueio && pousoPermitido(j,u,b)) adicionar(lista,u,6,b,p,b);
        }
    }
}

static void gerarRainha(const Combate *j, int u, ListaAcoes *lista) {
    Casa a = posicao(j,u);
    if (disponivel(j,u,9)) adicionar(lista,u,9,a,nenhuma,nenhuma);
    if (disponivel(j,u,11)) {
        Casa vertical = {4-a.l,a.c};
        Casa horizontal = {a.l,j->largura-1-a.c};
        if ((a.l == 0 || a.l == 4) && destinoLivre(j,u,vertical))
            adicionar(lista,u,11,vertical,nenhuma,nenhuma);
        if ((a.c == 0 || a.c == j->largura-1) && destinoLivre(j,u,horizontal))
            adicionar(lista,u,11,horizontal,nenhuma,nenhuma);
    }
    for (int v = 0; v < j->quantidade; v++) {
        if (v == u || !aliado(j,u,v)) continue;
        Casa b = posicao(j,v);
        if (disponivel(j,u,13) && adjacente(a,b) && j->unidades[v].tipo == 'P')
            adicionar(lista,u,13,b,nenhuma,nenhuma);
        if (!visivel(j,a,b)) continue;
        if (temCarta(j,u,4) && pousoPermitido(j,u,b) && pousoPermitido(j,v,a))
            adicionar(lista,u,4,b,nenhuma,nenhuma);
        if (temCarta(j,u,6) && j->unidades[v].tipo == 'P') gerarRegencia(j,u,v,lista);
        if (temCarta(j,u,17) && distancia(a,b) >= 2)
            adicionar(lista,u,17,b,nenhuma,nenhuma);
        if (disponivel(j,u,19)) {
            for (int w = v+1; w < j->quantidade; w++) {
                if (w == u || !aliado(j,u,w)) continue;
                Casa p = posicao(j,w);
                if (visivel(j,a,p) && pousoPermitido(j,v,p) && pousoPermitido(j,w,b)
                    && sinal(b.l-a.l) == -sinal(p.l-a.l)
                    && sinal(b.c-a.c) == -sinal(p.c-a.c))
                    adicionar(lista,u,19,b,p,nenhuma);
            }
        }
    }
}

void gerarRD(const Combate *j, int u, ListaAcoes *lista) {
    if (j->unidades[u].tipo == 'R') gerarRei(j,u,lista);
    if (j->unidades[u].tipo == 'D') gerarRainha(j,u,lista);
}

static int executarRei(Combate *j, Acao a) {
    int u = a.ator, v = ocupante(j,a.destino);
    switch (a.carta) {
        case 1: case 7: case 13:
            tocar(j,v);
            if (a.carta == 7) gastarCarta(j,u,7);
            return 1;
        case 2:
            return deslocar(j,v,a.extra);
        case 5: case 20:
            return moverOuCapturar(j,u,a.destino);
        case 11:
            j->unidades[v].silencio = 1;
            return 1;
        case 15: {
            int alvo = ocupante(j,a.extra);
            if (!capturar(j,u,v)) return 0;
            if (!j->unidades[v].vivo && j->unidades[u].vivo && j->unidades[alvo].vivo) {
                tocar(j,alvo);
                gastarCarta(j,u,15);
            }
            return 1;
        }
        case 17:
            j->unidades[v].guarda = 1;
            return 1;
        default:
            return 0;
    }
}

static int executarRainha(Combate *j, Acao a) {
    int u = a.ator, v = ocupante(j,a.destino);
    switch (a.carta) {
        case 4:
            return trocar(j,u,v);
        case 6: {
            Casa origem = posicao(j,v);
            int alvo = ocupante(j,a.extra);
            if (!moverOuCapturar(j,v,a.extra)) return 0;
            if (j->unidades[v].vivo) {
                j->unidades[v].herdeira = 0;
                j->unidades[v].corredor = nenhuma;
                depoisPC(j,v,origem,alvo >= 0 && !j->unidades[alvo].vivo);
            }
            if (dentro(j,a.extra2) && j->unidades[u].vivo
                && vazio(j,origem) && visivel(j,posicao(j,u),origem))
                deslocar(j,u,origem);
            if (j->unidades[v].vivo) depoisRD(j,v,origem,alvo >= 0 && !j->unidades[alvo].vivo);
            return 1;
        }
        case 9:
            j->unidades[u].gambito = 1;
            gastarCarta(j,u,9);
            return 1;
        case 11:
            if (!deslocar(j,u,a.destino)) return 0;
            gastarCarta(j,u,11);
            return 1;
        case 13:
            j->unidades[v].herdeira = 1;
            gastarCarta(j,u,13);
            return 1;
        case 17:
            j->unidades[v].corredor = posicao(j,u);
            return 1;
        case 19:
            if (!trocar(j,v,ocupante(j,a.extra))) return 0;
            gastarCarta(j,u,19);
            return 1;
        default:
            return 0;
    }
}

int executarRD(Combate *j, Acao a) {
    if (j->unidades[a.ator].tipo == 'R') return executarRei(j,a);
    if (j->unidades[a.ator].tipo == 'D') return executarRainha(j,a);
    return 0;
}

/* Uma casa liberada só revela um inimigo quando está estritamente entre
 * rainha e alvo, na linha ou diagonal de ataque. */
static int entre(Casa a, Casa b, Casa p) {
    if (!alinhada(a,b)) return 0;
    int dl = sinal(b.l-a.l), dc = sinal(b.c-a.c);
    for (Casa q={a.l+dl,a.c+dc}; !igualCasa(q,b); q=(Casa){q.l+dl,q.c+dc})
        if (igualCasa(q,p)) return 1;
    return 0;
}

void depoisRD(Combate *j, int u, Casa origem, int capturou) {
    (void)capturou;
    if (igualCasa(posicao(j,u),origem) || !vazio(j,origem)) return;
    for (int d = 0; d < j->quantidade; d++) {
        if (d == u || !aliado(j,u,d) || j->unidades[d].tipo != 'D'
            || !disponivel(j,d,20)) continue;
        Casa rainha = posicao(j,d);
        for (int v = 0; v < j->quantidade; v++) {
            if (!inimigo(j,d,v) || !podeCapturar(j,d,v)) continue;
            Casa alvo = posicao(j,v);
            if (!entre(rainha,alvo,origem) || !caminho(j,rainha,alvo)
                || !pousoPermitido(j,d,alvo)) continue;
            if (capturar(j,d,v)) gastarCarta(j,d,20);
            return;
        }
    }
}

int antesRD(Combate *j, int u, int v) {
    (void)j; (void)u; (void)v;
    return 0;
}

void efeitosMorteRD(Combate *j, int u, int v) {
    if (j->unidades[v].tipo == 'R' && disponivel(j,v,19)) {
        for (int p = 0; p < j->quantidade; p++) {
            if (!j->unidades[p].vivo || j->unidades[p].lado != j->unidades[v].lado
                || j->unidades[p].tipo != 'P') continue;
            gastarCarta(j,v,19);
            promover(j,p,'R',0);
            break;
        }
    }
    if (j->unidades[v].tipo != 'D') return;
    if (temCarta(j,v,7) && j->unidades[u].vivo) j->unidades[u].marca = 1;
    if (j->unidades[v].gambito && j->unidades[u].vivo) {
        j->unidades[v].gambito = 0;
        if (!j->unidades[u].chefe || contarLado(j,j->unidades[u].lado) == 1)
            remover(j,u);
    }
}

void aposCapturaRD(Combate *j, int u, int v, Casa casa) {
    if (j->unidades[v].tipo != 'D' || !j->unidades[u].vivo || !disponivel(j,v,16)) return;
    for (int p = 0; p < j->quantidade; p++) {
        if (!j->unidades[p].vivo || j->unidades[p].lado != j->unidades[v].lado
            || !podeCapturar(j,p,u)) continue;
        Casa a = posicao(j,p);
        char tipo = j->unidades[p].tipo;
        if ((tipo == 'T' && ortogonal(a,casa)) || (tipo == 'B' && diagonal(a,casa))) {
            if (!caminho(j,a,casa)) continue;
            if (!pousoPermitido(j,p,casa)) continue;
            if (capturar(j,p,u)) gastarCarta(j,v,16);
            return;
        }
    }
}
