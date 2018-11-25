#include "mainwindow.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>

#define DEFAULT_AIRPORT_NAME    QByteArray("Default Airport\0")

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    level_size(0),
    selected_item(-1),
    tileSet(tr("Space"), this),
    tileMoveUp(tr("Up"), this)
{
    ui.setupUi(this);
    this->setWindowTitle(APP_FULL_NAME);

    connect(ui.LoadMap_Btn,            SIGNAL(released()),                 this,   SLOT(loadMap()));
    connect(ui.CreateMap_Btn,          SIGNAL(released()),                 this,   SLOT(onCreateMap()));
    connect(ui.saveMap_Btn,            SIGNAL(released()),                 this,   SLOT(onSaveMap(void)));
    connect(ui.showNumbers_Checkbox,   SIGNAL(stateChanged(int)),          this,   SLOT(onShowNumbers(int)));
    connect(ui.airportName_Label,      SIGNAL(textChanged(QString)),       this,   SLOT(onAirportNameModified(QString)));

    connect(&gscene, SIGNAL(positionClicked(QPointF)),   this,   SLOT(onMapItemClicked(QPointF)));
    connect(&gscene, SIGNAL(noItemSelected(void)),       this,   SLOT(onNoItemSelected(void)));
    connect(&gscene, SIGNAL(updateSelectedItem(void)),   this,   SLOT(onListItemSelected(void)));

    // Configure keyboard shortcuts.
    connect(&tileSet, SIGNAL(activated()), this, SLOT(onListItemSelected(void)));
    connect(&tileMoveUp, SIGNAL(activated(void)), this, SLOT(moveUp(void)));

    appSettings();
    loadTilesetData();
    loadBuildingData();
}

MainWindow::~MainWindow()
{
    foreach (QGraphicsTextItem* it, textItems)
    {
        if (it != nullptr)
        {
            delete it;
        }
    }
}

void MainWindow::loadBuildingData(void)
{
    const QString path = "./buildings.ini";

    QSettings settings(path, QSettings::IniFormat);

    if (QFile(path).exists())
    {
        settings.beginGroup("buildings");

        buildingPath = settings.value("path").toString();

        int i = 0;

        while (1)
        {
            QString buildingNumber = "building" + QString::number(i);
            QString buildingName = settings.value(buildingNumber, "").toString();

            if (buildingName.isEmpty() )
            {
                break;
            }

            buildingData.insert(i++, buildingName);
            ui.buildingList->addItem(buildingName);
        }

        settings.endGroup();
    }
    else
    {
        showError(path + tr(" could not be found"));
    }
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
    foreach (const QListWidgetItem* const it, ui.tileList->selectedItems())
    {
        if (it != nullptr)
        {
            if (selected_item != -1)
            {
                const int map_buffer_pos = static_cast<int>(DATA_HEADER_SIZE
                                            + (static_cast<unsigned long>(selected_item + 1) * sizeof(quint16)));
                //map_buffer_pos++; // MSB: building data, LSB: terrain data.

                if (map_buffer_pos < map_buffer.count())
                {
                    const char row = static_cast<char>(ui.tileList->row(it));

                    map_buffer[map_buffer_pos] = row;

                    if (ui.mirror_CheckBox->isChecked() )
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

        if (not tilesetPaths[0].isEmpty()
                    &&
            not tilesetPaths[1].isEmpty())
        {
            const int expected_filesize = (DATA_HEADER_SIZE + (level_size * level_size));

            if (data.count() >= expected_filesize)
            {
                parseMapData(ds);
            }
            else
            {
                showError(tr("Invalid file size. Expected ")
                          + QString::number(expected_filesize, 10));
            }
        }
    }
    else
    {
        showError(tr("Invalid header") + "\"" + header + "\"");
    }
}

void MainWindow::parseMapData(QDataStream& ds)
{
    char airportName[0x1A];

    ds.readRawData(airportName, sizeof(airportName) / sizeof(airportName[0]));

    ui.airportName_Label->setText(QString(airportName));

    ds.skipRawData(0x3C - 0x1A);

    gscene.clear();
    gscene.clearFocus();

    QList<quint8> buildingData;


    for (int j = 0; j < level_size; j++)
    {
        for (int i = 0; i < level_size; i++)
        {
            char byte[2];
            ds.readRawData(byte, 2);

            qDebug() << "i = " + QString::number(i);
            qDebug() << "j = " + QString::number(j);
            qDebug() << QString::number(byte[0]);
            qDebug() << QString::number(byte[1]);
            qDebug() << "";

            buildingData.append(static_cast<quint8>(byte[0]));
            addTile(static_cast<quint8>(byte[1]), i, j);
        }
    }

    for (int j = 0; j < level_size; j++)
    {
        for (int i = 0; i < level_size; i++)
        {
            addBuilding(buildingData.at(i + (j* level_size)), i, j);
        }
    }

    ui.graphicsView->setScene(&gscene);
    ui.graphicsView->show();
    ui.graphicsView->centerOn(QPointF(320, 480));
}

#define NODATA  \
{               \
    false,      \
    {           \
        0,      \
        0,      \
        0       \
    },          \
    0,          \
    0,          \
    0,          \
    0,          \
    0,          \
    0           \
}

#define BUILDING_DATA(building) \
{                               \
    true,                       \
    {                           \
        building##_OFFSET_X, \
        building##_OFFSET_Y, \
        0                       \
    },                          \
    building##_ORIGIN_X,     \
    building##_ORIGIN_Y,     \
    building##_W,            \
    building##_H,            \
    building##_U,            \
    building##_V             \
}

void MainWindow::addBuilding(quint8 CurrentBuilding, const int i, const int j)
{
    if (CurrentBuilding)
    {
        enum
        {
            BUILDING_NONE,
            BUILDING_HANGAR,
            BUILDING_ILS,
            BUILDING_ATC_TOWER,
            BUILDING_ATC_LOC,
            BUILDING_TERMINAL,
            BUILDING_TERMINAL_2,
            BUILDING_GATE,

            LAST_BUILDING = BUILDING_GATE,
            MAX_BUILDING_ID
        };

        enum
        {
            BUILDING_ATC_LOC_OFFSET_X = TILE_SIZE >> 1,
            BUILDING_ATC_LOC_OFFSET_Y = TILE_SIZE >> 1,

            BUILDING_ILS_OFFSET_X = 0,
            BUILDING_ILS_OFFSET_Y = 0,

            BUILDING_GATE_OFFSET_X = (TILE_SIZE >> 1) - 4,
            BUILDING_GATE_OFFSET_Y = 0,

            BUILDING_HANGAR_OFFSET_X = 4,
            BUILDING_HANGAR_OFFSET_Y = TILE_SIZE >> 1,

            BUILDING_TERMINAL_OFFSET_X = 0,
            BUILDING_TERMINAL_OFFSET_Y = TILE_SIZE >> 1,

            BUILDING_TERMINAL_2_OFFSET_X = BUILDING_TERMINAL_OFFSET_X,
            BUILDING_TERMINAL_2_OFFSET_Y = BUILDING_TERMINAL_OFFSET_Y,

            BUILDING_ATC_TOWER_OFFSET_X = TILE_SIZE >> 2,
            BUILDING_ATC_TOWER_OFFSET_Y = TILE_SIZE >> 1,
        };

        enum
        {
            BUILDING_ILS_U = 34,
            BUILDING_ILS_V = 0,
            BUILDING_ILS_W = 24,
            BUILDING_ILS_H = 34,

            BUILDING_GATE_U = 0,
            BUILDING_GATE_V = 70,
            BUILDING_GATE_W = 28,
            BUILDING_GATE_H = 25,

            BUILDING_HANGAR_U = 0,
            BUILDING_HANGAR_V = 0,
            BUILDING_HANGAR_W = 34,
            BUILDING_HANGAR_H = 28,

            BUILDING_TERMINAL_U = 0,
            BUILDING_TERMINAL_V = 34,
            BUILDING_TERMINAL_W = 51,
            BUILDING_TERMINAL_H = 36,

            BUILDING_TERMINAL_2_U = 51,
            BUILDING_TERMINAL_2_V = BUILDING_TERMINAL_V,
            BUILDING_TERMINAL_2_W = BUILDING_TERMINAL_W,
            BUILDING_TERMINAL_2_H = BUILDING_TERMINAL_H,

            BUILDING_ATC_TOWER_U = 58,
            BUILDING_ATC_TOWER_V = 0,
            BUILDING_ATC_TOWER_W = 29,
            BUILDING_ATC_TOWER_H = 34,
        };

        enum
        {
            BUILDING_ILS_ORIGIN_X = 10,
            BUILDING_ILS_ORIGIN_Y = 22,

            BUILDING_GATE_ORIGIN_X = 20,
            BUILDING_GATE_ORIGIN_Y = 8,

            BUILDING_TERMINAL_ORIGIN_X = 20,
            BUILDING_TERMINAL_ORIGIN_Y = 11,

            BUILDING_TERMINAL_2_ORIGIN_X = BUILDING_TERMINAL_ORIGIN_X,
            BUILDING_TERMINAL_2_ORIGIN_Y = BUILDING_TERMINAL_ORIGIN_Y,

            BUILDING_HANGAR_ORIGIN_X = 16,
            BUILDING_HANGAR_ORIGIN_Y = 12,

            BUILDING_ATC_TOWER_ORIGIN_X = 12,
            BUILDING_ATC_TOWER_ORIGIN_Y = 20,
        };

        static const struct
        {
            bool init;
            IsometricPos IsoPos;  // Offset inside tile
            short orig_x;               // Coordinate X origin inside building sprite
            short orig_y;               // Coordinate Y origin inside building sprite
            short w;                    // Building width
            short h;                    // Building height
            short u;                    // Building X offset inside texture page
            short v;                    // Building Y offset inside texture page
        } GameBuildingData[MAX_BUILDING_ID] =
        {
            NODATA,
            BUILDING_DATA(BUILDING_HANGAR),
            BUILDING_DATA(BUILDING_ILS),
            BUILDING_DATA(BUILDING_ATC_TOWER),
            NODATA,
            BUILDING_DATA(BUILDING_TERMINAL),
            BUILDING_DATA(BUILDING_TERMINAL_2),
            BUILDING_DATA(BUILDING_GATE),
        };

        enum
        {
            TILE_SIZE_BIT_SHIFT = 6
        };

        const quint8 CurrentBuildingNoMirror = CurrentBuilding & 0x7F;

        if (CurrentBuildingNoMirror < MAX_BUILDING_ID)
        {
            if (GameBuildingData[CurrentBuildingNoMirror].init)
            {
                // Determine rendering order depending on Y value.
                const short x_bldg_offset = GameBuildingData[CurrentBuildingNoMirror].IsoPos.x;
                const short y_bldg_offset = GameBuildingData[CurrentBuildingNoMirror].IsoPos.y;
                const short z_bldg_offset = GameBuildingData[CurrentBuildingNoMirror].IsoPos.z;

                IsometricPos buildingIsoPos;

                buildingIsoPos.x = static_cast<short>(i << TILE_SIZE_BIT_SHIFT) + x_bldg_offset;
                buildingIsoPos.y = static_cast<short>(j << TILE_SIZE_BIT_SHIFT) - y_bldg_offset;
                buildingIsoPos.z = z_bldg_offset;

                // Isometric -> Cartesian conversion
                CartesianPos buildingCartPos = isometricToCartesian(buildingIsoPos);

                QPixmap p = QPixmap(buildingPath);

                QImage cropped = p.copy(GameBuildingData[CurrentBuildingNoMirror].u,
                                        GameBuildingData[CurrentBuildingNoMirror].v,
                                        GameBuildingData[CurrentBuildingNoMirror].w,
                                        GameBuildingData[CurrentBuildingNoMirror].h).toImage();

                cropped = cropped.convertToFormat(QImage::Format_ARGB32); // or maybe other format

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

                QGraphicsPixmapItem* const it = gscene.addPixmap(QPixmap::fromImage(cropped));

                if (it != nullptr)
                {
                    // Define new coordinates for building.
                    const int x = buildingCartPos.x - GameBuildingData[CurrentBuilding].orig_x;
                    const int y = buildingCartPos.y - GameBuildingData[CurrentBuilding].orig_y;

                    it->setX(x);
                    it->setY(y);
                }
            }
        }
    }
}
#undef NODATA
#undef BUILDING_DATA

void MainWindow::addTile(quint8 CurrentTile, const int i, const int j)
{
    enum
    {
        TILE_GRASS,
        TILE_ASPHALT_WITH_BORDERS,
        TILE_WATER,
        TILE_ASPHALT,

        TILE_RWY_MID,
        TILE_RWY_START_1,
        TILE_RWY_START_2,
        TILE_PARKING,

        TILE_PARKING_2,
        TILE_TAXIWAY_INTERSECT_GRASS,
        TILE_TAXIWAY_GRASS,
        TILE_TAXIWAY_CORNER_GRASS,

        TILE_HALF_WATER_1,
        TILE_HALF_WATER_2,
        TILE_RWY_HOLDING_POINT,
        TILE_RWY_HOLDING_POINT_2,

        TILE_RWY_EXIT,
        TILE_TAXIWAY_CORNER_GRASS_2,
        TILE_TAXIWAY_4WAY_CROSSING,
        TILE_RWY_EXIT_2,

        LAST_TILE_TILESET1 = TILE_RWY_EXIT_2,

        TILE_UNUSED_1,
        TILE_TAXIWAY_CORNER_GRASS_3,

        FIRST_TILE_TILESET2 = TILE_UNUSED_1,
        LAST_TILE_TILESET2 = TILE_TAXIWAY_CORNER_GRASS_3
    };

    const QPixmap tileset(tilesetPaths[0]);
    const QPixmap tileset2(tilesetPaths[1]);

    quint8 tileNoMirror = CurrentTile & 0x7F;
    const QPixmap* p = nullptr;

    if (tileNoMirror <= LAST_TILE_TILESET1)
    {
        p = &tileset;
    }
    else if (tileNoMirror <= LAST_TILE_TILESET2)
    {
        p = &tileset2;
        CurrentTile -= FIRST_TILE_TILESET2;
        tileNoMirror -= FIRST_TILE_TILESET2;
    }

    if (p != nullptr)
    {
        int u;
        int v;

        if (CurrentTile & TILE_MIRROR_FLAG)
        {
            u = static_cast<int>((tileNoMirror % 4) * 64);
            v = static_cast<int>((tileNoMirror / 4) * 48);
        }
        else
        {
            u = static_cast<int>((CurrentTile % 4) * 64);
            v = static_cast<int>((CurrentTile / 4) * 48);
        }

        QImage cropped = p->copy(u, v, 64, 48).toImage();

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

        QGraphicsPixmapItem* const it = gscene.addPixmap(QPixmap::fromImage(cropped));

        if (it != nullptr)
        {
            const int x = ((i * TILE_SIZE) - (i * (TILE_SIZE / 2))) - (j * (TILE_SIZE / 2));
            const int y = (j * (TILE_SIZE / 4)) + (i * (TILE_SIZE / 4));

            it->setX(x);
            it->setY(y);

            if (ui.showNumbers_Checkbox->isChecked() )
            {
                QGraphicsTextItem* const io = new QGraphicsTextItem();

                if (io != nullptr)
                {
                    io->setPos(x + (TILE_SIZE / 4), y);
                    io->setPlainText(QString::number(i + (j * level_size)));

                    gscene.addItem(io);

                    /* Append pointer to the list so it can be
                     * safely removed on the constructor. */
                    textItems.append(io);
                }
            }
        }
    }
}

bool MainWindow::checkFile(QFile& f, QFile::OpenModeFlag flags)
{
    const QFileInfo fi(f);

    if (not f.open(flags))
    {
        return false;
    }

    QDir d(fi.absoluteFilePath());

    _last_dir = d.absolutePath();

    return true;
}

void MainWindow::appSettings(void)
{
    QSettings settings("./settings.ini", QSettings::IniFormat);

    settings.beginGroup("app_settings");

    _last_dir = settings.value("last_dir").toString();

    restoreGeometry(settings.value("window_geometry").toByteArray());

    settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent*)
{
    QSettings settings("./settings.ini", QSettings::IniFormat);

    settings.beginGroup("app_settings");

    settings.setValue("last_dir", _last_dir);
    settings.setValue("window_geometry", saveGeometry());

    settings.endGroup();
}

void MainWindow::loadTilesetData(void)
{
    const QString filePath = "./tileset.ini";

    if (QFile(filePath).exists())
    {
        QSettings tilesetFile("./tileset.ini", QSettings::IniFormat);
        QStringList tilesets_to_check;

        tilesets_to_check << "tileset1";
        tilesets_to_check << "tileset2";

        int j = 0;
        int i = 0;

        foreach (QString tileset, tilesets_to_check)
        {
            tilesetFile.beginGroup(tileset);

            tilesetPaths[j++] = tilesetFile.value("path").toString();

            while (1)
            {
                QString tileNumber = "tile" + QString::number(i);
                QString tileName = tilesetFile.value(tileNumber, "").toString();

                if (tileName.isEmpty() )
                {
                    break;
                }

                tilesetData.insert(i++, tileName);
                ui.tileList->addItem(tileName);
            }

            tilesetFile.endGroup();
        }
    }
    else
    {
        showError(tr("Could not find tile data file ") + filePath);
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

void MainWindow::moveUp(void)
{

}

void MainWindow::showError(const QString& error)
{
    QMessageBox::critical(this, APP_FULL_NAME, error);
}
