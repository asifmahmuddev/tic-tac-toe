# Tic-Tac-Toe

A two-player Tic-Tac-Toe game for the console, written in pure C (C99) with no external dependencies. Matches are saved to disk automatically, so you can quit anytime and resume later.

```text
                                  Tic-Tac-Toe
--------------------------------------------------------------------------------
     John Smith (player 1), 'X'
     Mary Hill (player 2), 'O'
     First move: John Smith (player 1)

                                  Score: 7 - 4
--------------------------------------------------------------------------------

                                    |      |
                                X   |  O   |  3
                                    |      |
                              --------------------
                                    |      |
                                4   |  X   |  6
                                    |      |
                              --------------------
                                    |      |
                                7   |  8   |  O
                                    |      |

     John Smith (player 1), 'X'
     Move:
```

## Features

- **Two-player matches** — enter both names, pick X or O, and choose who moves first.
- **Multiple rounds per match** — play as many rounds as you like; the opening player alternates each round for fairness.
- **Automatic saving** — every finished round (and even a game left midway) is saved to `leaderboard.txt`.
- **Resume anytime** — continue any saved match later, exactly from the board position you left.
- **Leaderboard** — a score table of all matches showing wins, draws, and total rounds.
- **Robust input handling** — invalid names, symbols, moves, and menu choices are rejected with clear messages; save data that is corrupt or already decided is repaired on load.

## Game Screens

Starting the game brings up the main menu:

```text
--------------------------------------------------------------------------------

                                  Tic-Tac-Toe
--------------------------------------------------------------------------------
                                   Main Menu
                                   ---------

     1. New game
     2. Continue game
     3. Leaderboard
     E. Exit
--------------------------------------------------------------------------------

     Option (1-3, E):
```

Each free cell shows the number you type to claim it. Completing a row, column, or diagonal wins the round:

```text
     Ted (player 1) wins!

                                  Score: 1 - 0
```

The leaderboard summarizes every match — wins for each player, draws, and total rounds:

```text
================================= Leader Board =================================

  Game Player 1            Player 2             Symbols Wins     Draws  Total

   1.  John Smith          Mary Hill            X - O    7 - 4       1     12
   2.  Beth Cole           Jane Doe             X - O    1 - 2       0      3
   3.  Ted                 Anna Ward            X - O    0 - 0       0      0
   4.  Paul Reed           Tom Clark            O - X    2 - 3       4      9
   5.  Sam Grant           Kate Bell            X - O   11 - 9       4     24

============================= End of Leader Board ==============================
```

## Project Structure

```text
tic-tac-toe/
├── build/                Compiler output (created by the build, not tracked)
├── data/                 Example data, not used at runtime
│   └── leaderboard.txt   Sample save file you can copy next to the game
├── include/              Header files
│   ├── config.h          All constants in one place (file name, symbols, limits, keys, layout)
│   ├── game.h            Game logic and data structures
│   ├── storage.h         Save and load functionality
│   └── ui.h              User interface functions and declarations
├── src/                  C source files
│   ├── game.c            Core game rules (board, moves, win/draw detection) — no I/O
│   ├── main.c            Entry point; wires the menus and game flows together
│   ├── storage.c         Saving and loading matches to/from the save file
│   └── ui.c              All console screens, prompts, and input validation
├── .gitattributes        Line-ending and binary-file rules for Git
├── .gitignore            Build output, debug files, save data, and OS files
├── LICENSE               Apache License 2.0
├── Makefile              Build, run, and clean targets
└── README.md             This file
```

Every screen is laid out inside one shared width. `SCREEN_WIDTH` in `include/config.h` sets how wide the separator lines, banners, board, and score tables are; change it there and the whole interface re-centers to match.

## Build

With `make` on Linux or macOS, or `mingw32-make` with MinGW on Windows, from the project root:

```sh
make
```

`make run` builds the game and starts it from the `build` folder, `make clean` removes that folder, and `make rebuild` does both.

Without make, any C compiler will do:

```sh
mkdir build
gcc -std=c99 -Wall -Wextra -Iinclude src/*.c -o build/tic-tac-toe
```

## How to Play

Run the game from the `build` folder, since that is where it keeps its save file:

```sh
cd build
./tic-tac-toe
```

On Windows, `build\tic-tac-toe.exe` can simply be double-clicked. From the project root, `make run` does the same thing.

Then pick an option from the main menu:

| Key | Action                                                            |
|-----|-------------------------------------------------------------------|
| `1` | Start a new match (enter names, pick symbols, choose first mover) |
| `2` | Continue a previously saved match                                 |
| `3` | View the leaderboard                                              |
| `E` | Save and exit                                                     |

During a round, enter a number `1`–`9` to place your symbol on that cell. After a win or a draw, press `Y` to play the next round. Press `B` at any prompt to go back to the main menu — an unfinished board is saved so you can resume it later. On the Continue Game list, `0` does the same.

Most prompts take a single key, and letter keys are case-insensitive, so `b`, `e`, `y`, `x`, and `o` work just as well as `B`, `E`, `Y`, `X`, and `O`. Two prompts read a whole line instead: the player name, and the game number on the Continue Game list, where a number above 9 is typed in full.

Player names may contain only letters and single spaces, and must be 3 to 99 characters long.

## Save Data

Matches are stored in `leaderboard.txt` — a plain, human-readable text file created in the folder you run the game from. Delete it to start fresh; it is recreated on the next save.

`data/leaderboard.txt` ships with this repository as sample data. Copy it next to the executable to start with the five example matches loaded:

```sh
cp data/leaderboard.txt build/
```

## License

Licensed under the [Apache License 2.0](LICENSE).
