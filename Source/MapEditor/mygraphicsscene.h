#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

class MyGraphicsScene : public QGraphicsScene
{
public:
    MyGraphicsScene();

private:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);

};

#endif // MYGRAPHICSSCENE_H
