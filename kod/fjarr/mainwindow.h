#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class KeyPressEater : public QObject {
    Q_OBJECT

public:
    KeyPressEater(class MainWindow *w);

protected:
    bool eventFilter(QObject *recipient, QEvent *event);

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

private:
    Ui::MainWindow *ui;
    KeyPressEater *eat;
};

#endif // MAINWINDOW_H
