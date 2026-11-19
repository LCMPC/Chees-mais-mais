#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/ranking.h"

static RankingEntry run(const char *nick, int round) {
    RankingEntry entry = {0};
    snprintf(entry.nick, sizeof entry.nick, "%s", nick);
    entry.round = round;
    entry.counts[0] = 2;
    entry.counts[1] = 1;
    entry.counts[5] = 1;
    entry.upgrades[0] = (1u << 1) | (1u << 20);
    entry.upgrades[1] = 1u << 14;
    /* Uma carta de um tipo sem peças restantes também é registrada. */
    entry.upgrades[2] = 1u << 5;
    return entry;
}

static void verificar_run(const RankingEntry *actual, const RankingEntry *expected) {
    assert(strcmp(actual->nick, expected->nick) == 0);
    assert(actual->round == expected->round);
    for (int i = 0; i < TIPOS_PECA; ++i) {
        assert(actual->counts[i] == expected->counts[i]);
        assert(actual->upgrades[i] == expected->upgrades[i]);
    }
}

static void verificar_arquivo(const RankingEntry expected[], size_t count) {
    FILE *file = fopen("ranking.txt", "r");
    char line[4096];
    assert(file);
    for (size_t i = 0; i < count; ++i) {
        assert(fgets(line, sizeof line, file));
        line[strcspn(line, "\r\n")] = '\0';
        assert(strcmp(line, expected[i].nick) == 0);
        assert(fgets(line, sizeof line, file));
        char rodada[32];
        snprintf(rodada, sizeof rodada, "rodada=%d", expected[i].round);
        assert(strstr(line, rodada) && strstr(line, "peaox2") && strstr(line, "torrex1"));
        assert(fgets(line, sizeof line, file));
        assert(strstr(line, "Comboio") && strstr(line, "Baioneta"));
        assert(fgets(line, sizeof line, file));
        assert(strcmp(line, "\n") == 0 || strcmp(line, "\r\n") == 0);
    }
    assert(fgets(line, sizeof line, file) == NULL && feof(file));
    assert(fclose(file) == 0);
}

static void testar_nicks_aleatorios(void) {
    Ranking empty = {0};
    char base[RANK_NICK_MAX + 1], nick[RANK_NICK_MAX + 1], expected[RANK_NICK_MAX + 1];
    srand(42);
    RankingRandomNick(&empty, base);
    assert(base[0] && strlen(base) <= RANK_NICK_MAX - 1);

    Ranking occupied = {0};
    occupied.entries = calloc(2, sizeof *occupied.entries);
    assert(occupied.entries);
    occupied.count = 2;
    occupied.entries[0] = run(base, 1);
    snprintf(expected, sizeof expected, "%.*s1", RANK_NICK_MAX - 1, base);
    occupied.entries[1] = run(expected, 1);

    srand(42);
    RankingRandomNick(&occupied, nick);
    snprintf(expected, sizeof expected, "%.*s2", RANK_NICK_MAX - 1, base);
    assert(strcmp(nick, expected) == 0);
    assert(!RankingNickExists(&occupied, nick));
    RankingFree(&occupied);
}

int main(int argc, char **argv) {
    /* Execute pelo Makefile, que cria um diretório temporário exclusivo. */
    assert(argc == 2 && strcmp(argv[1], "--isolated") == 0);
    Ranking ranking = {0};
    assert(RankingLoad(&ranking) && ranking.count == 0);

    RankingEntry first = run("BobTeste", 4);
    RankingEntry high = run("AppaTeste", 9);
    RankingEntry tie = run("BmoTeste", 4);
    RankingEntry low = run("RigbyTeste", 1);
    assert(RankingSaveRun(&ranking, &first));
    assert(RankingSaveRun(&ranking, &high));
    assert(RankingSaveRun(&ranking, &tie));
    assert(RankingSaveRun(&ranking, &low));
    assert(RankingNickExists(&ranking, "bobteste"));
    assert(!RankingNickExists(&ranking, "BobTeste1"));
    RankingEntry duplicate = run("BOBTESTE", 99);
    assert(!RankingSaveRun(&ranking, &duplicate));
    assert(!RankingSaveRun(&ranking, &(RankingEntry){.nick = "", .round = 10}));

    RankingEntry expected[] = {high, first, tie, low};
    assert(ranking.count == 4);
    for (size_t i = 0; i < ranking.count; ++i) verificar_run(&ranking.entries[i], &expected[i]);
    verificar_arquivo(expected, 4);
    RankingFree(&ranking);

    assert(RankingLoad(&ranking) && ranking.count == 4);
    for (size_t i = 0; i < ranking.count; ++i) verificar_run(&ranking.entries[i], &expected[i]);

    /* Uma segunda instância não pode sobrescrever as runs da primeira. */
    Ranking stale = {0};
    assert(RankingLoad(&stale) && stale.count == 4);
    RankingEntry next = run("TophTeste", 10);
    assert(RankingSaveRun(&ranking, &next));
    assert(!RankingSaveRun(&stale, &next));
    RankingEntry newest = run("FinnTeste", 12);
    assert(RankingSaveRun(&stale, &newest) && stale.count == 6);
    assert(strcmp(stale.entries[0].nick, newest.nick) == 0);
    assert(strcmp(stale.entries[1].nick, next.nick) == 0);
    RankingFree(&ranking);
    RankingFree(&stale);

    testar_nicks_aleatorios();
    puts("Testes do ranking: OK");
    return 0;
}
