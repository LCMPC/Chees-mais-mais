#ifndef CHESS_RANKING_H
#define CHESS_RANKING_H

#include <stdbool.h>
#include <stddef.h>
#include "campo/combate.h"

#define RANK_NICK_MAX 24

typedef struct {
    char nick[RANK_NICK_MAX + 1];
    int round;
    int counts[TIPOS_PECA];
    unsigned upgrades[TIPOS_PECA];
} RankingEntry;

typedef struct {
    RankingEntry *entries;
    size_t count;
} Ranking;

/* Inicializa um ranking vazio se o arquivo ainda não existir. */
bool RankingLoad(Ranking *r);
void RankingFree(Ranking *r);
bool RankingNickExists(const Ranking *r, const char *nick);
/* Grava a nova run e mantém as anteriores em ordem decrescente de rodada. */
bool RankingSaveRun(Ranking *r, const RankingEntry *entry);
void RankingRandomNick(const Ranking *r, char out[RANK_NICK_MAX + 1]);

#endif
