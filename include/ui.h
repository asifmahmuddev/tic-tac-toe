/* Console interaction: menus, prompts, board and score tables. Prompts loop until valid. */

#ifndef UI_H
#define UI_H

#include "game.h"

// ***** Main menu *****

// Returns KEY_NEW_MATCH, KEY_CONTINUE_MATCH, KEY_LEADERBOARD, KEY_EXIT, EOF, or an invalid key.
int promptMainMenuChoice(void);

void showInvalidOption(void);
void showMatchListFull(void);
void showGoodbye(void);

// ***** New match setup *****

// Stores a cleaned, valid name in nameOut. Returns 1, or 0 for "back".
int promptPlayerName(int playerNumber, char *nameOut);

// Returns SYMBOL_X, SYMBOL_O, or 0 for "back".
char promptSymbolChoice(const char *playerName);

void showAssignedSymbol(const char *playerName, char symbol);

// Returns 1, 2, or 0 for "back".
int promptFirstMover(const char *name1, char symbol1, const char *name2, char symbol2);

// ***** In-round screens *****

void drawBoard(const Match *match);

// Returns a free cell (1 to BOARD_CELL_COUNT), or 0 for "back".
int promptMove(const Match *match);

void showRoundWin(const Match *match, const Player *winner);
void showRoundDraw(const Match *match);

// Returns 1 to play the next round, 0 to go back to the menu.
int promptPlayAgain(void);

// ***** Match tables *****

void showLeaderboard(void);

// Returns the match index, or -1 for "back" or when none exist.
int promptMatchSelection(void);

#endif // UI_H
