#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QKeyEvent>
#include <iostream>
#include <QDebug>

#include <stdio.h>  // We need sprintf()

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

    ui->labelWarning->setVisible(false);

    bookmarks.append("V:%x\001,%x\002 F:%x\003    H:%x\004,%x\005");
    bookmarks.append("Tejp:%x\006,%x\007,%x\010,%x\011,%x\012,%x\013,%x\014,%x\015,%x\016");
    ui->bookmarks->addItem("Avstånd (default)", bookmarks.at(0));
    ui->bookmarks->addItem("Tejp", bookmarks.at(1));

    debug_message = sensor_data = 0;
    /* Ladda bokmärken från QSettings! */
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

void MainWindow::decode_comm(unsigned char b)
{
    if(debug_message) {
        if(b) {
            char str[2];
            str[0] = b;
            str[1] = '\0';
            ui->debugOutput->insertPlainText(QString(str));
        }
        else {
            QScrollBar *bar = ui->debugOutput->verticalScrollBar();
            bar->setValue(bar->maximum());
            debug_message = 0;
        }
    }
    else if(sensor_data == 1) {
        sensor_bytes_left = b;
        sensor_buffer_p = 0;
        sensor_data = 2;
    }
    else if(sensor_data == 2) {
        sensor_buffer[sensor_buffer_p++] = b;
        if(!--sensor_bytes_left) {
            ui->label_2->setText(QString("%1 cm").arg((int)sensor_buffer[1]));
            ui->label_4->setText(QString("%1 cm").arg((int)sensor_buffer[2]));
            sensor_data = 0;
        }
    }
    else {
        if(b == 'd') debug_message = 1;
        else if(b == 's') sensor_data = 1;
    }
}

void MainWindow::onDataAvailable()
{
    int i;
    QByteArray a = port->readAll();
    for(i = 0; i < a.size(); ++i) {
        decode_comm(a[i]);
    }
}

MainWindow::~MainWindow()
{
    /* Spara bokmärken med QSettings */
    port->close();
    delete port;
    delete eat;
    delete ui;
}

void MainWindow::setDirection(unsigned dir)
{
    if(dir == current_direction) {
        if(dir == 4) {
            // Så att man alltid kan trycka på stopp
            port->write("s", 1);
            port->flush();
        }
        return;
    }

    arrow_keys[current_direction]->setChecked(false);
    arrow_keys[dir]->setChecked(true);
    current_direction = dir;

    port->write(QByteArray(1, "vdhlsrb"[dir]));
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
    if(event->type() == QEvent::KeyPress && (static_cast<QKeyEvent *>(event))->key() == Qt::Key_Q)
    {
        w->ui->pushButton_9->toggle();
    }
    if(!w->ui->pushButton_9->isChecked()) {
        if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
            QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
            if(!key_event->isAutoRepeat())
            {
                switch(key_event->key()) {
                case Qt::Key_W: { w->is_pressed[0] = event->type() == QEvent::KeyPress; break; }
                case Qt::Key_S: { w->is_pressed[1] = event->type() == QEvent::KeyPress; break; }
                case Qt::Key_A: { w->is_pressed[2] = event->type() == QEvent::KeyPress; break; }
                case Qt::Key_D: { w->is_pressed[3] = event->type() == QEvent::KeyPress; break; }
                case Qt::Key_E:
                    if(event->type() == QEvent::KeyPress) {
                        w->open_claw();
                    }
                    else if(event->type() == QEvent::KeyRelease) {
                        w->close_claw();
                    }
                    break;
                }

                switch(key_event->key()) {
                case Qt::Key_W:
                case Qt::Key_S:
                case Qt::Key_A:
                case Qt::Key_D:
                    w->updateKeyState();
                    return true;
                }
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
    port->write("t");
    port->flush();
}

void MainWindow::open_claw()
{
    port->write("o");
    port->flush();
}

void MainWindow::close_claw()
{
    port->write("c");
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
//    pid.show();
}

void MainWindow::on_pushButton_9_toggled(bool pressed)
{
    if(pressed) {
        setDirection(4);
        ui->frame->setEnabled(false);
        port->write("n");
        port->flush();
    }
    else {
        port->write("ms");
        port->flush();
        ui->frame->setEnabled(true);
    }
}

/*
void KeyPressEater::on_pushButton_clicked(){}
void MainWindow::on_pushButton_clicked()
{
//    QString str = ui->stringEdit;
    if(ui->stringEdit->text().length() > 0)
    {
        QByteArray array;

        for(int i = 0; i < ui->stringEdit->text().length(); i++)
        {
            array.append('z');
            array.append(ui->stringEdit->text().at(i));
        }
        port->write(array);
        port->flush();
    }
}
*/

void KeyPressEater::on_pushButtonPID_clicked(){}
void MainWindow::on_pushButtonPID_clicked()
{
    QByteArray array;
    array.append('p');
    array.append(ui->spinBoxP->value());
    array.append(ui->spinBoxA->value());
    array.append((ui->spinBoxD->value() >> 8) & 0xff);
    array.append(ui->spinBoxD->value() & 0xff);
    port->write(array);
    port->flush();
}
/*
void KeyPressEater::on_pushButtonClearDisplay_clicked(){}
void MainWindow::on_pushButtonClearDisplay_clicked()
{
    port->write("q");
    port->flush();
    ui->stringEdit->setText("");
}
*/
void KeyPressEater::on_pushButtonLeft90_clicked(){}
void MainWindow::on_pushButtonLeft90_clicked()
{
    port->write("w");
    port->flush();
}

void KeyPressEater::on_pushButtonRight90_clicked(){}
void MainWindow::on_pushButtonRight90_clicked()
{
    port->write("e");
    port->flush();
}

void KeyPressEater::on_pid_toggle_clicked(){}

void KeyPressEater::on_pushButtonAddToDisplay_clicked(){}
void MainWindow::on_pushButtonAddToDisplay_clicked()
{
    if(ui->radioButtonText->isChecked())
    {
        printfString.append(ui->displayText->text());
    } else if(ui->radioButtonSensor->isChecked())
    {
        int sensor = ui->comboBoxSensor->currentIndex() + 1;
        QString str;
        if(ui->comboBoxBase->currentText() == "decimal")
        {
            str = QString("%d") + sensor;
        } else  // Hex!
        {
            str = QString("%X") +  sensor;
        }
        printfString.append(str);
    } else // ui->radioButtonBookmark->isChecked == true
    {
        printfString = bookmarks.at(ui->bookmarks->currentIndex());
    }
    updateDisplayExample();
}

void MainWindow::updateDisplayExample()
{
    uint8_t printfRawString[BUFFER_SIZE];
    for(int i = 0; i < printfString.length(); i++)
    {
        printfRawString[i] = printfString.at(i);
    }
    printfRawString[printfString.length()] = '\0';

    /* Dummy-värden, för exempel-fönstret */
    uint8_t sensor_buffer[BUFFER_SIZE] = {100, 101, 102, 103, 104, 105, 106, 107, 108,
                                  109, 110, 111, 112, 113, 114, 115, 116 };
    uint8_t *inp = printfRawString;

    char tmpStr[BUFFER_SIZE];
    char *tmpp = tmpStr;
    while(inp < printfRawString + BUFFER_SIZE)
    {
        if(*inp != '%') // Normal-tecken, kopiera och gå vidare!
        {
            *tmpp = *inp;
            if(*inp == '\0')
                break;
            inp++;
            tmpp++;
            continue;
        } else
        {
            inp++;    // Bortom %-tecknet

            uint8_t base;   // Nästa är d för decimal, x för hex
            if(*inp == 'd')
                base = 10;
            else if((*inp == 'x') || (*inp == 'X'))
                base = 16;
            inp++;

            uint8_t sensor = *inp; // Nästa är sensor-index
            inp++;
            if(sensor > MAX_SENSORS)
                continue;
            if(base == 10)
            {
                sprintf(tmpp, "%3d", sensor_buffer[sensor]);
                tmpp++; // Decimal-strängen är tre tecken
                tmpp++;
//                tmpp++;
            } else
            {
                sprintf(tmpp, "%02X", sensor_buffer[sensor]);
                tmpp++; // Hex-strängen är två tecken
                tmpp++;
            }
            continue;
        }
    }
    QString finishedString = QString(tmpStr);
    ui->displayExample->setText(finishedString);
    if(finishedString.length() > 32)
        ui->labelWarning->setVisible(true);
    else
        ui->labelWarning->setVisible(false);

}

void KeyPressEater::on_pushButton_15_clicked(){}
void MainWindow::on_pushButton_15_clicked()
{
    printfString.clear();
    ui->displayExample->clear();
    ui->labelWarning->setVisible(false);
}


void KeyPressEater::on_pushButton_13_clicked(){}
void MainWindow::on_pushButton_13_clicked()
{
    QByteArray array;

    if(printfString.length() == 0) {
        array.append('z');
        array.append(' ');
    }
    else {
        for(int i = 0; i < printfString.length(); i++)
        {
            array.append('z');
            array.append(printfString.at(i));
        }
    }
    array.append('z');
    array.append('\0');
    port->write(array);
    port->flush();
}

void MainWindow::on_pushButton_8_clicked()
{
    port->write("1");
    port->flush();
}

void KeyPressEater::on_pushButton_5_clicked() {}
void KeyPressEater::on_pushButton_8_clicked() {}

void KeyPressEater::on_pushButton_17_clicked(){}
void MainWindow::on_pushButton_17_clicked()
{
    if(ui->bookmarks->currentIndex() < 2)
    {
        ui->debugOutput->append("Kan inte ta bort de två första bokmärkena!");
        return;
    }
    ui->debugOutput->append("Removed: "  + ui->bookmarks->currentText());
    ui->bookmarks->removeItem(ui->bookmarks->currentIndex());
    bookmarks.remove(ui->bookmarks->currentIndex());
}


void KeyPressEater::on_pushButton_16_clicked(){}
void MainWindow::on_pushButton_16_clicked()
{
    ui->debugOutput->append("Added!");
    QString name = QInputDialog::getText(this, "Lägg till bokmärke", "Namn på bokmärke", QLineEdit::Normal);
    ui->bookmarks->addItem(name, printfString);
    bookmarks.append(printfString);
    ui->debugOutput->append("Added: "  + name);
}

void KeyPressEater::on_bookmarks_currentIndexChanged(int index) {(void) index;}
void MainWindow::on_bookmarks_currentIndexChanged(int index)
{
    if(index < 2)
        ui->pushButton_17->setEnabled(false);
    else
        ui->pushButton_17->setEnabled(true);
}

void KeyPressEater::on_speed_valueChanged(int arg1) { (void)arg1; }
void MainWindow::on_speed_valueChanged(int arg1) {
    unsigned char spd;
    port->write("#", 1);
    spd = arg1;
    port->write((const char *)&spd, 1);
    port->flush();
}
