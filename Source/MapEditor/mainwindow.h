#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>

#include "mygraphicsscene.h"
#include "ui_mainwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void closeEvent(QCloseEvent*);

private:
    Ui::MainWindow *ui;
    bool checkFile(QFile &f);
    void appSettings(void);
    QString _last_dir;
    MyGraphicsScene *gscene;

protected slots:
    void onLoadMap(void);
    void onCreateMap(void);
    void onProcessMapFile(QByteArray data);
};

#endif // MAINWINDOW_H
