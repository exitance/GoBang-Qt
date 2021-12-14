#include "historywindow.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPainter>

FileDialog::FileDialog(QWidget *parent)
{
    QDir curDir;
    // current dir for this program
    QString curPath = curDir.currentPath();
    QString filePath = HISDIR;
    // set execute path to filePath
    curDir.setCurrent(filePath);
    // get full path
    dirFullPath = curDir.currentPath();
    // find all files
    int f = findFiles(dirFullPath);
    fileBox = new QComboBox(this);
    if (f!=-1)
    {
        fileBox->addItems(fileList);
        selectedFile = fileBox->currentText();
    }
    // recover current path
    curDir.setCurrent(curPath);

    connect(fileBox, &QComboBox::currentTextChanged, this, &FileDialog::changeSelectedFile);

    infoLabel = new QLabel(this);
    if (f!=-1) infoLabel->setText(tr("请选择历史记录"));
    else infoLabel->setText(tr("暂无历史记录"));
    refreshBtn = new QPushButton(this);
    refreshBtn->setText(tr("刷新"));
    confirmBtn = new QPushButton(this);
    confirmBtn->setText(tr("确认"));
    cancleBtn = new QPushButton(this);
    cancleBtn->setText(tr("取消"));
    layout = new QGridLayout(this);
    layout->addWidget(infoLabel, 0, 0, 1, 3);
    layout->addWidget(fileBox, 1, 0, 1, 2);
    layout->addWidget(refreshBtn, 1, 2);
    layout->addWidget(confirmBtn, 2, 0);
    layout->addWidget(cancleBtn, 2, 3);
    this->setLayout(layout);

    connect(refreshBtn, &QPushButton::clicked, this, &FileDialog::refresh);
    connect(confirmBtn, &QPushButton::clicked, this, &FileDialog::confirm);
    connect(cancleBtn, &QPushButton::clicked, this, &FileDialog::close);
}

FileDialog::~FileDialog()
{
    delete infoLabel;
    delete refreshBtn; delete confirmBtn; delete cancleBtn;
    delete fileBox;
    delete layout;
}

QString FileDialog::getSelectedFile()
{
    return selectedFile;
}

int FileDialog::findFiles(const QString &fullPath)
{   // fullPath: absolute path
    QDir dir(fullPath);

    if (!dir.exists())
    {
        qDebug()<< "[GoBang] error: history directory" << fullPath << "dose not exist" << "\n";
        return -1;
    }

    // get all files and directories except . and .. directories
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst);
    // turn to a list
    QFileInfoList list = dir.entryInfoList();
    // empty
    if (list.size() < 1) return -1;

    QList<QFileInfo>::Iterator it = list.begin();
    for (; it != list.end(); it ++)
    {
        QFileInfo fileInfo = *it;
        bool isDir = fileInfo.isDir();
        // recursively find files
        if (isDir) findFiles(fileInfo.filePath());
        else
        {
            QStringList pathList = fileInfo.filePath().split('/');
            pathList.removeLast();
            QString DateDir = pathList.last();
            // without extension
            QString fileName = fileInfo.fileName().split('.').first();
            fileName.replace("-", ":");
            QString fileSubPath = DateDir + "/" + fileName;
            // add file
            fileList.append(fileSubPath);
        }
    }

    return 0;
}

void FileDialog::changeSelectedFile(QString str)
{
    selectedFile = str;
}

void FileDialog::refresh()
{
    int f = findFiles(dirFullPath);
    fileBox = new QComboBox(this);
    if (f!=-1) fileBox->addItems(fileList);

    if (f!=-1) infoLabel->setText(tr("请选择历史记录"));
    else infoLabel->setText(tr("暂无历史记录"));
}

void FileDialog::confirm()
{
    accept();
}

PopDialog::PopDialog(const char *time, const char *result)
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

    connect(menuBtn, &QPushButton::clicked, this, &PopDialog::accept);
}

PopDialog::~PopDialog()
{
    delete timeLabel; delete resultLabel;
    delete menuBtn;
    delete layout;
}

HistoryWindow::HistoryWindow(QString file, QWidget *parent)
    : QMainWindow{parent}
{
    // register meta type Board and its pointer
    qRegisterMetaType<Board>();
    qRegisterMetaType<Board*>();

    filePath = "/" + file.replace(":", "-") + ".txt";
    filePath = HISDIR + filePath;

    thread = new HistoryThread(filePath, this);
    gameMode mode = thread->getMode();
    bool piece = thread->getPiece();
    board = new Board(false, mode, piece, true);

    this->setGeometry(320, 160, 150+boardSize+boardOffset+2*boardGap, 30+boardSize+boardOffset+2*boardGap);

    connect(this, &HistoryWindow::shareBoard, thread, &HistoryThread::setBoard);
    connect(thread, &HistoryThread::roundUpdate, this, &HistoryWindow::updatePaint);
    connect(thread, &HistoryThread::done, this, &HistoryWindow::finish);

    running = false;

    whiteLabel = new QLabel(this);
    blackLabel = new QLabel(this);
    infoLabel = new QLabel(this);
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
        infoLabel->setText(tr("人机对战"));
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
        infoLabel->setText(tr("机器对战"));
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
        infoLabel->setText(tr("双人对战"));
    }
    blackLabel->setGeometry(60+boardSize+boardOffset+2*boardGap, 30, 60, 15);
    whiteLabel->setGeometry(60+boardSize+boardOffset+2*boardGap, 50, 60, 15);
    infoLabel->setGeometry(60+boardSize+boardOffset+2*boardGap, 80, 60, 15);
    startBtn = new QPushButton(this);
    startBtn->setText(tr("开始播放"));
    menuBtn = new QPushButton(this);
    menuBtn->setText(tr("返回菜单"));
    startBtn->setGeometry(50+boardSize+boardOffset+2*boardGap, 420, 60, 30);
    menuBtn->setGeometry(50+boardSize+boardOffset+2*boardGap, 500, 60, 30);
    connect(startBtn, &QPushButton::clicked, this, &HistoryWindow::startThread);
    connect(menuBtn, &QPushButton::clicked, this, &HistoryWindow::menu);
    startBtn->setEnabled(true);
    setMouseTracking(true);
}

HistoryWindow::~HistoryWindow()
{
    if (board != nullptr) 
    {
        delete board;
        board = nullptr;
    }
    if (thread != nullptr) 
    {
        delete thread;
        board = nullptr;
    }

    delete whiteLabel;
    delete blackLabel;
    delete infoLabel;
    delete startBtn;
    delete menuBtn;
}

void HistoryWindow::paintEvent(QPaintEvent *e)
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

}

void HistoryWindow::startThread()
{
    if (thread != nullptr)
    {
        thread->start();
        emit shareBoard(board);
    }
    running = true;

    startBtn->setEnabled(false);
}

void HistoryWindow::menu()
{
    if (running) thread->terminate();
    this->parentWidget()->show();
    delete this;
}

void HistoryWindow::updatePaint()
{
    update();
}

void HistoryWindow::finish()
{
    gameMode mode = thread->getMode();
    long long sec = thread->getTime();
    gameStatus res = thread->getResult();

    long long hours = sec / 3600; sec %= 3600;
    long long minutes = sec / 60;
    long long seconds = sec % 60;
    char time[BUFSZ];
    sprintf(time, "此局总用时: %lld 时 %lld 分 %lld 秒", hours, minutes, seconds);

    char result[BUFSZ];
    if (mode == PVE)
    {
        if (res == minWin)
            strcpy(result, "结果：玩家获胜");
        else if (res == maxWin)
            strcpy(result, "结果：AI获胜");
        else
            strcpy(result, "结果：平局");
    }
    if (mode == EVE)
    {
        if (res == minWin)
            strcpy(result, "结果：AI-2获胜");
        else if (res == maxWin)
            strcpy(result, "结果：AI-1获胜");
        else
            strcpy(result, "结果：平局");
    }
    if (mode == PVP)
    {
        if (res == minWin)
            strcpy(result, "结果：玩家2获胜");
        else if (res == maxWin)
            strcpy(result, "结果：玩家1获胜");
        else
            strcpy(result, "结果：平局");
    }

    PopDialog *pdlg = new PopDialog(time, result);

    if (pdlg->exec() == QDialog::Accepted)
    {
        // if (running) thread->terminate();
        this->parentWidget()->show();
        close();
    }
}
