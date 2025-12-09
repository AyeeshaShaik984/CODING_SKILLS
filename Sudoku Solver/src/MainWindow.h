#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QTimer>

#include "SudokuEngine.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QStackedWidget *stack;

    QWidget *homePage;
    QPushButton *startButton;
    QComboBox *difficultyBox;

    QWidget *gamePage;
    QTableWidget *table;
    QPushButton *newGameButton;
    QPushButton *solveButton;
    QPushButton *changeLevelButton;   // <-- ADD THIS
    QLabel *statusLabel;
    QLabel *timerLabel;

    QTimer *gameTimer;
    int elapsedSeconds = 0;

    static constexpr int MaxWrongPerCell = 5;
    int wrongAttempts[SudokuEngine::MaxSize][SudokuEngine::MaxSize]{};

    SudokuEngine engine;

    int currentSize = 9;   // 6, 9, or 12
    int boxRows = 3;
    int boxCols = 3;

    void setupUi();
    void setupHomePage();
    void setupGamePage();

    void loadRandomPuzzle(const QString &diff);
    void loadPuzzleForCurrentDifficulty();
    void syncFromEngineToUi();
    void syncFromUiToEngine();
    bool isUserMoveValid(int row, int col, int val) const;

private slots:
    void onStartClicked();
    void onNewGame();
    void onSolve();
    void onCellChanged(int row, int col);
};
