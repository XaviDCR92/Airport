#include "mygraphicsscene.h"
#include <QDebug>

MyGraphicsScene::MyGraphicsScene()
{

}

void MyGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsItem *it = this->itemAt(mouseEvent->scenePos(), QTransform());
    if (it != NULL)
    {

    }
    else
    {
        // No items selected
    }
}
