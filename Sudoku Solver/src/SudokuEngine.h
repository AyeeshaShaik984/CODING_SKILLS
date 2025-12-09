#pragma once
#include <cstring>

class SudokuEngine {
public:
    static constexpr int MaxSize = 12;   // supports 6, 9, 12

    SudokuEngine();

    // size must be 6, 9, or 12.
    void loadPuzzle(const int src[MaxSize][MaxSize], int size);
    void getGrid(int dest[MaxSize][MaxSize]) const;

    // full solve for the current size
    bool solve(int size);

private:
    int grid[MaxSize][MaxSize]{};

    // rowUsed[r][v] == true if value v is used in row r
    // v index from 1..size, we allocate up to 12.
    bool rowUsed[MaxSize][MaxSize + 1]{};
    bool colUsed[MaxSize][MaxSize + 1]{};
    bool boxUsed[MaxSize][MaxSize + 1]{};

    int currentSize = 9;
    int boxRows = 3;
    int boxCols = 3;

    bool solveRecursive(int r, int c);
    bool canPlace(int r, int c, int val) const;
    int boxIndex(int r, int c) const;
};
