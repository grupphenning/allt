#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QKeyEvent>
#include <iostream>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(width(), height());

    is_pressed[0] = is_pressed[1] = is_pressed[2] = is_pressed[3] = false;
    eat = new KeyPressEater(this);

    QList<QPushButton *> list = findChildren<QPushButton *>();
    for(int i = 0; i < list.size(); ++i) {
        list.at(i)->installEventFilter(eat);
    }

    arrow_keys[0] = ui->pushButton_3;
    arrow_keys[1] = ui->pushButton_4;
    arrow_keys[2] = ui->pushButton_6;
    arrow_keys[3] = ui->pushButton_2;
    arrow_keys[4] = ui->pushButton_5;
    arrow_keys[5] = ui->pushButton_7;
    arrow_keys[6] = ui->pushButton_10;

    for(unsigned i = 0; i < sizeof arrow_keys / sizeof arrow_keys[0]; ++i) {
        arrow_keys[i]->setCheckable(true);
    }

    current_direction = 4;
    arrow_keys[4]->setChecked(true);

    // Serieport
    PortSettings settings = {BAUD115200, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
    port = new QextSerialPort("COM12", settings);
    connect(port, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
    port->open(QIODevice::ReadWrite);
}

void MainWindow::onDataAvailable()
{
    std::cout << port->readAll().constData();
    std::cout.flush();
}

MainWindow::~MainWindow()
{
    port->close();
    delete port;
    delete eat;
    delete ui;
}

void MainWindow::setDirection(unsigned dir)
{
    if(dir == current_direction) return;

    arrow_keys[current_direction]->setChecked(false);
    arrow_keys[dir]->setChecked(true);
    current_direction = dir;

    port->write(QByteArray(1, "ldrlsrb"[dir]));
    port->flush();
}

void MainWindow::updateKeyState()
{
    // Bitarna i index motsvarar Höger, Vänster, Ner, Upp.
    // Siffrorna är 0: fram vänster, 1: fram, 2: fram höger, 3: vänster, 4: stopp, 5: höger, 6: bakåt
    static const unsigned directions[16] = {
        4, 1, 6, 4, 3, 0, 3, 4, 5, 2, 5, 4, 4, 4, 4, 4
    };
    setDirection(directions[
            (is_pressed[0] << 0)
            + (is_pressed[1] << 1)
            + (is_pressed[2] << 2)
            + (is_pressed[3] << 3)]);
}

KeyPressEater::KeyPressEater(MainWindow *w)
{
    this->w = w;
}

bool KeyPressEater::eventFilter(QObject *recipient, QEvent *event)
{
    if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        if(!key_event->isAutoRepeat())
        {
            switch(key_event->key()) {
            case Qt::Key_Up: { w->is_pressed[0] = event->type() == QEvent::KeyPress; break; }
            case Qt::Key_Down: { w->is_pressed[1] = event->type() == QEvent::KeyPress; break; }
            case Qt::Key_Left: { w->is_pressed[2] = event->type() == QEvent::KeyPress; break; }
            case Qt::Key_Right: { w->is_pressed[3] = event->type() == QEvent::KeyPress; break; }
            }

            switch(key_event->key()) {
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Left:
            case Qt::Key_Right:
                w->updateKeyState();
                return true;
            }
        }
    }
    return QObject::eventFilter(recipient, event);
}

void MainWindow::on_pushButton_2_clicked() { setDirection(3); }
void MainWindow::on_pushButton_3_clicked() { setDirection(0); }
void MainWindow::on_pushButton_4_clicked() { setDirection(1); }
void MainWindow::on_pushButton_5_clicked() { setDirection(4); }
void MainWindow::on_pushButton_6_clicked() { setDirection(2); }
void MainWindow::on_pushButton_7_clicked() { setDirection(5); }
void MainWindow::on_pushButton_10_clicked() { setDirection(6); }

void MainWindow::on_pushButton_2_released() { ui->pushButton_2->setChecked(true); }
void MainWindow::on_pushButton_3_released() { ui->pushButton_3->setChecked(true); }
void MainWindow::on_pushButton_4_released() { ui->pushButton_4->setChecked(true); }
void MainWindow::on_pushButton_5_released() { ui->pushButton_5->setChecked(true); }
void MainWindow::on_pushButton_6_released() { ui->pushButton_6->setChecked(true); }
void MainWindow::on_pushButton_7_released() { ui->pushButton_7->setChecked(true); }
void MainWindow::on_pushButton_10_released() { ui->pushButton_10->setChecked(true); }

void KeyPressEater::on_pushButton_3_released(){}
