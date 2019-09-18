/* Saving and loading matches to/from LEADERBOARD_FILE. */

#ifndef STORAGE_H
#define STORAGE_H

// Prints an error if the file cannot be written.
void saveMatches(void);

// A missing file means "no saved data". Stops at the first corrupt entry; a bad board is reset.
void loadMatches(void);

#endif // STORAGE_H
