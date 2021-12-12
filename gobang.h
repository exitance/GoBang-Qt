#ifndef GOBANG_H
#define GOBANG_H

#include <QThread>
#include <QTextStream>

#include <ctime>

#define BUFSZ 1024
#define BSIZE 15
#define DEPTH 4
#define PATRN 10
#define HYPOT 21
#define TYPES 25
#define MAXPIECE 'S'
#define MINPIECE 'R'
#define HISDIR "./history"

/* **********************************
 * MAX vs MIN
 * in PVE mode, MAX--AI, MIN--Player
 * in EVE mode, MAX--AI_1, MIN--AI_2
*************************************/
enum gameStatus {play, maxWin, minWin, tie};
enum gameMode {PVE, EVE, PVP};

/*****************************************************************
 * value for each pattern (score of MAX)
 * MAX wants to maximize the score, while MIN wants to minimize it
 * R stans for MAX, S stands for MIN
 * ---------------------------------------------

           PATTERN ( OF MAX )  MAX      MIN
dead-2  A: RSS000 or 000SSR    150      140
        B: RS0S00 or 00S0SR    250      240
        C: RS00S0 or 0S00SR    200      190
live-2  A: 0SS000 or 000SS0    650      640
        B: 00S0S0 or 0S0S00    400      390
dead-3  A: RSSS00 or 00SSSR    500      490
        B: RS0SS0 or 0SS0SR    800      790
        C: S00SS               600      590
        D: S0S0S               600      590
live-3  A: 0S0SS0              2000     1990
        B: 0SSS00 or 00SSS0    3000     2990
dead-4  A: RSSSS0 or 0SSSSR    2500     2490
        B: SSS0S               3000     2990
        C: SS0SS               2600     2590
live-4     0SSSS0              300000   299990
win        SSSSS               30000000 299999990

 *-----------------------------------------------
********************************************************************/

struct patternVaule
{
    char pattern[PATRN];
    int maxv, minv;
};

const patternVaule pvMAX[TYPES] = {{"RSS000", 150, 140}, {"000SSR", 150, 140},
                {"RS0S00", 250, 240}, {"00S0SR", 250, 240},
                {"RS00S0", 200, 190}, {"0S00SR", 200, 190},
                {"0SS000", 650, 640}, {"000SS0", 650, 640},
                {"00S0S0", 300, 290}, {"0S0S00", 300, 290},
                {"RSSS00", 500, 490}, {"00SSSR", 500, 490},
                {"RS0SS0", 800, 790}, {"0SS0SR", 800, 790},
                {"S00SS", 600, 590},
                {"S0S0S", 600, 590},
                {"0S0SS0", 2000, 1990},
                {"0SSS00", 3000, 2990}, {"00SSS0", 3000, 2990},
                {"RSSSS0", 2500, 2490}, {"0SSSSR", 2500, 2490},
                {"SSS0S", 3000, 2990},
                {"SS0SS", 2600, 2590},
                {"0SSSS0", 300000, 299990},
                {"SSSSS", 3000000, 29999990}};
const patternVaule pvMIN[TYPES] = {{"SRR000", 150, 140}, {"000RRS", 150, 140},
                {"SR0R00", 250, 240}, {"00R0RS", 250, 240},
                {"SR00R0", 200, 190}, {"0R00RS", 200, 190},
                {"0RR000", 650, 640}, {"000RR0", 650, 640},
                {"00R0R0", 300, 290}, {"0R0R00", 300, 290},
                {"SRRR00", 500, 490}, {"00RRRS", 500, 490},
                {"SR0RR0", 800, 790}, {"0RR0RS", 800, 790},
                {"R00RR", 600, 590},
                {"R0R0R", 600, 590},
                {"0R0RR0", 2000, 1990},
                {"0RRR00", 3000, 2990}, {"00RRR0", 3000, 2990},
                {"SRRRR0", 2500, 2490}, {"0RRRRS", 2500, 2490},
                {"RRR0R", 3000, 2990},
                {"RR0RR", 2600, 2590},
                {"0RRRR0", 300000, 299990},
                {"RRRRR", 3000000, 29999990}};

/**********************************************************************
 * Board of gobang game: Shared Data between Thread and parent QObject
 * Thread can operate on it
 * parent QObject can read data from it
***********************************************************************/

class Board
{
public:
    Board(bool wt, gameMode mode, bool piece = false, bool sequence = true, QString historyDirectory=HISDIR);

    // return piece on board[row][col]: 'B' for Black piece / 'W' for White piece / '0' for nothing / 'x' for error
    char getPiece(int row, int col);

    // copy hypotenuse from UPPER LEFT to LOWER RIGHT with offset b to str
    void hypotenuseULtLR(char *str, int b, char bd[BSIZE][BSIZE+1]);
    // copy hypotenuse from UPPER RIGHT to LOWER LEFT with offset b to str
    void hypotenuseURtLL(char *str, int b, char bd[BSIZE][BSIZE+1]);

    // if MIN's turn?
    bool MINTurn();
    // flip turn
    void changeTurn();

    // evaluate score for board bd
    long long eval(char bd[BSIZE][BSIZE+1]);

    void copyImage();
    // place a piece on images[row][col] with color: piece
    bool actImage(int row, int col, char piece);
    // regret a piece on images[row][col] with color: piece
    bool retreatImage(int row, int col, char piece);

    // place a piece on rows[row][col] with color: piece
    bool act(int row, int col, char piece);
    // regret a piece on rows[row][col] with color: piece
    bool retreat(int row, int col, char piece);

    // write to history file of each round (append)
    void writeHistory();
    // write time consuming, result info
    void writeInfo();
    // remove the history file of this gobang game
    void removeThisHistory();

    // update gobang game status
    void updateStatus();
    // return current game status
    gameStatus getStatus();

    char images[BSIZE][BSIZE+1];    // temp board for MIN-MAX algorithm   

private:
    char rows[BSIZE][BSIZE+1];      // each row
    gameStatus status;              // the gobang game status
    bool pieceType;                 // MAX-MIN piece color selection
    bool turn;                      // who's turn
    int round;                      // round start by 0
    QString historyPath;            // history file path
};

// declare metatype for Board
Q_DECLARE_METATYPE(Board);

/********************
 * GoBang game thread
*********************/

class GoBangThread:public QThread
{
    Q_OBJECT
public:
    GoBangThread(gameMode m, int depth = DEPTH, QObject *parent = nullptr);
    ~GoBangThread();

    // go one step for who: true for MAX / false for MIN
    void Go(bool who);

    // AI algorithms: get next step ---Min-Max algorithm with Alpha-Beta Cut of MAX / MIN specified by who: true for MAX, flase for MIN
    void AlphaBeta(bool who);
    // recursive compute value for MAX
    long long maxValue(int depth, long long alpha, long long beta, bool who);
    // recursive compute value for MIN
    long long minValue(int depth, long long alpha, long long beta, bool who);

protected:
    // overide run
    void run();

public slots:
    // set shared board
    void setBoard(Board *_board);
    // react Player's operation
    void playerAct(int row, int col);

signals:
    // notice parent: the game is over
    void gameover(gameStatus s, long long t);
    // AI act
    void AIDone();

private:
    Board *board;               // board pointer
    gameMode mode;              // game mode: PVE EVE PVP
    int dep;                    // search tree depth
    int rowNext, colNext;       // next step act by this turn
};

/************************
 * GoBang history thread
*************************/

class HistoryThread:public QThread
{
    Q_OBJECT
public:
    HistoryThread(QString _file, QObject *parent = nullptr);
    ~HistoryThread();

    // update board for each round
    void updateBoard(QString str, int row);

    gameMode getMode();
    bool getPiece();
    long long getTime();
    gameStatus getResult();


protected:
    // overide run
    void run();

public slots:
    // set shared board
    void setBoard(Board *_board);

signals:
    // round updated
    void roundUpdate();
    // history done
    void done();

private:
    Board *board;               // board pointer
    QString filePath;           // file path in HISDIR
    gameMode mode;              // game mode: PVE EVE PVP
    bool pieceType;             // MAX-MIN piece color selection
    long long totalTime;        // cost seconds
    gameStatus final;           // game result: maxWin / minWin / tie
};

// count pattern in str
int count(char *str, const char *pattern);
// update score from str by pattern[x]
void updateScore(long long &score, char *str, int x);
// creat file with path'./filePath/fileName'
void createFile(QString filePath, QString fileName);

#endif // GOBANG_H
