#ifndef TILE_GRAPHICS_VIEW_H
#define TILE_GRAPHICS_VIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QJsonObject>
#include <QUuid>
#include <QStack>
#include <QMenu>
#include <QPushButton>

#include "base_tile.h"

namespace Tile {

/**
 * Custom QGraphicsView.
 * Designed for advanced handling of Tile (see Tile.h).
 * Evaulates drops, instanciating derived Tile objects.
 * Implements bahavior for adapting screen size to widget resize.
 * Implements forwarding of drops to colliding Tile instances.
 * Holds functionality to convert all tiles in scene to JSON description
 * and be set from JSON.
*/
class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    GraphicsView(QGraphicsScene *scene, QWidget *parent);
    GraphicsView(QWidget *parent = 0);
    ~GraphicsView();

    /**
     * Parses all tiles in scene to JSON object.
    */
    const QJsonObject toJsonObject() const;

    /**
     * Creates all tiles in scene from JSON object.
     * Deletes existing scene.
     * Returns success of parsing.
    */
    bool setFromJsonObject(const QJsonObject& obj);

    /**
     * pushes a scene onto the scene stack and shows it.
    */
    void pushScene(QGraphicsScene*);

    /*
     * pops a scene from the scene stack and shows the next.
     * will not pop if there is only one scene left.
     * last scene on stakc is always main scene.
    */
    void popScene();

    void clearTiles();


private:
    /**
     * Handle scene size when widget resizes.
     * Scene extends with bigger widget size.
     * Scene never shrinks.
     * (so items placed in scene bounds can not get lost without moving).
    */
    void resizeEvent(QResizeEvent* e);

    void wheelEvent(QWheelEvent *event);

signals:
    void dropAccepted();

private slots:
    void onEmptyTile();

private:
    /**
     * accept drags.
    */
    void dragEnterEvent(QDragEnterEvent *event);

    /**
     * accept move.
    */
    void dragMoveEvent(QDragMoveEvent *event);

    /**
    * Derived tile types get deleted on drop into empty area of scene.
    * Drops are forwarded to Tile containing mouse position.
    */
    void dropEvent(QDropEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void mousePressEvent(QMouseEvent *event);

    void initContextMenu();

    QGraphicsScene* main_scene_;
    QStack<QGraphicsScene*> scene_stack_;
    QPoint click_pos_;
    QMenu* context_menu_;
};

} // namespace Tile

#endif // TILE_GRAPHICS_VIEW_H
