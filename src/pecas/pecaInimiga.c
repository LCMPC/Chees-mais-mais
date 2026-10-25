#include <stdlib.h>
#include "pecaInimiga.h"

PecaInimiga *criarPecaInimiga(PecaInimiga ** head, int fase) {
    if (head == NULL || fase < 1) return NULL;

     char tipos[5] = {'P', 'B', 'T', 'C', 'R'};
    int quantidadeTipos = 1;
    if (fase >= 3) quantidadeTipos = 3;
    if (fase >= 5) quantidadeTipos = 4;
    if (fase >= 10) quantidadeTipos = 5;

    PecaInimiga *nova = malloc(sizeof(PecaInimiga));
    if (nova == NULL) return NULL;

    nova->inicial = tipos[rand() % quantidadeTipos];
    nova->posicaoInicial[0] = -1;
    nova->posicaoInicial[1] = -1;
    nova->quantidadeMelhorias = 0;
    nova->ehChefe = 0;
    nova->jaMoveu = 0;
    for (int i = 0; i < MAX_MELHORIAS_INIMIGAS; i++) {
        nova->melhorias[i] = 0;
    }

    nova->proximo = *head;
    *head = nova;
    return nova;
}

void liberarPecasInimigas(PecaInimiga ** head) {
    if (head == NULL) return;

    while (*head != NULL) {
        PecaInimiga *atual = *head;
        *head = atual->proximo;
        free(atual);
    }
}

static int podeReceberMelhoria(PecaInimiga *peca) {
    return peca->inicial != 'D' && !peca->ehChefe
        && peca->quantidadeMelhorias < MAX_MELHORIAS_INIMIGAS;
}

static void sortearMelhoria(PecaInimiga *peca) {
    int disponiveis[MAX_MELHORIAS_INIMIGAS];
    int quantidadeDisponiveis = 0;

    /* Monta a lista de IDs ainda nao usados por ESTA unidade. */
    for (int id = 1; id <= MAX_MELHORIAS_INIMIGAS; id++) {
        int repetida = 0;
        for (int i = 0; i < peca->quantidadeMelhorias; i++) {
            if (peca->melhorias[i] == id) repetida = 1;
        }
        if (!repetida) {
            disponiveis[quantidadeDisponiveis] = id;
            quantidadeDisponiveis++;
        }
    }

    /* Sorteia apenas entre os IDs disponiveis, sem ficar tentando repetidos. */
    int indice = rand() % quantidadeDisponiveis;
    peca->melhorias[peca->quantidadeMelhorias] = disponiveis[indice];
    peca->quantidadeMelhorias++;
}

int criarRodadaInimiga(PecaInimiga ** head, int fase, int qtdMelhorias, int qtdPecas) {
    if (head == NULL || fase < 1 || qtdMelhorias < 0 || qtdPecas < 1) return -1;

    int largura = 5 + (fase - 1) / 5;
    if (largura > 8) largura = 8;

    /* As duas ultimas linhas sao reservadas ao posicionamento aliado. */
    int casasLivres[24]; /* tres linhas, no maximo oito colunas */
    int quantidadeCasas = largura * 3;
    if (qtdPecas > quantidadeCasas) return -1;
    for (int i = 0; i < quantidadeCasas; i++) casasLivres[i] = i;

    PecaInimiga *novaLista = NULL;
    for (int i = 0; i < qtdPecas; i++) {
        PecaInimiga * nova = criarPecaInimiga(&novaLista, fase);
        if (nova == NULL) {
            liberarPecasInimigas(&novaLista);
            return -1;
        }

        int indice = rand() % quantidadeCasas;
        int casa = casasLivres[indice];
        nova->posicaoInicial[0] = casa / largura;
        nova->posicaoInicial[1] = casa % largura;

        /* Retira a casa sorteada para nenhuma outra unidade ocupa-la. */
        casasLivres[indice] = casasLivres[quantidadeCasas - 1];
        quantidadeCasas--;
    }

    /* Substitui um inimigo por rei ANTES de distribuir melhorias. */
    if (fase % 5 == 0) {
        int escolhido = rand() % qtdPecas;
        PecaInimiga *chefe = novaLista;
        for (int i = 0; i < escolhido; i++) chefe = chefe->proximo;
        chefe->inicial = 'R';
        chefe->ehChefe = 1;    }

    int aplicadas = 0;
    while (aplicadas < qtdMelhorias) {
        int quantidadeElegiveis = 0;
        PecaInimiga *atual = novaLista;
        while (atual != NULL) {
            if (podeReceberMelhoria(atual)) quantidadeElegiveis++;
            atual = atual->proximo;
        }
        if (quantidadeElegiveis == 0) break;

        int escolhido = rand() % quantidadeElegiveis;
        atual = novaLista;
        while (atual != NULL) {
            if (podeReceberMelhoria(atual)) {
                if (escolhido == 0) break;
                escolhido--;
            }
            atual = atual->proximo;
        }
        sortearMelhoria(atual);
        aplicadas++;
    }

    liberarPecasInimigas(head);
    *head = novaLista;
    return aplicadas;
}
