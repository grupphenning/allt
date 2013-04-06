#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QKeyEvent>
#include <iostream>


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
}

MainWindow::~MainWindow()
{
    delete eat;
    delete ui;
}

void MainWindow::updateKeyState()
{
    if(is_pressed[0]){};
}

KeyPressEater::KeyPressEater(MainWindow *w)
{
    this->w = w;
}

bool KeyPressEater::eventFilter(QObject *recipient, QEvent *event)
{
    if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        switch(key_event->key()) {
        case Qt::Key_Up: w->is_pressed[0] = event->type() == QEvent::KeyPress;
        case Qt::Key_Down: w->is_pressed[1] = event->type() == QEvent::KeyPress;
        case Qt::Key_Left: w->is_pressed[2] = event->type() == QEvent::KeyPress;
        case Qt::Key_Right: w->is_pressed[3] = event->type() == QEvent::KeyPress;
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
    return QObject::eventFilter(recipient, event);
}
