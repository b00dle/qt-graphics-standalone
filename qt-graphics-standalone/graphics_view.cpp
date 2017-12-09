#include "graphics_view.h"

#include <QDebug>
#include <QJsonArray>
#include <QMimeData>

namespace Tile {

GraphicsView::GraphicsView(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
    , main_scene_(scene)
    , scene_stack_()
    , click_pos_()
    , context_menu_()
{
    pushScene(main_scene_);
    setAcceptDrops(true);
    setFocusPolicy(Qt::ClickFocus);
    initContextMenu();
}

GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , main_scene_(0)
    , scene_stack_()
    , click_pos_()
    , context_menu_()
{
    main_scene_ = new QGraphicsScene(QRectF(0,0,100,100),this);
    pushScene(main_scene_);
    setAcceptDrops(true);
    setFocusPolicy(Qt::ClickFocus);
    setMinimumSize(400, 400);
    initContextMenu();
}

GraphicsView::~GraphicsView()
{
}

const QJsonObject GraphicsView::toJsonObject() const
{
    QJsonObject obj;

    // parse scene properties
    QJsonObject scene_obj;
    QJsonObject scene_rect_obj;
    scene_rect_obj["x"] = sceneRect().x();
    scene_rect_obj["y"] = sceneRect().y();
    scene_rect_obj["width"] = sceneRect().width();
    scene_rect_obj["height"] = sceneRect().height();
    scene_obj["scene_rect"] = scene_rect_obj;

    // parse all tiles in scene
    QJsonArray arr_tiles;
    foreach(QGraphicsItem* it, scene()->items()) {
        QObject *obj = dynamic_cast<QObject*>(it);
        if(obj) {
            BaseTile* t = qobject_cast<BaseTile*>(obj);
            QJsonObject obj_tile;
            obj_tile["type"] = QJsonValue(t->metaObject()->className());
            obj_tile["data"] = QJsonValue(t->toJsonObject());
            arr_tiles.append(obj_tile);
        }
    }
    scene_obj["tiles"] = QJsonValue(arr_tiles);

    obj["scene"] = scene_obj;

    return obj;
}

bool GraphicsView::setFromJsonObject(const QJsonObject &obj)
{
    if(obj.isEmpty() || !obj.contains("scene"))
        return false;
    if(!obj["scene"].isObject())
        return false;

    QJsonObject sc_obj = obj["scene"].toObject();
    if(!sc_obj.contains("scene_rect") || !sc_obj["scene_rect"].isObject())
        return false;
    if(!sc_obj.contains("tiles") || !sc_obj["tiles"].isArray())
        return false;

    // scene rect
    QJsonObject rc_obj = sc_obj["scene_rect"].toObject();
    if(rc_obj.contains("x") && rc_obj.contains("y") && rc_obj.contains("width") && rc_obj.contains("height")) {
        QRectF scene_rect = sceneRect();
        scene_rect.setX((qreal) rc_obj["x"].toDouble());
        scene_rect.setY((qreal) rc_obj["y"].toDouble());
        scene_rect.setWidth((qreal) rc_obj["width"].toDouble());
        scene_rect.setHeight((qreal) rc_obj["height"].toDouble());
        //setSceneRect(scene_rect);
        scene()->setSceneRect(scene_rect);
    }

    clearTiles();

    // tiles
    QJsonArray arr_tiles = sc_obj["tiles"].toArray();
    foreach(QJsonValue val, arr_tiles) {
        if(!val.isObject())
            continue;
        QJsonObject t_obj = val.toObject();
        if(!t_obj.contains("type") || !t_obj.contains("data") || !t_obj["data"].isObject())
            continue;

        // create tile, if type is Tile::PlaylistTile
        if(t_obj["type"].toString().compare("Tile::BaseTile") == 0) {
            BaseTile* tile = new BaseTile;
            tile->setFlag(QGraphicsItem::ItemIsMovable, true);
            tile->init();
            if(tile->setFromJsonObject(t_obj["data"].toObject())) {
                scene()->addItem(tile);
            }
            else {
                qDebug() << "FAILURE: Could not set Tile data from JSON.";
                qDebug() << " > data:" << t_obj["data"];
                qDebug() << " > Aborting.";
                delete tile;
                return false;
            }
        }
    }
    return true;
}

void GraphicsView::pushScene(QGraphicsScene* scene)
{
    scene_stack_.push(scene);
    setScene(scene);
}

void GraphicsView::popScene()
{
    if(scene_stack_.size() > 1) {
        scene_stack_.pop();
        setScene(scene_stack_.top());
    }
}
void GraphicsView::resizeEvent(QResizeEvent *e)
{
    QGraphicsView::resizeEvent(e);
    if(e->isAccepted()) {
        QRectF r = scene()->sceneRect();
        if(e->size().width() > r.width()) {
            r.setWidth(e->size().width());
        }
        if(e->size().height() > r.height()) {
            r.setHeight(e->size().height());
        }
        scene()->setSceneRect(r);
        //setSceneRect(r);
    }
}

void GraphicsView::wheelEvent(QWheelEvent *event)
{
    if(!scene())
        return;

    QPointF p(mapToScene(event->pos()));

    foreach(QGraphicsItem* item, scene()->items()){
        if (item->contains(item->mapFromScene(p))){
            QObject *selected_object = dynamic_cast<QObject*>(item);
            if(selected_object)
            {
                BaseTile* t = qobject_cast<BaseTile*>(selected_object);
                if(t){
                    t->receiveWheelEvent(event);
                    return;
                }
            }
        }
    }
    QGraphicsView::wheelEvent(event);
}

void GraphicsView::onEmptyTile()
{
    BaseTile* tile = new BaseTile;
    tile->setFlag(QGraphicsItem::ItemIsMovable, true);
    tile->init();
    tile->setPos(click_pos_);
    tile->setSize(0);
    tile->setName("Empty");

    // add to scene
    scene()->addItem(tile);
    tile->setSmallSize();
}

void GraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    //qDebug() << "GraphicView: drag Enter Event ";
    GraphicsView *source = qobject_cast<GraphicsView*>(event->source());
    if (event->source() && source != this) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void GraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    //qDebug() << "GraphicView: drag Enter Move";
    GraphicsView *source = qobject_cast<GraphicsView*>(event->source());
    if (event->source() && source != this) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void GraphicsView::dropEvent(QDropEvent *event)
{
    if(!scene())
        return;

    QPointF p(mapToScene(event->pos()));

    foreach(QGraphicsItem* item, scene()->items()){
        if (item->contains(item->mapFromScene(p))){
            QObject *selected_object = dynamic_cast<QObject*>(item);
            if(selected_object)
            {
                BaseTile* t = qobject_cast<BaseTile*>(selected_object);
                if(t){
                    t->receiveExternalData(event->mimeData());
                    return;
                }
            }
        }
    }

    // except event
    event->setDropAction(Qt::CopyAction);
    event->accept();
    emit dropAccepted();
}

void GraphicsView::keyPressEvent(QKeyEvent*)
{
    //qDebug() << event->key();
}

void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Backspace) {
        popScene();
        return;
    }

    foreach(QGraphicsItem* it, scene()->items()) {
        QObject* o = dynamic_cast<QObject*>(it);
        if(o) {
            BaseTile* t = qobject_cast<BaseTile*>(o);
            if(t->hasActivateKey() && t->getActivateKey() == QChar(event->key()))
                t->onActivate();
        }
    }
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    if(!event->isAccepted()) {
        if(event->button() == Qt::RightButton) {
            context_menu_->popup(event->globalPos());
            click_pos_ = event->pos();
            event->accept();
        }
    }
}

void GraphicsView::clearTiles()
{
    foreach(QGraphicsItem* it, scene()->items()) {
        QObject* o = dynamic_cast<QObject*>(it);
        if(o) {
            BaseTile* t = qobject_cast<BaseTile*>(o);
            t->onDelete();
        }
    }
}

void GraphicsView::initContextMenu()
{
    context_menu_ = new QMenu;

    QAction* empty = new QAction(tr("Create Tile"));
    connect(empty, SIGNAL(triggered()),
            this, SLOT(onEmptyTile()));

    context_menu_->addAction(empty);
}

} // namespace TwoD
