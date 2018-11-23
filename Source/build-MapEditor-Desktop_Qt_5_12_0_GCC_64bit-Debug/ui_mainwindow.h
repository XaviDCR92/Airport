/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QGraphicsView *graphicsView;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_2;
    QListWidget *tileList;
    QLabel *label_3;
    QListWidget *buildingList;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *showNumbers_Checkbox;
    QCheckBox *mirror_CheckBox;
    QHBoxLayout *horizontalLayout;
    QPushButton *CreateMap_Btn;
    QPushButton *LoadMap_Btn;
    QPushButton *saveMap_Btn;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QLineEdit *airportName_Label;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(920, 648);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy);
        centralWidget->setAutoFillBackground(true);
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        graphicsView = new QGraphicsView(centralWidget);
        graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(99);
        sizePolicy1.setVerticalStretch(99);
        sizePolicy1.setHeightForWidth(graphicsView->sizePolicy().hasHeightForWidth());
        graphicsView->setSizePolicy(sizePolicy1);
        graphicsView->setMinimumSize(QSize(0, 0));

        gridLayout->addWidget(graphicsView, 2, 2, 1, 1);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        verticalLayout_2->addWidget(label_2);

        tileList = new QListWidget(centralWidget);
        tileList->setObjectName(QString::fromUtf8("tileList"));
        tileList->setMaximumSize(QSize(256, 16777215));

        verticalLayout_2->addWidget(tileList);

        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout_2->addWidget(label_3);

        buildingList = new QListWidget(centralWidget);
        buildingList->setObjectName(QString::fromUtf8("buildingList"));

        verticalLayout_2->addWidget(buildingList);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        showNumbers_Checkbox = new QCheckBox(centralWidget);
        showNumbers_Checkbox->setObjectName(QString::fromUtf8("showNumbers_Checkbox"));
        showNumbers_Checkbox->setChecked(true);

        horizontalLayout_2->addWidget(showNumbers_Checkbox);

        mirror_CheckBox = new QCheckBox(centralWidget);
        mirror_CheckBox->setObjectName(QString::fromUtf8("mirror_CheckBox"));

        horizontalLayout_2->addWidget(mirror_CheckBox);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        CreateMap_Btn = new QPushButton(centralWidget);
        CreateMap_Btn->setObjectName(QString::fromUtf8("CreateMap_Btn"));

        horizontalLayout->addWidget(CreateMap_Btn);

        LoadMap_Btn = new QPushButton(centralWidget);
        LoadMap_Btn->setObjectName(QString::fromUtf8("LoadMap_Btn"));

        horizontalLayout->addWidget(LoadMap_Btn);

        saveMap_Btn = new QPushButton(centralWidget);
        saveMap_Btn->setObjectName(QString::fromUtf8("saveMap_Btn"));

        horizontalLayout->addWidget(saveMap_Btn);


        verticalLayout_2->addLayout(horizontalLayout);


        gridLayout->addLayout(verticalLayout_2, 2, 1, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_3->addWidget(label);

        airportName_Label = new QLineEdit(centralWidget);
        airportName_Label->setObjectName(QString::fromUtf8("airportName_Label"));
        airportName_Label->setMaxLength(18);

        horizontalLayout_3->addWidget(airportName_Label);


        gridLayout->addLayout(horizontalLayout_3, 0, 1, 1, 1);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 920, 20));
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        label_2->setText(QApplication::translate("MainWindow", "Tile list:", nullptr));
        label_3->setText(QApplication::translate("MainWindow", "Building list:", nullptr));
        showNumbers_Checkbox->setText(QApplication::translate("MainWindow", "Show numbers on map", nullptr));
        mirror_CheckBox->setText(QApplication::translate("MainWindow", "Mirror tile", nullptr));
        CreateMap_Btn->setText(QApplication::translate("MainWindow", "Create map", nullptr));
        LoadMap_Btn->setText(QApplication::translate("MainWindow", "Load map", nullptr));
        saveMap_Btn->setText(QApplication::translate("MainWindow", "Save map", nullptr));
        label->setText(QApplication::translate("MainWindow", "Airport name:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
