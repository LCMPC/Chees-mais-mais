#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/campo/combate.h"

static const Casa ausente = {-1, -1};

static void iniciarCenario(Combate *j, int largura) {
    memset(j, 0, sizeof *j);
    j->largura = largura;
    j->rodada = 1;
    j->bonus = -1;
}

static int unidade(Combate *j, char tipo, int lado, int l, int c) {
    int n = j->quantidade++;
    assert(n < 40 && dentro(j, (Casa){l, c}) && vazio(j, (Casa){l, c}));
    j->unidades[n] = (Unidade){
        .tipo = tipo,
        .original = tipo,
        .lado = lado,
        .origem = {l, c},
        .vivo = 1,
        .proximo = j->primeira[lado],
        .corredor = {-1, -1}
    };
    j->tab[l][c] = n + 1;
    j->primeira[lado] = n + 1;
    return n;
}

static void darCarta(Combate *j, char tipo, int id) {
    assert(buscarCarta(tipo, id));
    j->adquiridas[indiceTipo(tipo)] |= 1u << id;
}

static void verificarTabuleiro(const Combate *j) {
    int contagem[40] = {0};
    for (int l = 0; l < 5; ++l) {
        for (int c = 0; c < j->largura; ++c) {
            int u = ocupante(j, (Casa){l, c});
            if (u < 0) continue;
            assert(u < j->quantidade && j->unidades[u].vivo);
            assert(igualCasa(posicao(j, u), (Casa){l, c}));
            ++contagem[u];
        }
    }
    for (int u = 0; u < j->quantidade; ++u) {
        assert(contagem[u] == !!j->unidades[u].vivo);
    }
    int nos[40] = {0};
    for (int lado = 0; lado < 2; ++lado) {
        for (int no = j->primeira[lado]; no; no = j->unidades[no - 1].proximo) {
            assert(no > 0 && no <= j->quantidade);
            int u = no - 1;
            assert(!nos[u] && j->unidades[u].vivo && j->unidades[u].lado == lado);
            nos[u] = 1;
        }
    }
    for (int u = 0; u < j->quantidade; ++u) assert(nos[u] == contagem[u]);
    int registros[40] = {0};
    int quantidade = 0;
    for (int no = j->primeiraCaptura; no; no = j->historicoRodada[no - 1].proximo) {
        assert(no > 0 && no <= j->quantidadeCapturas && !registros[no - 1]);
        registros[no - 1] = 1;
        const RegistroCaptura *r = &j->historicoRodada[no - 1];
        assert(r->lado == 0 || r->lado == 1);
        assert(r->rodada == j->rodada);
        assert(dentro(j, r->casa) && indiceTipo(r->tipo) >= 0);
        quantidade++;
    }
    assert(quantidade == j->quantidadeCapturas);
}

static int encontrar(const Combate *j, int ator, int id, Casa destino,
                     Acao *escolhida) {
    ListaAcoes acoes = {0};
    gerarAcoes(j, ator, &acoes);
    assert(acoes.n >= 0 && acoes.n <= MAX_ACOES);
    for (int i = 0; i < acoes.n; ++i) {
        Acao a = acoes.itens[i];
        if (a.carta == id && igualCasa(a.destino, destino)) {
            assert(a.ator == ator);
            if (escolhida) *escolhida = a;
            return 1;
        }
    }
    return 0;
}

static void agir(Combate *j, int ator, int id, Casa destino) {
    Acao a;
    assert(encontrar(j, ator, id, destino, &a));
    assert(executarAcao(j, a));
    verificarTabuleiro(j);
}

static void agirComExtra(Combate *j, int ator, int id, Casa destino, Casa extra,
                         Casa extra2) {
    ListaAcoes acoes;
    gerarAcoes(j, ator, &acoes);
    for (int i = 0; i < acoes.n; ++i) {
        Acao a = acoes.itens[i];
        if (a.carta == id && igualCasa(a.destino, destino) &&
            igualCasa(a.extra, extra) && igualCasa(a.extra2, extra2)) {
            assert(executarAcao(j, a));
            verificarTabuleiro(j);
            return;
        }
    }
    assert(!"Acao com destino adicional ausente");
}

static void confirmarPosicao(Combate *j, int u, int l, int c) {
    assert(igualCasa(posicao(j, u), (Casa){l, c}));
}

static void testarCatalogo(void) {
    const char tipos[] = "PTBCRD";
    const int ids[][10] = {
        {1, 2, 3, 4, 5, 11, 12, 16, 20, 21},
        {4, 5, 7, 8, 9, 14, 15, 16, 18, 20},
        {1, 2, 4, 5, 6, 7, 11, 13, 16, 19},
        {1, 2, 3, 5, 6, 7, 8, 10, 16, 19},
        {1, 2, 5, 7, 11, 13, 15, 17, 19, 20},
        {4, 6, 7, 9, 11, 13, 16, 17, 19, 20}
    };
    int vistos[TIPOS_PECA] = {0};
    for (int t = 0; t < TIPOS_PECA; ++t) {
        int indice = indiceTipo(tipos[t]);
        assert(indice >= 0 && indice < TIPOS_PECA && !vistos[indice]);
        vistos[indice] = 1;
        for (int i = 0; i < CARTAS_TIPO; ++i) {
            const Carta *carta = buscarCarta(tipos[t], ids[t][i]);
            assert(carta && carta->tipo == tipos[t] && carta->id == ids[t][i]);
            assert(carta->nome && *carta->nome);
            assert(carta->descricao && *carta->descricao);
        }
        for (int i = 0; i < CARTAS_TIPO; ++i) {
            const Carta *carta = &catalogo[indice][i];
            assert(carta->tipo == tipos[t]);
            for (int k = 0; k < i; ++k) {
                assert(catalogo[indice][k].id != carta->id);
            }
        }
    }
    assert(!buscarCarta('P', 19));
    assert(!buscarCarta('X', 1));
}

static void testarCompraESorteio(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int p1 = unidade(&j, 'P', 0, 4, 0);
    int p2 = unidade(&j, 'P', 0, 4, 1);
    int inimigoP = unidade(&j, 'P', 1, 0, 1);
    int inimigoP2 = unidade(&j, 'P', 1, 0, 2);
    const Carta *opcoes[60];
    int n = sortearCartas(&j, opcoes, 60);
    assert(n == 10);
    for (int i = 0; i < n; ++i) {
        assert(opcoes[i]->tipo == 'P');
        for (int k = 0; k < i; ++k) {
            assert(opcoes[k]->id != opcoes[i]->id);
        }
    }
    assert(adquirirCarta(&j, buscarCarta('P', 12)));
    assert(!adquirirCarta(&j, buscarCarta('P', 12)));
    assert(temCarta(&j, p1, 12) && temCarta(&j, p2, 12));
    assert(!temCarta(&j, inimigoP, 12));
    gastarCarta(&j, p1, 12);
    assert(usouCarta(&j, p2, 12));
    assert(!usouCarta(&j, inimigoP, 12));
    j.unidades[inimigoP].cartas = 1u << 12;
    assert(temCarta(&j, inimigoP, 12));
    assert(!temCarta(&j, inimigoP2, 12));
    n = sortearCartas(&j, opcoes, 60);
    assert(n == 9);
    for (int i = 0; i < n; ++i) assert(opcoes[i]->id != 12);
    assert(!adquirirCarta(&j, buscarCarta('D', 9)));
}

static void testarSorteioInimigo(void) {
    PecaInimiga *inimigos = NULL;
    /* Na fase 1 existem apenas peões, cada um recebe no máximo nove
     * melhorias: a carta de aquisição imediata P21 é exclusiva do jogador. */
    assert(criarRodadaInimiga(&inimigos, 1, 100, 3) == 27);
    for (PecaInimiga *p = inimigos; p; p = p->proximo) {
        assert(p->inicial == 'P' && p->quantidadeMelhorias == 9);
        unsigned vistos = 0;
        for (int i = 0; i < p->quantidadeMelhorias; ++i) {
            int indice = p->melhorias[i] - 1;
            assert(indice >= 0 && indice < CARTAS_TIPO);
            int id = catalogo[indiceTipo('P')][indice].id;
            assert(id != 21 && !(vistos & (1u << id)));
            vistos |= 1u << id;
        }
    }
    liberarPecasInimigas(&inimigos);
    assert(!inimigos);
}

static void testarPeao(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int p = unidade(&j, 'P', 0, 3, 2);
    unidade(&j, 'P', 0, 3, 1);
    int alvo = unidade(&j, 'P', 1, 2, 2);
    assert(!encontrar(&j, p, 0, (Casa){2, 2}, NULL));
    darCarta(&j, 'P', 1);
    agir(&j, p, 1, (Casa){2, 2});
    assert(!j.unidades[alvo].vivo && j.unidades[p].capturou);
    assert(j.lado == 1);

    iniciarCenario(&j, 5);
    p = unidade(&j, 'P', 0, 4, 1);
    j.unidades[p].moveu = 1;
    assert(!encontrar(&j, p, 0, (Casa){2, 1}, NULL));
    darCarta(&j, 'P', 4);
    agir(&j, p, 4, (Casa){2, 1});
    assert(igualCasa(posicao(&j, p), (Casa){2, 1}));

    iniciarCenario(&j, 5);
    p = unidade(&j, 'P', 0, 3, 2);
    darCarta(&j, 'P', 2);
    agir(&j, p, 2, (Casa){3, 1});
    confirmarPosicao(&j, p, 3, 1);
    j.lado = 0;
    darCarta(&j, 'P', 3);
    agir(&j, p, 3, (Casa){4, 1});
    confirmarPosicao(&j, p, 4, 1);

    iniciarCenario(&j, 5);
    p = unidade(&j, 'P', 0, 3, 2);
    alvo = unidade(&j, 'P', 1, 2, 2);
    darCarta(&j, 'P', 5);
    agir(&j, p, 5, (Casa){1, 2});
    confirmarPosicao(&j, p, 1, 2);
    assert(j.unidades[alvo].vivo);

    iniciarCenario(&j, 5);
    p = unidade(&j, 'P', 0, 3, 2);
    alvo = unidade(&j, 'P', 1, 2, 2);
    darCarta(&j, 'P', 11);
    agir(&j, p, 11, (Casa){2, 1});
    confirmarPosicao(&j, p, 2, 1);
    assert(j.unidades[alvo].vivo);

    iniciarCenario(&j, 5);
    p = unidade(&j, 'P', 0, 3, 2);
    alvo = unidade(&j, 'P', 1, 4, 3);
    darCarta(&j, 'P', 16);
    assert(!encontrar(&j, p, 16, (Casa){4, 3}, NULL));
    j.unidades[p].capturou = 1;
    agir(&j, p, 16, (Casa){4, 3});
    assert(!j.unidades[alvo].vivo);
}

static void testarPromocoes(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int p1 = unidade(&j, 'P', 0, 3, 0);
    int p2 = unidade(&j, 'P', 0, 4, 0);
    assert(adquirirCarta(&j, buscarCarta('P', 21)));
    assert((j.unidades[p1].tipo == 'D') != (j.unidades[p2].tipo == 'D'));
    assert(igualCasa(posicao(&j, p1), (Casa){3, 0}));
    assert(igualCasa(posicao(&j, p2), (Casa){4, 0}));
    int rainha = j.unidades[p1].tipo == 'D' ? p1 : p2;
    assert(j.unidades[rainha].original == 'D');
    assert(!adquirirCarta(&j, buscarCarta('P', 21)));
    const Carta *opcoes[60];
    int n = sortearCartas(&j, opcoes, 60);
    int d = 0;
    for (int i = 0; i < n; ++i) d += opcoes[i]->tipo == 'D';
    assert(d == 10);
    verificarTabuleiro(&j);

    iniciarCenario(&j, 5);
    p1 = unidade(&j, 'P', 0, 1, 0);
    p2 = unidade(&j, 'P', 0, 1, 1);
    unidade(&j, 'P', 1, 2, 4);
    assert(adquirirCarta(&j, buscarCarta('P', 20)));
    agir(&j, p1, 0, (Casa){0, 0});
    assert(j.unidades[p1].tipo == 'D' && j.unidades[p1].original == 'D');
    j.lado = 0;
    agir(&j, p2, 0, (Casa){0, 1});
    assert(j.unidades[p2].tipo == 'P');
    assert(promocaoPendente(&j) == p2);
    assert(escolherPromocao(&j, p2, 'B'));
    assert(j.unidades[p2].tipo == 'B' && j.unidades[p2].original == 'P');
}

static void testarPromocaoTemporaria(void) {
    const char tipos[] = "DTBC";
    for (int i = 0; i < 4; ++i) {
        Combate j;
        iniciarCenario(&j, 5);
        int peao = unidade(&j, 'P', 0, 1, 2);
        j.unidades[peao].origem = (Casa){4, 2};
        int inimigoP = unidade(&j, 'P', 1, 2, 4);
        agir(&j, peao, 0, (Casa){0, 2});
        assert(promocaoPendente(&j) == peao);
        assert(!encontrar(&j, inimigoP, 0, (Casa){3, 4}, NULL));
        assert(!escolherPromocao(&j, peao, 'R'));
        assert(!escolherPromocao(&j, inimigoP, 'D'));
        assert(!prepararRodada(&j));
        assert(escolherPromocao(&j, peao, tipos[i]));
        assert(j.unidades[peao].tipo == tipos[i] && j.unidades[peao].original == 'P');
        assert(promocaoPendente(&j) == -1 && j.lado == 1);
        assert(!escolherPromocao(&j, peao, 'D'));
        remover(&j, inimigoP);
        j.pecasInimigas = 3;
        assert(prepararRodada(&j));
        assert(j.unidades[peao].tipo == 'P' && j.unidades[peao].original == 'P');
        confirmarPosicao(&j, peao, 4, 2);
        verificarTabuleiro(&j);
    }

    Combate j;
    iniciarCenario(&j, 5);
    int peao = unidade(&j, 'P', 1, 3, 2);
    int alvo = unidade(&j, 'T', 0, 4, 3);
    unidade(&j, 'P', 0, 3, 4);
    j.lado = 1;
    agir(&j, peao, 0, (Casa){4, 3});
    assert(!j.unidades[alvo].vivo && promocaoPendente(&j) == peao);
    assert(escolherPromocao(&j, peao, 'C'));
    assert(j.unidades[peao].tipo == 'C' && j.unidades[peao].original == 'P');

    /* Uma peça transportada por outra também pode se promover. */
    iniciarCenario(&j, 5);
    int torre = unidade(&j, 'T', 0, 3, 2);
    peao = unidade(&j, 'P', 0, 2, 2);
    unidade(&j, 'P', 1, 1, 4);
    darCarta(&j, 'T', 5);
    agirComExtra(&j, torre, 5, (Casa){2, 2}, (Casa){0, 2}, ausente);
    assert(promocaoPendente(&j) == peao);
    assert(escolherPromocao(&j, peao, 'T'));
    confirmarPosicao(&j, peao, 0, 2);
}

static void testarVariantesBispo(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int bispo = unidade(&j, 'B', 0, 2, 2);
    darCarta(&j, 'B', 5);
    darCarta(&j, 'B', 13);
    ListaAcoes lista;
    gerarAcoes(&j, bispo, &lista);
    int destinos[5][8] = {{0}};
    int peregrinacoes = 0, selamentos = 0;
    for (int i = 0; i < lista.n; ++i) {
        Acao a = lista.itens[i];
        if (!igualCasa(a.destino, (Casa){0, 0})) continue;
        if (a.carta == 5) {
            assert(dentro(&j, a.extra));
            assert(!destinos[a.extra.l][a.extra.c]++);
            ++peregrinacoes;
        }
        if (a.carta == 13) ++selamentos;
    }
    assert(peregrinacoes == 3 && selamentos == 1);

    lista.n = 0;
    adicionar(&lista, bispo, 13, (Casa){0, 0}, ausente, ausente);
    adicionar(&lista, bispo, 13, (Casa){0, 0}, ausente, ausente);
    assert(lista.n == 1);
}

static void testarTorre(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int torre = unidade(&j, 'T', 0, 3, 0);
    int peao = unidade(&j, 'P', 0, 3, 3);
    unidade(&j, 'P', 1, 0, 4);
    darCarta(&j, 'T', 4);
    agir(&j, torre, 4, (Casa){3, 3});
    assert(igualCasa(posicao(&j, torre), (Casa){3, 0}));
    assert(igualCasa(posicao(&j, peao), (Casa){3, 1}));

    iniciarCenario(&j, 5);
    torre = unidade(&j, 'T', 0, 4, 0);
    peao = unidade(&j, 'P', 0, 4, 1);
    unidade(&j, 'B', 1, 4, 2);
    darCarta(&j, 'T', 5);
    agirComExtra(&j, torre, 5, (Casa){4, 1}, (Casa){4, 4}, ausente);
    confirmarPosicao(&j, peao, 4, 4);
    assert(usouCarta(&j, torre, 5));

    iniciarCenario(&j, 5);
    torre = unidade(&j, 'T', 0, 4, 0);
    int alvo = unidade(&j, 'P', 1, 1, 0);
    darCarta(&j, 'T', 7);
    agirComExtra(&j, torre, 7, (Casa){1, 0}, (Casa){3, 0}, ausente);
    confirmarPosicao(&j, torre, 3, 0);
    assert(!j.unidades[alvo].vivo && usouCarta(&j, torre, 7));

    iniciarCenario(&j, 5);
    torre = unidade(&j, 'T', 0, 4, 0);
    int rei = unidade(&j, 'R', 0, 4, 3);
    darCarta(&j, 'T', 8);
    agir(&j, torre, 8, (Casa){4, 3});
    confirmarPosicao(&j, rei, 4, 0);
    confirmarPosicao(&j, torre, 4, 3);

    iniciarCenario(&j, 5);
    torre = unidade(&j, 'T', 0, 2, 2);
    darCarta(&j, 'T', 9);
    agirComExtra(&j, torre, 9, (Casa){0, 2}, (Casa){0, 3}, ausente);
    confirmarPosicao(&j, torre, 0, 3);

    iniciarCenario(&j, 5);
    torre = unidade(&j, 'T', 0, 3, 2);
    peao = unidade(&j, 'P', 0, 4, 2);
    darCarta(&j, 'T', 14);
    agirComExtra(&j, torre, 14, (Casa){1, 2}, (Casa){4, 2}, (Casa){2, 2});
    confirmarPosicao(&j, torre, 1, 2);
    confirmarPosicao(&j, peao, 2, 2);

    iniciarCenario(&j, 5);
    torre = unidade(&j, 'T', 0, 4, 2);
    darCarta(&j, 'T', 16);
    agirComExtra(&j, torre, 16, (Casa){2, 3}, (Casa){2, 2}, ausente);
    confirmarPosicao(&j, torre, 2, 3);

    iniciarCenario(&j, 5);
    torre = unidade(&j, 'T', 0, 4, 2);
    int outra = unidade(&j, 'T', 0, 4, 3);
    darCarta(&j, 'T', 18);
    agirComExtra(&j, torre, 18, (Casa){1, 2}, (Casa){4, 3}, (Casa){1, 3});
    confirmarPosicao(&j, torre, 1, 2);
    confirmarPosicao(&j, outra, 1, 3);

    iniciarCenario(&j, 5);
    torre = unidade(&j, 'T', 0, 3, 2);
    alvo = unidade(&j, 'P', 1, 2, 1);
    darCarta(&j, 'T', 20);
    agir(&j, torre, 20, (Casa){2, 1});
    assert(!j.unidades[alvo].vivo);
    confirmarPosicao(&j, torre, 2, 1);
}

static void testarBispo(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int bispo = unidade(&j, 'B', 0, 3, 3);
    darCarta(&j, 'B', 1);
    assert(!encontrar(&j, bispo, 0, (Casa){3, 2}, NULL));
    agir(&j, bispo, 1, (Casa){3, 2});
    assert(igualCasa(posicao(&j, bispo), (Casa){3, 2}));

    iniciarCenario(&j, 8);
    bispo = unidade(&j, 'B', 0, 2, 1);
    int alvo = unidade(&j, 'P', 1, 2, 5);
    darCarta(&j, 'B', 2);
    agirComExtra(&j, bispo, 2, (Casa){2, 5}, (Casa){0, 3}, ausente);
    confirmarPosicao(&j, bispo, 2, 5);
    assert(!j.unidades[alvo].vivo);

    iniciarCenario(&j, 5);
    bispo = unidade(&j, 'B', 0, 4, 0);
    int aliadoP = unidade(&j, 'P', 0, 1, 3);
    darCarta(&j, 'B', 4);
    agir(&j, bispo, 4, (Casa){1, 3});
    confirmarPosicao(&j, aliadoP, 4, 0);

    iniciarCenario(&j, 5);
    bispo = unidade(&j, 'B', 0, 2, 2);
    darCarta(&j, 'B', 5);
    agirComExtra(&j, bispo, 5, (Casa){0, 0}, (Casa){4, 4}, ausente);
    confirmarPosicao(&j, bispo, 4, 4);
    assert(usouCarta(&j, bispo, 5));

    iniciarCenario(&j, 5);
    bispo = unidade(&j, 'B', 0, 4, 0);
    unidade(&j, 'P', 1, 3, 1);
    unidade(&j, 'T', 0, 1, 0);
    darCarta(&j, 'B', 6);
    agir(&j, bispo, 6, (Casa){1, 3});
    confirmarPosicao(&j, bispo, 1, 3);

    iniciarCenario(&j, 5);
    bispo = unidade(&j, 'B', 0, 4, 0);
    alvo = unidade(&j, 'P', 1, 1, 3);
    darCarta(&j, 'B', 7);
    agirComExtra(&j, bispo, 7, (Casa){1, 3}, (Casa){2, 2}, ausente);
    confirmarPosicao(&j, bispo, 2, 2);
    assert(!j.unidades[alvo].vivo);

    iniciarCenario(&j, 5);
    bispo = unidade(&j, 'B', 0, 4, 0);
    alvo = unidade(&j, 'P', 1, 1, 3);
    j.unidades[alvo].cartas = 1u << 2;
    darCarta(&j, 'B', 11);
    agir(&j, bispo, 11, (Casa){1, 3});
    assert(j.unidades[alvo].silencio && !temCarta(&j, alvo, 2));
    agir(&j, alvo, 0, (Casa){2, 3});
    assert(!j.unidades[alvo].silencio && temCarta(&j, alvo, 2));

    iniciarCenario(&j, 5);
    bispo = unidade(&j, 'B', 0, 4, 0);
    aliadoP = unidade(&j, 'P', 0, 1, 3);
    darCarta(&j, 'B', 19);
    agir(&j, bispo, 19, (Casa){1, 3});
    assert(j.unidades[aliadoP].duplo && usouCarta(&j, bispo, 19));
}

static void testarCoice(void) {
    Combate j;
    iniciarCenario(&j, 8);
    int cavalo = unidade(&j, 'C', 0, 4, 3);
    int norte = unidade(&j, 'P', 1, 1, 4);
    int sul = unidade(&j, 'P', 1, 3, 4);
    int oeste = unidade(&j, 'P', 1, 2, 3);
    int leste = unidade(&j, 'P', 1, 2, 5);
    int bloqueador = unidade(&j, 'P', 0, 2, 6);
    darCarta(&j, 'C', 8);
    agir(&j, cavalo, 8, (Casa){2, 4});
    assert(igualCasa(posicao(&j, cavalo), (Casa){2, 4}));
    assert(igualCasa(posicao(&j, norte), (Casa){0, 4}));
    assert(igualCasa(posicao(&j, sul), (Casa){4, 4}));
    assert(igualCasa(posicao(&j, oeste), (Casa){2, 2}));
    assert(igualCasa(posicao(&j, leste), (Casa){2, 5}));
    assert(igualCasa(posicao(&j, bloqueador), (Casa){2, 6}));
}

static void testarOutrasCartasCavalo(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int c = unidade(&j, 'C', 0, 4, 2);
    darCarta(&j, 'C', 1);
    agir(&j, c, 1, (Casa){1, 3});
    confirmarPosicao(&j, c, 1, 3);
    j.lado = 0;
    darCarta(&j, 'C', 2);
    agir(&j, c, 2, (Casa){1, 1});
    confirmarPosicao(&j, c, 1, 1);
    j.lado = 0;
    darCarta(&j, 'C', 3);
    agir(&j, c, 3, (Casa){1, 2});
    confirmarPosicao(&j, c, 1, 2);

    iniciarCenario(&j, 5);
    c = unidade(&j, 'C', 0, 4, 2);
    int p = unidade(&j, 'P', 1, 2, 3);
    darCarta(&j, 'C', 5);
    agir(&j, c, 5, (Casa){2, 3});
    assert(!j.unidades[p].vivo && usouCarta(&j, c, 5));
    confirmarPosicao(&j, c, 4, 2);

    iniciarCenario(&j, 5);
    c = unidade(&j, 'C', 0, 4, 2);
    p = unidade(&j, 'P', 0, 2, 3);
    darCarta(&j, 'C', 6);
    agir(&j, c, 6, (Casa){2, 3});
    confirmarPosicao(&j, c, 2, 3);
    confirmarPosicao(&j, p, 4, 2);

    iniciarCenario(&j, 5);
    c = unidade(&j, 'C', 0, 4, 2);
    p = unidade(&j, 'P', 0, 2, 3);
    unidade(&j, 'T', 1, 0, 3);
    darCarta(&j, 'C', 7);
    agirComExtra(&j, c, 7, (Casa){2, 3}, (Casa){4, 1}, ausente);
    confirmarPosicao(&j, c, 2, 3);
    confirmarPosicao(&j, p, 4, 1);
    assert(usouCarta(&j, c, 7));

    iniciarCenario(&j, 5);
    c = unidade(&j, 'C', 0, 4, 2);
    int outro = unidade(&j, 'C', 0, 2, 1);
    darCarta(&j, 'C', 10);
    agirComExtra(&j, c, 10, (Casa){2, 3}, (Casa){2, 1}, ausente);
    confirmarPosicao(&j, c, 2, 3);
    confirmarPosicao(&j, outro, 4, 2);

    iniciarCenario(&j, 5);
    c = unidade(&j, 'C', 0, 2, 1);
    darCarta(&j, 'C', 16);
    agirComExtra(&j, c, 16, (Casa){0, 0}, (Casa){0, 1}, ausente);
    confirmarPosicao(&j, c, 0, 1);

    iniciarCenario(&j, 5);
    c = unidade(&j, 'C', 0, 4, 2);
    p = unidade(&j, 'P', 0, 4, 3);
    darCarta(&j, 'C', 19);
    agirComExtra(&j, c, 19, (Casa){2, 3}, (Casa){4, 3}, ausente);
    confirmarPosicao(&j, p, 3, 3);
}

static void testarRei(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int rei = unidade(&j, 'R', 0, 4, 2);
    int peao = unidade(&j, 'P', 0, 2, 2);
    unidade(&j, 'P', 1, 0, 4);
    darCarta(&j, 'R', 1);
    agir(&j, rei, 1, (Casa){2, 2});
    assert(j.unidades[peao].duplo > 0);
    assert(igualCasa(posicao(&j, rei), (Casa){4, 2}));
    assert(igualCasa(posicao(&j, peao), (Casa){2, 2}));

    iniciarCenario(&j, 5);
    rei = unidade(&j, 'R', 0, 4, 0);
    peao = unidade(&j, 'P', 0, 1, 0);
    darCarta(&j, 'R', 2);
    agirComExtra(&j, rei, 2, (Casa){1, 0}, (Casa){4, 1}, ausente);
    confirmarPosicao(&j, peao, 4, 1);
    confirmarPosicao(&j, rei, 4, 0);

    iniciarCenario(&j, 5);
    rei = unidade(&j, 'R', 0, 4, 0);
    int alvo = unidade(&j, 'P', 1, 2, 2);
    darCarta(&j, 'R', 5);
    agir(&j, rei, 5, (Casa){2, 2});
    confirmarPosicao(&j, rei, 2, 2);
    assert(!j.unidades[alvo].vivo);

    iniciarCenario(&j, 5);
    rei = unidade(&j, 'R', 0, 4, 0);
    peao = unidade(&j, 'P', 0, 0, 4);
    darCarta(&j, 'R', 7);
    agir(&j, rei, 7, (Casa){0, 4});
    assert(j.unidades[peao].duplo && usouCarta(&j, rei, 7));

    iniciarCenario(&j, 5);
    rei = unidade(&j, 'R', 0, 3, 2);
    alvo = unidade(&j, 'P', 1, 2, 2);
    j.unidades[alvo].cartas = 1u << 2;
    darCarta(&j, 'R', 11);
    agir(&j, rei, 11, (Casa){2, 2});
    assert(j.unidades[alvo].silencio && !temCarta(&j, alvo, 2));

    iniciarCenario(&j, 5);
    rei = unidade(&j, 'R', 0, 4, 0);
    peao = unidade(&j, 'P', 0, 1, 3);
    darCarta(&j, 'R', 13);
    agir(&j, rei, 13, (Casa){1, 3});
    assert(j.unidades[peao].duplo);

    iniciarCenario(&j, 5);
    rei = unidade(&j, 'R', 0, 4, 2);
    peao = unidade(&j, 'P', 0, 3, 3);
    alvo = unidade(&j, 'P', 1, 3, 2);
    darCarta(&j, 'R', 15);
    agirComExtra(&j, rei, 15, (Casa){3, 2}, (Casa){3, 3}, ausente);
    assert(j.unidades[peao].duplo && !j.unidades[alvo].vivo);
    assert(usouCarta(&j, rei, 15));

    iniciarCenario(&j, 5);
    rei = unidade(&j, 'R', 0, 4, 2);
    peao = unidade(&j, 'P', 0, 4, 3);
    alvo = unidade(&j, 'T', 1, 0, 2);
    darCarta(&j, 'R', 19);
    j.lado = 1;
    agir(&j, alvo, 0, (Casa){4, 2});
    assert(!j.unidades[rei].vivo && j.unidades[peao].tipo == 'R');
    assert(j.unidades[peao].original == 'P');

    iniciarCenario(&j, 5);
    rei = unidade(&j, 'R', 0, 4, 2);
    unidade(&j, 'P', 0, 4, 0);
    alvo = unidade(&j, 'R', 1, 2, 2);
    j.unidades[alvo].chefe = 1;
    darCarta(&j, 'R', 20);
    agir(&j, rei, 20, (Casa){2, 2});
    assert(!j.unidades[alvo].vivo);
}

static void testarOutrasCartasRainha(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int rainha = unidade(&j, 'D', 0, 4, 0);
    int peao = unidade(&j, 'P', 0, 1, 3);
    darCarta(&j, 'D', 4);
    agir(&j, rainha, 4, (Casa){1, 3});
    confirmarPosicao(&j, peao, 4, 0);
    confirmarPosicao(&j, rainha, 1, 3);

    iniciarCenario(&j, 5);
    rainha = unidade(&j, 'D', 0, 4, 0);
    peao = unidade(&j, 'P', 0, 4, 2);
    darCarta(&j, 'D', 6);
    agirComExtra(&j, rainha, 6, (Casa){4, 2}, (Casa){3, 2}, (Casa){4, 2});
    confirmarPosicao(&j, peao, 3, 2);
    confirmarPosicao(&j, rainha, 4, 2);

    iniciarCenario(&j, 8);
    rainha = unidade(&j, 'D', 0, 2, 0);
    unidade(&j, 'P', 1, 2, 4);
    darCarta(&j, 'D', 11);
    agir(&j, rainha, 11, (Casa){2, 7});
    confirmarPosicao(&j, rainha, 2, 7);
    assert(usouCarta(&j, rainha, 11));

    iniciarCenario(&j, 5);
    rainha = unidade(&j, 'D', 0, 4, 2);
    int torre = unidade(&j, 'T', 0, 4, 4);
    int alvo = unidade(&j, 'T', 1, 0, 2);
    darCarta(&j, 'D', 16);
    j.lado = 1;
    agir(&j, alvo, 0, (Casa){4, 2});
    assert(!j.unidades[alvo].vivo && !j.unidades[rainha].vivo);
    confirmarPosicao(&j, torre, 4, 2);

    iniciarCenario(&j, 5);
    rainha = unidade(&j, 'D', 0, 4, 0);
    peao = unidade(&j, 'P', 0, 4, 4);
    int inimigoP = unidade(&j, 'P', 1, 0, 0);
    darCarta(&j, 'D', 17);
    agir(&j, rainha, 17, (Casa){4, 4});
    assert(igualCasa(j.unidades[peao].corredor, (Casa){4, 0}));
    agir(&j, inimigoP, 0, (Casa){1, 0});
    agir(&j, peao, 0, (Casa){4, 2});
    confirmarPosicao(&j, peao, 4, 2);
    assert(!dentro(&j, j.unidades[peao].corredor));

    iniciarCenario(&j, 5);
    rainha = unidade(&j, 'D', 0, 2, 2);
    peao = unidade(&j, 'P', 0, 2, 0);
    torre = unidade(&j, 'T', 0, 2, 4);
    darCarta(&j, 'D', 19);
    agirComExtra(&j, rainha, 19, (Casa){2, 0}, (Casa){2, 4}, ausente);
    confirmarPosicao(&j, peao, 2, 4);
    confirmarPosicao(&j, torre, 2, 0);
    confirmarPosicao(&j, rainha, 2, 2);
}

static void testarGambito(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int rainha = unidade(&j, 'D', 0, 2, 2);
    int torre = unidade(&j, 'T', 1, 0, 2);
    unidade(&j, 'P', 0, 4, 4);
    unidade(&j, 'P', 1, 0, 4);
    darCarta(&j, 'D', 9);
    agir(&j, rainha, 9, (Casa){2, 2});
    assert(j.unidades[rainha].gambito > 0);
    agir(&j, torre, 0, (Casa){2, 2});
    assert(!j.unidades[rainha].vivo && !j.unidades[torre].vivo);
    assert(vazio(&j, (Casa){2, 2}));

    iniciarCenario(&j, 5);
    rainha = unidade(&j, 'D', 0, 2, 2);
    torre = unidade(&j, 'T', 1, 0, 2);
    int inimigoP = unidade(&j, 'P', 1, 0, 4);
    unidade(&j, 'P', 0, 4, 4);
    darCarta(&j, 'D', 9);
    agir(&j, rainha, 9, (Casa){2, 2});
    agir(&j, inimigoP, 0, (Casa){1, 4});
    assert(!j.unidades[rainha].gambito);
    j.lado = 1;
    agir(&j, torre, 0, (Casa){2, 2});
    assert(!j.unidades[rainha].vivo && j.unidades[torre].vivo);
}

static void testarChefe(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int rainha = unidade(&j, 'D', 0, 4, 2);
    int chefe = unidade(&j, 'R', 1, 0, 2);
    int peao = unidade(&j, 'P', 1, 1, 4);
    j.unidades[chefe].chefe = 1;
    assert(!podeCapturar(&j, rainha, chefe));
    assert(!encontrar(&j, rainha, 0, (Casa){0, 2}, NULL));
    remover(&j, peao);
    assert(podeCapturar(&j, rainha, chefe));
    agir(&j, rainha, 0, (Casa){0, 2});
    assert(!j.unidades[chefe].vivo);
}

static void testarReacoes(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int defensor = unidade(&j, 'P', 0, 2, 1);
    int vitima = unidade(&j, 'T', 0, 2, 2);
    int agressor = unidade(&j, 'T', 1, 0, 2);
    int outro = unidade(&j, 'P', 1, 0, 4);
    j.lado = 1;
    darCarta(&j, 'P', 12);
    agir(&j, agressor, 0, (Casa){2, 2});
    assert(!j.unidades[vitima].vivo && !j.unidades[agressor].vivo);
    assert(igualCasa(posicao(&j, defensor), (Casa){2, 2}));
    assert(j.unidades[outro].vivo && usouCarta(&j, defensor, 12));

    iniciarCenario(&j, 5);
    defensor = unidade(&j, 'P', 0, 2, 1);
    vitima = unidade(&j, 'T', 0, 2, 2);
    agressor = unidade(&j, 'T', 1, 0, 2);
    j.lado = 1;
    darCarta(&j, 'P', 12);
    assert(cartaReativa('P', 12) && !cartaReativa('P', 21));
    j.reacoesDesativadas[indiceTipo('P')] = 1u << 12;
    agir(&j, agressor, 0, (Casa){2, 2});
    assert(!j.unidades[vitima].vivo && j.unidades[agressor].vivo);
    confirmarPosicao(&j, defensor, 2, 1);
    assert(!usouCarta(&j, defensor, 12));

    iniciarCenario(&j, 5);
    defensor = unidade(&j, 'T', 0, 4, 2);
    agressor = unidade(&j, 'T', 1, 0, 2);
    unidade(&j, 'P', 1, 0, 4);
    j.lado = 1;
    darCarta(&j, 'T', 15);
    agir(&j, agressor, 0, (Casa){4, 2});
    assert(j.unidades[defensor].vivo && j.unidades[agressor].vivo);
    assert(igualCasa(posicao(&j, defensor), (Casa){4, 2}));
    assert(igualCasa(posicao(&j, agressor), (Casa){0, 2}));
    assert(usouCarta(&j, defensor, 15) && j.lado == 0);

    iniciarCenario(&j, 5);
    defensor = unidade(&j, 'B', 0, 4, 0);
    vitima = unidade(&j, 'T', 0, 2, 2);
    agressor = unidade(&j, 'T', 1, 0, 2);
    unidade(&j, 'P', 1, 0, 4);
    j.lado = 1;
    darCarta(&j, 'B', 16);
    agir(&j, agressor, 0, (Casa){2, 2});
    assert(!j.unidades[defensor].vivo && j.unidades[vitima].vivo);
    assert(igualCasa(posicao(&j, agressor), (Casa){0, 2}));
    assert(igualCasa(posicao(&j, vitima), (Casa){2, 2}));

    iniciarCenario(&j, 5);
    defensor = unidade(&j, 'D', 0, 4, 0);
    int peao = unidade(&j, 'P', 0, 2, 0);
    agressor = unidade(&j, 'T', 1, 0, 0);
    unidade(&j, 'P', 1, 0, 4);
    j.lado = 1;
    darCarta(&j, 'D', 7);
    /* A torre precisa chegar à rainha sem atravessar o peão. */
    assert(deslocar(&j, peao, (Casa){3, 1}));
    agir(&j, agressor, 0, (Casa){4, 0});
    assert(!j.unidades[defensor].vivo && j.unidades[agressor].marca);
    agir(&j, peao, 0, (Casa){4, 0});
    assert(!j.unidades[agressor].vivo);
}

static void testarJogadaDupla(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int rei = unidade(&j, 'R', 0, 4, 2);
    int peao = unidade(&j, 'P', 0, 4, 1);
    int inimigoP = unidade(&j, 'P', 1, 0, 4);
    agir(&j, rei, 0, (Casa){4, 1});
    assert(j.unidades[peao].duplo && j.lado == 1);
    agir(&j, inimigoP, 0, (Casa){1, 4});
    agir(&j, peao, 0, (Casa){3, 1});
    assert(j.lado == 0 && j.bonus == peao);
    assert(!encontrar(&j, rei, 0, (Casa){3, 2}, NULL));
    agir(&j, peao, 0, (Casa){2, 1});
    assert(j.lado == 1 && j.bonus < 0 && !j.unidades[peao].duplo);
}

static void testarEfeitosTemporarios(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int bispo = unidade(&j, 'B', 0, 4, 0);
    int torre = unidade(&j, 'T', 1, 0, 2);
    unidade(&j, 'P', 0, 4, 4);
    darCarta(&j, 'B', 13);
    agir(&j, bispo, 13, (Casa){2, 2});
    assert(j.selada[2][2] != 0);
    assert(!encontrar(&j, torre, 0, (Casa){2, 2}, NULL));
    /* A diagonal selada permite atravessar, apenas veta o pouso. */
    agir(&j, torre, 0, (Casa){3, 2});
    assert(j.selada[2][2] == 0);

    iniciarCenario(&j, 5);
    int rei = unidade(&j, 'R', 0, 3, 2);
    int guarda = unidade(&j, 'T', 0, 3, 3);
    int vitima = unidade(&j, 'P', 0, 1, 3);
    int agressor = unidade(&j, 'T', 1, 1, 0);
    unidade(&j, 'P', 1, 0, 4);
    darCarta(&j, 'R', 17);
    agir(&j, rei, 17, (Casa){3, 3});
    assert(j.unidades[guarda].guarda);
    agir(&j, agressor, 0, (Casa){1, 3});
    assert(!j.unidades[vitima].vivo && !j.unidades[agressor].vivo);
    assert(igualCasa(posicao(&j, guarda), (Casa){1, 3}));
    assert(!j.unidades[guarda].guarda);

    iniciarCenario(&j, 5);
    rei = unidade(&j, 'R', 0, 3, 2);
    guarda = unidade(&j, 'T', 0, 3, 3);
    vitima = unidade(&j, 'P', 0, 1, 3);
    agressor = unidade(&j, 'T', 1, 1, 0);
    int outro = unidade(&j, 'P', 1, 0, 4);
    darCarta(&j, 'R', 17);
    agir(&j, rei, 17, (Casa){3, 3});
    agir(&j, outro, 0, (Casa){1, 4});
    agir(&j, rei, 0, (Casa){4, 2});
    assert(!j.unidades[guarda].guarda);
    agir(&j, agressor, 0, (Casa){1, 3});
    assert(!j.unidades[vitima].vivo && j.unidades[agressor].vivo);
    assert(igualCasa(posicao(&j, guarda), (Casa){3, 3}));

    iniciarCenario(&j, 5);
    int rainha = unidade(&j, 'D', 0, 4, 2);
    int peao = unidade(&j, 'P', 0, 4, 3);
    int inimigoP = unidade(&j, 'P', 1, 0, 4);
    darCarta(&j, 'D', 13);
    agir(&j, rainha, 13, (Casa){4, 3});
    agir(&j, inimigoP, 0, (Casa){1, 4});
    assert(j.unidades[peao].herdeira);
    agir(&j, peao, 0, (Casa){1, 0});
    assert(igualCasa(posicao(&j, peao), (Casa){1, 0}));
    assert(!j.unidades[peao].herdeira);
}

static void testarXequeDescoberto(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int rainha = unidade(&j, 'D', 0, 4, 0);
    int peao = unidade(&j, 'P', 0, 3, 0);
    int alvo = unidade(&j, 'P', 1, 0, 0);
    unidade(&j, 'P', 1, 0, 4);
    darCarta(&j, 'P', 2);
    darCarta(&j, 'D', 20);
    agir(&j, peao, 2, (Casa){3, 1});
    assert(!j.unidades[alvo].vivo);
    assert(igualCasa(posicao(&j, rainha), (Casa){0, 0}));
    assert(usouCarta(&j, rainha, 20));

    iniciarCenario(&j, 5);
    rainha = unidade(&j, 'D', 0, 4, 0);
    peao = unidade(&j, 'P', 0, 3, 0);
    alvo = unidade(&j, 'R', 1, 0, 0);
    j.unidades[alvo].chefe = 1;
    unidade(&j, 'P', 1, 0, 4);
    darCarta(&j, 'P', 2);
    darCarta(&j, 'D', 20);
    agir(&j, peao, 2, (Casa){3, 1});
    assert(j.unidades[alvo].vivo);
    assert(igualCasa(posicao(&j, rainha), (Casa){4, 0}));
    assert(!usouCarta(&j, rainha, 20));
}

static void testarTransicaoDeRodada(void) {
    Combate j;
    iniciarCombate(&j, 'P');
    int p = ocupante(&j, (Casa){4, 2});
    assert(p >= 0 && j.unidades[p].tipo == 'P');
    assert(contarLado(&j, 0) == 1 && contarLado(&j, 1) == 3 && j.largura == 5);
    verificarTabuleiro(&j);
    darCarta(&j, 'P', 2);
    agir(&j, p, 2, (Casa){4, 1});
    j.rodada = 5;
    for (int n = 0; n < j.quantidade; ++n) {
        if (j.unidades[n].vivo && j.unidades[n].lado) remover(&j, n);
    }
    assert(prepararRodada(&j));
    assert(j.rodada == 6 && j.largura == 6 && j.lado == 0);
    assert(igualCasa(posicao(&j, p), (Casa){4, 2}));
    assert(j.unidades[p].moveu && temCarta(&j, p, 2));
    assert(recrutar(&j, 'P', (Casa){4, 5}));
    assert(!recrutar(&j, 'P', (Casa){4, 5}));
    assert(temCarta(&j, ocupante(&j, (Casa){4, 5}), 2));
    verificarTabuleiro(&j);
}

static void testarCombinacoes(void) {
    /* Exercita a API de geração e execução em tabuleiros lotados com várias
     * cartas adquiridas, inclusive capturas, reações e ações com múltiplas
     * peças. Cada alternativa é executada em uma cópia independente. */
    const char tipos[] = "PTBCRD";
    long total = 0;
    for (int teste = 0; teste < 50; ++teste) {
        Combate j;
        iniciarCenario(&j, 5 + teste % 4);
        for (int t = 0; t < TIPOS_PECA; ++t) {
            for (int i = 0; i < CARTAS_TIPO; ++i) {
                int id = catalogo[t][i].id;
                darCarta(&j, catalogo[t][i].tipo, id);
            }
        }
        for (int i = 0; i < 16; ++i) {
            Casa p;
            do {
                p = (Casa){rand() % 5, rand() % j.largura};
            } while (!vazio(&j, p));
            int u = unidade(&j, tipos[i % TIPOS_PECA], (i / 6 + teste) % 2, p.l, p.c);
            if (j.unidades[u].lado) {
                j.unidades[u].cartas = j.adquiridas[indiceTipo(j.unidades[u].tipo)];
            }
        }
        verificarTabuleiro(&j);
        ListaAcoes acoes;
        for (int u = 0; u < j.quantidade; ++u) {
            j.lado = j.unidades[u].lado;
            gerarAcoes(&j, u, &acoes);
            for (int i = 0; i < acoes.n; ++i) {
                Acao atual = acoes.itens[i];
                for (int k = 0; k < i; ++k) {
                    Acao outra = acoes.itens[k];
                    if (atual.ator == outra.ator && atual.carta == outra.carta
                        && igualCasa(atual.destino, outra.destino)) {
                        char tipo = j.unidades[u].tipo;
                        assert(!(tipo == 'B' && atual.carta == 2));
                        assert(!(tipo == 'T' && atual.carta == 16));
                    }
                    assert(atual.ator != outra.ator || atual.carta != outra.carta
                        || !igualCasa(atual.destino, outra.destino)
                        || !igualCasa(atual.extra, outra.extra)
                        || !igualCasa(atual.extra2, outra.extra2));
                }
                Combate copia = j;
                assert(executarAcao(&copia, acoes.itens[i]));
                for (int p = promocaoPendente(&copia); p >= 0; p = promocaoPendente(&copia))
                    assert(escolherPromocao(&copia, p, 'D'));
                verificarTabuleiro(&copia);
                total++;
            }
        }
    }
    assert(total >= 1000);
    printf("%ld alternativas de movimento verificadas.\n", total);
}

static void testarAcaoIlegal(void) {
    Combate j;
    iniciarCenario(&j, 5);
    int torre = unidade(&j, 'T', 0, 4, 2);
    unidade(&j, 'P', 1, 0, 2);
    Acao invalida = {torre, 0, {3, 3}, ausente, ausente};
    Combate anterior = j;
    assert(!executarAcao(&j, invalida));
    assert(memcmp(&j, &anterior, sizeof j) == 0);
}

int main(void) {
    srand(42);
    testarCatalogo();
    testarCompraESorteio();
    testarSorteioInimigo();
    testarPeao();
    testarPromocoes();
    testarPromocaoTemporaria();
    testarTorre();
    testarBispo();
    testarVariantesBispo();
    testarCoice();
    testarOutrasCartasCavalo();
    testarRei();
    testarGambito();
    testarOutrasCartasRainha();
    testarChefe();
    testarReacoes();
    testarJogadaDupla();
    testarEfeitosTemporarios();
    testarXequeDescoberto();
    testarTransicaoDeRodada();
    testarAcaoIlegal();
    testarCombinacoes();
    puts("Testes de melhorias concluidos com sucesso.");
    return 0;
}
