#include "mainwindow.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QInputDialog>
#include <QMessageBox>

#define DEFAULT_AIRPORT_NAME    QByteArray("Default Airport\0")

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    gscene(new MyGraphicsScene),
    selected_item(-1)
{
    ui->setupUi(this);
    this->setWindowTitle(APP_FULL_NAME);

    connect(ui->LoadMap_Btn,            SIGNAL(released()),                 this,   SLOT(loadMap()));
    connect(ui->CreateMap_Btn,          SIGNAL(released()),                 this,   SLOT(onCreateMap()));
    connect(ui->saveMap_Btn,            SIGNAL(released()),                 this,   SLOT(onSaveMap(void)));
    connect(ui->showNumbers_Checkbox,   SIGNAL(stateChanged(int)),          this,   SLOT(onShowNumbers(int)));
    connect(ui->airportName_Label,      SIGNAL(textChanged(QString)),       this,   SLOT(onAirportNameModified(QString)));

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
    processMapFile(map_buffer);
}

void MainWindow::onMapItemClicked(QPointF pos)
{
    QPoint realPos;

    pos.setX(pos.x() - (TILE_SIZE / 2));

    realPos.setX(static_cast<int>(pos.x() + (pos.y() * 2)));
    realPos.setY(static_cast<int>((pos.y() * 2) - pos.x()));

    int tile_no = 0;

    tile_no = realPos.x() / TILE_SIZE;
    tile_no += (realPos.y() / TILE_SIZE) * level_size;

    if (tile_no < (level_size * level_size))
    {
        selected_item = tile_no;
        processMapFile(map_buffer);
    }
    else
    {
        selected_item = -1;
    }
}

void MainWindow::onListItemSelected(void)
{
    foreach (const QListWidgetItem* const it, ui->tileList->selectedItems())
    {
        if (it != nullptr)
        {
            if (selected_item != -1)
            {
                const int map_buffer_pos = static_cast<int>((DATA_HEADER_SIZE + 1)
                                            + (static_cast<unsigned long>(selected_item) * sizeof(quint16)));
                //map_buffer_pos++; // MSB: building data, LSB: terrain data.

                if (map_buffer_pos < map_buffer.count())
                {
                    const char row = static_cast<char>(ui->tileList->row(it));

                    map_buffer[map_buffer_pos] = row;

                    if (ui->mirror_CheckBox->isChecked() )
                    {
                        map_buffer[map_buffer_pos] = map_buffer[map_buffer_pos] | TILE_MIRROR_FLAG;
                    }

                    processMapFile(map_buffer);
                }
            }
        }
    }
}

void MainWindow::onSaveMap(void)
{
    const QString path = QFileDialog::getSaveFileName(this,
                                                      "Save map file",
                                                      _last_dir,
                                                      "Map files (*.LVL)");

    QFile f(path);

    if (checkFile(f, QFile::WriteOnly) == false)
    {
        return;
    }

    f.write(map_buffer);

    f.close();
}

void MainWindow::onNoItemSelected(void)
{
    selected_item = -1;
    processMapFile(map_buffer);
}

void MainWindow::onCreateMap(void)
{
    bool ok;
    QStringList items;

    items << "8";
    items << "16";
    items << "24";

    const QString strSize = QInputDialog::getItem(this,
                                                  tr("Create new map"),
                                                  tr("Select map size:"),
                                                  items,
                                                  0,
                                                  false,
                                                  &ok     );

    if (ok && (not strSize.isEmpty()))
    {
        QByteArray data;

        data.append("ATC");

        const quint8 size = static_cast<quint8>(strSize.toInt(&ok));

        if (ok)
        {
            switch (size)
            {
                case 8:
                    // Fall through.
                case 16:
                    // Fall through.
                case 24:
                    level_size = size;
                    data.append(static_cast<char>(size));
                break;

                default:
                    showError(tr("Invalid map size ") + strSize);
                break;
            }

            data.append(DEFAULT_AIRPORT_NAME);

            for (int i = 0x04 + DEFAULT_AIRPORT_NAME.count(); i < 0x1C; i++)
            {
                data.append('\0');
            }

            for (int i = (data.count() - 1); i < DATA_HEADER_SIZE; i++)
            {
                data.append(static_cast<char>(0xFF));
            }

            for (quint8 i = 0; i < size; i++)
            {
                for (quint8 j = 0; j < size; j++)
                {
                    data.append(static_cast<char>(0)); // Building data
                    data.append(static_cast<char>(0)); // Terrain data
                }
            }

            processMapFile(data);
        }
    }
}

void MainWindow::loadMap(void)
{
    const QString path = QFileDialog::getOpenFileName(this,
                                                      "Open map file",
                                                      _last_dir,
                                                      "Map files (*.LVL)");

    QFile f(path);

    if (checkFile(f))
    {   
        processMapFile(f.readAll());
    }
}

void MainWindow::processMapFile(const QByteArray& data)
{
    map_buffer = data;

    QDataStream ds(data);

    char header[3];

    ds.readRawData(header, 3);

    if (strncmp(header, "ATC", 3) == 0)
    {
        char ch;

        ds.readRawData(&ch, sizeof(char));

        level_size = ch;

        const QString filePath = "../../Sprites/TILESET1.bmp";

        QPixmap tile1(filePath);

        const int expected_filesize = (DATA_HEADER_SIZE + (level_size * level_size));

        if (data.count() >= expected_filesize)
        {
            parseMapData(ds, tile1);
        }
        else
        {
            showError(tr("Invalid file size. Expected ")
                      + QString::number(expected_filesize, 10));
        }
    }
    else
    {
        showError(tr("Invalid header") + "\"" + header + "\"");
    }
}

void MainWindow::parseMapData(QDataStream& ds, const QPixmap& tileSet)
{
    char airportName[0x1A];

    ds.readRawData(airportName, sizeof(airportName) / sizeof(airportName[0]));

    ui->airportName_Label->setText(QString(airportName));

    ds.skipRawData(0x3B - 0x1A);

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
            quint8 CurrentTile = static_cast<quint8>(byte[1]);

            if (CurrentTile & TILE_MIRROR_FLAG)
            {
                u = static_cast<int>(((CurrentTile & 0x7F) % 4) * 64);
                v = static_cast<int>(((CurrentTile & 0x7F) / 4) * 48);
            }
            else
            {
                u = static_cast<int>((CurrentTile % 4) * 64);
                v = static_cast<int>((CurrentTile / 4) * 48);
            }

            QImage cropped = tileSet.copy(u, v, 64, 48).toImage();

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
                    else if (selected )
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

            if (ui->showNumbers_Checkbox->isChecked() )
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

    if (f.open(flags) == false)
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

                if (tileName.isEmpty() )
                {
                    break;
                }

                tilesetData.insert(i++, tileName);
                ui->tileList->addItem(tileName);
            }

            tilesetFile.endGroup();
        }
    }
}

void MainWindow::onAirportNameModified(QString name)
{
    if (map_buffer.isEmpty() )
    {
        return;
    }

    for (int i = 0x04, j = 0; i < 0x1C; i++)
    {
        if (j < name.count() )
        {
            map_buffer[i] = name.at(j++).toLatin1();
        }
        else
        {
            map_buffer[i] = '\0';
        }
    }
}

void MainWindow::showError(const QString& error)
{
    QMessageBox::critical(this, APP_FULL_NAME, error);
}
