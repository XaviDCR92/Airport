#include "mainwindow.h"
#include <QPainter>
#include <QGraphicsScene>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->LoadMap_Btn, SIGNAL(released()), this, SLOT(onLoadMap()));

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

    QPixmap pix;

    if(pix.load("C:\\cygwin\\home\\Xavier\\Airport\\Sprites\\Tileset1.bmp") == false)
    {
        qDebug() << "Error loading bitmap.";
    }

    QPainter p;

    p.begin(&pix);

    p.setWindow(ui->widget->geometry());

    p.drawPixmap(ui->widget->x(), ui->widget->y(), pix);

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
