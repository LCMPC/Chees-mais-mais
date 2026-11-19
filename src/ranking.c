#include "ranking.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define RANK_FILE "ranking.txt"
#define RANK_TEMP "ranking.txt.tmp"

static const char *const piece_names[TIPOS_PECA] = {
    "peao", "torre", "bispo", "cavalo", "rei", "rainha"
};

/* 100 apelidos: referências aos desenhos, misturadas com piadas curtas. */
static const char *const random_nicks[100] = {
    "BobXadrez", "PatrickXP", "LulaTilt", "SiriMao", "PlanktonGG",
    "GaryVeloz", "SandyCrit", "HolandesVo", "CalcaQuadrada", "KrabbyPro",
    "AangAir", "ZukoTilt", "TophTank", "KataraHeal", "SokkaBoom",
    "IrohTea", "AppaZoom", "MomoMestre", "AzulaCrit", "CouveNinja",
    "BenTornado", "GwenMana", "KevinPedra", "VovoMax", "ChamaRush",
    "QuatroBracos", "DiamanteGG", "MassaCinzenta", "UltraToma", "BalaDeCanhao",
    "FinnGG", "JakeElastico", "BMOzinho", "ReiGelado", "MarcelineBass",
    "JujubaLab", "LichLag", "CarocoReal", "GunterOP", "LimonGrab",
    "MordecaiAFK", "RigbyRush", "BensonTilt", "MusculosoXP", "Fantasmao",
    "SaltitaoGG", "PopsPaz", "MaellardGold", "EuSouAMorte", "CafeinaXP",
    "GumballTilt", "DarwinDash", "AnaisBrain", "RicardoTank", "NicoleOP",
    "TobiasFlash", "TinaRex", "BananaJoeGG", "PennyPixel", "CarrieGhost",
    "DipperCode", "MabelGlitter", "SoosPro", "TioStanGold", "BillTri",
    "WendyMachado", "Waddles", "FordMind", "PacificaXP", "GideaoGamer",
    "KimPossivelGG", "RonDeuRuim", "RufusCrit", "ShegoRush", "DrakkenLab",
    "PhineasBuild", "FerbQuieto", "PerryStealth", "DoofInador", "CandaceTilt",
    "NumbuhOne", "KukiRainbow", "HoagieVoa", "WallabeeRush", "AbbyScout",
    "MandyManda", "BillyBoboca", "PuroOssoGG", "IrwinTilt", "FredFredburg",
    "SamuraiJack", "AkuSombrio", "JohnnyTestXP", "DukeyGG", "DexterLab",
    "DeeDeeChaos", "MegasXLR", "ChowderSnack", "PaniniRush", "SchnitzelRada"
};

static bool same_nick(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
        ++a;
        ++b;
    }
    return *a == *b;
}

static bool valid_nick(const char *nick) {
    size_t length = 0;
    bool nonspace = false;
    while (length <= RANK_NICK_MAX && nick[length]) {
        unsigned char c = (unsigned char)nick[length++];
        if (iscntrl(c)) return false;
        if (!isspace(c)) nonspace = true;
    }
    return nonspace && length <= RANK_NICK_MAX;
}

bool RankingNickExists(const Ranking *r, const char *nick) {
    for (size_t i = 0; i < r->count; ++i)
        if (same_nick(r->entries[i].nick, nick)) return true;
    return false;
}

void RankingRandomNick(const Ranking *r, char out[RANK_NICK_MAX + 1]) {
    const char *base = random_nicks[rand() % (sizeof random_nicks / sizeof *random_nicks)];
    snprintf(out, RANK_NICK_MAX + 1, "%s", base);
    for (size_t n = 1; RankingNickExists(r, out); ++n) {
        char suffix[32];
        snprintf(suffix, sizeof suffix, "%zu", n);
        int prefix = RANK_NICK_MAX - (int)strlen(suffix);
        snprintf(out, RANK_NICK_MAX + 1, "%.*s%s", prefix, base, suffix);
    }
}

void RankingFree(Ranking *r) {
    free(r->entries);
    r->entries = NULL;
    r->count = 0;
}

/* Insere depois dos empates: a run mais antiga preserva sua colocação. */
static bool insert_entry(Ranking *r, const RankingEntry *entry) {
    if (r->count >= (size_t)-1 / sizeof *r->entries) return false;
    RankingEntry *entries = realloc(r->entries, (r->count + 1) * sizeof *entries);
    if (!entries) return false;
    r->entries = entries;
    size_t i = r->count;
    while (i && r->entries[i - 1].round < entry->round) {
        r->entries[i] = r->entries[i - 1];
        --i;
    }
    r->entries[i] = *entry;
    ++r->count;
    return true;
}

/* Lê inclusive arquivos CRLF, sem aceitar linhas truncadas. */
static bool read_line(FILE *file, char *line, size_t capacity) {
    if (!fgets(line, (int)capacity, file)) return false;
    size_t n = strlen(line);
    if (n && line[n - 1] == '\n') line[--n] = '\0';
    else if (!feof(file)) return false;
    if (n && line[n - 1] == '\r') line[--n] = '\0';
    return true;
}

static bool parse_pieces(const char *line, RankingEntry *entry) {
    /* Os nomes e a rodada também ficam legíveis ao abrir o TXT. */
    int used = 0;
    if (sscanf(line, "rodada=%d; peaox%d; torrex%d; bispox%d; cavalox%d; reix%d; rainhax%d%n",
               &entry->round, &entry->counts[0], &entry->counts[1], &entry->counts[2],
               &entry->counts[3], &entry->counts[4], &entry->counts[5], &used) != 7 ||
        line[used] || entry->round < 1) return false;
    for (int i = 0; i < TIPOS_PECA; ++i)
        if (entry->counts[i] < 0 || entry->counts[i] > 40) return false;
    return true;
}

static bool parse_upgrades(char *line, RankingEntry *entry) {
    /* A máscara torna a leitura independente de alterações futuras nos nomes. */
    for (int t = 0; t < TIPOS_PECA; ++t) {
        size_t prefix = strlen(piece_names[t]);
        if (strncmp(line, piece_names[t], prefix) || line[prefix] != '=') return false;
        line += prefix + 1;
        char *end;
        errno = 0;
        unsigned long value = strtoul(line, &end, 16);
        if (errno || end == line || value > UINT_MAX || *end != '(') return false;
        entry->upgrades[t] = (unsigned)value;
        end = strchr(end, ')');
        if (!end) return false;
        if (t == TIPOS_PECA - 1) return end[1] == '\0';
        if (end[1] != ';' || end[2] != ' ') return false;
        line = end + 3;
    }
    return false;
}

bool RankingLoad(Ranking *r) {
    r->entries = NULL;
    r->count = 0;
    FILE *file = fopen(RANK_FILE, "r");
    if (!file) return errno == ENOENT;
    bool ok = true;
    char line[4096];
    while (read_line(file, line, sizeof line)) {
        RankingEntry entry = {0};
        if (!valid_nick(line) || RankingNickExists(r, line)) { ok = false; break; }
        memcpy(entry.nick, line, strlen(line) + 1);
        if (!read_line(file, line, sizeof line) || !parse_pieces(line, &entry) ||
            !read_line(file, line, sizeof line) || !parse_upgrades(line, &entry) ||
            !read_line(file, line, sizeof line) || *line || !insert_entry(r, &entry)) {
            ok = false;
            break;
        }
    }
    if (!feof(file) || ferror(file)) ok = false;
    if (fclose(file) != 0) ok = false;
    if (!ok) RankingFree(r);
    return ok;
}

static bool write_entry(FILE *file, const RankingEntry *entry) {
    if (fprintf(file, "%s\nrodada=%d", entry->nick, entry->round) < 0) return false;
    for (int t = 0; t < TIPOS_PECA; ++t)
        if (fprintf(file, "; %sx%d", piece_names[t], entry->counts[t]) < 0) return false;
    if (fputc('\n', file) == EOF) return false;
    for (int t = 0; t < TIPOS_PECA; ++t) {
        if (fprintf(file, "%s%s=%x(", t ? "; " : "", piece_names[t], entry->upgrades[t]) < 0)
            return false;
        bool first = true;
        for (int c = 0; c < CARTAS_TIPO; ++c) {
            const Carta *card = &catalogo[t][c];
            if (!(entry->upgrades[t] & (1u << card->id))) continue;
            if (fprintf(file, "%s%s", first ? "" : ", ", card->nome) < 0) return false;
            first = false;
        }
        if (fputc(')', file) == EOF) return false;
    }
    return fputs("\n\n", file) != EOF;
}

static bool write_ranking(const Ranking *r) {
    FILE *file = fopen(RANK_TEMP, "w");
    if (!file) return false;
    bool ok = true;
    for (size_t i = 0; i < r->count && ok; ++i) ok = write_entry(file, &r->entries[i]);
    if (fclose(file) != 0) ok = false;
    if (ok) {
#ifdef _WIN32
        ok = MoveFileExA(RANK_TEMP, RANK_FILE, MOVEFILE_REPLACE_EXISTING) != 0;
#else
        ok = rename(RANK_TEMP, RANK_FILE) == 0;
#endif
    }
    if (!ok) remove(RANK_TEMP);
    return ok;
}

bool RankingSaveRun(Ranking *r, const RankingEntry *entry) {
    if (!valid_nick(entry->nick) || entry->round < 1 || RankingNickExists(r, entry->nick))
        return false;
    for (int t = 0; t < TIPOS_PECA; ++t)
        if (entry->counts[t] < 0 || entry->counts[t] > 40) return false;
    /* Se outra instância gravou durante a run, não apagamos o resultado dela. */
    Ranking fresh;
    if (!RankingLoad(&fresh)) return false;
    if (RankingNickExists(&fresh, entry->nick) || !insert_entry(&fresh, entry) || !write_ranking(&fresh)) {
        RankingFree(&fresh);
        return false;
    }
    RankingFree(r);
    *r = fresh;
    return true;
}
