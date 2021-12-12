#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include "gobang.h"

#include <QMainWindow>
#include <QDialog>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>
#include <QButtonGroup>
#include <QLayout>
#include <QMouseEvent>

#ifndef BOARD_DARW
#define BOARD_DARW
const int boardOffset = 10;
const int boardStep = 35;
const int boardSize = (BSIZE+1) * boardStep;
const int boardGap = 10;
#endif

class ChoseDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ChoseDialog(QWidget *parent = 0);
    ~ChoseDialog();

    bool playerPiece();
    bool playerSequence();
    int AIdepth();

private slots:
    void piece();
    void sequence();
    void level();
    void confirm();

private:
    QLabel *pieceLabel;
    QLabel *sequenceLabel;
    QLabel *levelLabel;

    QButtonGroup *pieceBtnGp;
    QButtonGroup *sequenceBtnGp;
    QButtonGroup *levelBtnGp;

    QRadioButton *whiteBtn;
    QRadioButton *blackBtn;

    QRadioButton *offensiveBtn;
    QRadioButton *defensiveBtn;

    QRadioButton *simpleBtn;
    QRadioButton *mediumBtn;
    QRadioButton *hardBtn;

    QPushButton *confirmBtn;
    QPushButton *cancleBtn;

    QGridLayout *layout;

    bool pieceOfPlayer;     // true for white
    bool sequenceOfPlayer;  // true for offensive
    bool levelOfAI;         // decide search depth
};

class DoneDialog : public QDialog
{
    Q_OBJECT
public:
    DoneDialog(const char *time, const char *result);

private:
    QLabel *timeLabel;
    QLabel *resultLabel;
    QPushButton *menuBtn;
    QGridLayout *layout;
};

class GameWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit GameWindow(gameMode _mode, bool _piece = true, bool _sequence = true, QWidget *parent = nullptr, int _depth = -1);
    ~GameWindow();

protected:
    void paintEvent(QPaintEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);

signals:
    // share board data
    void shareBoard(Board *board);
    // notice Player to act
    void playerDone(int row, int col);

private slots:
    // start game
    void startThread();
    // restart game
    void restartThread();
    // return to menu
    void menu();
    // game is Over
    void finished(gameStatus s, long long t);
    // AI done
    void AIpaint();

private:
    Board *board = nullptr;
    GoBangThread *thread = nullptr;
    int rowAct, colAct;
    gameMode mode;
    int depth = -1;
    bool running;
    bool piece, sequence;

    QLabel *whiteLabel;
    QLabel *blackLabel;
    QLabel *infoLabel = nullptr;
    QPushButton *startBtn;
    QPushButton *restartBtn;
    QPushButton *menuBtn;
};

#endif // GAMEWINDOW_H
