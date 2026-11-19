#ifndef COMBATE_H
#define COMBATE_H

#include "tabuleiro.h"

#define TIPOS_PECA 6
#define CARTAS_TIPO 10
#define MAX_ACOES 4096

typedef struct { int l, c; } Casa;
typedef struct {
    char tipo, original;
    int lado; /* 0 aliado, 1 inimigo */
    Casa origem;
    int vivo, chefe, moveu, capturou;
    int proximo; /* próximo nó da lista da equipe, índice + 1 */
    int promocaoPendente; /* peão aguarda escolha ao chegar à última fileira */
    unsigned cartas, usadas;
    int duplo, guarda, silencio, marca, herdeira, gambito;
    Casa corredor;
} Unidade;

typedef struct {
    char tipo;
    int lado, rodada, proximo;
    unsigned cartas;
    Casa casa;
} RegistroCaptura;

typedef struct {
    int largura, rodada, lado, quantidade, bonus;
    int pecasInimigas, melhoriasInimigas;
    int tab[5][8]; /* indice + 1; zero = vazio */
    Unidade unidades[40];
    int primeira[2]; /* cabeças das listas encadeadas de unidades ativas */
    unsigned adquiridas[TIPOS_PECA], usadas[TIPOS_PECA];
    unsigned reacoesDesativadas[TIPOS_PECA];
    int selada[5][8]; /* lado proibido + 1 */
    int coroacaoUsada;
    int capturas[2][TIPOS_PECA];
    RegistroCaptura historicoRodada[40];
    int primeiraCaptura, quantidadeCapturas;
} Combate;

typedef struct {
    int ator, carta; /* carta 0 = movimento base, IDs do plano */
    Casa destino, extra, extra2;
} Acao;

typedef struct {
    Acao itens[MAX_ACOES];
    int n;
} ListaAcoes;

typedef struct { char tipo; int id; const char *nome, *descricao; } Carta;
extern const Carta catalogo[TIPOS_PECA][CARTAS_TIPO];
int indiceTipo(char tipo);
int cartaReativa(char tipo, int id);
const Carta *buscarCarta(char tipo, int id);
int temCarta(const Combate *j, int u, int id);
int usouCarta(const Combate *j, int u, int id);
void gastarCarta(Combate *j, int u, int id);
int sortearCartas(const Combate *j, const Carta **saida, int limite);
int adquirirCarta(Combate *j, const Carta *carta);

int dentro(const Combate *j, Casa p);
int igualCasa(Casa a, Casa b);
int ocupante(const Combate *j, Casa p); /* -1 se vazio/fora */
Casa posicao(const Combate *j, int u);
int vazio(const Combate *j, Casa p);
int aliado(const Combate *j, int u, int v);
int inimigo(const Combate *j, int u, int v);
int ortogonal(Casa a, Casa b);
int diagonal(Casa a, Casa b);
int adjacente(Casa a, Casa b);
int caminho(const Combate *j, Casa a, Casa b);
int podeCapturar(const Combate *j, int u, int v);
int movimentoBase(const Combate *j, int u, Casa destino);
int ameaca(const Combate *j, int u, Casa destino);
int ameacado(const Combate *j, int u);
int contarLado(const Combate *j, int lado);
void adicionar(ListaAcoes *lista, int u, int carta, Casa destino, Casa extra, Casa extra2);
int deslocar(Combate *j, int u, Casa p);
int trocar(Combate *j, int u, int v);
int capturar(Combate *j, int u, int v); /* captura e move; pode ser interceptada */
void remover(Combate *j, int u);
void tocar(Combate *j, int u);
void promover(Combate *j, int u, char tipo, int permanente);
/* A escolha de promoção não consome uma ação nem altera o lado atual.
 * Retorna o índice de um peão aguardando promoção, ou -1. */
int promocaoPendente(const Combate *j);
int escolherPromocao(Combate *j, int u, char tipo);
int moverOuCapturar(Combate *j, int u, Casa p);
void gerarAcoes(const Combate *j, int u, ListaAcoes *lista);
int executarAcao(Combate *j, Acao a);
/* Executa uma acao gerada por gerarAcoes sem repetir a validacao. */
int executarAcaoGerada(Combate *j, Acao a);
void iniciarCombate(Combate *j, char tipo);
int prepararRodada(Combate *j);
int recrutar(Combate *j, char tipo, Casa p);
void encerrarAcao(Combate *j, int ator);

/* Cada modulo gera alternativas explicitas e executa apenas seus IDs.
 * Acoes com carta 0 sao base. Reacoes anteriores/depois da captura retornam
 * 1 se cancelaram a captura; as posteriores podem alterar o tabuleiro. */
void gerarPC(const Combate *, int, ListaAcoes *);
int executarPC(Combate *, Acao);
void depoisPC(Combate *, int, Casa, int);
int antesPC(Combate *, int, int);
void aposCapturaPC(Combate *, int, int, Casa);
void gerarTB(const Combate *, int, ListaAcoes *);
int executarTB(Combate *, Acao);
void depoisTB(Combate *, int, Casa, int);
int antesTB(Combate *, int, int);
void aposCapturaTB(Combate *, int, int, Casa);
void gerarRD(const Combate *, int, ListaAcoes *);
int executarRD(Combate *, Acao);
void depoisRD(Combate *, int, Casa, int);
int antesRD(Combate *, int, int);
void aposCapturaRD(Combate *, int, int, Casa);
void efeitosMorteRD(Combate *, int, int);

#endif
