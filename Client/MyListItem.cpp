
#include "MyListItem.hpp"

MyListItem::MyListItem(QWidget *parent) : QWidget(parent) {
  setMinimumHeight(40);
  hlayout = std::make_unique<QHBoxLayout>();
  filename = std::make_unique<QLabel>();
  sizeL = std::make_unique<QLabel>();
  lastddateL = std::make_unique<QLabel>();
  hlayout->addWidget(filename.get(),50);
  hlayout->addWidget(sizeL.get(),20);
  hlayout->addWidget(lastddateL.get(),30);
  setLayout(hlayout.get());
}