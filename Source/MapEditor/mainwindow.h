#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>

#include "mygraphicsscene.h"
#include "ui_mainwindow.h"

#define APP_NAME                    QString("Airport Map Editor")
#define APP_VERSION_STRING          QString("0.2")

#define TILE_SIZE           64
#define DATA_HEADER_SIZE    0x3F
#define TILE_MIRROR_FLAG    ((char) 0x80)

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
    bool checkFile(QFile &f, QFile::OpenModeFlag flags = QFile::ReadOnly);
    void appSettings(void);
    void loadTilesetData(void);
    QString _last_dir;
    MyGraphicsScene *gscene;
    int level_size;
    QByteArray map_buffer;
    int selected_item;
    QHash<int, QString> tilesetData;

private slots:
    void onLoadMap(void);
    void onCreateMap(void);
    void onProcessMapFile(QByteArray);
    void onMapItemClicked(QPointF);
    void onNoItemSelected(void);
    void onListItemSelected(void);
    void onSaveMap(void);
    void onShowNumbers(int);
    void onAirportNameModified(QString);
};

#endif // MAINWINDOW_H
