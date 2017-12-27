#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPointF>

class MyGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit MyGraphicsScene();
    ~MyGraphicsScene();

signals:
    void positionClicked(QPointF);
    void noItemSelected(void);
    void updateSelectedItem(void);

private:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);

};

#endif // MYGRAPHICSSCENE_H
