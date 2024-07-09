#pragma once
#include "protos/Transfer.pb.h"
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMimeData>
#include <iostream>

class MyListItem : public QWidget {
  Q_OBJECT
public:
  MyListItem(QWidget *parent = nullptr);
  void SetFileInfo(const ::File &NewFileInfo) {
    file = NewFileInfo;
    filename->setText(QString::fromStdString(file.name()));
    sizeL->setText(QString("%1").arg(file.filesize()));
    lastddateL->setText(QString("%1").arg(file.lastdate()));
  }
  virtual ~MyListItem() final {}

  ::File file = {};
private:
  std::unique_ptr<QHBoxLayout> hlayout = nullptr;
  std::unique_ptr<QLabel> filename = nullptr;
  std::unique_ptr<QLabel> sizeL = nullptr;
  std::unique_ptr<QLabel> lastddateL = nullptr;
};