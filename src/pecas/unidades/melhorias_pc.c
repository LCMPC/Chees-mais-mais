#include <stdlib.h>
#include "../../campo/combate.h"
#include "../../campo/movimento.h"

static const Casa nenhuma = {-1, -1};
static const Casa ortogonais[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

static Casa somar(Casa a, Casa b) {
    return (Casa){a.l + b.l, a.c + b.c};
}

static int livre(const Combate *j, int u, Casa p) {
    return dentro(j, p) && vazio(j, p)
        && j->selada[p.l][p.c] != j->unidades[u].lado + 1;
}

static int capturaDisponivel(const Combate *j, int u, Casa destino) {
    return dentro(j, destino) && j->selada[destino.l][destino.c] != j->unidades[u].lado + 1
        && podeCapturar(j, u, ocupante(j, destino));
}

static int trocaDisponivel(const Combate *j, int u, int v, Casa a, Casa b) {
    return aliado(j, u, v) && j->selada[a.l][a.c] != j->unidades[v].lado + 1
        && j->selada[b.l][b.c] != j->unidades[u].lado + 1;
}

static int cartaDisponivel(const Combate *j, int u, int carta) {
    return temCarta(j, u, carta) && !usouCarta(j, u, carta);
}

static void adicionarDestino(ListaAcoes *lista, int u, int carta, Casa destino) {
    adicionar(lista, u, carta, destino, nenhuma, nenhuma);
}

static void gerarPeao(const Combate *j, int u, ListaAcoes *lista) {
    Casa p = posicao(j, u);
    int passo = j->unidades[u].lado ? 1 : -1;
    Casa frente = {p.l + passo, p.c};
    Casa longe = {p.l + 2 * passo, p.c};
    int naFrente = ocupante(j, frente);
    int companhia = aliado(j, u, ocupante(j, (Casa){p.l, p.c - 1}))
                 || aliado(j, u, ocupante(j, (Casa){p.l, p.c + 1}));

    if (temCarta(j, u, 1) && companhia && capturaDisponivel(j, u, frente))
        adicionarDestino(lista, u, 1, frente);
    if (temCarta(j, u, 3)) {
        Casa tras = {p.l - passo, p.c};
        if (livre(j, u, tras)) adicionarDestino(lista, u, 3, tras);
    }
    if (livre(j, u, longe)) {
        if (temCarta(j, u, 4) && vazio(j, frente))
            adicionarDestino(lista, u, 4, longe);
        if (temCarta(j, u, 5) && naFrente >= 0)
            adicionarDestino(lista, u, 5, longe);
    }
    for (int dc = -1; dc <= 1; dc += 2) {
        Casa lado = {p.l, p.c + dc};
        Casa flanco = {p.l + passo, p.c + dc};
        Casa diagonalTras = {p.l - passo, p.c + dc};
        if (temCarta(j, u, 2) && livre(j, u, lado))
            adicionarDestino(lista, u, 2, lado);
        if (temCarta(j, u, 11) && naFrente >= 0 && livre(j, u, flanco))
            adicionarDestino(lista, u, 11, flanco);
        if (temCarta(j, u, 16) && j->unidades[u].capturou
                && capturaDisponivel(j, u, diagonalTras))
            adicionarDestino(lista, u, 16, diagonalTras);
    }
}

/* Os saltos opcionais podem acompanhar tanto o L normal como C01/C02. */
static int formatoSalto(const Combate *j, int u, Casa origem, Casa destino) {
    int dl = abs(destino.l - origem.l), dc = abs(destino.c - origem.c);
    if (movimentoCavalo(origem.l, origem.c, destino.l, destino.c)) return 1;
    if (temCarta(j, u, 1) && ((dl == 3 && dc == 1) || (dl == 1 && dc == 3))) return 1;
    return temCarta(j, u, 2) && ((dl == 2 && dc == 0) || (dl == 0 && dc == 2));
}

static int saltoVazio(const Combate *j, int u, Casa origem, Casa destino) {
    return livre(j, u, destino) && formatoSalto(j, u, origem, destino);
}

static int canto(const Combate *j, Casa p) {
    return (p.l == 0 || p.l == 4) && (p.c == 0 || p.c == j->largura - 1);
}

static void gerarResgate(const Combate *j, int u, int v, Casa p, Casa destino, ListaAcoes *lista) {
    if (!cartaDisponivel(j, u, 7) || !ameacado(j, v)) return;
    adicionarDestino(lista, u, 7, destino);
    for (int dl = -1; dl <= 1; dl++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (!dl && !dc) continue;
            Casa extra = {p.l + dl, p.c + dc};
            if (livre(j, v, extra)) adicionar(lista, u, 7, destino, extra, nenhuma);
        }
    }
}

static int haInimigoEmpurravel(const Combate *j, int u, Casa destino) {
    for (int i = 0; i < 4; i++) {
        Casa vizinho = somar(destino, ortogonais[i]);
        Casa apos = somar(vizinho, ortogonais[i]);
        int v = ocupante(j, vizinho);
        if (inimigo(j, u, v) && (livre(j, v, apos)
                || (igualCasa(apos, posicao(j, u))
                    && j->selada[apos.l][apos.c] != j->unidades[v].lado + 1)))
            return 1;
    }
    return 0;
}

static void gerarMontaria(const Combate *j, int u, Casa p, Casa destino, ListaAcoes *lista) {
    if (!temCarta(j, u, 19)) return;
    int passo = j->unidades[u].lado ? 1 : -1;
    Casa pousoPeao = {destino.l - passo, destino.c};
    if (!dentro(j, pousoPeao)) return;
    for (int i = 0; i < 4; i++) {
        Casa vizinho = somar(p, ortogonais[i]);
        int v = ocupante(j, vizinho);
        if (!aliado(j, u, v) || j->unidades[v].tipo != 'P'
                || j->selada[pousoPeao.l][pousoPeao.c] == j->unidades[v].lado + 1)
            continue;
        if (vazio(j, pousoPeao) || igualCasa(pousoPeao, p) || igualCasa(pousoPeao, vizinho))
            adicionar(lista, u, 19, destino, vizinho, nenhuma);
    }
}

static void gerarExtrasSalto(const Combate *j, int u, Casa p, Casa destino, ListaAcoes *lista) {
    if (temCarta(j, u, 8) && haInimigoEmpurravel(j, u, destino))
        adicionarDestino(lista, u, 8, destino);
    if (temCarta(j, u, 10)) {
        for (int v = 0; v < j->quantidade; v++) {
            if (v == u || !aliado(j, u, v) || j->unidades[v].tipo != 'C') continue;
            Casa outro = posicao(j, v);
            if (formatoSalto(j, v, outro, p) && j->selada[p.l][p.c] != j->unidades[v].lado + 1)
                adicionar(lista, u, 10, destino, outro, nenhuma);
        }
    }
    gerarMontaria(j, u, p, destino, lista);
}

static void gerarCavalo(const Combate *j, int u, ListaAcoes *lista) {
    Casa p = posicao(j, u);
    for (int l = 0; l < 5; l++) {
        for (int c = 0; c < j->largura; c++) {
            Casa destino = {l, c};
            if (igualCasa(destino, p)) continue;
            int dl = abs(l - p.l), dc = abs(c - p.c);
            int v = ocupante(j, destino);
            int saltoNormal = movimentoCavalo(p.l, p.c, l, c);
            int saltoLivre = saltoVazio(j, u, p, destino);
            int alvo = saltoNormal && capturaDisponivel(j, u, destino);

            if (livre(j, u, destino)) {
                if (temCarta(j, u, 1) && ((dl == 3 && dc == 1) || (dl == 1 && dc == 3)))
                    adicionarDestino(lista, u, 1, destino);
                if (temCarta(j, u, 2) && ((dl == 2 && dc == 0) || (dl == 0 && dc == 2)))
                    adicionarDestino(lista, u, 2, destino);
                if (temCarta(j, u, 3) && dl + dc == 1)
                    adicionarDestino(lista, u, 3, destino);
            }
            if (alvo && cartaDisponivel(j, u, 5)
                    && j->selada[p.l][p.c] != j->unidades[u].lado + 1)
                adicionarDestino(lista, u, 5, destino);
            if (saltoNormal && trocaDisponivel(j, u, v, p, destino)) {
                if (temCarta(j, u, 6)) adicionarDestino(lista, u, 6, destino);
                gerarResgate(j, u, v, p, destino, lista);
            }
            if (saltoLivre) gerarExtrasSalto(j, u, p, destino, lista);
            if ((saltoLivre || alvo) && canto(j, destino) && temCarta(j, u, 16)) {
                for (int i = 0; i < 4; i++) {
                    Casa extra = somar(destino, ortogonais[i]);
                    if (livre(j, u, extra) || (igualCasa(extra, p)
                        && j->selada[p.l][p.c] != j->unidades[u].lado + 1))
                        adicionar(lista, u, 16, destino, extra, nenhuma);
                }
            }
            /* Montaria também acompanha saltos que capturam. */
            if (alvo) gerarMontaria(j, u, p, destino, lista);
        }
    }
}

void gerarPC(const Combate *j, int u, ListaAcoes *lista) {
    if (j->unidades[u].tipo == 'P') gerarPeao(j, u, lista);
    if (j->unidades[u].tipo == 'C') gerarCavalo(j, u, lista);
}

static void aplicarCoice(Combate *j, int u, Casa destino) {
    for (int i = 0; i < 4; i++) {
        Casa vizinho = somar(destino, ortogonais[i]);
        Casa apos = somar(vizinho, ortogonais[i]);
        int v = ocupante(j, vizinho);
        if (inimigo(j, u, v) && livre(j, v, apos)) deslocar(j, v, apos);
    }
}

static int executarCavalo(Combate *j, Acao a) {
    int u = a.ator;
    Casa origem = posicao(j, u);
    int v = ocupante(j, a.destino);

    switch (a.carta) {
        case 5:
            if (!capturar(j, u, v)) return 0;
            gastarCarta(j, u, 5);
            if (j->unidades[u].vivo && igualCasa(posicao(j, u), a.destino))
                deslocar(j, u, origem);
            return 1;
        case 6:
            return trocar(j, u, v);
        case 7:
            if (!trocar(j, u, v)) return 0;
            gastarCarta(j, u, 7);
            if (dentro(j, a.extra)) deslocar(j, v, a.extra);
            return 1;
        case 8:
            if (!deslocar(j, u, a.destino)) return 0;
            aplicarCoice(j, u, a.destino);
            return 1;
        case 10: {
            int outro = ocupante(j, a.extra);
            if (!deslocar(j, u, a.destino)) return 0;
            deslocar(j, outro, origem);
            return 1;
        }
        case 16:
            if (!moverOuCapturar(j, u, a.destino)) return 0;
            if (j->unidades[u].vivo && igualCasa(posicao(j, u), a.destino))
                deslocar(j, u, a.extra);
            return 1;
        case 19: {
            int peao = ocupante(j, a.extra);
            int passo = j->unidades[u].lado ? 1 : -1;
            Casa pousoPeao = {a.destino.l - passo, a.destino.c};
            if (!moverOuCapturar(j, u, a.destino)) return 0;
            if (j->unidades[u].vivo && igualCasa(posicao(j, u), a.destino)
                    && j->unidades[peao].vivo) {
                if (!igualCasa(posicao(j, peao), pousoPeao))
                    deslocar(j, peao, pousoPeao);
            }
            return 1;
        }
        case 1: case 2: case 3:
            return deslocar(j, u, a.destino);
        default:
            return 0;
    }
}

int executarPC(Combate *j, Acao a) {
    if (j->unidades[a.ator].tipo == 'P') {
        switch (a.carta) {
            case 1: case 16:
                return capturar(j, a.ator, ocupante(j, a.destino));
            case 2: case 3: case 4: case 5: case 11:
                return deslocar(j, a.ator, a.destino);
            default:
                return 0;
        }
    }
    if (j->unidades[a.ator].tipo == 'C') return executarCavalo(j, a);
    return 0;
}

void depoisPC(Combate *j, int u, Casa origem, int capturou) {
    (void)capturou;
    if (!j->unidades[u].vivo || j->unidades[u].tipo != 'P') return;
    Casa p = posicao(j, u);
    int ultima = j->unidades[u].lado ? 4 : 0;
    if (p.l != ultima || igualCasa(origem, p)) return;
    if (temCarta(j, u, 20) &&
        (j->unidades[u].lado ? !usouCarta(j, u, 20) : !j->coroacaoUsada)) {
        if (!j->unidades[u].lado) j->coroacaoUsada = 1;
        gastarCarta(j, u, 20);
        promover(j, u, 'D', 1);
    } else {
        j->unidades[u].promocaoPendente = 1;
    }
}

int antesPC(Combate *j, int u, int v) {
    (void)j; (void)u; (void)v;
    return 0;
}

void aposCapturaPC(Combate *j, int atacante, int vitima, Casa destino) {
    if (!j->unidades[atacante].vivo || !igualCasa(posicao(j, atacante), destino)) return;
    for (int dc = -1; dc <= 1; dc += 2) {
        int peao = ocupante(j, (Casa){destino.l, destino.c + dc});
        if (peao < 0 || j->unidades[peao].lado != j->unidades[vitima].lado
                || j->unidades[peao].tipo != 'P' || !cartaDisponivel(j, peao, 12)
                || !capturaDisponivel(j, peao, destino)) continue;
        gastarCarta(j, peao, 12);
        Casa origem = posicao(j, peao);
        if (capturar(j, peao, atacante)) depoisPC(j, peao, origem, 1);
        return;
    }
}
