#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets/QPushButton>
#include "qextserialport.h"

namespace Ui {
class MainWindow;
}

class KeyPressEater : public QObject {
    Q_OBJECT

public:
    KeyPressEater(class MainWindow *w);

protected:
    bool eventFilter(QObject *recipient, QEvent *event);

private slots:
    void on_pushButton_3_released();

private:
    MainWindow *w;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool is_pressed[4];
    void updateKeyState();
    void setDirection(unsigned);

private:
    //QPushButton *arrow_keys[7];
    QPushButton *arrow_keys[9];
    unsigned current_direction;
    Ui::MainWindow *ui;
    KeyPressEater *eat;
    QextSerialPort *port;

private slots:
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_10_clicked();

    void on_pushButton_2_released();
    void on_pushButton_3_released();
    void on_pushButton_4_released();
    void on_pushButton_5_released();
    void on_pushButton_6_released();
    void on_pushButton_7_released();
    void on_pushButton_10_released();

    void onDataAvailable();
};

#endif // MAINWINDOW_H
