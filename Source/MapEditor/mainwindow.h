#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>
#include <QPixmap>
#include <QShortcut>

#include "mygraphicsscene.h"
#include "ui_mainwindow.h"

#define APP_NAME                    QString("Airport Map Editor")
#define APP_VERSION_STRING          QString("0.3")
#define APP_FULL_NAME               APP_NAME + " " + APP_VERSION_STRING

#define TILE_SIZE           (64)
#define DATA_HEADER_SIZE    (0x3F)
#define TILE_MIRROR_FLAG    (0x80)

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void closeEvent(QCloseEvent*);

private:
    struct IsometricPos
    {
        short x;
        short y;
        short z;
    };

    struct CartesianPos
    {
        short x;
        short y;
    };

    bool checkFile(QFile &f, QFile::OpenModeFlag flags = QFile::ReadOnly);
    void appSettings(void);
    void loadTilesetData(void);
    void loadBuildingData(void);
    void parseMapData(QDataStream &ds);
    void addTile(quint8 CurrentTile, const int i, const int j);
    void addBuilding(quint8 CurrentBuilding, const int i, const int j);
    CartesianPos isometricToCartesian(const IsometricPos& ptrIsoPos) const
    {
        CartesianPos retCartPos;

        retCartPos.x = ptrIsoPos.x - (ptrIsoPos.x >> 1);
        retCartPos.x -= ptrIsoPos.y >> 1;

        retCartPos.y = ptrIsoPos.y >> 2;
        retCartPos.y += ptrIsoPos.x >> 2;
        retCartPos.y -= ptrIsoPos.z;

        return retCartPos;
    }

    Ui::MainWindow ui;
    QString _last_dir;
    MyGraphicsScene gscene;
    int level_size;
    QByteArray map_buffer;
    int selected_item;
    QHash<int, QString> tilesetData;
    QHash<int, QString> buildingData;
    QList<QGraphicsTextItem*> textItems;
    QShortcut tileSet;
    QShortcut tileMoveUp;
    QString tilesetPaths[2];
    QString buildingPath;

private slots:
    void loadMap(void);
    void onCreateMap(void);
    void processMapFile(const QByteArray &);
    void onMapItemClicked(QPointF);
    void onNoItemSelected(void);
    void onListItemSelected(void);
    void onSaveMap(void);
    void onShowNumbers(int);
    void onAirportNameModified(QString);
    void showError(const QString& error);
    void moveUp(void);
};

#endif // MAINWINDOW_H
