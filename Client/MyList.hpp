#pragma once
#include "MyListItem.hpp"
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QListWidget>
#include <QMimeData>
#include <iostream>

class MyList : public QListWidget {
  Q_OBJECT
public:
  MyList() {
    setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    //setDragEnabled(true);
    //setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    viewport()->setAcceptDrops(true);
    //setDropIndicatorShown(true);
    //setDefaultDropAction(Qt::CopyAction);
    setAcceptDrops(true);
    //setDragDropOverwriteMode(true);
  }

protected:
  // void dragEnterEvent(QDragEnterEvent *event) override
  // {
  //     if (event->source() != this)
  //     {
  //         setDragDropMode(QAbstractItemView::DragDrop);
  //         event->accept();
  //     }
  //     else
  //     {
  //         setDragDropMode(QAbstractItemView::InternalMove);
  //         event->accept();
  //     }
  // }
signals:
  void ItemDropped();

protected:
  void dropEvent(QDropEvent *event) override {
    std::cout << "Accept" << std::endl;
    if (event->source() != this) {
      const auto Mimi = event->mimeData();
      File *file = reinterpret_cast<File *>(Mimi->property("File").toULongLong());

      QListWidgetItem *Item = new QListWidgetItem;
      MyListItem *widget = new MyListItem(this);
      widget->SetFileInfo(*file);
      Item->setSizeHint(widget->sizeHint());
      std::cout << widget->sizeHint().width() << " " << widget->sizeHint().height() << std::endl;
      addItem(Item);
      setItemWidget(Item, widget);

      event->accept();
    } else {
      event->setDropAction(Qt::DropAction::IgnoreAction);
      event->ignore();
    }
    // QListWidget::dropEvent(event);
    emit ItemDropped();
  }

  void mousePressEvent(QMouseEvent *e) override {
    if (e->button() == Qt::LeftButton)
      startPos = e->pos();
    QListWidget::mousePressEvent(e);
  }

  void mouseMoveEvent(QMouseEvent *e) override {
    if (e->buttons() & Qt::LeftButton) {
      int distance = (e->pos() - startPos).manhattanLength();
      if (distance >= 25)
        performDrag();
    }
    QListWidget::mouseMoveEvent(e);
  }
  void performDrag() {
    QListWidgetItem *item = currentItem();
    MyListItem *widget = static_cast<MyListItem *>(itemWidget(item));

    QMimeData *data = new QMimeData;
    data->setProperty("File", QVariant(reinterpret_cast<qulonglong>(&widget->file)));
    QDrag *drag = new QDrag(this);
    drag->setMimeData(data);
    // data->setUserData(0,);
    if (drag->exec(Qt::CopyAction) == Qt::CopyAction) {
      std::cout << "COpy" << std::endl;
    } else {
      std::cout << "Copy doomed" << std::endl;
    }
  }
  QPoint startPos;
  void startDrag(Qt::DropActions supportedActions) override { QListWidget::startDrag(supportedActions); }

  // if this two will not be implemented DnD will works not properly
  void dragMoveEvent(QDragMoveEvent *event) override {
    if (event->source() != this) {
      // std::cout << "From Not here" << std::endl;
      event->setDropAction(Qt::CopyAction);
      event->accept();
    } else {
      // std::cout << "From here" << std::endl;
      event->setDropAction(Qt::DropAction::IgnoreAction);
      event->ignore();
    }
    QWidget::dragMoveEvent(event);
  }
  Qt::DropAction supportedDropActions() { return Qt::CopyAction; }
};