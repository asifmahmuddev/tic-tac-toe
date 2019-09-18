/* Console screens, prompts, and input handling. Every screen is laid out inside SCREEN_WIDTH. */

#include "ui.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Lets the compiler check variadic format strings. No-op off GCC/Clang.
#if defined(__GNUC__)
#define PRINTF_FORMAT(formatIndex, firstArgIndex) __attribute__((format(printf, formatIndex, firstArgIndex)))
#else
#define PRINTF_FORMAT(formatIndex, firstArgIndex)
#endif

// ***** Layout helpers *****

static void printIndented(const char *format, ...) PRINTF_FORMAT(1, 2);
static void printCentered(const char *format, ...) PRINTF_FORMAT(1, 2);

static void printRepeated(char fill, int count) {
    for (int i = 0; i < count; i++) {
        putchar(fill);
    }
}

static void printRule(char fill) {
    printRepeated(fill, SCREEN_WIDTH);
    putchar('\n');
}

// e.g. "===== Leader Board =====".
static void printTitledRule(char fill, const char *title) {
    int titleWidth = (int)strlen(title) + 2; // One blank on each side
    int leftFill = (SCREEN_WIDTH - titleWidth) / 2;

    printRepeated(fill, leftFill);
    printf(" %s ", title);
    printRepeated(fill, SCREEN_WIDTH - titleWidth - leftFill);
    putchar('\n');
}

static void printIndented(const char *format, ...) {
    va_list args;

    printf("%*s", CONTENT_INDENT, "");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// Text too wide to centre falls back to the body margin.
static void printCentered(const char *format, ...) {
    char line[DISPLAY_BUFFER_SIZE];
    va_list args;

    va_start(args, format);
    vsnprintf(line, sizeof line, format, args);
    va_end(args);

    int padding = (SCREEN_WIDTH - (int)strlen(line)) / 2;
    printf("%*s%s\n", (padding < CONTENT_INDENT) ? CONTENT_INDENT : padding, "", line);
}

static void printHeading(const char *title) {
    char underline[DISPLAY_BUFFER_SIZE];
    size_t length = strlen(title);

    if (length >= sizeof underline) {
        length = sizeof underline - 1;
    }

    memset(underline, SECTION_RULE_CHAR, length);
    underline[length] = '\0';

    printCentered("%s", title);
    printCentered("%s", underline);
}

static void endScreen(void) {
    printRule(SECTION_RULE_CHAR);
    printf("\n");
}

static void printBackHint(void) {
    printIndented("%c = back to menu\n", KEY_BACK);
}

// ***** Low-level input helpers *****

typedef enum {
    LINE_END_OF_INPUT,
    LINE_OK,
    LINE_TOO_LONG // Did not fit the buffer; the overflow was read and discarded
} LineResult;

// Drains any overflow, so an over-long entry never leaks into the next prompt.
// A line that exactly fills the buffer is still LINE_OK.
static LineResult readLine(char *buffer, int size) {
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return LINE_END_OF_INPUT;
    }

    LineResult result = LINE_OK;
    if (strchr(buffer, '\n') == NULL) {
        int ch = getchar();
        if (ch != '\n' && ch != EOF) {
            result = LINE_TOO_LONG;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
        }
    }

    size_t len = strlen(buffer);
    while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
        buffer[--len] = '\0';
    }

    return result;
}

// The one non-blank character in `line`, or '\0' if blank or more than one.
// Stops a single-key prompt from reading "5abc" as "5".
static int lineKey(const char *line) {
    while (isspace((unsigned char)*line)) {
        line++;
    }

    // Unsigned, so a high byte is never sign-extended into a false EOF.
    int key = (unsigned char)*line;
    if (key == '\0') {
        return '\0';
    }

    line++;
    while (isspace((unsigned char)*line)) {
        line++;
    }

    return (*line == '\0') ? key : '\0';
}

// The key on the next input line, or EOF when the input stream has ended.
static int readKey(void) {
    char line[INPUT_BUFFER_SIZE];

    if (readLine(line, sizeof line) == LINE_END_OF_INPUT) {
        return EOF;
    }

    return lineKey(line);
}

// Case-insensitive, so 'b' matches 'B'.
static int keyMatches(int key, char expected) {
    return key != EOF && toupper(key) == toupper((unsigned char)expected);
}

// Requires the whole line to be the key, so "B" goes back but "Bob" does not.
static int isBackCommand(const char *line) {
    return keyMatches(lineKey(line), KEY_BACK);
}

// ***** Main menu *****

static void showMainMenu(void) {
    printRule(SECTION_RULE_CHAR);
    printf("\n");
    printCentered("%s", GAME_TITLE);
    printRule(SECTION_RULE_CHAR);
    printHeading("Main Menu");
    printf("\n");
    printIndented("%c. New game\n", KEY_NEW_MATCH);
    printIndented("%c. Continue game\n", KEY_CONTINUE_MATCH);
    printIndented("%c. Leaderboard\n", KEY_LEADERBOARD);
    printIndented("%c. Exit\n", KEY_EXIT);
    printRule(SECTION_RULE_CHAR);
    printf("\n");
}

int promptMainMenuChoice(void) {
    showMainMenu();
    printIndented("Option (%c-%c, %c): ", KEY_NEW_MATCH, KEY_LEADERBOARD, KEY_EXIT);

    int key = readKey();
    endScreen();

    return (key == EOF) ? EOF : toupper(key);
}

void showInvalidOption(void) {
    printIndented("Invalid option.\n");
    endScreen();
}

void showMatchListFull(void) {
    printf("\n");
    printIndented("Match list is full (%d).\n", MAX_MATCHES);
}

void showGoodbye(void) {
    printIndented("Closing...\n");
    endScreen();
}

// ***** New match setup *****

// Collapses runs of spaces to one and trims both ends, in place.
static void collapseSpaces(char *name) {
    int length = 0;
    int pendingSpace = 0;

    for (const char *p = name; *p != '\0'; p++) {
        if (*p == ' ') {
            pendingSpace = (length > 0); // Leading spaces are dropped
            continue;
        }

        if (pendingSpace) {
            name[length++] = ' ';
            pendingSpace = 0;
        }

        name[length++] = *p;
    }

    name[length] = '\0';
}

int promptPlayerName(int playerNumber, char *nameOut) {
    char line[PLAYER_NAME_SIZE];

    for (;;) {
        printBackHint();
        printf("\n");
        printIndented("Player %d name: ", playerNumber);

        LineResult status = readLine(line, sizeof line);
        if (status == LINE_END_OF_INPUT) {
            return 0;
        }

        printRule(SECTION_RULE_CHAR);

        if (isBackCommand(line)) {
            return 0;
        }

        // Rejected before validation: the discarded tail could have broken the rules.
        if (status == LINE_TOO_LONG) {
            printf("\n");
            printIndented("Too long (max %d characters).\n\n", PLAYER_NAME_MAX_LENGTH);
            continue;
        }

        collapseSpaces(line);
        if (isValidPlayerName(line)) {
            snprintf(nameOut, PLAYER_NAME_SIZE, "%s", line);
            return 1;
        }

        printf("\n");
        printIndented("Letters and single spaces only, at least %d characters.\n\n", PLAYER_NAME_MIN_LENGTH);
    }
}

char promptSymbolChoice(const char *playerName) {
    for (;;) {
        printf("\n");
        printBackHint();
        printf("\n");
        printIndented("%s (player 1)\n", playerName);
        printIndented("Symbol (%c/%c): ", SYMBOL_X, SYMBOL_O);

        int key = readKey();
        printRule(SECTION_RULE_CHAR);

        char symbol = keyMatches(key, SYMBOL_X) ? SYMBOL_X : (keyMatches(key, SYMBOL_O) ? SYMBOL_O : 0);
        if (symbol != 0) {
            printf("\n");
            return symbol;
        }

        if (keyMatches(key, KEY_BACK) || key == EOF) {
            return 0;
        }

        printf("\n");
        printIndented("Invalid symbol.\n");
    }
}

void showAssignedSymbol(const char *playerName, char symbol) {
    printf("\n");
    printIndented("%s (player 2), '%c'\n", playerName, symbol);
}

int promptFirstMover(const char *name1, char symbol1, const char *name2, char symbol2) {
    for (;;) {
        printf("\n");
        printHeading("Who moves first?");
        printIndented("1. %s (player 1), '%c'\n", name1, symbol1);
        printIndented("2. %s (player 2), '%c'\n", name2, symbol2);
        printf("\n");
        printBackHint();
        printf("\n");
        printIndented("Option (1-2): ");

        int key = readKey();
        printRule(SECTION_RULE_CHAR);

        if (key == '1' || key == '2') {
            return key - '0';
        }

        if (keyMatches(key, KEY_BACK) || key == EOF) {
            return 0;
        }

        printf("\n");
        printIndented("Invalid option.\n");
    }
}

// ***** In-round screens *****

static void printScoreLine(const Match *match) {
    printCentered("Score: %d - %d", match->player1.wins, match->player2.wins);
}

// The dividers on their own, which gives every cell the same height.
static void printGridSpacerLine(void) {
    printf("%*s", BOARD_INDENT, "");

    for (int col = 0; col < BOARD_SIZE - 1; col++) {
        printf("      |");
    }

    putchar('\n');
}

static void printGridRow(const Match *match, int row) {
    printf("%*s", BOARD_INDENT, "");

    for (int col = 0; col < BOARD_SIZE; col++) {
        printf("  %c", match->board[row][col]);

        if (col < BOARD_SIZE - 1) {
            printf("   |");
        }
    }

    putchar('\n');
}

static void drawGrid(const Match *match) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        printGridSpacerLine();
        printGridRow(match, row);
        printGridSpacerLine();

        if (row < BOARD_SIZE - 1) {
            printf("%*s", BOARD_INDENT, "");
            printRepeated(SECTION_RULE_CHAR, BOARD_GRID_WIDTH);
            putchar('\n');
        }
    }
}

void drawBoard(const Match *match) {
    printf("\n");
    printCentered("%s", GAME_TITLE);
    printRule(SECTION_RULE_CHAR);
    printIndented("%s (player 1), '%c'\n", match->player1.name, match->player1.symbol);
    printIndented("%s (player 2), '%c'\n", match->player2.name, match->player2.symbol);
    printIndented(
        "First move: %s (player %d)\n",
        (match->currentRoundFirstMover == 1) ? match->player1.name : match->player2.name, match->currentRoundFirstMover
    );

    printf("\n");
    printScoreLine(match);
    printRule(SECTION_RULE_CHAR);
    printf("\n");

    drawGrid(match);
}

int promptMove(const Match *match) {
    const Player *mover = playerOnTurn(match);
    int playerNumber = match->currentTurn;

    for (;;) {
        printf("\n");
        printIndented("%s (player %d), '%c'\n", mover->name, playerNumber, mover->symbol);
        printIndented("Move: ");

        int key = readKey();

        if (key > '0' && key <= '0' + BOARD_CELL_COUNT) {
            int cell = key - '0';
            if (isCellFree(match, cell)) {
                return cell;
            }

            printf("\n");
            printIndented("Cell taken.\n");
        } else if (keyMatches(key, KEY_BACK) || key == EOF) {
            return 0;
        } else {
            printf("\n");
            printIndented("Invalid choice.\n");
        }
    }
}

static void printRoundFooter(const Match *match) {
    printf("\n");
    printScoreLine(match);
    endScreen();
}

void showRoundWin(const Match *match, const Player *winner) {
    int playerNumber = (winner == &match->player1) ? 1 : 2;

    printRule(SECTION_RULE_CHAR);
    printf("\n\n");
    printIndented("%s (player %d) wins!\n", winner->name, playerNumber);
    printRoundFooter(match);
}

void showRoundDraw(const Match *match) {
    printRule(SECTION_RULE_CHAR);
    printf("\n\n");
    printCentered("Draw!");
    printRoundFooter(match);
}

int promptPlayAgain(void) {
    for (;;) {
        printf("\n");
        printHeading("Play again!");
        printCentered("%c. Yes", KEY_PLAY_AGAIN);
        printf("\n");
        printBackHint();
        printf("\n");
        printIndented("%c = play again: ", KEY_PLAY_AGAIN);

        int key = readKey();
        printRule(SECTION_RULE_CHAR);

        if (keyMatches(key, KEY_PLAY_AGAIN)) {
            return 1;
        }

        if (keyMatches(key, KEY_BACK) || key == EOF) {
            return 0;
        }

        printf("\n");
        printIndented("Invalid choice.\n");
    }
}

// ***** Match tables *****

static void printEmptyTableNotice(void) {
    printf("\n");
    printCentered("No saved games.");
}

static void printScoreTable(void) {
    printf(
        "\n  %-5s%-*s %-*s  %-7s %-7s %6s %6s\n\n", "Game", TABLE_NAME_WIDTH, "Player 1",
        TABLE_NAME_WIDTH, "Player 2", "Symbols", "Wins", "Draws", "Total"
    );

    for (int i = 0; i < matchCount; i++) {
        const Match *m = &matches[i];
        int draws = m->roundsPlayed - (m->player1.wins + m->player2.wins);

        printf(
            "  %2d.  %-*.*s %-*.*s  %c - %c   %2d - %-2d %6d %6d\n", i + 1, TABLE_NAME_WIDTH, TABLE_NAME_WIDTH,
            m->player1.name, TABLE_NAME_WIDTH, TABLE_NAME_WIDTH, m->player2.name, m->player1.symbol, m->player2.symbol,
            m->player1.wins, m->player2.wins, draws, m->roundsPlayed
        );
    }
}

void showLeaderboard(void) {
    printf("\n\n");
    printTitledRule(BANNER_RULE_CHAR, "Leader Board");

    if (matchCount > 0) {
        printScoreTable();
    } else {
        printEmptyTableNotice();
    }

    printf("\n");
    printTitledRule(BANNER_RULE_CHAR, "End of Leader Board");
    printf("\n");
}

int promptMatchSelection(void) {
    if (matchCount == 0) {
        printf("\n\n");
        printTitledRule(SECTION_RULE_CHAR, "Continue Game");
        printEmptyTableNotice();
        endScreen();
        return -1;
    }

    char line[INPUT_BUFFER_SIZE];

    for (;;) {
        printf("\n\n");
        printTitledRule(SECTION_RULE_CHAR, "Continue Game");
        printScoreTable();
        printf("\n\n");
        printIndented("%c or %c = back to menu\n", KEY_LIST_BACK, KEY_BACK);
        printf("\n");
        printIndented("Game number: ");

        if (readLine(line, sizeof line) == LINE_END_OF_INPUT) {
            return -1;
        }

        if (isBackCommand(line) || keyMatches(lineKey(line), KEY_LIST_BACK)) {
            return -1;
        }

        // Accept a whole number only, with nothing but spacing after it.
        char *end;
        long choice = strtol(line, &end, 10);
        while (isspace((unsigned char)*end)) {
            end++;
        }

        if (end != line && *end == '\0' && choice >= 1 && choice <= matchCount) {
            return (int)choice - 1;
        }

        printf("\n");
        printIndented("Invalid choice.\n");
        endScreen();
    }
}
