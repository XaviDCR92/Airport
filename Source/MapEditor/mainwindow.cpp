#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->LoadMap_Btn, SIGNAL(released()), this, SLOT(onLoadMap()));

    ui->openGLWidget->

    appSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onLoadMap(void)
{
    QString path = QFileDialog::getOpenFileName(this,
                                                "Open map file",
                                                _last_dir,
                                                "Map files (*.LVL)");

    QFile f(path);

    if(checkFile(f) == false)
    {
        return;
    }

    QDataStream txt(&f);


}

bool MainWindow::checkFile(QFile& f)
{
    QFileInfo fi(f);

    if(fi.exists() == false)
    {
        return false;
    }

    if(f.open(QFile::ReadWrite) == false)
    {
        return false;
    }

    QDir d(fi.absoluteFilePath());

    _last_dir = d.absolutePath();

    return true;
}

void MainWindow::appSettings(void)
{
    QSettings set("./settings.ini",QSettings::IniFormat);

    set.beginGroup("app_settings");

    _last_dir = set.value("last_dir").toString();

    set.endGroup();
}

void MainWindow::closeEvent(QCloseEvent*)
{
    QSettings set("./settings.ini",QSettings::IniFormat);

    set.beginGroup("app_settings");

    set.setValue("last_dir",_last_dir);

    set.endGroup();
}
