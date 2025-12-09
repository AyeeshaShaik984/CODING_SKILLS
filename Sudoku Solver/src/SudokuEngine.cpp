#include "SudokuEngine.h"

// ---------------- constructor ----------------

SudokuEngine::SudokuEngine() {
    // arrays are zero-initialized by default
}

// ---------------- helpers ----------------

// compute box index from row/col given current boxRows/boxCols
int SudokuEngine::boxIndex(int r, int c) const {
    int br = r / boxRows;
    int bc = c / boxCols;
    return br * (currentSize / boxCols) + bc;
}

bool SudokuEngine::canPlace(int r, int c, int val) const {
    if (rowUsed[r][val]) return false;
    if (colUsed[c][val]) return false;
    if (boxUsed[boxIndex(r, c)][val]) return false;
    return true;
}

// ---------------- public API ----------------

void SudokuEngine::loadPuzzle(const int src[MaxSize][MaxSize], int size) {
    // set currentSize and box shape based on size
    currentSize = size;
    if (currentSize == 6) {
        boxRows = 2;
        boxCols = 3;
    } else if (currentSize == 9) {
        boxRows = 3;
        boxCols = 3;
    } else { // 12
        boxRows = 3;
        boxCols = 4;
    }

    std::memset(grid,     0, sizeof(grid));
    std::memset(rowUsed,  0, sizeof(rowUsed));
    std::memset(colUsed,  0, sizeof(colUsed));
    std::memset(boxUsed,  0, sizeof(boxUsed));

    for (int r = 0; r < currentSize; ++r) {
        for (int c = 0; c < currentSize; ++c) {
            grid[r][c] = src[r][c];
            int val = grid[r][c];
            if (val != 0) {
                rowUsed[r][val] = true;
                colUsed[c][val] = true;
                boxUsed[boxIndex(r, c)][val] = true;
            }
        }
    }
}

void SudokuEngine::getGrid(int dest[MaxSize][MaxSize]) const {
    for (int r = 0; r < currentSize; ++r) {
        for (int c = 0; c < currentSize; ++c) {
            dest[r][c] = grid[r][c];
        }
    }
}

bool SudokuEngine::solve(int size) {
    currentSize = size;
    // boxRows/boxCols already set in loadPuzzle
    return solveRecursive(0, 0);
}

// ---------------- backtracking core ----------------

bool SudokuEngine::solveRecursive(int r, int c) {
    if (r == currentSize) {
        return true; // finished all rows
    }

    int nextR = r;
    int nextC = c + 1;
    if (nextC == currentSize) {
        nextC = 0;
        ++nextR;
    }

    if (grid[r][c] != 0) {
        // cell already filled; move on
        return solveRecursive(nextR, nextC);
    }

    for (int val = 1; val <= currentSize; ++val) {
        if (canPlace(r, c, val)) {
            grid[r][c] = val;
            rowUsed[r][val] = true;
            colUsed[c][val] = true;
            boxUsed[boxIndex(r, c)][val] = true;

            if (solveRecursive(nextR, nextC)) {
                return true;
            }

            // backtrack
            grid[r][c] = 0;
            rowUsed[r][val] = false;
            colUsed[c][val] = false;
            boxUsed[boxIndex(r, c)][val] = false;
        }
    }
    return false; // no value fits here
}
