/* Core game rules: board state, move resolution, and validation. */

#include "game.h"

#include <ctype.h>
#include <stdio.h>

Match matches[MAX_MATCHES];
int matchCount = 0;

// ***** Board helpers *****

static int isPlayerSymbol(char cell) {
    return cell == SYMBOL_X || cell == SYMBOL_O;
}

// Row 1, col 0 -> '4'.
static char boardPositionDigit(int row, int col) {
    return (char)('1' + row * BOARD_SIZE + col);
}

static void clearBoard(Match *match) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            match->board[row][col] = boardPositionDigit(row, col);
        }
    }
}

int isCellFree(const Match *match, int cell) {
    return !isPlayerSymbol(match->board[(cell - 1) / BOARD_SIZE][(cell - 1) % BOARD_SIZE]);
}

// Walks BOARD_SIZE cells from one square. Free cells hold distinct position digits, so equal cells all the way along can only be one player's symbol.
static int isWinningLine(const Match *match, int row, int col, int rowStep, int colStep) {
    char first = match->board[row][col];

    if (!isPlayerSymbol(first)) {
        return 0;
    }

    for (int step = 1; step < BOARD_SIZE; step++) {
        if (match->board[row + step * rowStep][col + step * colStep] != first) {
            return 0;
        }
    }

    return 1;
}

static int hasWinningLine(const Match *match) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (isWinningLine(match, i, 0, 0, 1) || isWinningLine(match, 0, i, 1, 0)) {
            return 1;
        }
    }

    return isWinningLine(match, 0, 0, 1, 1) || isWinningLine(match, 0, BOARD_SIZE - 1, 1, -1);
}

static int isBoardFull(const Match *match) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if (!isPlayerSymbol(match->board[row][col])) {
                return 0;
            }
        }
    }

    return 1;
}

// ***** Players *****

static int isPlayerNumber(int playerNumber) {
    return playerNumber == 1 || playerNumber == 2;
}

static int otherPlayer(int playerNumber) {
    return (playerNumber == 1) ? 2 : 1;
}

char opposingSymbol(char symbol) {
    return (symbol == SYMBOL_X) ? SYMBOL_O : SYMBOL_X;
}

int isValidPlayerName(const char *name) {
    int length = 0;
    int previousWasSpace = 1; // Rejects a leading space and an empty name

    for (const char *p = name; *p != '\0'; p++, length++) {
        if (*p == ' ') {
            if (previousWasSpace) {
                return 0;
            }

            previousWasSpace = 1;
        } else if (isalpha((unsigned char)*p)) {
            previousWasSpace = 0;
        } else {
            return 0;
        }
    }

    return !previousWasSpace && length >= PLAYER_NAME_MIN_LENGTH && length <= PLAYER_NAME_MAX_LENGTH;
}

const Player *playerOnTurn(const Match *match) {
    return (match->currentTurn == 1) ? &match->player1 : &match->player2;
}

// ***** Match lifecycle *****

Match *createMatch(const char *player1Name, char player1Symbol, const char *player2Name, int firstMover) {
    if (matchCount >= MAX_MATCHES) {
        return NULL;
    }

    Match *match = &matches[matchCount++];

    snprintf(match->player1.name, PLAYER_NAME_SIZE, "%s", player1Name);
    snprintf(match->player2.name, PLAYER_NAME_SIZE, "%s", player2Name);
    match->player1.symbol = player1Symbol;
    match->player2.symbol = opposingSymbol(player1Symbol);
    match->player1.wins = 0;
    match->player2.wins = 0;
    match->firstMover = firstMover;
    match->currentRoundFirstMover = firstMover;
    match->roundsPlayed = 0;

    resetRound(match);

    return match;
}

int isConsistentMatch(const Match *match) {
    if (!isValidPlayerName(match->player1.name) || !isValidPlayerName(match->player2.name)) {
        return 0;
    }

    if (!isPlayerSymbol(match->player1.symbol) || match->player2.symbol != opposingSymbol(match->player1.symbol)) {
        return 0;
    }

    if (!isPlayerNumber(match->firstMover) || !isPlayerNumber(match->currentRoundFirstMover) || !isPlayerNumber(match->currentTurn)) {
        return 0;
    }

    // Subtract rather than add the wins: a crafted file could overflow the sum.
    if (match->player1.wins < 0 || match->player2.wins < 0 || match->roundsPlayed < 0 || match->roundsPlayed - match->player1.wins < match->player2.wins) {
        return 0;
    }

    // startNextRound hands the opening move to the other player every round, so after an even number of rounds it is back with whoever opened the match.
    int openerNow = (match->roundsPlayed % 2 == 0) ? match->firstMover : otherPlayer(match->firstMover);

    return match->currentRoundFirstMover == openerNow;
}

// ***** Round state *****

void resetRound(Match *match) {
    clearBoard(match);
    match->currentTurn = match->currentRoundFirstMover;
}

int isRoundPlayable(const Match *match) {
    char openingSymbol = (match->currentRoundFirstMover == 1) ? match->player1.symbol : match->player2.symbol;
    int openerMoves = 0;
    int responderMoves = 0;

    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            char cell = match->board[row][col];

            if (cell == openingSymbol) {
                openerMoves++;
            } else if (isPlayerSymbol(cell)) {
                responderMoves++;
            } else if (cell != boardPositionDigit(row, col)) {
                return 0;
            }
        }
    }

    // Players alternate, so the opener is either level and about to move, or one move ahead.
    int expectedTurn;
    if (openerMoves == responderMoves) {
        expectedTurn = match->currentRoundFirstMover;
    } else if (openerMoves == responderMoves + 1) {
        expectedTurn = otherPlayer(match->currentRoundFirstMover);
    } else {
        return 0;
    }

    return match->currentTurn == expectedTurn && !hasWinningLine(match) && !isBoardFull(match);
}

// The opening player alternates so both get equal first-move advantage.
static void startNextRound(Match *match) {
    match->currentRoundFirstMover = otherPlayer(match->currentRoundFirstMover);
    match->roundsPlayed++;
    resetRound(match);
}

RoundResult playMove(Match *match, int cell) {
    // Not playerOnTurn: the win count is updated below.
    Player *mover = (match->currentTurn == 1) ? &match->player1 : &match->player2;

    match->board[(cell - 1) / BOARD_SIZE][(cell - 1) % BOARD_SIZE] = mover->symbol;

    if (hasWinningLine(match)) {
        mover->wins++;
        startNextRound(match);
        return ROUND_WON;
    }

    if (isBoardFull(match)) {
        startNextRound(match);
        return ROUND_DRAW;
    }

    match->currentTurn = otherPlayer(match->currentTurn);
    return ROUND_CONTINUE;
}
