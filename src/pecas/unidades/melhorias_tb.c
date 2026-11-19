#include "../../campo/combate.h"

#include <stdlib.h>

static const Casa SEM_CASA = {-1, -1};
static const Casa RETAS[] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
static const Casa DIAGONAIS[] = {{-1, -1}, {-1, 1}, {1, 1}, {1, -1}};

static Casa somar(Casa a, Casa b) {
    return (Casa){a.l + b.l, a.c + b.c};
}

static int sinal(int n) {
    return (n > 0) - (n < 0);
}

static Casa direcao(Casa a, Casa b) {
    return (Casa){sinal(b.l - a.l), sinal(b.c - a.c)};
}

static int distancia(Casa a, Casa b) {
    int l = abs(b.l - a.l), c = abs(b.c - a.c);
    return l > c ? l : c;
}

static int borda(const Combate *j, Casa p) {
    return p.l == 0 || p.l == 4 || p.c == 0 || p.c == j->largura - 1;
}

static int canto(const Combate *j, Casa p) {
    return (p.l == 0 || p.l == 4) &&
           (p.c == 0 || p.c == j->largura - 1);
}

static int pousoPermitido(const Combate *j, int u, Casa p) {
    return dentro(j, p) && j->selada[p.l][p.c] != j->unidades[u].lado + 1;
}

static int destinoLivre(const Combate *j, int u, Casa p) {
    return pousoPermitido(j, u, p) && vazio(j, p);
}

static int disponivel(const Combate *j, int u, int id) {
    return temCarta(j, u, id) && !usouCarta(j, u, id);
}

/* Caminho do acompanhante: a casa de origem da torre que iniciou o
 * movimento fica livre simultaneamente. */
static int caminhoComboio(const Combate *j, Casa a, Casa b, Casa liberada) {
    Casa passo = direcao(a, b);
    for (Casa p = somar(a, passo); ; p = somar(p, passo)) {
        if (!dentro(j, p) || (!vazio(j, p) && !igualCasa(p, liberada)))
            return 0;
        if (igualCasa(p, b)) return 1;
    }
}

static int visivelOrtogonal(const Combate *j, Casa a, Casa b) {
    return ortogonal(a, b) && !igualCasa(a, b) && caminho(j, a, b);
}

static int visivelDiagonal(const Combate *j, Casa a, Casa b) {
    return diagonal(a, b) && !igualCasa(a, b) && caminho(j, a, b);
}

static int pousoOuCaptura(const Combate *j, int u, Casa p) {
    if (!pousoPermitido(j, u, p)) return 0;
    int v = ocupante(j, p);
    return v < 0 || podeCapturar(j, u, v);
}

/* Reflexo e Curva fechada usam a casa intermediária apenas para mostrar o
 * trajeto validado. O efeito é idêntico para qualquer caminho até o mesmo
 * destino, portanto basta oferecer o primeiro caminho livre encontrado. */
static void adicionarTrajeto(ListaAcoes *lista, int u, int carta, Casa destino,
                            Casa intermediaria) {
    for (int i = 0; i < lista->n; i++) {
        Acao a = lista->itens[i];
        if (a.ator == u && a.carta == carta && igualCasa(a.destino, destino))
            return;
    }
    adicionar(lista, u, carta, destino, intermediaria, SEM_CASA);
}

static void gerarTorre(const Combate *j, int u, ListaAcoes *lista) {
    Casa origem = posicao(j, u);
    /* Guincho e roque selecionam diretamente a unidade aliada. */
    for (int v = 0; v < j->quantidade; v++) {
        if (v == u || !aliado(j, u, v)) continue;
        Casa alvo = posicao(j, v);
        if (visivelOrtogonal(j, origem, alvo)) {
            Casa perto = somar(origem, direcao(origem, alvo));
            if (temCarta(j, u, 4) && destinoLivre(j, v, perto))
                adicionar(lista, u, 4, alvo, perto, SEM_CASA);
            if (temCarta(j, u, 8) && j->unidades[v].tipo == 'R' &&
                pousoPermitido(j, u, alvo) && pousoPermitido(j, v, origem))
                adicionar(lista, u, 8, alvo, SEM_CASA, SEM_CASA);
        }
        if (disponivel(j, u, 5) && ortogonal(origem, alvo) &&
            distancia(origem, alvo) == 1) {
            Casa passo = direcao(origem, alvo);
            Casa destino = alvo;
            for (int d = 1; d <= 3; d++) {
                destino = somar(destino, passo);
                if (!dentro(j, destino)) break;
                if (destinoLivre(j, v, destino))
                    adicionar(lista, u, 5, alvo, destino, SEM_CASA);
            }
        }
    }
    for (int l = 0; l < 5; l++) {
        for (int c = 0; c < j->largura; c++) {
            Casa destino = {l, c};
            if (igualCasa(origem, destino)) continue;
            if (temCarta(j, u, 20) && contarLado(j, j->unidades[u].lado) == 1 &&
                diagonal(origem, destino) && distancia(origem, destino) == 1 &&
                pousoOuCaptura(j, u, destino))
                adicionar(lista, u, 20, destino, SEM_CASA, SEM_CASA);
            if (!visivelOrtogonal(j, origem, destino) ||
                !pousoOuCaptura(j, u, destino)) continue;
            Casa passo = direcao(origem, destino);
            int v = ocupante(j, destino);
            if (v >= 0) {
                if (disponivel(j, u, 7)) {
                    /* Toda casa atravessada (inclusive a origem) é uma
                     * alternativa de retirada. */
                    for (Casa recuo = origem; !igualCasa(recuo, destino);
                         recuo = somar(recuo, passo)) {
                        if (pousoPermitido(j, u, recuo))
                            adicionar(lista, u, 7, destino, recuo, SEM_CASA);
                    }
                }
                continue;
            }
            if (temCarta(j, u, 9) && borda(j, destino)) {
                for (int d = 0; d < 4; d++) {
                    Casa final = somar(destino, RETAS[d]);
                    /* Compartilhar a mesma borda evita um passo para dentro
                     * da área central ou pelo lado errado de um canto. */
                    int mesmaBorda =
                        (destino.l == final.l && (final.l == 0 || final.l == 4)) ||
                        (destino.c == final.c && (final.c == 0 || final.c == j->largura - 1));
                    if (mesmaBorda && pousoPermitido(j, u, final) &&
                        (vazio(j, final) || igualCasa(final, origem)))
                        adicionar(lista, u, 9, destino, final, SEM_CASA);
                }
            }
            if (temCarta(j, u, 14)) {
                Casa atras = {origem.l - passo.l, origem.c - passo.c};
                Casa final = {destino.l - passo.l, destino.c - passo.c};
                int acompanhante = ocupante(j, atras);
                if (aliado(j, u, acompanhante) && pousoPermitido(j, acompanhante, final))
                    adicionar(lista, u, 14, destino, atras, final);
            }
            if (temCarta(j, u, 18)) {
                for (int lado = -1; lado <= 1; lado += 2) {
                    Casa vizinha = {origem.l, origem.c + lado};
                    int acompanhante = ocupante(j, vizinha);
                    Casa final = {destino.l, destino.c + lado};
                    if (aliado(j, u, acompanhante) &&
                        j->unidades[acompanhante].tipo == 'T' &&
                        pousoPermitido(j, acompanhante, final) &&
                        caminhoComboio(j, vizinha, final, origem))
                        adicionar(lista, u, 18, destino, vizinha, final);
                }
            }
        }
    }
    /* Curva de 90 graus: dois segmentos ortogonais de pelo menos um passo,
     * totalizando no máximo três passos. */
    if (temCarta(j, u, 16)) {
        for (int d = 0; d < 4; d++) {
            Casa curva = origem;
            for (int a = 1; a <= 2; a++) {
                curva = somar(curva, RETAS[d]);
                if (!vazio(j, curva)) break;
                for (int rotacao = -1; rotacao <= 1; rotacao += 2) {
                    Casa final = curva;
                    Casa passo = RETAS[(d + rotacao + 4) % 4];
                    for (int b = 1; b <= 3 - a; b++) {
                        final = somar(final, passo);
                        if (!vazio(j, final)) break;
                        if (pousoPermitido(j, u, final))
                            adicionarTrajeto(lista, u, 16, final, curva);
                    }
                }
            }
        }
    }
}

static void gerarReflexo(const Combate *j, int u, ListaAcoes *lista, Casa origem) {
    for (int d = 0; d < 4; d++) {
        Casa passo = DIAGONAIS[d], margem = somar(origem, passo);
        while (dentro(j, margem) && vazio(j, margem) && !borda(j, margem))
            margem = somar(margem, passo);
        if (!dentro(j, margem) || !vazio(j, margem) ||
            !borda(j, margem) || canto(j, margem)) continue;
        /* A reflexão inverte somente o componente que tocou a borda. */
        if (margem.l == 0 || margem.l == 4) passo.l *= -1;
        else passo.c *= -1;
        for (Casa final = somar(margem, passo); dentro(j, final); final = somar(final, passo)) {
            if (pousoOuCaptura(j, u, final))
                adicionarTrajeto(lista, u, 2, final, margem);
            if (!vazio(j, final)) break;
        }
    }
}

static void gerarBispo(const Combate *j, int u, ListaAcoes *lista) {
    Casa origem = posicao(j, u);
    if (temCarta(j, u, 1)) {
        for (int d = 0; d < 4; d++) {
            Casa destino = somar(origem, RETAS[d]);
            if (destinoLivre(j, u, destino))
                adicionar(lista, u, 1, destino, SEM_CASA, SEM_CASA);
        }
    }
    if (temCarta(j, u, 2)) gerarReflexo(j, u, lista, origem);

    for (int l = 0; l < 5; l++) {
        for (int c = 0; c < j->largura; c++) {
            Casa destino = {l, c};
            if (igualCasa(origem, destino) || !diagonal(origem, destino))
                continue;
            int v = ocupante(j, destino);
            int visivel = caminho(j, origem, destino);
            if (!visivel) {
                if (temCarta(j, u, 6) && destinoLivre(j, u, destino)) {
                    for (int t = 0; t < j->quantidade; t++) {
                        if (aliado(j, u, t) && j->unidades[t].tipo == 'T' &&
                            visivelOrtogonal(j, posicao(j, t), destino)) {
                            adicionar(lista, u, 6, destino, SEM_CASA, SEM_CASA);
                            break;
                        }
                    }
                }
                continue;
            }
            if (aliado(j, u, v)) {
                if (temCarta(j, u, 4) && pousoPermitido(j, u, destino) &&
                    pousoPermitido(j, v, origem))
                    adicionar(lista, u, 4, destino, SEM_CASA, SEM_CASA);
                if (disponivel(j, u, 19) && canto(j, origem) && j->unidades[v].tipo != 'R')
                    adicionar(lista, u, 19, destino, SEM_CASA, SEM_CASA);
                continue;
            }
            if (inimigo(j, u, v) && temCarta(j, u, 11) && !j->unidades[v].chefe)
                adicionar(lista, u, 11, destino, SEM_CASA, SEM_CASA);
            if (v < 0 && temCarta(j, u, 13))
                adicionar(lista, u, 13, destino, SEM_CASA, SEM_CASA);
            if (!pousoOuCaptura(j, u, destino)) continue;
            if (v >= 0 && temCarta(j, u, 7)) {
                Casa passo = direcao(destino, origem);
                Casa recuo = somar(destino, passo);
                if (pousoPermitido(j, u, recuo))
                    adicionar(lista, u, 7, destino, recuo, SEM_CASA);
            }
            if (disponivel(j, u, 5) && canto(j, destino)) {
                for (int linha = 0; linha <= 4; linha += 4) {
                    for (int coluna = 0; coluna < j->largura; coluna += j->largura - 1) {
                        Casa final = {linha, coluna};
                        if (!igualCasa(final, destino) && pousoPermitido(j, u, final) &&
                            (vazio(j, final) || igualCasa(final, origem)))
                            adicionar(lista, u, 5, destino, final, SEM_CASA);
                    }
                }
            }
        }
    }
}

void gerarTB(const Combate *j, int u, ListaAcoes *lista) {
    if (j->unidades[u].tipo == 'T') gerarTorre(j, u, lista);
    if (j->unidades[u].tipo == 'B') gerarBispo(j, u, lista);
}

static int executarRetirada(Combate *j, Acao a, int limitada) {
    int resultado = moverOuCapturar(j, a.ator, a.destino);
    if (resultado && igualCasa(posicao(j, a.ator), a.destino)) {
        if (deslocar(j, a.ator, a.extra) && limitada)
            gastarCarta(j, a.ator, a.carta);
    }
    return resultado;
}

static int executarTorre(Combate *j, Acao a) {
    int u = a.ator;
    int v = ocupante(j, a.destino);
    switch (a.carta) {
        case 4:
            return deslocar(j, v, a.extra);
        case 5:
            if (!deslocar(j, v, a.extra)) return 0;
            gastarCarta(j, u, 5);
            return 1;
        case 7:
            return executarRetirada(j, a, 1);
        case 8:
            return trocar(j, u, v);
        case 9:
            return executarRetirada(j, a, 0);
        case 14:
        case 18: {
            int acompanhante = ocupante(j, a.extra);
            if (!deslocar(j, u, a.destino)) return 0;
            return deslocar(j, acompanhante, a.extra2);
        }
        case 16:
        case 20:
            return moverOuCapturar(j, u, a.destino);
        default:
            return 0;
    }
}

static int executarBispo(Combate *j, Acao a) {
    int u = a.ator;
    int v = ocupante(j, a.destino);
    switch (a.carta) {
        case 1:
        case 6:
            return deslocar(j, u, a.destino);
        case 2:
            return moverOuCapturar(j, u, a.destino);
        case 4:
            return trocar(j, u, v);
        case 5:
            return executarRetirada(j, a, 1);
        case 7:
            return executarRetirada(j, a, 0);
        case 11:
            j->unidades[v].silencio = 1;
            return 1;
        case 13:
            j->selada[a.destino.l][a.destino.c] = 2 - j->unidades[u].lado;
            return 1;
        case 19:
            tocar(j, v);
            gastarCarta(j, u, 19);
            return 1;
        default:
            return 0;
    }
}

int executarTB(Combate *j, Acao a) {
    if (j->unidades[a.ator].tipo == 'T') return executarTorre(j, a);
    if (j->unidades[a.ator].tipo == 'B') return executarBispo(j, a);
    return 0;
}

int antesTB(Combate *j, int atacante, int vitima) {
    Unidade *alvo = &j->unidades[vitima];
    Casa casa = posicao(j, vitima);
    if (alvo->tipo == 'T' && igualCasa(casa, alvo->origem) && disponivel(j, vitima, 15)) {
        gastarCarta(j, vitima, 15);
        return 1;
    }
    for (int b = 0; b < j->quantidade; b++) {
        if (b == vitima || !aliado(j, vitima, b) ||
            j->unidades[b].tipo != 'B' || !disponivel(j, b, 16) ||
            !visivelDiagonal(j, posicao(j, b), casa) ||
            !podeCapturar(j, atacante, b)) continue;
        gastarCarta(j, b, 16);
        remover(j, b);
        return 1;
    }
    return 0;
}

void depoisTB(Combate *j, int u, Casa origem, int capturou) {
    /* As ações opcionais de torre e bispo são escolhidas antes da execução. */
    (void)j;
    (void)u;
    (void)origem;
    (void)capturou;
}

void aposCapturaTB(Combate *j, int atacante, int vitima, Casa casa) {
    (void)j;
    (void)atacante;
    (void)vitima;
    (void)casa;
}
