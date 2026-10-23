#include <stdlib.h>

typedef struct {
    char inicial;
    int posicaoInicial[2];
    int melhorias[10];
    struct Peca * proximo;
}Peca;

Peca * criarPeca (Peca ** head, char inicial, int melhorias[10]) {
    if (head == NULL) return NULL;

    Peca * nova = malloc(sizeof(Peca));
    if (nova == NULL) return NULL;

    nova->inicial = inicial;
    nova->posicaoInicial[0] = -1;
    nova->posicaoInicial[1] = -1;
    for (int i = 0; i < 10; i++) {
        nova->melhorias[i] = melhorias[i];
    }

    nova->proximo = *head;
    *head = nova;
    return nova;
}
