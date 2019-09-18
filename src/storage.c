/*
 * Match persistence. Each entry is:
 *
 *   Match <n>:
 *   Player 1: <name>, Symbol: <X|O>, Wins: <n>
 *   Player 2: <name>, Symbol: <X|O>, Wins: <n>
 *   First Move: <n>, Round First Move: <n>, Current Turn: <n>, Rounds Played: <n>
 *   Board:
 *   <3 rows of 3 cells>
 */

#include "storage.h"
#include "config.h"
#include "game.h"

#include <stdio.h>

// scanf needs the name width inside the format string, so build it from the constant.
#define STRINGIFY_VALUE(value) #value
#define STRINGIFY(value)       STRINGIFY_VALUE(value)
#define NAME_SCAN_WIDTH        STRINGIFY(PLAYER_NAME_MAX_LENGTH)

static void reportWriteError(void) {
    printf("\n%*sError: could not write to %s!\n", CONTENT_INDENT, "", LEADERBOARD_FILE);
}

void saveMatches(void) {
    FILE *file = fopen(LEADERBOARD_FILE, "w");
    if (file == NULL) {
        reportWriteError();
        return;
    }

    for (int i = 0; i < matchCount; i++) {
        const Match *m = &matches[i];

        fprintf(file, "Match %d:\n", i + 1);
        fprintf(file, "Player 1: %s, Symbol: %c, Wins: %d\n", m->player1.name, m->player1.symbol, m->player1.wins);
        fprintf(file, "Player 2: %s, Symbol: %c, Wins: %d\n", m->player2.name, m->player2.symbol, m->player2.wins);
        fprintf(
            file, "First Move: %d, Round First Move: %d, Current Turn: %d, Rounds Played: %d\n",
            m->firstMover, m->currentRoundFirstMover, m->currentTurn, m->roundsPlayed
        );

        fprintf(file, "Board:\n");
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                fprintf(file, col == 0 ? "%c" : " %c", m->board[row][col]); // No trailing space
            }

            fprintf(file, "\n");
        }

        fprintf(file, "\n");
    }

    // Buffered data flushes at fclose, so either step can fail, and fclose must always run.
    int writeFailed = ferror(file);
    if (fclose(file) != 0) {
        writeFailed = 1;
    }

    if (writeFailed) {
        reportWriteError();
    }
}

// The player number is positional, so it is skipped.
static int readPlayer(FILE *file, Player *player) {
    return fscanf(
        file, " Player %*d: %" NAME_SCAN_WIDTH "[^,], Symbol: %c, Wins: %d",
        player->name, &player->symbol, &player->wins
    ) == 3;
}

static int readBoard(FILE *file, Match *match) {
    // Literal text assigns nothing, so EOF is the only failure: the file ended early.
    if (fscanf(file, " Board:") == EOF) {
        return 0;
    }

    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if (fscanf(file, " %c", &match->board[row][col]) != 1) {
                return 0;
            }
        }
    }

    return 1;
}

// Returns 0 on a malformed entry, which also stops loadMatches from looping.
// A corrupt or already decided board is reset rather than rejected.
static int readMatch(FILE *file, Match *match) {
    if (!readPlayer(file, &match->player1) || !readPlayer(file, &match->player2)) {
        return 0;
    }

    int roundFields = fscanf(
        file, " First Move: %d, Round First Move: %d, Current Turn: %d, Rounds Played: %d",
        &match->firstMover, &match->currentRoundFirstMover, &match->currentTurn, &match->roundsPlayed
    );

    if (roundFields != 4 || !isConsistentMatch(match) || !readBoard(file, match)) {
        return 0;
    }

    if (!isRoundPlayable(match)) {
        resetRound(match);
    }

    return 1;
}

void loadMatches(void) {
    matchCount = 0;

    FILE *file = fopen(LEADERBOARD_FILE, "r");
    if (file == NULL) {
        return; // Normal on first run
    }

    while (matchCount < MAX_MATCHES && fscanf(file, " Match %*d:") != EOF && readMatch(file, &matches[matchCount])) {
        matchCount++;
    }

    fclose(file);
}
