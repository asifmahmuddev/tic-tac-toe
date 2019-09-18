/* Core game types and rules. No console I/O and no file access. */

#ifndef GAME_H
#define GAME_H

#include "config.h"

typedef struct {
    char name[PLAYER_NAME_SIZE];
    char symbol; // SYMBOL_X or SYMBOL_O
    int wins;
} Player;

// A match is a series of rounds between the same two players.
typedef struct {
    Player player1;
    Player player2;
    char board[BOARD_SIZE][BOARD_SIZE]; // A player symbol, or the cell's position digit '1'-'9'
    int firstMover;                     // Who opened round 1 (1 or 2); fixed for the match
    int currentRoundFirstMover;         // Who opened the round in progress; alternates each round
    int currentTurn;                    // Whose turn it is (1 or 2)
    int roundsPlayed;                   // Wins plus draws
} Match;

typedef enum {
    ROUND_CONTINUE,
    ROUND_WON,
    ROUND_DRAW
} RoundResult;

extern Match matches[MAX_MATCHES];
extern int matchCount;

// ***** Players *****

char opposingSymbol(char symbol);

// Letters with single interior spaces, within the PLAYER_NAME length bounds.
int isValidPlayerName(const char *name);

const Player *playerOnTurn(const Match *match);

// ***** Match lifecycle *****

// Returns NULL when the match list is full.
Match *createMatch(const char *player1Name, char player1Symbol, const char *player2Name, int firstMover);

// Whether names, symbols, player numbers, and win counts are consistent. Ignores the board.
int isConsistentMatch(const Match *match);

// ***** Round state *****

// Whether the board is a legal, undecided position with the right player on turn.
int isRoundPlayable(const Match *match);

// Clears the board and gives the opening move to the round's first mover.
void resetRound(Match *match);

// Whether the cell (1 to BOARD_CELL_COUNT) is unclaimed.
int isCellFree(const Match *match, int cell);

// Claims a free cell. A win or draw scores the round and starts the next one; caller saves.
RoundResult playMove(Match *match, int cell);

#endif // GAME_H
