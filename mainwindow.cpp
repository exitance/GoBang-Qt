#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gamewindow.h"
#include "historywindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pveBtn_clicked()
{
    ChoseDialog *cdlg = new ChoseDialog(this);

    if (cdlg->exec() == QDialog::Accepted)
    {
        GameWindow *gw = new GameWindow(PVE, cdlg->playerPiece(), cdlg->playerSequence(), this, cdlg->AIdepth());
        QPalette pal(gw->palette());
        pal.setColor(QPalette::Window , QColor(222, 184, 135, 100));
        gw->setAutoFillBackground(true);
        gw->setPalette(pal);
        gw->setWindowTitle("人机对战");
        gw->show();
        this->hide();
    }
}


void MainWindow::on_eveBtn_clicked()
{
    GameWindow *gw = new GameWindow(EVE, false, true, this);
    QPalette pal(gw->palette());
    pal.setColor(QPalette::Window , QColor(222, 184, 135, 100));
    gw->setAutoFillBackground(true);
    gw->setPalette(pal);
    gw->setWindowTitle("机器对战");
    gw->show();
    this->hide();
}


void MainWindow::on_pvpBtn_clicked()
{
    GameWindow *gw = new GameWindow(PVP, false, true, this);
    QPalette pal(gw->palette());
    pal.setColor(QPalette::Window , QColor(222, 184, 135, 100));
    gw->setAutoFillBackground(true);
    gw->setPalette(pal);
    gw->setWindowTitle("双人对战");
    gw->show();
    this->hide();
}


void MainWindow::on_historyBtn_clicked()
{
    FileDialog *fdlg = new FileDialog(this);

    if (fdlg->exec() == QDialog::Accepted)
    {
        HistoryWindow *hw = new HistoryWindow(fdlg->getSelectedFile(), this);
        QPalette pal(hw->palette());
        pal.setColor(QPalette::Window , QColor(222, 184, 135, 100));
        hw->setAutoFillBackground(true);
        hw->setPalette(pal);
        hw->setWindowTitle("历史棋局");
        hw->show();
        this->hide();
    }
}

