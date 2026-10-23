#ifndef PECA_INIMIGA_H
#define PECA_INIMIGA_H

#define MAX_MELHORIAS_INIMIGAS 10

typedef struct PecaInimiga {
    char inicial; /* P = peao, B = bispo, T = torre, C = cavalo, D = rainha, R = rei */
    int posicaoInicial[2]; /* [linha][coluna], sem duplicar posicao atual */
    int melhorias[MAX_MELHORIAS_INIMIGAS]; /* IDs 1 a 10; zero = vazio */
    int quantidadeMelhorias;
    int ehChefe; /* 1 apenas para o rei especial da bossfight; 0 nos demais */
    struct PecaInimiga *proximo;
} PecaInimiga;

/* Cria uma unidade sem melhorias e ainda sem posicionamento. */
PecaInimiga *criarPecaInimiga(PecaInimiga **head, int fase);
void liberarPecasInimigas(PecaInimiga **head);

/* Substitui a lista anterior somente se conseguir criar a rodada completa.
 * Retorna melhorias aplicadas (pode ser menor que o orcamento), ou -1 em erro.
 * Os contadores recebidos nao sao alterados aqui.
 * Posiciona inimigos nas linhas 0, 1 e 2, reservando 3 e 4 para aliados.
 * Inicializar srand UMA vez no inicio do programa, antes dos sorteios.
 */
int criarRodadaInimiga(PecaInimiga **head, int fase,
                      int qtdMelhorias, int qtdPecas);

#endif
