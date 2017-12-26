#include "mainwindow.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    gscene(new MyGraphicsScene)
{
    ui->setupUi(this);

    connect(ui->LoadMap_Btn,    SIGNAL(released()), this,   SLOT(onLoadMap()));
    connect(ui->CreateMap_Btn,  SIGNAL(released()), this,   SLOT(onCreateMap()));

    appSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete gscene;
}

void MainWindow::onCreateMap(void)
{
    bool ok;
    QStringList items;

    items << "8";
    items << "16";
    items << "24";

    QString size = QInputDialog::getItem(this, tr("Create new map"),
                                         tr("Select map size:"), items, 0, false, &ok);

    if ( (ok == false) || (size.isEmpty() == true) )
    {
        return;
    }

    QByteArray data;

    data.append("ATC");

    if (size == "8")
    {
        data.append((char)0x08);
    }
    else if (size == "16")
    {
        data.append((char)0x10);
    }
    else if (size == "24")
    {
        data.append((char)0x18);
    }

    data.append("Default airport");

    for (int i = (data.count() - 1); i < 0x3F; i++)
    {
        data.append(0xFF);
    }

    qDebug() << data.count();

    int size_int = size.toInt(&ok, 10);

    if (ok == false)
    {
        qDebug() << "Invalid map size.";
        return;
    }

    for (int i = 0; i < size_int; i++)
    {
        for (int j = 0; j < size_int; j++)
        {
            data.append((char)0); // Building data
            data.append((char)0); // Terrain data
        }
    }

    qDebug() << "Created default map. Bytes: " + QString::number(data.count());

    onProcessMapFile(data);
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

    QByteArray data = f.readAll();

    onProcessMapFile(data);
}

void MainWindow::onProcessMapFile(QByteArray data)
{
    QDataStream ds(&data, QIODevice::ReadWrite);

    char header[3];

    ds.readRawData(header, 3);

    if (strncmp(header, "ATC", 3) != 0)
    {
        qDebug() << "Incorrect header";
        return;
    }

    char ch;

    ds.readRawData(&ch, sizeof(char));

    int level_size = ch;
    qDebug() << level_size;
    qDebug() << data.count();

    QPixmap tile1("..\\..\\Sprites\\TILESET1.bmp");

    int expected_filesize = (0x3F + (level_size * level_size));

    if (data.count() < expected_filesize)
    {
        qDebug() << "Invalid count. Expected " + QString::number(expected_filesize, 10);
        return;
    }

    ds.skipRawData(0x3B);

    gscene->clear();
    gscene->clearFocus();

    for (int j = 0; j < level_size; j++)
    {
        for (int i = 0; i < level_size; i++)
        {
            int u;
            int v;
            char byte[2];
            ds.readRawData(byte, 2);
            quint8 CurrentTile = byte[1];

            if (CurrentTile & 0x80)
            {
                u = (int)((CurrentTile & 0x7F) % 4) * 64;
                v = (int)((CurrentTile & 0x7F) / 4) * 48;
            }
            else
            {
                u = (int)(CurrentTile % 4) * 64;
                v = (int)(CurrentTile / 4) * 48;
            }

            QImage cropped = tile1.copy(u, v, 64, 48).toImage();

            if (CurrentTile & 0x80)
            {
                cropped = cropped.mirrored(true, false);
            }

            qDebug() << CurrentTile;

            cropped = cropped.convertToFormat(QImage::Format_ARGB32); // or maybe other format

            for (int i = 0; i < cropped.width(); i++)
            {
                for (int j = 0; j < cropped.height(); j++)
                {
                    QColor rgb = cropped.pixel(i, j);

                    if (rgb == QColor(Qt::magenta))
                    {
                        cropped.setPixel(i, j, qRgba(0,0,0,0));
                    }
                }
            }

            QGraphicsPixmapItem* it = gscene->addPixmap(QPixmap::fromImage(cropped));
            int x;
            int y;

            x = (i * 64) - (i * 32);
            x -= (j * 32);

            y = (j * 16);
            y += i * 16;

            it->setX(x);
            it->setY(y);
        }
    }

    ui->graphicsView->setScene(gscene);
    ui->graphicsView->show();
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
