#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QDialog>
#include <QDialogButtonBox>

// ---------------- Constructor ----------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      stack(new QStackedWidget(this)),
      homePage(new QWidget(this)),
      startButton(new QPushButton("Start New Game", this)),
      difficultyBox(new QComboBox(this)),
      gamePage(new QWidget(this)),
      table(new QTableWidget(this)),
      newGameButton(new QPushButton("New Game", this)),
      solveButton(new QPushButton("Solve", this)),
      changeLevelButton(new QPushButton("Change Level", this)),
      statusLabel(new QLabel(this)),
      timerLabel(new QLabel(this)),
      gameTimer(new QTimer(this)) {

    setupUi();

    stack->setCurrentWidget(homePage);
    setCentralWidget(stack);
    resize(900, 800);
    setWindowTitle("Sudoku Solver");
}

// ================= UI setup =================

void MainWindow::setupUi() {
    setupHomePage();
    setupGamePage();

    stack->addWidget(homePage);
    stack->addWidget(gamePage);

    connect(startButton, &QPushButton::clicked,
            this, &MainWindow::onStartClicked);
    connect(newGameButton, &QPushButton::clicked,
            this, &MainWindow::onNewGame);
    connect(solveButton, &QPushButton::clicked,
            this, &MainWindow::onSolve);
    connect(changeLevelButton, &QPushButton::clicked,
            this, [this]() {
                QDialog dlg(this);
                dlg.setWindowTitle("Change Level");

                QVBoxLayout *layout = new QVBoxLayout(&dlg);
                QComboBox *combo = new QComboBox(&dlg);
                combo->addItem("Easy");
                combo->addItem("Medium");
                combo->addItem("Hard");
                combo->setCurrentText(difficultyBox->currentText());

                QDialogButtonBox *buttons =
                    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                         Qt::Horizontal, &dlg);

                layout->addWidget(combo);
                layout->addWidget(buttons);

                connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
                connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

                if (dlg.exec() == QDialog::Accepted) {
                    difficultyBox->setCurrentText(combo->currentText());
                    elapsedSeconds = 0;
                    timerLabel->setText("Time: 00:00");
                    gameTimer->start(1000);

                    for (int r = 0; r < SudokuEngine::MaxSize; ++r)
                        for (int c = 0; c < SudokuEngine::MaxSize; ++c)
                            wrongAttempts[r][c] = 0;

                    loadPuzzleForCurrentDifficulty();
                }
            });
    connect(table, &QTableWidget::cellChanged,
            this, &MainWindow::onCellChanged);

    connect(gameTimer, &QTimer::timeout, this, [this]() {
        ++elapsedSeconds;
        int m = elapsedSeconds / 60;
        int s = elapsedSeconds % 60;
        timerLabel->setText(
            QString("Time: %1:%2")
                .arg(m, 2, 10, QLatin1Char('0'))
                .arg(s, 2, 10, QLatin1Char('0')));
    });
}

void MainWindow::setupHomePage() {
    auto *layout = new QVBoxLayout(homePage);

    homePage->setStyleSheet(
        "QWidget {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #1e293b, stop:1 #0f172a);"
        "  color: white;"
        "}"
    );

    auto *title = new QLabel("Sudoku Solver");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "QLabel {"
        "  font-size: 32px;"
        "  font-weight: 600;"
        "  margin-bottom: 24px;"
        "}"
    );

    difficultyBox->addItem("Easy");   // 6x6
    difficultyBox->addItem("Medium"); // 9x9
    difficultyBox->addItem("Hard");   // 12x12
    difficultyBox->setStyleSheet(
        "QComboBox {"
        "  background-color: #0f172a;"
        "  color: white;"
        "  padding: 6px 12px;"
        "  border-radius: 6px;"
        "  font-size: 14px;"
        "}"
    );
    difficultyBox->setFixedWidth(160);

    startButton->setCursor(Qt::PointingHandCursor);
    startButton->setMinimumSize(220, 64);
    startButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #22c55e;"
        "  color: #0f172a;"
        "  border-radius: 12px;"
        "  font-size: 20px;"
        "  font-weight: 600;"
        "  padding: 12px 32px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #16a34a;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #15803d;"
        "}"
    );

    layout->addStretch();
    layout->addWidget(title, 0, Qt::AlignHCenter);
    layout->addSpacing(8);
    layout->addWidget(difficultyBox, 0, Qt::AlignHCenter);
    layout->addSpacing(24);
    layout->addWidget(startButton, 0, Qt::AlignHCenter);
    layout->addStretch();
}

void MainWindow::setupGamePage() {
    auto *centralLayout = new QVBoxLayout(gamePage);
    auto *btnLayout = new QHBoxLayout();
    auto *bottomLayout = new QHBoxLayout();

    gamePage->setStyleSheet(
        "QWidget {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #020617, stop:0.5 #0f172a, stop:1 #0d9488);"
        "  color: #e5e7eb;"
        "}"
        "QTableWidget {"
        "  gridline-color: #22c55e;"
        "  selection-background-color: #22c55e;"
        "  background-color: rgba(15, 23, 42, 0.9);"
        "  color: #e5e7eb;"
        "  border: 2px solid #0f766e;"
        "  border-radius: 12px;"
        "}"
    );

    table->setRowCount(9);
    table->setColumnCount(9);
    table->horizontalHeader()->setVisible(false);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::AllEditTriggers);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectItems);
    table->setMaximumWidth(700);
    table->setMinimumWidth(500);
    table->setMinimumHeight(500);

    const char *buttonStyle =
        "QPushButton {"
        "  background-color: #0f172a;"
        "  color: #e5e7eb;"
        "  border-radius: 8px;"
        "  padding: 8px 18px;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #111827;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #020617;"
        "}";

    newGameButton->setStyleSheet(buttonStyle);
    solveButton->setStyleSheet(buttonStyle);
    changeLevelButton->setStyleSheet(buttonStyle);

    btnLayout->addStretch();
    btnLayout->addWidget(newGameButton);
    btnLayout->addWidget(solveButton);
    btnLayout->addWidget(changeLevelButton);
    btnLayout->addStretch();

    timerLabel->setText("Time: 00:00");
    timerLabel->setAlignment(Qt::AlignCenter);
    timerLabel->setMinimumWidth(180);
    timerLabel->setStyleSheet(
        "QLabel {"
        "  background-color: rgba(15,23,42,0.9);"
        "  color: #fefce8;"
        "  font-size: 18px;"
        "  font-weight: 600;"
        "  padding: 10px 18px;"
        "  border-radius: 12px;"
        "  border: 1px solid #22c55e;"
        "}"
    );

    statusLabel->setText("Make a move…");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setMinimumWidth(260);
    statusLabel->setStyleSheet(
        "QLabel {"
        "  background-color: rgba(15,23,42,0.9);"
        "  color: #a5f3fc;"
        "  font-size: 16px;"
        "  padding: 10px 20px;"
        "  border-radius: 12px;"
        "  border: 1px solid #38bdf8;"
        "}"
    );

    bottomLayout->addStretch();
    bottomLayout->addWidget(timerLabel);
    bottomLayout->addSpacing(24);
    bottomLayout->addWidget(statusLabel);
    bottomLayout->addStretch();

    centralLayout->addStretch();
    centralLayout->addWidget(table, 0, Qt::AlignHCenter);
    centralLayout->addSpacing(12);
    centralLayout->addLayout(btnLayout);
    centralLayout->addSpacing(8);
    centralLayout->addLayout(bottomLayout);
    centralLayout->addStretch();

    loadPuzzleForCurrentDifficulty();
}

// ================= puzzles and sync =================

void MainWindow::loadRandomPuzzle(const QString &diff) {
    static int easy6x6[1][SudokuEngine::MaxSize][SudokuEngine::MaxSize] = {
        {
            {0,0,3, 0,6,0, 0,0,0, 0,0,0},
            {0,6,0, 1,0,4, 0,0,0, 0,0,0},
            {4,0,6, 0,1,0, 0,0,0, 0,0,0},
            {0,3,0, 6,0,2, 0,0,0, 0,0,0},
            {6,0,2, 0,4,0, 0,0,0, 0,0,0},
            {0,4,0, 2,0,3, 0,0,0, 0,0,0},

            {0,0,0, 0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0, 0,0,0}
        }
    };

    static int medium9x9[1][SudokuEngine::MaxSize][SudokuEngine::MaxSize] = {
        {
            {5,3,0, 0,7,0, 0,0,0, 0,0,0},
            {6,0,0, 1,9,5, 0,0,0, 0,0,0},
            {0,9,8, 0,0,0, 0,6,0, 0,0,0},
            {8,0,0, 0,6,0, 0,0,3, 0,0,0},
            {4,0,0, 8,0,3, 0,0,1, 0,0,0},
            {7,0,0, 0,2,0, 0,0,6, 0,0,0},
            {0,6,0, 0,0,0, 2,8,0, 0,0,0},
            {0,0,0, 4,1,9, 0,0,5, 0,0,0},
            {0,0,0, 0,8,0, 0,7,9, 0,0,0},

            {0,0,0, 0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0, 0,0,0}
        }
    };

    static int hard12x12[1][SudokuEngine::MaxSize][SudokuEngine::MaxSize] = {
        {
            { 1, 0, 0, 0, 9, 0, 0, 0,12, 0, 0, 6},
            { 0,12, 0, 0, 0, 6, 0, 3, 0, 0, 9, 0},
            { 0, 0, 9, 0, 0,12, 0, 0, 0, 4, 0, 0},
            { 0, 0, 0, 6, 0, 0,11, 0, 0, 0, 0,10},

            { 0, 0,12, 0, 0, 0, 0, 9, 0, 0, 4, 0},
            { 0, 5, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0},
            { 0, 0, 0, 8, 0, 0, 0, 1, 0,11, 0, 0},
            { 0, 4, 0, 0,11, 0, 0, 0, 2, 0, 0, 0},

            {11, 0, 0, 0, 0, 8, 0, 0, 0, 0, 6, 0},
            { 0, 0, 6, 0, 0, 0, 9, 0, 0, 3, 0, 0},
            { 0, 8, 0, 0, 0, 5, 0,10, 0, 0, 2, 0},
            { 4, 0, 0,12, 0, 0, 0, 0, 9, 0, 0, 1}
        }
    };

    if (diff == "Easy") {
        engine.loadPuzzle(easy6x6[0], 6);
    } else if (diff == "Medium") {
        engine.loadPuzzle(medium9x9[0], 9);
    } else {
        engine.loadPuzzle(hard12x12[0], 12);
    }

    for (int r = 0; r < SudokuEngine::MaxSize; ++r)
        for (int c = 0; c < SudokuEngine::MaxSize; ++c)
            wrongAttempts[r][c] = 0;
}

void MainWindow::loadPuzzleForCurrentDifficulty() {
    QString diff = difficultyBox->currentText();

    if (diff == "Easy") {
        currentSize = 6;
        boxRows = 2;
        boxCols = 3;
    } else if (diff == "Medium") {
        currentSize = 9;
        boxRows = 3;
        boxCols = 3;
    } else {
        currentSize = 12;
        boxRows = 3;
        boxCols = 4;
    }

    table->clear();
    table->setRowCount(currentSize);
    table->setColumnCount(currentSize);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int r = 0; r < currentSize; ++r) {
        for (int c = 0; c < currentSize; ++c) {
            auto *item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            table->setItem(r, c, item);
        }
    }

    loadRandomPuzzle(diff);
    syncFromEngineToUi();

    statusLabel->setText("New game (" + diff + ")");
}

// map engine grid → UI
void MainWindow::syncFromEngineToUi() {
    int grid[SudokuEngine::MaxSize][SudokuEngine::MaxSize]{};
    engine.getGrid(grid);

    for (int r = 0; r < currentSize; ++r) {
        for (int c = 0; c < currentSize; ++c) {
            auto *item = table->item(r, c);
            if (!item) {
                item = new QTableWidgetItem();
                item->setTextAlignment(Qt::AlignCenter);
                table->setItem(r, c, item);
            }

            if (grid[r][c] == 0) {
                item->setText("");
                item->setFlags(item->flags() | Qt::ItemIsEditable);
                item->setForeground(QColor("#e5e7eb"));
                item->setBackground(QColor("#020617"));
            } else {
                item->setText(QString::number(grid[r][c]));
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item->setForeground(QColor("#22c55e"));
                item->setBackground(QColor("#0f172a"));
            }
        }
    }
}

// map UI → engine grid
void MainWindow::syncFromUiToEngine() {
    int grid[SudokuEngine::MaxSize][SudokuEngine::MaxSize]{};

    for (int r = 0; r < currentSize; ++r) {
        for (int c = 0; c < currentSize; ++c) {
            QTableWidgetItem *item = table->item(r, c);
            int val = 0;
            if (item) {
                const QString txt = item->text().trimmed();
                if (!txt.isEmpty()) {
                    bool ok = false;
                    int v = txt.toInt(&ok);
                    if (ok && v >= 1 && v <= currentSize) {
                        val = v;
                    }
                }
            }
            grid[r][c] = val;
        }
    }

    engine.loadPuzzle(grid, currentSize);
}

// ================= helpers =================

bool MainWindow::isUserMoveValid(int row, int col, int val) const {
    int grid[SudokuEngine::MaxSize][SudokuEngine::MaxSize]{};

    for (int r = 0; r < currentSize; ++r) {
        for (int c = 0; c < currentSize; ++c) {
            QTableWidgetItem *item = table->item(r, c);
            int v = 0;
            if (item) {
                bool ok = false;
                int t = item->text().toInt(&ok);
                if (ok && t >= 1 && t <= currentSize) {
                    v = t;
                }
            }
            grid[r][c] = v;
        }
    }

    grid[row][col] = val;

    // row check
    for (int c = 0; c < currentSize; ++c) {
        if (c == col) continue;
        if (grid[row][c] == val) return false;
    }

    // column check
    for (int r = 0; r < currentSize; ++r) {
        if (r == row) continue;
        if (grid[r][col] == val) return false;
    }

    // box check
    int startRow = (row / boxRows) * boxRows;
    int startCol = (col / boxCols) * boxCols;
    for (int r = 0; r < boxRows; ++r) {
        for (int c = 0; c < boxCols; ++c) {
            int rr = startRow + r;
            int cc = startCol + c;
            if (rr == row && cc == col) continue;
            if (grid[rr][cc] == val) return false;
        }
    }

    return true;
}

// ================= slots =================

void MainWindow::onStartClicked() {
    stack->setCurrentWidget(gamePage);

    elapsedSeconds = 0;
    timerLabel->setText("Time: 00:00");
    gameTimer->start(1000);

    for (int r = 0; r < SudokuEngine::MaxSize; ++r)
        for (int c = 0; c < SudokuEngine::MaxSize; ++c)
            wrongAttempts[r][c] = 0;

    loadPuzzleForCurrentDifficulty();
}

void MainWindow::onNewGame() {
    elapsedSeconds = 0;
    timerLabel->setText("Time: 00:00");
    gameTimer->start(1000);

    for (int r = 0; r < SudokuEngine::MaxSize; ++r)
        for (int c = 0; c < SudokuEngine::MaxSize; ++c)
            wrongAttempts[r][c] = 0;

    loadPuzzleForCurrentDifficulty();
}

void MainWindow::onSolve() {
    syncFromUiToEngine();
    if (engine.solve(currentSize)) {
        syncFromEngineToUi();
        statusLabel->setText("Solved!");
    } else {
        QMessageBox::warning(this, "Sudoku Solver",
                             "No solution exists for this grid.");
    }
}

void MainWindow::onCellChanged(int row, int col) {
    QTableWidgetItem *item = table->item(row, col);
    if (!item) return;

    const QString txt = item->text().trimmed();

    if (txt.isEmpty()) {
        item->setBackground(QColor("#020617"));
        statusLabel->setText("Cell cleared.");
        return;
    }

    bool ok = false;
    int val = txt.toInt(&ok);
    if (!ok || val < 1 || val > currentSize) {
        item->setText("");
        item->setBackground(QColor("#020617"));
        statusLabel->setText("Enter value in range.");
        return;
    }

    if (isUserMoveValid(row, col, val)) {
        item->setBackground(QColor("#14532d"));
        statusLabel->setText("Nice move!");
    } else {
        ++wrongAttempts[row][col];

        item->setBackground(QColor("#b91c1c"));
        statusLabel->setText(
            QString("Wrong (%1/%2)")
                .arg(wrongAttempts[row][col])
                .arg(MaxWrongPerCell));

        item->setText("");
        item->setBackground(QColor("#020617"));

        if (wrongAttempts[row][col] >= MaxWrongPerCell) {
            gameTimer->stop();

            for (int r = 0; r < currentSize; ++r) {
                for (int c2 = 0; c2 < currentSize; ++c2) {
                    if (QTableWidgetItem *it = table->item(r, c2)) {
                        it->setFlags(it->flags() & ~Qt::ItemIsEditable);
                    }
                }
            }

            statusLabel->setText("Game over");
            QMessageBox::information(
                this,
                "Game over",
                "Game over. Click \"New Game\" to try again.");
        }
    }
}
