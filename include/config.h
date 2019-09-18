/* Central configuration: limits, symbols, file names, keys, and screen layout. */

#ifndef CONFIG_H
#define CONFIG_H

#define GAME_TITLE             "Tic-Tac-Toe"

// ***** Storage *****
#define LEADERBOARD_FILE       "leaderboard.txt" // Created in the folder the game is run from
#define MAX_MATCHES            1000

// ***** Players *****
#define PLAYER_NAME_MIN_LENGTH 3
#define PLAYER_NAME_MAX_LENGTH 99
#define PLAYER_NAME_SIZE       (PLAYER_NAME_MAX_LENGTH + 1)
#define SYMBOL_X               'X'
#define SYMBOL_O               'O'

// ***** Board *****
#define BOARD_SIZE             3 // Up to 3: a cell is picked with one digit, so BOARD_CELL_COUNT cannot pass 9
#define BOARD_CELL_COUNT       (BOARD_SIZE * BOARD_SIZE)

// ***** Input *****
#define KEY_NEW_MATCH          '1'
#define KEY_CONTINUE_MATCH     '2'
#define KEY_LEADERBOARD        '3'
#define KEY_EXIT               'E'
#define KEY_BACK               'B'
#define KEY_PLAY_AGAIN         'Y'
#define KEY_LIST_BACK          '0' // The list is numbered from 1, which leaves 0 free
#define INPUT_BUFFER_SIZE      128

// ***** Screen layout *****
#define SCREEN_WIDTH           80 // Drives every rule, banner, board, and table
#define CONTENT_INDENT         5
#define TABLE_NAME_WIDTH       19 // Longer names are truncated to keep rows aligned
#define BOARD_GRID_WIDTH       (BOARD_SIZE * 6 + BOARD_SIZE - 1) // Six columns per cell, plus one per divider
#define BOARD_INDENT           ((SCREEN_WIDTH - BOARD_GRID_WIDTH) / 2)
#define DISPLAY_BUFFER_SIZE    128
#define SECTION_RULE_CHAR      '-'
#define BANNER_RULE_CHAR       '='

#endif // CONFIG_H
