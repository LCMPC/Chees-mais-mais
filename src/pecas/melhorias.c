#include <stdlib.h>
#include <stddef.h>

#include "../campo/combate.h"

const Carta catalogo[TIPOS_PECA][CARTAS_TIPO] = {
    {
        {'P', 1, "Baioneta", "Com um inimigo à frente e um aliado ao lado, pode capturar em linha reta."},
        {'P', 2, "Passo de lado", "Anda uma casa para a esquerda ou direita, se estiver vazia."},
        {'P', 3, "Meia-volta", "Recua uma casa em linha reta para uma casa vazia."},
        {'P', 4, "Marcha longa", "Pode avançar duas casas sempre que ambas estiverem livres."},
        {'P', 5, "Salto de trincheira", "Salta sobre uma peça à frente e pousa na casa vazia logo atrás dela."},
        {'P', 11, "Flanquear", "Com a frente bloqueada, pode avançar na diagonal para uma casa vazia."},
        {'P', 12, "Guarda lateral", "Pode recapturar imediatamente o inimigo que capturou um aliado ao lado. 1x/rodada."},
        {'P', 16, "Veterano", "Após a primeira captura no combate, também captura nas diagonais para trás."},
        {'P', 20, "Coroação", "O primeiro peão que alcançar a última fileira após a compra torna-se rainha pelo restante da run."},
        {'P', 21, "Ascensão real", "Um peão aliado aleatório transforma-se imediatamente em rainha pelo restante da run."}
    },
    {
        {'T', 4, "Guincho", "Puxa o primeiro aliado visível em linha reta até a casa vazia junto à torre."},
        {'T', 5, "Catapulta", "Lança um aliado adjacente em linha reta até três casas além dele, saltando obstáculos. 1x/rodada."},
        {'T', 7, "Bate e volta", "Após capturar, pode recuar pelo mesmo caminho até uma casa que atravessou. 1x/rodada."},
        {'T', 8, "Roque de campanha", "Troca de lugar com um rei aliado na mesma linha ou coluna, com caminho livre."},
        {'T', 9, "Esquina", "Ao terminar um movimento sem captura numa borda, pode deslizar mais uma casa vazia pela borda."},
        {'T', 14, "Comboio", "Ao mover em linha reta sem capturar, leva o aliado que estava logo atrás para a casa imediatamente atrás dela."},
        {'T', 15, "Bastião", "Na origem, pode cancelar uma tentativa de captura. O atacante fica na partida e gasta a ação. 1x/rodada."},
        {'T', 16, "Curva fechada", "Pode fazer uma curva de 90 graus num movimento sem captura de até três casas com caminho livre."},
        {'T', 18, "Torres gêmeas", "Outra torre aliada na coluna vizinha pode acompanhá-la paralelamente, com caminho e destino livres."},
        {'T', 20, "Último reduto", "Quando é a única aliada viva, ganha um passo diagonal de uma casa, inclusive para capturar."}
    },
    {
        {'B', 1, "Conversão", "Gasta uma ação para dar um passo ortogonal a uma casa vazia e trocar de cor."},
        {'B', 2, "Reflexo", "Faz uma curva diagonal ao tocar uma borda. Captura apenas na casa final e exige caminho livre."},
        {'B', 4, "Confessionário", "Troca de lugar com o primeiro aliado visível numa diagonal."},
        {'B', 5, "Peregrinação", "Ao terminar num canto, pode saltar para outro canto vazio na mesma ação. 1x/rodada."},
        {'B', 6, "Convergência", "Salta para casa vazia da sua diagonal, mesmo além de bloqueio, se torre aliada tiver linha livre até ela."},
        {'B', 7, "Penitência", "Depois de capturar, pode recuar uma casa pela diagonal de onde veio."},
        {'B', 11, "Excomunhão", "Desativa as melhorias de um inimigo visível na diagonal até a próxima ação aliada. Custa uma ação."},
        {'B', 13, "Diagonal selada", "Marca uma casa vazia na diagonal. Inimigos podem atravessá-la, mas não terminar nela até a próxima ação aliada."},
        {'B', 16, "Martírio", "Pode ser sacrificado em lugar de um aliado visível na diagonal que seria capturado. 1x/rodada."},
        {'B', 19, "Altar", "Num canto, pode tocar um aliado na diagonal livre e conceder o bônus normal do rei. 1x/rodada."}
    },
    {
        {'C', 1, "Salto longo", "Salta três casas em um eixo e uma no outro para uma casa vazia."},
        {'C', 2, "Salto curto", "Salta exatamente duas casas na horizontal ou vertical até uma casa vazia."},
        {'C', 3, "Desmontar", "Anda uma casa ortogonalmente para um destino vazio."},
        {'C', 5, "Bote e fuga", "Ao capturar em L, pode voltar imediatamente à casa de partida. 1x/rodada."},
        {'C', 6, "Revezamento de sela", "Troca de lugar com um aliado a um salto normal de distância."},
        {'C', 7, "Resgate", "Troca de lugar com aliado ameaçado a um L. O aliado pode dar um passo extra para casa vazia. 1x/rodada."},
        {'C', 8, "Coice", "Após salto sem captura, empurra todos os inimigos ortogonalmente adjacentes uma casa para fora, se os destinos estiverem livres."},
        {'C', 10, "Cavalaria em dupla", "Após salto sem captura, outro cavalo que alcance a casa deixada pode ocupá-la imediatamente."},
        {'C', 16, "Cascos leves", "Após pousar num canto, pode dar um passo ortogonal para uma casa vazia."},
        {'C', 19, "Montaria", "Um peão adjacente pode acompanhar o salto, pousando na casa vazia atrás do destino em seu sentido de avanço."}
    },
    {
        {'R', 1, "Cetro longo", "Toca um aliado a duas casas em linha reta, se a casa intermediária estiver livre."},
        {'R', 2, "Audiência", "Traz um aliado visível na mesma linha ou coluna até uma casa vazia junto ao rei."},
        {'R', 5, "Rei guerreiro", "Quando sozinho, pode avançar duas casas em linha reta ou diagonal livre, inclusive para capturar."},
        {'R', 7, "Comando à distância", "Toca um aliado em qualquer casa do tabuleiro. 1x/rodada."},
        {'R', 11, "Decreto de silêncio", "Toca inimigo adjacente, desativando suas melhorias até o fim da próxima ação inimiga."},
        {'R', 13, "Trono", "Na origem, pode tocar o primeiro aliado visível em qualquer diagonal."},
        {'R', 15, "Vanguarda", "Ao capturar, pode tocar um aliado adjacente à chegada. 1x/rodada."},
        {'R', 17, "Ordem de contenção", "Concede guarda em vez de jogada dupla: o aliado recaptura inimigo que capturar aliado ao seu alcance."},
        {'R', 19, "Sucessão", "Se o rei for capturado, um peão vivo escolhido torna-se rei até o fim da rodada. 1x/rodada."},
        {'R', 20, "Duelo de reis", "Quando o chefe estiver sozinho, pode capturá-lo de duas casas em linha reta ou diagonal livre."}
    },
    {
        {'D', 4, "Dança da corte", "Troca de lugar com o primeiro aliado visível em qualquer uma das oito direções."},
        {'D', 6, "Regência", "Comanda um peão visível a mover ou capturar usando sua ação. Pode ocupar a casa deixada pelo peão."},
        {'D', 7, "Coroa de espinhos", "Ao ser capturada, marca o atacante. Na próxima ação aliada, um peão adjacente pode capturá-lo em qualquer direção."},
        {'D', 9, "Gambito da dama", "Prepara sacrifício. Se for capturada na próxima ação inimiga, ela e o atacante são eliminados. 1x/rodada."},
        {'D', 11, "Espelho", "Na borda, pode saltar à casa correspondente vazia da borda oposta. 1x/rodada."},
        {'D', 13, "Escolha da herdeira", "Toca um peão adjacente, que poderá mover ou capturar como rainha uma vez. 1x/rodada."},
        {'D', 16, "Linha de sucessão", "Se for capturada, torre ou bispo aliado com caminho livre pode recapturar o atacante. 1x/rodada."},
        {'D', 17, "Corredor real", "Liga sua casa à de aliado visível, que poderá deslizar até uma casa vazia entre os dois na próxima ação."},
        {'D', 19, "Baile de máscaras", "Troca as posições de dois aliados visíveis em direções opostas, mantendo-se parada. 1x/rodada."},
        {'D', 20, "Xeque descoberto", "Quando aliado revelar caminho livre entre rainha e inimigo, ela pode capturá-lo imediatamente. 1x/rodada."}
    }
};

int indiceTipo(char tipo) {
    switch (tipo) {
        case 'P': return 0;
        case 'T': return 1;
        case 'B': return 2;
        case 'C': return 3;
        case 'R': return 4;
        case 'D': return 5;
        default: return -1;
    }
}

const Carta *buscarCarta(char tipo, int id) {
    int t = indiceTipo(tipo);
    if (t < 0) return NULL;
    for (int i = 0; i < CARTAS_TIPO; i++) {
        if (catalogo[t][i].id == id) return &catalogo[t][i];
    }
    return NULL;
}

int cartaReativa(char tipo, int id) {
    return (tipo == 'P' && id == 12) || (tipo == 'T' && id == 15)
        || (tipo == 'B' && id == 16) || (tipo == 'R' && id == 19)
        || (tipo == 'D' && (id == 16 || id == 20));
}

int temCarta(const Combate *j, int u, int id) {
    if (!j || u < 0 || u >= j->quantidade) return 0;
    const Unidade *p = &j->unidades[u];
    int tipo = indiceTipo(p->tipo);
    if (tipo < 0 || !buscarCarta(p->tipo, id) || p->silencio) return 0;
    if (p->lado && (p->chefe || p->tipo == 'D')) return 0;
    if (!p->lado && cartaReativa(p->tipo, id)
        && (j->reacoesDesativadas[tipo] & (1u << id))) return 0;
    unsigned cartas = p->lado ? p->cartas : j->adquiridas[tipo];
    return (cartas & (1u << id)) != 0;
}

int usouCarta(const Combate *j, int u, int id) {
    if (!j || u < 0 || u >= j->quantidade) return 0;
    const Unidade *p = &j->unidades[u];
    int tipo = indiceTipo(p->tipo);
    if (tipo < 0 || !buscarCarta(p->tipo, id)) return 0;
    unsigned usadas = p->lado ? p->usadas : j->usadas[tipo];
    return (usadas & (1u << id)) != 0;
}

void gastarCarta(Combate *j, int u, int id) {
    if (!temCarta(j, u, id)) return;
    Unidade *p = &j->unidades[u];
    if (p->lado) p->usadas |= 1u << id;
    else j->usadas[indiceTipo(p->tipo)] |= 1u << id;
}

static int existeAliado(const Combate *j, char tipo) {
    for (int u = 0; u < j->quantidade; u++) {
        const Unidade *p = &j->unidades[u];
        if (p->vivo && !p->lado && p->tipo == tipo) return 1;
    }
    return 0;
}

int sortearCartas(const Combate *j, const Carta **saida, int limite) {
    if (!j || !saida || limite <= 0) return 0;
    const Carta *disponiveis[TIPOS_PECA * CARTAS_TIPO];
    int total = 0;
    for (int t = 0; t < TIPOS_PECA; t++) {
        if (!existeAliado(j, catalogo[t][0].tipo)) continue;
        for (int i = 0; i < CARTAS_TIPO; i++) {
            const Carta *c = &catalogo[t][i];
            if (!(j->adquiridas[t] & (1u << c->id))) disponiveis[total++] = c;
        }
    }
    int quantidade = 0;
    while (quantidade < limite && total > 0) {
        int escolha = rand() % total;
        saida[quantidade++] = disponiveis[escolha];
        disponiveis[escolha] = disponiveis[--total];
    }
    return quantidade;
}

int adquirirCarta(Combate *j, const Carta *carta) {
    if (!j || !carta || !buscarCarta(carta->tipo, carta->id)) return 0;
    int tipo = indiceTipo(carta->tipo);
    if (!existeAliado(j, carta->tipo) || (j->adquiridas[tipo] & (1u << carta->id))) return 0;

    if (carta->tipo == 'P' && carta->id == 21) {
        int peoes[40];
        int total = 0;
        for (int u = 0; u < j->quantidade; u++) {
            const Unidade *p = &j->unidades[u];
            if (p->vivo && !p->lado && p->tipo == 'P') peoes[total++] = u;
        }
        if (!total) return 0;
        promover(j, peoes[rand() % total], 'D', 1);
    }
    j->adquiridas[tipo] |= 1u << carta->id;
    return 1;
}
