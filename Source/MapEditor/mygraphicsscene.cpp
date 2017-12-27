#include "mygraphicsscene.h"
#include <QDebug>

MyGraphicsScene::MyGraphicsScene()
{

}

MyGraphicsScene::~MyGraphicsScene()
{

}

void MyGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsItem *it = this->itemAt(mouseEvent->scenePos(), QTransform());

    if (it != NULL)
    {        
        emit positionClicked(mouseEvent->scenePos());
    }
    else
    {
        // No items selected
        emit noItemSelected();
    }
}

void MyGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    emit updateSelectedItem();
}
