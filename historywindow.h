#ifndef HISTORYWINDOW_H
#define HISTORYWINDOW_H

#include "gobang.h"

#include <QMainWindow>
#include <QDialog>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>
#include <QButtonGroup>
#include <QLayout>
#include <QComboBox>

#ifndef BOARD_DARW
#define BOARD_DARW
const int boardOffset = 10;
const int boardStep = 35;
const int boardSize = (BSIZE+1) * boardStep;
const int boardGap = 10;
#endif

class FileDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FileDialog(QWidget *parent = 0);
    ~FileDialog();

    QString getSelectedFile();

    int findFiles(const QString &fullPath);

private slots:
    void changeSelectedFile(QString str);
    void refresh();
    void confirm();

private:
    QString dirFullPath;
    QStringList fileList;
    QString selectedFile;

    QLabel *infoLabel;
    QPushButton *refreshBtn;
    QPushButton *confirmBtn;
    QPushButton *cancleBtn;
    QComboBox *fileBox;
    QGridLayout *layout;
};

class PopDialog : public QDialog
{
    Q_OBJECT
public:
    PopDialog(const char *time, const char *result);
    ~PopDialog();

private:
    QLabel *timeLabel;
    QLabel *resultLabel;
    QPushButton *menuBtn;
    QGridLayout *layout;
};

class HistoryWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit HistoryWindow(QString file, QWidget *parent = nullptr);
    ~HistoryWindow();

protected:
    void paintEvent(QPaintEvent *e);

signals:
    void shareBoard(Board *board);

private slots:
    // start game
    void startThread();
    // return to menu
    void menu();
    // update paint
    void updatePaint();
    // recall finished
    void finish();

private:
    Board *board;
    HistoryThread *thread;
    QString filePath;
    bool running = false;

    QLabel *whiteLabel;
    QLabel *blackLabel;
    QLabel *infoLabel = nullptr;
    QPushButton *startBtn;
    QPushButton *menuBtn;
};

#endif // HISTORYWINDOW_H
