#include "gobang.h"

#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

#include <utility>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cstring>

static long long startTime;
static long long finishTime;

/*************
 * Board Data
 * functions
**************/

Board::Board(bool wt, gameMode mode, bool piece, bool sequence, QString historyDirectory)
{
    status = play;
    pieceType = piece;
    turn = sequence;
    round = 0;
    if (wt)
    {
        QDateTime time = QDateTime::currentDateTime();
        QString fileSubPath = time.toString("/yyyy-MM-dd/");
        QString fileName = time.toString("hh-mm-ss");
        fileName += ".txt";
        QString filePath = historyDirectory + fileSubPath;
        historyPath = filePath + fileName;

        createFile(filePath, fileName);
        QFile file(historyPath);

        if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            qDebug() << "[GoBang] error: Fail to open file" << historyPath;
        }

        QTextStream out(&file);
        // game mode
        out << mode << " ";
        // mark the piece color of player
        if(piece) out << "1\n";
        else out << "0\n";

        file.close();
    }

    // init the board
    // each row
    for (int r = 0; r < BSIZE; r ++)
    {
        for (int c = 0; c < BSIZE; c ++)
        {
            rows[r][c] = '0';
        }
        rows[r][BSIZE] = '\0';
    }

    qDebug() << "[GoBang] Board has been created";
}

Board::~Board()
{
    qDebug() << "[GoBang] Board has been destoryed";
}

char Board::getPiece(int row, int col)
{
    if (row < 0 || row >= BSIZE || col < 0 || col >= BSIZE) return 'x';
    else if (rows[row][col] == '0') return '0';
    else if (rows[row][col] == MAXPIECE)
    {   // pieceType=false: MIN=white, MAX=black
        if (pieceType) return 'W';
        else return 'B';
    }
    else
    {
        if (pieceType) return 'B';
        else return 'W';
    }
}

void Board::hypotenuseULtLR(char *str, int b, char bd[BSIZE][BSIZE+1])
{
    for (int c = 0; c < BSIZE; c ++)
    {
        int r = b - c;
        if (r >= 0 && r < BSIZE) str[c] = bd[r][c];
        else str[c] = 'x';
    }
    str[BSIZE] = '\0';
}

void Board::hypotenuseURtLL(char *str, int b, char bd[BSIZE][BSIZE+1])
{
    for (int c = 0; c < BSIZE; c ++)
    {
        int r = c - b;
        if (r >= 0 && r < BSIZE) str[c] = bd[r][c];
        else str[c] = 'x';
    }
    str[BSIZE] = '\0';
}

bool Board::MINTurn()
{
    if (status == play) return turn;
    return false;
}

bool Board::MAXTurn()
{
    if (status == play) return !turn;
    return false;
}

void Board::changeTurn()
{
    turn = turn==true?false:true;
}

long long Board::eval(char bd[BSIZE][BSIZE+1])
{
    long long sc = 0;
    // each row
    for (int r = 0; r < BSIZE; r ++)
    {
        char row[BSIZE+1];
        strcpy(row, bd[r]);
        for (int i = 0; i < TYPES; i ++)
        {
            updateScore(sc, row, i);
        }
    }
    // each column
    for (int c = 0; c < BSIZE; c ++)
    {
        char col[BSIZE+1];
        for (int r = 0; r < BSIZE; r ++)
            col[r] = bd[r][c];
        col[BSIZE] = '\0';
        for (int i = 0; i < TYPES; i ++)
        {
            updateScore(sc, col, i);
        }

    }
    // each hypotenuse from UPPER LEFT to LOWER RIGHT
    // y = -x + b: c = -r + b: r + c = b
    // to place 5 pieces, 4 <= b <= 24
    for (int b = 4; b < 25; b ++)
    {
        char hlr[BSIZE+1];
        hypotenuseULtLR(hlr, b, bd);
        for (int i = 0; i < TYPES; i ++)
        {
            updateScore(sc, hlr, i);
        }
    }
    // each hypotenuse from UPPER RIGHT to LOWER LEFT
    // y = x + b: c = r + b: c - r = b
    // to place 5 pieces, -10 <= b <= 10
    for (int b = -10; b < 11; b ++)
    {
        char hrl[BSIZE+1];
        hypotenuseURtLL(hrl, b, bd);
        for (int i = 0; i < TYPES; i ++)
        {
            updateScore(sc, hrl, i);
        }
    }

    return sc;
}

void Board::copyImage()
{
    for (int r = 0; r < BSIZE; r ++)
    strcpy(images[r], rows[r]);
}

bool Board::actImage(int row, int col, char piece)
{
    if (row < 0 || row >= BSIZE || col < 0 || col >= BSIZE) return false;
    else if (images[row][col] == '0')
    {   // update
        images[row][col] = piece;
        return true;
    }
    else return false;
}

bool Board::retreatImage(int row, int col, char piece)
{
    if (row < 0 || row >= BSIZE || col < 0 || col >= BSIZE) return false;
    else if (images[row][col] == piece)
    {   // update
        images[row][col] = '0';
        return true;
    }
    else return false;
}

bool Board::act(int row, int col, char piece)
{
    if (row < 0 || row >= BSIZE || col < 0 || col >= BSIZE) return false;
    else if (rows[row][col] == '0')
    {   // update
        rows[row][col] = piece;
        return true;
    }
    else return false;
}

bool Board::retreat(int row, int col, char piece)
{
    if (row < 0 || row >= BSIZE || col < 0 || col >= BSIZE) return false;
    else if (rows[row][col] == piece)
    {   // update
        rows[row][col] = '0';
        return true;
    }
    else return false;
}

void Board::writeHistory()
{
    QFile file(historyPath);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        qDebug() << "[GoBang] error: fail to open file " << historyPath << "\n";
    }

    QTextStream out(&file);

    out << round << "\n";

    for (int r = 0; r < BSIZE; r ++)
    {
        out << rows[r] << "\n";
    }

    round ++;

    file.close();
}

void Board::writeInfo()
{
    QFile file(historyPath);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        qDebug() << "[GoBang] error: fail to open file " << historyPath << "\n";
    }

    QTextStream out(&file);

    long long totalTime = (finishTime - startTime) / 1000;
    out << totalTime << " ";
    if (status == maxWin) out << "1" << "\n";
    else if (status == minWin) out << "2" << "\n";
    else if (status == tie) out << "3" << "\n";

    file.close();
}

void Board::removeThisHistory()
{
    QFile file(historyPath);
    if (!file.exists())
    {
        qDebug() << "[GoBang] error: file " << historyPath << " does not exist\n";
    }
    file.remove();
}

void Board::updateStatus()
{   // check board
    int cnt = 0;
    // each row
    for (int r = 0; r < BSIZE; r ++)
    {
        char row[BSIZE+1];
        strcpy(row, rows[r]);
        cnt = count(row, pvMAX[TYPES-1].pattern);
        if (cnt > 0)
        {   // MAX wins
            status = maxWin;
            return;
        }
        cnt = count(row, pvMIN[TYPES-1].pattern);
        if (cnt > 0)
        {   // MIN wins
            status = minWin;
            return;
        }
    }
    // each column
    for (int c = 0; c < BSIZE; c ++)
    {
        char col[BSIZE+1];
        for (int r = 0; r < BSIZE; r ++)
            col[r] = rows[r][c];
        col[BSIZE] = '\0';
        cnt = count(col, pvMAX[TYPES-1].pattern);
        if (cnt > 0)
        {   // MAX wins
            status = maxWin;
            return;
        }
        cnt = count(col, pvMIN[TYPES-1].pattern);
        if (cnt > 0)
        {   // MIN wins
            status = minWin;
            return;
        }
    }
    // each hypotenuse from LOWER LEFT to UPPER RIGHT
    // y = -x + b: c = -r + b: r + c = b
    // to place 5 pieces, 4 <= b <= 24
    for (int b = 4; b < 25; b ++)
    {
        char hlr[BSIZE+1];
        hypotenuseULtLR(hlr, b, rows);
        cnt = count(hlr, pvMAX[TYPES-1].pattern);
        if (cnt > 0)
        {   // MAX wins
            status = maxWin;
            return;
        }
        cnt = count(hlr, pvMIN[TYPES-1].pattern);
        if (cnt > 0)
        {   // MIN wins
            status = minWin;
            return;
        }
    }
    // each hypotenuse from LOWER RIGHT to UPPER LEFT
    // y = x + b: c = r + b: c - r = b
    // to place 5 pieces, -10 <= b <= 10
    for (int b = -10; b < 11; b ++)
    {
        char hrl[BSIZE+1];
        hypotenuseURtLL(hrl, b, rows);
        cnt = count(hrl, pvMAX[TYPES-1].pattern);
        if (cnt > 0)
        {   // MAX wins
            status = maxWin;
            return;
        }
        cnt = count(hrl, pvMIN[TYPES-1].pattern);
        if (cnt > 0)
        {   // MIN wins
            status = minWin;
            return;
        }
    }
    // if no place to place piece
    bool t = true;
    for (int r = 0; r < BSIZE; r ++)
    {
        for (int c = 0; c < BSIZE; c ++)
        {
            if (rows[r][c] == '0')
            {
                t = false;
                break;
            }
        }
    }
    if (t) status = tie;
}

gameStatus Board::getStatus()
{
    return status;
}

/***********************
 * GoBang Game Thread
 * functions and slots
************************/

GoBangThread::GoBangThread(gameMode m, int depth, QObject *parent):QThread(parent)
{
    // register meta type Board and its pointer
    qRegisterMetaType<Board>();
    qRegisterMetaType<Board*>();

    qDebug() << "[GoBang] Game Thread has been created";

    board = nullptr;
    mode = m;
    dep = depth;
    rowNext = colNext = -1;
}

GoBangThread::~GoBangThread()
{
    qDebug() << "[GoBang] Game Thread has been destoryed";
}

void GoBangThread::Go(bool who)
{
    if (who) board->act(rowNext, colNext, MAXPIECE);
    else board->act(rowNext, colNext, MINPIECE);
    board->writeHistory();
    board->updateStatus();
    // notice game windows to update
    emit AIDone();
    msleep(1000);
    // if need continue ...
    if (board->getStatus() == play)
    {   // flip turn
        board->changeTurn();
    }
    else
    {   // notice game over
        finishTime = clock();
        long long totalTime = (finishTime - startTime) / 1000;
        board->writeInfo();
        emit gameover(board->getStatus(), totalTime);
    }
}

/******************************************************************************************
 * Get next step Min-Max algorithm with Alpha-Beta Cut
 * DFS
 * each node maintain 3 values: \alpha, \beta and v, v in [\alpha, \beta]
 * once we get \alpha > \beta, this node dead, do not need go on searching on this branch
 * how to update \alpha & \beta:
 * in maxValue(), \alpha = max(v of children), and v = \alpha
 * in minValue(), \beta = min(v of children), and v = \beta
 * WHY: for example, MAX want get max value from next layer,
 *                   however above MIN want get min value from this layer
 *                   we only update \alpha in maxValue() and \beta in minValue()
 *                   once \alpha > \beta means this brance cannot get better result
 * the initial value of \alpha and \beta inherited from the node's parent
 * \alpha = -inf, \beta = inf for root
 * ****************************************************************************************
*/

void GoBangThread::AlphaBeta(bool who)
{
    qDebug() << "[GoBang] AI is running alpha-beta algorithm...";
    int alpha = -0x3f3f3f3f, beta = 0x3f3f3f3f;
    board->copyImage();
    if (who) maxValue(0, alpha, beta, who);
    else minValue(0, alpha, beta, who);
    qDebug() << "[GoBang] AI has finished alpha-beta algorithm...";
}

long long GoBangThread::maxValue(int depth, long long alpha, long long beta, bool who)
{
    if (depth == dep) return board->eval(board->images);
    long long v = -0x3f3f3f3f;
    long long old_alpha = 0;
    for (int r = 0; r < BSIZE; r ++)
    {
        for (int c = 0; c < BSIZE; c ++)
        {   // check all postion
            char p = board->getPiece(r, c);
            if (p == '0')
            {   // no piece on the postion
                // MAX place a piece here
                board->actImage(r, c, MAXPIECE);
                // get max value from next layer evaluated by MIN
                v = std::max(v, minValue(depth+1, alpha, beta, who));
                if (v >= beta)
                {   // alpha > beta, kill this branch
                    board->retreatImage(r, c, MAXPIECE); // traceback
                    return v;
                }
                // update
                alpha = std::max(alpha, v);
                if (who)
                {
                    if (depth == 0 && alpha == v && alpha != old_alpha)
                    {   // root & alpha has been update & not same as old alpha (rep)
                        // update MAX's next step
                        rowNext = r, colNext = c;
                        // update old alpha
                        old_alpha = alpha;
                    }
                }
                board->retreatImage(r, c, MAXPIECE); // traceback
            }
        }
    }
    return v;
}

long long GoBangThread::minValue(int depth, long long alpha, long long beta, bool who)
{
    if (depth == dep) return board->eval(board->images);
    long long v = 0x3f3f3f3f;
    long long old_beta = 0;
    for (int r = 0; r < BSIZE; r ++)
    {
        for (int c = 0; c < BSIZE; c ++)
        {
            char p = board->getPiece(r, c);
            if (p == '0')
            {   // no piece on the postion
                // MIN place a piece here
                board->actImage(r, c, MINPIECE);
                // get min value from next layer evaluated by MAX
                v = std::min(v, maxValue(depth+1, alpha, beta, who));
                if (v <= alpha)
                {   // alpha > beta, kill this branch
                    board->retreatImage(r, c, MINPIECE); // traceback
                    return v;
                }
                // update
                beta = std::min(beta, v);
                if (!who)
                {
                    if (depth == 0 && beta == v && beta != old_beta)
                    {   // root & alpha has been update & not same as old beta (rep)
                        // update MIN's next step
                        rowNext = r, colNext = c;
                        // update old alpha
                        old_beta = beta;
                    }
                }
                board->retreatImage(r, c, MINPIECE); // traceback
            }
        }
    }
    return v;
}

void GoBangThread::run()
{
    if (board == nullptr) return;
    qDebug() << "[GoBang] Game Thread is running...";
    startTime = clock();
    board->writeHistory();  // init
    if (board->getStatus() == play && ((mode == PVE && !board->MINTurn()) || mode == EVE) )
    {   // first step for AI
        rowNext = colNext = 7;
        Go(!board->MINTurn());
    }
    while(board->getStatus() == play)
    {
        if (board->MAXTurn() && mode != PVP)
        {   // MAX's turn
            rowNext = colNext = -1;
            AlphaBeta(true);
            Go(true);
        }
        else if (board->MINTurn() && mode == EVE)
        {   // if MIN is AI_2
            rowNext = colNext = -1;
            AlphaBeta(false);
            Go(false);
        }   // else wait player's action
    }
}

void GoBangThread::setBoard(Board *_board)
{
    board = _board;
}

void GoBangThread::playerAct(int row, int col)
{
    if (mode == EVE) return;
    if (mode == PVE && !board->MINTurn()) return;
    if (!board->MINTurn() && !board->MAXTurn()) return;

    char p = board->getPiece(row, col);
    if (p == 'x' || p != '0') return;

    if (board->MINTurn())
    {
        if (!board->act(row, col, MINPIECE)) return;
    }
    else if (board->MAXTurn())
    {
        if (!board->act(row, col, MAXPIECE)) return;
    }
    board->writeHistory();
    board->updateStatus();
    // if need continue ...
    if (board->getStatus() == play)
    {   // flip turn
        board->changeTurn();
    }
    else
    {   // notice game over
        finishTime = clock();
        long long totalTime = (finishTime - startTime) / 1000;
        board->writeInfo();
        emit gameover(board->getStatus(), totalTime);
    }
}

/***********************
 * History Recall Thread
 * functions and slots
************************/

HistoryThread::HistoryThread(QString _file, QObject *parent):QThread(parent)
{
    // register meta type Board and its pointer
    qRegisterMetaType<Board>();
    qRegisterMetaType<Board*>();

    qDebug() << "[GoBang] History Thread has been created";

    filePath = _file;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<< "[GoBang] error: fail to open file" << filePath << "\n";
        return;
    }

    QTextStream in(&file);
    QString line = in.readLine();
    QStringList list = line.split(" ");
    QString smode = list.first();
    QString spieceType = list.last();
    if (smode == "0") mode = PVE;
    else if (smode == "1") mode = EVE;
    else if (smode == "2") mode = PVP;
    if (spieceType == "0") pieceType = false;
    else if (spieceType == "1") pieceType = true;

    file.close();
}

HistoryThread::~HistoryThread()
{
    qDebug() << "[GoBang] History Thread has been destoryed";
}

gameMode HistoryThread::getMode()
{
    return mode;
}

bool HistoryThread::getPiece()
{
    return pieceType;
}

long long HistoryThread::getTime()
{
    return totalTime;
}

gameStatus HistoryThread::getResult()
{
    return final;
}

void HistoryThread::updateBoard(QString str, int row)
{
    for (int i = 0; i < str.size(); i ++)
    {
        if (str[i] != '0')
            board->act(row, i, str[i].toLatin1());
    }
}

void HistoryThread::run()
{
    qDebug() << "[GoBang] History Thread is running...";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<< "[GoBang] error: fail to open file" << filePath << "\n";
        return;
    }

    QTextStream in(&file);
    QString line = in.readLine();

    while (in.readLineInto(&line))
    {
        QStringList list = line.split(" ");
        if (list.first() == list.last())
        {
            for (int r = 0; r < BSIZE; r ++)
            {
                line = in.readLine();
                updateBoard(line, r);
            }
            emit roundUpdate();
        }
        else
        {
            QString stt = list.first();
            QByteArray ba = stt.toLatin1();
            char *str = ba.data();
            sscanf(str, "%lld", &totalTime);
            QString res = list.last();
            if (res == "1") final = maxWin;
            else if (res == "2") final = minWin;
            else if (res == "3") final = tie;
            break;
        }
        msleep(500);
    }

    file.close();
    emit done();    
}

void HistoryThread::setBoard(Board *_board)
{
    board = _board;
}

/******************
 * public functions
*******************/

int count(char *str, const char *pattern)
{
    int cnt = 0, cur = 0;
    char *s = nullptr;
    while ( (s = strstr(str+cur, pattern)) != nullptr)
    {
        cur = s - str + strlen(pattern);
        cnt ++;
    }
    return cnt;
}

void updateScore(long long &score, char *str, int x)
{
    int cnt = 0;
    cnt = count(str, pvMAX[x].pattern);
    score += cnt * pvMAX[x].maxv;
    cnt = count(str, pvMIN[x].pattern);
    score -= cnt * pvMIN[x].minv;
}

void createFile(QString filePath, QString fileName)
{
    QDir curDir;
    // current dir for this program
    QString curPath = curDir.currentPath();
    // if filePath does not exist, create it
    if(!curDir.exists(filePath))
    {
        curDir.mkpath(filePath);
        qDebug()<< "[GoBang] create directory" << filePath;
    }
    QFile *thisFile = new QFile;
    // set execute path to filePath
    curDir.setCurrent(filePath);

    // if ./filePath/fileName exists, stop
    if(thisFile->exists(fileName)) return;
    // else create file
    thisFile->setFileName(fileName);
    if(!thisFile->open(QIODevice::WriteOnly|QIODevice::Text))
        qDebug()<< "[GoBang] error: fail to create file" << fileName;
    else
        qDebug() << "[GoBang] Create file" << filePath+fileName;
    thisFile->close();
    // set current path of program back to origin path
    curDir.setCurrent(curPath);
}
