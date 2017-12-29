#include "mainwindow.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    gscene(new MyGraphicsScene),
    selected_item(-1)
{
    ui->setupUi(this);

    ui->centralWidget->setWindowTitle("Airport Map Editor");

    connect(ui->LoadMap_Btn,            SIGNAL(released()),                 this,   SLOT(onLoadMap()));
    connect(ui->CreateMap_Btn,          SIGNAL(released()),                 this,   SLOT(onCreateMap()));
    connect(ui->saveMap_Btn,            SIGNAL(released()),                 this,   SLOT(onSaveMap(void)));
    connect(ui->showNumbers_Checkbox,   SIGNAL(stateChanged(int)),          this,   SLOT(onShowNumbers(int)));

    connect(gscene,                     SIGNAL(positionClicked(QPointF)),   this,   SLOT(onMapItemClicked(QPointF)));
    connect(gscene,                     SIGNAL(noItemSelected(void)),       this,   SLOT(onNoItemSelected(void)));
    connect(gscene,                     SIGNAL(updateSelectedItem(void)),   this,   SLOT(onListItemSelected(void)));

    appSettings();
    loadTilesetData();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete gscene;
}

void MainWindow::onShowNumbers(int)
{
    onProcessMapFile(map_buffer);
}

void MainWindow::onMapItemClicked(QPointF pos)
{
    QPoint realPos;

    pos.setX(pos.x() - (TILE_SIZE / 2));

    realPos.setX(pos.x() + (pos.y() * 2));
    realPos.setY((pos.y() * 2) - pos.x());

    int tile_no = 0;

    tile_no = realPos.x() / TILE_SIZE;
    tile_no += (realPos.y() / TILE_SIZE) * level_size;

    if (tile_no < (level_size * level_size))
    {
        selected_item = tile_no;
        onProcessMapFile(map_buffer);
    }
    else
    {
        selected_item = -1;
    }
}

void MainWindow::onListItemSelected(void)
{
    QList<QListWidgetItem *> item = ui->listWidget->selectedItems();

    foreach (QListWidgetItem *it, item)
    {
        int row = ui->listWidget->row(it);

        if (selected_item != -1)
        {
            int map_buffer_pos = (DATA_HEADER_SIZE + 1) + (selected_item * sizeof(quint16));
            //map_buffer_pos++; // MSB: building data, LSB: terrain data.

            qDebug() << "Calculated file offset: 0x" + QString::number(map_buffer_pos, 16);

            if (map_buffer_pos < map_buffer.count())
            {
                qDebug() << "Current data at 0x" + QString::number(map_buffer_pos, 16) + ": " + tilesetData.value(map_buffer[map_buffer_pos]);
                map_buffer[map_buffer_pos] = row;

                if (ui->mirror_CheckBox->isChecked() == true)
                {
                    map_buffer[map_buffer_pos] = map_buffer[map_buffer_pos] | TILE_MIRROR_FLAG;
                }

                onProcessMapFile(map_buffer);
            }
        }
    }
}

void MainWindow::onSaveMap(void)
{
    QString path = QFileDialog::getSaveFileName(this,
                                                "Save map file",
                                                _last_dir,
                                                "Map files (*.LVL)");

    QFile f(path);

    if(checkFile(f, QFile::WriteOnly) == false)
    {
        return;
    }

    f.write(map_buffer);

    f.close();
}

void MainWindow::onNoItemSelected(void)
{
    selected_item = -1;
    onProcessMapFile(map_buffer);
}

void MainWindow::onCreateMap(void)
{
    bool ok;
    QStringList items;

    items << "8";
    items << "16";
    items << "24";

    QString size = QInputDialog::getItem(   this,
                                            tr("Create new map"),
                                            tr("Select map size:"),
                                            items,
                                            0,
                                            false,
                                            &ok     );

    if ( (ok == false) || (size.isEmpty() == true) )
    {
        return;
    }

    QByteArray data;

    data.append("ATC");

    if (size == "8")
    {
        level_size = 8;
        data.append((char)0x08);
    }
    else if (size == "16")
    {
        level_size = 16;
        data.append((char)0x10);
    }
    else if (size == "24")
    {
        level_size = 24;
        data.append((char)0x18);
    }

    data.append("Default airport");

    for (int i = (data.count() - 1); i < DATA_HEADER_SIZE; i++)
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
    map_buffer = data;

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

    level_size = ch;

    QPixmap tile1("..\\..\\Sprites\\TILESET1.bmp");

    int expected_filesize = (DATA_HEADER_SIZE + (level_size * level_size));

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

            if (CurrentTile & TILE_MIRROR_FLAG)
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

            if (CurrentTile & TILE_MIRROR_FLAG)
            {
                cropped = cropped.mirrored(true, false);
            }

            bool selected = false;

            if (selected_item != -1)
            {
                if (selected_item == ((j * level_size) + i))
                {
                    selected = true;
                }
            }

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
                    else if (selected == true)
                    {
                        QColor c = cropped.pixelColor(i, j);

                        c.setRed(255 - c.red());
                        c.setBlue(255 - c.blue());
                        c.setGreen(255 - c.green());

                        cropped.setPixel(i, j, qRgb(c.red(), c.green(), c.blue()));
                    }
                }
            }

            QGraphicsPixmapItem* it = gscene->addPixmap(QPixmap::fromImage(cropped));
            int x;
            int y;

            x = (i * TILE_SIZE) - (i * (TILE_SIZE / 2));
            x -= (j * (TILE_SIZE / 2));

            y = (j * (TILE_SIZE / 4));
            y += i * (TILE_SIZE / 4);

            it->setX(x);
            it->setY(y);

            if (ui->showNumbers_Checkbox->isChecked() == true)
            {
                QGraphicsTextItem* io = new QGraphicsTextItem();
                io->setPos(x + (TILE_SIZE / 4), y);
                io->setPlainText(QString::number(i + (j * level_size)));

                gscene->addItem(io);
            }
        }
    }

    ui->graphicsView->setScene(gscene);
    ui->graphicsView->show();
}

bool MainWindow::checkFile(QFile& f, QFile::OpenModeFlag flags)
{
    QFileInfo fi(f);

    if(f.open(flags) == false)
    {
        return false;
    }

    QDir d(fi.absoluteFilePath());

    _last_dir = d.absolutePath();

    return true;
}

void MainWindow::appSettings(void)
{
    QSettings set("./settings.ini", QSettings::IniFormat);

    set.beginGroup("app_settings");

    _last_dir = set.value("last_dir").toString();

    set.endGroup();
}

void MainWindow::closeEvent(QCloseEvent*)
{
    QSettings set("./settings.ini", QSettings::IniFormat);

    set.beginGroup("app_settings");

    set.setValue("last_dir", _last_dir);

    set.endGroup();
}

void MainWindow::loadTilesetData(void)
{
    QFile f("./tileset.ini");

    if (f.exists() == false)
    {
        qDebug() << "tileset.ini does not exist. Please create it";
    }
    else
    {
        QSettings tilesetFile("./tileset.ini", QSettings::IniFormat);
        QStringList tilesets_to_check;

        tilesets_to_check << "tileset1";
        tilesets_to_check << "tileset2";

        int i = 0;

        foreach (QString tileset, tilesets_to_check)
        {
            tilesetFile.beginGroup(tileset);

            while (1)
            {
                QString tileNumber = "tile" + QString::number(i);
                QString tileName = tilesetFile.value(tileNumber, "").toString();

                if (tileName.isEmpty() == true)
                {
                    break;
                }

                tilesetData.insert(i++, tileName);
                ui->listWidget->addItem(tileName);
            }

            tilesetFile.endGroup();
        }
    }
}
