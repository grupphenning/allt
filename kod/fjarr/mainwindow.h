#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include "qextserialport.h"
#include "qextserialenumerator.h"

#define MAX_SENSORS 17
#define BUFFER_SIZE 100

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
    void on_pushButton_9_clicked();
    void on_pushButtonPID_clicked();
    void on_pushButtonLeft90_clicked();
    void on_pushButtonRight90_clicked();
    void on_pid_toggle_clicked();
    void on_pushButtonAddToDisplay_clicked();
    void on_pushButton_15_clicked();
    void on_pushButton_13_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_17_clicked();
    void on_pushButton_16_clicked();
    void on_bookmarks_currentIndexChanged(int index);

    void on_speed_valueChanged(int arg1);

private:
    MainWindow *w;
};


class PIDDialog : public QDialog
{
    Q_OBJECT

public:
    PIDDialog()
    {
        l1.setText("P");
        l2.setText("I");
        l3.setText("D");

        lo.addRow(&l1, &spin1);
        lo.addRow(&l2, &spin2);
        lo.addRow(&l3, &spin3);

        b.setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        g.addLayout(&lo);
        g.addWidget(&b);

        //this->setGeometry();
        this->setWindowTitle("PID-konstanter");
        this->setLayout(&g);

        connect(&b, SIGNAL(accepted()), this, SLOT(accept()));
        connect(&b, SIGNAL(rejected()), this, SLOT(reject()));
    }


    int getP() { return spin1.value(); }
    int getI() { return spin2.value(); }
    int getD() { return spin3.value(); }

private:
    QDialogButtonBox b;
    QVBoxLayout g;
    QFormLayout lo;
    QLabel l1, l2, l3;
    QSpinBox spin1, spin2, spin3;
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

    void open_claw();
    void close_claw();
    Ui::MainWindow *ui;

private:
    QPushButton *arrow_keys[7];
    QVector<QByteArray> bookmarks;
    unsigned current_direction;
    KeyPressEater *eat;
    QextSerialPort *port;
    PIDDialog pid;
    QByteArray printfString;   // Ingen riktig printf-sträng, Davids special!

private slots:
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_10_clicked();
    void on_pushButton_11_clicked();
    void on_pushButton_12_clicked();
    void on_pushButton_14_clicked();
    void on_pushButton_16_clicked();
    void on_pushButton_17_clicked();
    void on_pushButton_2_released();
    void on_pushButton_3_released();
    void on_pushButton_4_released();
    void on_pushButton_5_released();
    void on_pushButton_6_released();
    void on_pushButton_7_released();
    void on_pushButton_10_released();
    void onDataAvailable();
    void on_bookmarks_currentIndexChanged(int index);
    void on_pushButton_9_clicked();
    void on_pid();
    void on_pushButtonPID_clicked();
    void on_pid_toggle_clicked();
    void on_pushButtonLeft90_clicked();
    void on_pushButtonRight90_clicked();
    void on_pushButtonAddToDisplay_clicked();
    void on_pushButton_15_clicked();
    void updateDisplayExample();
    void on_pushButton_13_clicked();
    void on_pushButton_9_toggled(bool pressed);
    void on_pushButton_8_clicked();
    void on_speed_valueChanged(int arg1);
};

#endif // MAINWINDOW_H
