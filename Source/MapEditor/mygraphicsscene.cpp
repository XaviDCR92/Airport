#include "mygraphicsscene.h"
#include <QDebug>

MyGraphicsScene::MyGraphicsScene()
{

}

MyGraphicsScene::~MyGraphicsScene()
{

}

void MyGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* const mouseEvent)
{
    const QGraphicsItem* const it = itemAt(mouseEvent->scenePos(), QTransform());

    if (it != nullptr)
    {
        emit positionClicked(mouseEvent->scenePos());
    }
    else
    {
        // No items selected
        emit noItemSelected();
    }
}

void MyGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* const mouseEvent)
{
    emit updateSelectedItem(mouseEvent->button());
}
