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
    void positionClicked(QPointF position);
    void noItemSelected(void);
    void updateSelectedItem(Qt::MouseButton button);

private:
    void mousePressEvent(QGraphicsSceneMouseEvent * const mouseEvent);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * const mouseEvent);

};

#endif // MYGRAPHICSSCENE_H
