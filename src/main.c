/* Program entry point: wires the UI, game rules, and storage into the menu and match flows. */

#include <stdio.h>

#include "config.h"
#include "game.h"
#include "storage.h"
#include "ui.h"

static void playMatch(Match *match) {
    for (;;) {
        drawBoard(match);

        const Player *mover = playerOnTurn(match);
        int cell = promptMove(match);
        if (cell == 0) {
            saveMatches(); // Leaving mid-round: keep the board resumable
            return;
        }

        RoundResult result = playMove(match, cell);
        if (result == ROUND_CONTINUE) {
            continue;
        }

        saveMatches();
        if (result == ROUND_WON) {
            showRoundWin(match, mover);
        } else {
            showRoundDraw(match);
        }

        if (!promptPlayAgain()) {
            return;
        }
    }
}

static void startNewMatchFlow(void) {
    char name1[PLAYER_NAME_SIZE];
    char name2[PLAYER_NAME_SIZE];

    if (!promptPlayerName(1, name1)) {
        return;
    }

    char symbol1 = promptSymbolChoice(name1);
    if (symbol1 == 0) {
        return;
    }

    if (!promptPlayerName(2, name2)) {
        return;
    }

    char symbol2 = opposingSymbol(symbol1);
    showAssignedSymbol(name2, symbol2);

    int firstMover = promptFirstMover(name1, symbol1, name2, symbol2);
    if (firstMover == 0) {
        return;
    }

    Match *match = createMatch(name1, symbol1, name2, firstMover);
    if (match == NULL) {
        showMatchListFull();
        return;
    }

    saveMatches();
    playMatch(match);
}

static void continueMatchFlow(void) {
    int index = promptMatchSelection();
    if (index >= 0) {
        playMatch(&matches[index]);
    }
}

int main(void) {
    loadMatches();

    for (;;) {
        switch (promptMainMenuChoice()) {
        case KEY_NEW_MATCH:
            startNewMatchFlow();
            break;
        case KEY_CONTINUE_MATCH:
            continueMatchFlow();
            break;
        case KEY_LEADERBOARD:
            showLeaderboard();
            break;
        case KEY_EXIT:
        case EOF: // End of input
            saveMatches();
            showGoodbye();
            return 0;
        default:
            showInvalidOption();
            break;
        }
    }
}
