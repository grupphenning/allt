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

    connect(&pid, SIGNAL(accepted()), this, SLOT(on_pid()));
}

/*
 *5  d0  aa
 *6  d1  mosi
 *7  d2  miso
 *33 d3  int0
 *8  d4  clk
 */

void MainWindow::on_pid()
{
    unsigned char t;
    port->write(QByteArray(1, 'p'));
    t = pid.getP();
    port->write(QByteArray(1, t));
    t = pid.getI();
    port->write(QByteArray(1, t));
    t = pid.getD();
    port->write(QByteArray(1, t));
    port->flush();
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

    port->write(QByteArray(1, "vdhlsrb"[dir]));

    /* Eventuellt ändra hastigheten */
    unsigned char speed = ui->speed->value(); // Standardhastighet
    switch("vdhlsrb"[dir])
    {
    case 'v':
        speed = ui->speedDriveLeft->value();
        break;
    case 'd':
        speed = ui->speedDrive->value();
        break;
    case 'h':
        speed = ui->speedDriveRight->value();
        break;
    case 'l':
        speed = ui->speedLeft->value();
        break;
    case 'r':
        speed = ui->speedRight->value();
        break;
    case 'b':
        speed = ui->speedBack->value();
        break;
    default:
        break;
    }

    // Behöver ingen hastighet för "stop"!
    if("vdhlsrb"[dir] != 's')
        port->write(QByteArray(1, speed));
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
            case Qt::Key_C:
                if(event->type() == QEvent::KeyPress) {
                    w->open_claw();
                }
                else if(event->type() == QEvent::KeyRelease) {
                    w->close_claw();
                }
                break;
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
//c - claw in
//o - claw out
void MainWindow::on_pushButton_11_clicked() { open_claw(); }
void MainWindow::on_pushButton_12_clicked() { close_claw(); }

void MainWindow::on_pushButton_14_clicked()
{
    port->write(QByteArray(1, 't'));
    port->flush();
}

void MainWindow::open_claw()
{
    port->write(QByteArray(1, 'o'));
    port->write(QByteArray(1, ui->speedClawIn->value()));
    port->flush();
}

void MainWindow::close_claw()
{
    port->write(QByteArray(1, 'c'));
    port->write(QByteArray(1, ui->speedClawOut->value()));
    port->flush();
}

void MainWindow::on_pushButton_2_released() { ui->pushButton_2->setChecked(true); }
void MainWindow::on_pushButton_3_released() { ui->pushButton_3->setChecked(true); }
void MainWindow::on_pushButton_4_released() { ui->pushButton_4->setChecked(true); }
void MainWindow::on_pushButton_5_released() { ui->pushButton_5->setChecked(true); }
void MainWindow::on_pushButton_6_released() { ui->pushButton_6->setChecked(true); }
void MainWindow::on_pushButton_7_released() { ui->pushButton_7->setChecked(true); }
void MainWindow::on_pushButton_10_released() { ui->pushButton_10->setChecked(true); }


void KeyPressEater::on_pushButton_9_clicked(){}
void MainWindow::on_pushButton_9_clicked()
{
    pid.show();
}

void KeyPressEater::on_pushButton_clicked(){}
void MainWindow::on_pushButton_clicked()
{
//    QString str = ui->stringEdit;
    QByteArray array;

    for(int i = 0; i < ui->stringEdit->text().length(); i++)
    {
        array.append('z');
        array.append(ui->stringEdit->text().at(i));
    }
    port->write(array);
    port->flush();
}


void KeyPressEater::on_pushButtonPID_clicked(){}
void MainWindow::on_pushButtonPID_clicked()
{
    QByteArray array;
    array.append('p');
    array.append(ui->spinBoxP->value());
    array.append(ui->spinBoxI->value());
    array.append(ui->spinBoxD->value());
    port->write(array);
    port->flush();
}

void KeyPressEater::on_pushButtonClearDisplay_clicked(){}
void MainWindow::on_pushButtonClearDisplay_clicked()
{
    QByteArray array;
    array.append('q');      // Clear display command!
    port->write(array);
    port->flush();
}

void KeyPressEater::on_pushButtonLeft90_clicked(){}
void MainWindow::on_pushButtonLeft90_clicked()
{
    QByteArray array;
    array.append('w');
    port->write(array);
    port->flush();
}

void KeyPressEater::on_pushButtonRight90_clicked(){}
void MainWindow::on_pushButtonRight90_clicked()
{
    QByteArray array;
    array.append('e');
    port->write(array);
    port->flush();
}
