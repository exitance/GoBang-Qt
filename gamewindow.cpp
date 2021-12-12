#include "gamewindow.h"

#include <QPainter>

#include <cmath>
#include <cstring>

ChoseDialog::ChoseDialog(QWidget *parent)
{
    pieceLabel = new QLabel(this);
    pieceLabel->setText(tr("请选择白子或黑子"));

    whiteBtn = new QRadioButton(this);
    blackBtn = new QRadioButton(this);

    pieceBtnGp = new QButtonGroup(this);
    pieceBtnGp->addButton(whiteBtn, 0);
    pieceBtnGp->addButton(blackBtn, 1);
    whiteBtn->setText("白");
    blackBtn->setText("黑");
    whiteBtn->setChecked(true); // default selection
    pieceOfPlayer = false;

    sequenceLabel = new QLabel(this);
    sequenceLabel->setText(tr("请选择先手或后手"));

    offensiveBtn = new QRadioButton(this);
    defensiveBtn = new QRadioButton(this);

    sequenceBtnGp = new QButtonGroup(this);
    sequenceBtnGp->addButton(offensiveBtn, 0);
    sequenceBtnGp->addButton(defensiveBtn, 1);
    offensiveBtn->setText("先手");
    defensiveBtn->setText("后手");
    offensiveBtn->setChecked(true); // default selection
    sequenceOfPlayer = true;

    levelLabel = new QLabel(this);
    levelLabel->setText(tr("请选择游戏难度"));

    simpleBtn = new QRadioButton(this);
    mediumBtn = new QRadioButton(this);
    hardBtn = new QRadioButton(this);

    levelBtnGp = new QButtonGroup(this);
    levelBtnGp->addButton(simpleBtn, 0);
    levelBtnGp->addButton(mediumBtn, 1);
    levelBtnGp->addButton(hardBtn, 2);
    simpleBtn->setText("简单");
    mediumBtn->setText("中等");
    hardBtn->setText("困难");
    simpleBtn->setChecked(true);        // default selection
    levelOfAI = 0;

    confirmBtn = new QPushButton(this);
    confirmBtn->setText(tr("确认"));

    cancleBtn = new QPushButton(this);
    cancleBtn->setText(tr("取消"));

    layout = new QGridLayout(this);
    layout->addWidget(pieceLabel, 0, 0, 1, 3);
    layout->addWidget(whiteBtn, 1, 0);
    layout->addWidget(blackBtn, 1, 2);
    layout->addWidget(sequenceLabel, 2, 0, 1, 3);
    layout->addWidget(offensiveBtn, 3, 0);
    layout->addWidget(defensiveBtn, 3, 2);
    layout->addWidget(levelLabel, 4, 0, 1, 3);
    layout->addWidget(simpleBtn, 5, 0);
    layout->addWidget(mediumBtn, 5, 1);
    layout->addWidget(hardBtn, 5, 2);
    layout->addWidget(confirmBtn, 6, 0);
    layout->addWidget(cancleBtn, 6, 2);

    this->setLayout(layout);

    connect(blackBtn, &QPushButton::clicked, this, &ChoseDialog::piece);
    connect(whiteBtn, &QPushButton::clicked, this, &ChoseDialog::piece);
    connect(offensiveBtn, &QPushButton::clicked, this, &ChoseDialog::sequence);
    connect(defensiveBtn, &QPushButton::clicked, this, &ChoseDialog::sequence);
    connect(simpleBtn, &QPushButton::clicked, this, &ChoseDialog::level);
    connect(mediumBtn, &QPushButton::clicked, this, &ChoseDialog::level);
    connect(hardBtn, &QPushButton::clicked, this, &ChoseDialog::level);

    connect(confirmBtn, &QPushButton::clicked, this, &ChoseDialog::confirm);
    connect(cancleBtn, &QPushButton::clicked, this, &ChoseDialog::close);
}

ChoseDialog::~ChoseDialog()
{
    delete pieceLabel; delete sequenceLabel;
    delete pieceBtnGp; delete sequenceBtnGp;
    delete whiteBtn; delete blackBtn;
    delete offensiveBtn; delete defensiveBtn;
    delete confirmBtn; delete cancleBtn;
    delete layout;
}

bool ChoseDialog::playerPiece()
{
    return pieceOfPlayer;
}

bool ChoseDialog::playerSequence()
{
    return sequenceOfPlayer;
}

int ChoseDialog::AIdepth()
{
    return levelOfAI + 2;
}

void ChoseDialog::piece()
{
    switch (pieceBtnGp->checkedId())
    {
        case 0:
            pieceOfPlayer = false;
            break;
        case 1:
            pieceOfPlayer = true;
            break;
    }
}

void ChoseDialog::sequence()
{
    switch (sequenceBtnGp->checkedId())
    {
        case 0:
            sequenceOfPlayer = true;
            break;
        case 1:
            sequenceOfPlayer = false;
            break;
    }
}

void ChoseDialog::level()
{
    switch (levelBtnGp->checkedId())
    {
        case 0:
            levelOfAI = 0;
            break;
        case 1:
            levelOfAI = 1;
            break;
        case 2:
            levelOfAI = 2;
            break;
    }
}

void ChoseDialog::confirm()
{
    accept();
}

DoneDialog::DoneDialog(const char *time, const char *result)
{
    timeLabel = new QLabel(this);
    timeLabel->setText(tr(time));
    resultLabel = new QLabel(this);
    resultLabel->setText(tr(result));

    menuBtn = new QPushButton(this);
    menuBtn->setText(tr("确认"));

    layout = new QGridLayout(this);
    layout->addWidget(resultLabel, 0, 0, 1, 2);
    layout->addWidget(timeLabel, 1, 0, 1, 2);
    layout->addWidget(menuBtn, 2, 1);

    this->setLayout(layout);

    this->setGeometry(320+boardSize/2+boardOffset+boardGap, 160+boardSize/2+boardOffset+boardGap, 150, 130);

    // connect(againBtn, &QPushButton::clicked, this, &DoneDialog::again);
    connect(menuBtn, &QPushButton::clicked, this, &DoneDialog::accept);
}

GameWindow::GameWindow(gameMode _mode, bool _piece, bool _sequence, QWidget *parent, int _depth):
    QMainWindow(parent)
{
    // register meta type Board and its pointer
    qRegisterMetaType<Board>();
    qRegisterMetaType<Board*>();

    mode = _mode;
    piece = _piece; sequence = _sequence;
    depth = _depth;
    rowAct = colAct = -1;
    board = new Board(true, mode, piece, sequence);

    this->setGeometry(320, 160, 150+boardSize+boardOffset+2*boardGap, 30+boardSize+boardOffset+2*boardGap);
    // this->setGeometry(0, 0, 800, 600);

    if (depth > 0) thread = new GoBangThread(mode, depth, this);
    else thread = new GoBangThread(mode, 2, this);

    connect(this, &GameWindow::shareBoard, thread, &GoBangThread::setBoard);
    connect(this, &GameWindow::playerDone, thread, &GoBangThread::playerAct);
    connect(thread, &GoBangThread::gameover, this, &GameWindow::finished);
    connect(thread, &GoBangThread::AIDone, this, &GameWindow::AIpaint);

    running = false;

    whiteLabel = new QLabel(this);
    blackLabel = new QLabel(this);
    if (mode == PVE)
    {
        if (!piece)
        {
            whiteLabel->setText(tr("你 ⚪"));
            blackLabel->setText(tr("AI ⚫"));
        }
        else
        {
            whiteLabel->setText(tr("AI ⚪"));
            blackLabel->setText(tr("你 ⚫"));
        }
    }
    else if (mode == EVE)
    {
        if (!piece)
        {
            whiteLabel->setText(tr("AI-2 ⚪"));
            blackLabel->setText(tr("AI-1 ⚫"));
        }
        else
        {
            whiteLabel->setText(tr("AI-1 ⚪"));
            blackLabel->setText(tr("AI-2 ⚫"));
        }
    }
    else if (mode == PVP)
    {
        if (!piece)
        {
            whiteLabel->setText(tr("玩家2 ⚪"));
            blackLabel->setText(tr("玩家1 ⚫"));
        }
        else
        {
            whiteLabel->setText(tr("玩家1 ⚪"));
            blackLabel->setText(tr("玩家2 ⚫"));
        }
        infoLabel = new QLabel(this);
        infoLabel->setText(tr("玩家1后手"));
        infoLabel->setGeometry(60+boardSize+boardOffset+2*boardGap, 200, 80, 15);
    }
    blackLabel->setGeometry(60+boardSize+boardOffset+2*boardGap, 30, 60, 15);
    whiteLabel->setGeometry(60+boardSize+boardOffset+2*boardGap, 50, 60, 15);
    startBtn = new QPushButton(this);
    startBtn->setText(tr("开始游戏"));
    restartBtn = new QPushButton(this);
    restartBtn->setText(tr("重新开始"));
    menuBtn = new QPushButton(this);
    menuBtn->setText(tr("返回菜单"));
    startBtn->setGeometry(50+boardSize+boardOffset+2*boardGap, 420, 60, 30);
    restartBtn->setGeometry(50+boardSize+boardOffset+2*boardGap, 460, 60, 30);
    menuBtn->setGeometry(50+boardSize+boardOffset+2*boardGap, 500, 60, 30);
    connect(startBtn, &QPushButton::clicked, this, &GameWindow::startThread);
    connect(restartBtn, &QPushButton::clicked, this, &GameWindow::restartThread);
    connect(menuBtn, &QPushButton::clicked, this, &GameWindow::menu);
    startBtn->setEnabled(true);
    restartBtn->setEnabled(false);
    setMouseTracking(true);
}

GameWindow::~GameWindow()
{
    if (board != nullptr)
    {
        if (board->getStatus() == play)
            board->removeThisHistory();
        delete board;
    }
    if (thread != nullptr) delete thread;

    delete whiteLabel;
    delete blackLabel;
    delete startBtn;
    delete restartBtn;
    delete menuBtn;
    if (infoLabel != nullptr) delete infoLabel;
}

void GameWindow::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    // anti-aliasing
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(QColor(0, 160, 230));
    pen.setWidth(3);

    // draw board's boundary rectangle
    painter.setPen(pen);
    painter.drawRect(boardOffset, boardOffset, boardSize+2*boardGap, boardSize+2*boardGap);

    pen.setWidth(1);

    // draw grid line
    painter.setPen(pen);
    // each column (y)
    for (int c = 0; c <= BSIZE+1; c ++)
    {
        painter.drawLine(boardOffset+boardGap, boardOffset+boardGap+c*boardStep,
                         boardOffset+boardGap+boardSize, boardOffset+boardGap+c*boardStep);
    }
    // each row (x)
    for (int r = 0; r <= BSIZE+1; r ++)
    {
        painter.drawLine(boardOffset+boardGap+r*boardStep, boardOffset+boardGap,
                         boardOffset+boardGap+r*boardStep, boardOffset+boardGap+boardSize);
    }

    if (board != nullptr)
    {
        // draw pieces
        QPen blackPen(QColor(0, 0, 0));
        QPen whitePen(QColor(255, 255, 255));
        QBrush blackBrush(QBrush(QColor(0, 0, 0), Qt::SolidPattern));
        QBrush whiteBrush(QBrush(QColor(255, 255, 255), Qt::SolidPattern));
        // check board
        for (int r = 0; r < BSIZE; r ++)
        {
            for (int c = 0; c < BSIZE; c++)
            {
                char p = board->getPiece(r, c);
                if (p == 'B')
                {
                    painter.setPen(blackPen);
                    painter.setBrush(blackBrush);
                    painter.drawEllipse(QPoint(boardOffset+boardGap+(r+1)*boardStep , boardOffset+boardGap+(c+1)*boardStep),
                                        (int)(boardStep*0.35),(int)(boardStep*0.35));
                }
                if (p == 'W')
                {
                    painter.setPen(whitePen);
                    painter.setBrush(whiteBrush);
                    painter.drawEllipse(QPoint(boardOffset+boardGap+(r+1)*boardStep , boardOffset+boardGap+(c+1)*boardStep),
                                        (int)(boardStep*0.35),(int)(boardStep*0.35));
                }
            }
        }
    }

    // draw cursor
    QPen greenPen(QColor(0, 255, 0));
    QBrush greenBrush(QBrush(QColor(0, 255, 0, 100), Qt::SolidPattern));

    QPen redPen(QColor(255, 0, 0));
    QBrush redBrush(QBrush(QColor(255, 0, 0, 100), Qt::SolidPattern));
    painter.setPen(redPen);
    painter.setBrush(redBrush);

    if (running && board != nullptr)
    {
        char p = board->getPiece(rowAct, colAct);
        if (p == '0')
        {
            if (board->MINTurn())
            {
                painter.setPen(greenPen);
                painter.setBrush(greenBrush);
                painter.drawEllipse(QPoint(boardOffset+boardGap+(rowAct+1)*boardStep , boardOffset+boardGap+(colAct+1)*boardStep),
                            (int)(boardStep*0.35),(int)(boardStep*0.35));
            }
            else if (mode == PVP && !board->MINTurn())
            {
                painter.setPen(redPen);
                painter.setBrush(redBrush);
                painter.drawEllipse(QPoint(boardOffset+boardGap+(rowAct+1)*boardStep , boardOffset+boardGap+(colAct+1)*boardStep),
                            (int)(boardStep*0.35),(int)(boardStep*0.35));
            }
        }
    }
}

void GameWindow::mouseMoveEvent(QMouseEvent *e)
{
    rowAct = colAct = -1;

    if (!running || board == nullptr || thread == nullptr) return;
    if (mode == EVE) return;
    if (mode == PVE && !board->MINTurn()) return;

    int x = e->position().x();
    int y = e->position().y();

    for (int r = 0; r < BSIZE; r ++)
    {
        for (int c = 0; c < BSIZE; c ++)
        {
            int row = boardOffset+boardGap+(r+1)*boardStep;
            int col = boardOffset+boardGap+(c+1)*boardStep;
            int dis = sqrt(pow(row-x, 2) + pow(col-y, 2));
            if (dis <= boardStep*0.35)
            {
                if (board->getPiece(r, c) == '0')
                {
                    rowAct = r;
                    colAct = c;
                    this->update();
                }
                break;
            }
        }
    }
}

void GameWindow::mousePressEvent(QMouseEvent *e)
{
    rowAct = colAct = -1;

    if (!running || board == nullptr || thread == nullptr) return;
    if (mode == EVE) return;
    if (mode == PVE && !board->MINTurn()) return;

    int x = e->position().x();
    int y = e->position().y();

    for (int r = 0; r < BSIZE; r ++)
    {
        for (int c = 0; c < BSIZE; c ++)
        {
            int row = boardOffset+boardGap+(r+1)*boardStep;
            int col = boardOffset+boardGap+(c+1)*boardStep;
            int dis = sqrt(pow(row-x, 2) + pow(col-y, 2));
            if (dis <= boardStep*0.35)
            {
                if (board->getPiece(r, c) == '0')
                {
                    emit playerDone(r, c);
                    this->update();
                }
                break;
            }
        }
    }
}

void GameWindow::startThread()
{
    if (thread != nullptr)
    {
        thread->start();
        emit shareBoard(board);
    }
    running = true;

    startBtn->setEnabled(false);
    restartBtn->setEnabled(true);
}

void GameWindow::restartThread()
{
    if (board != nullptr && board->getStatus() == play) board->removeThisHistory();
    board = new Board(true, mode, piece, sequence);
    thread->start();
    emit shareBoard(board);

    startBtn->setEnabled(false);
    restartBtn->setEnabled(true);
}

void GameWindow::menu()
{
    if (board != nullptr && board->getStatus() == play) board->removeThisHistory();
    if (running) thread->terminate();
    this->parentWidget()->show();
    delete this;
}

void GameWindow::finished(gameStatus s, long long t)
{
    char time[BUFSZ];
    long long hours = t / 3600; t %= 3600;
    long long minutes = t / 60;
    long long seconds = t % 60;
    sprintf(time, "总用时: %lld 时 %lld 分 %lld 秒", hours, minutes, seconds);

    char result[BUFSZ];
    if (mode == PVE)
    {
        if (s == minWin)
            strcpy(result, "你取得了胜利！");
        else if (s == maxWin)
            strcpy(result, "你输给了AI！");
        else
            strcpy(result, "平局");
    }
    if (mode == EVE)
    {
        if (s == minWin)
            strcpy(result, "AI-2取得了胜利！");
        else if (s == maxWin)
            strcpy(result, "AI-1取得了胜利！");
        else
            strcpy(result, "平局");
    }
    if (mode == PVP)
    {
        if (s == minWin)
            strcpy(result, "玩家2取得了胜利！");
        else if (s == maxWin)
            strcpy(result, "玩家1取得了胜利！");
        else
            strcpy(result, "平局");
    }

    DoneDialog *odlg = new DoneDialog(time, result);

    if (odlg->exec() == QDialog::Accepted)
    {
        // if (running) thread->terminate();
        this->parentWidget()->show();
        delete this;
    }
}

void GameWindow::AIpaint()
{
    update();
}
