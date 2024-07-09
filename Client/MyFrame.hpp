#pragma once
#include "MyList.hpp"

#include <string>

#include <QComboBox>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

struct MyFrame : public QWidget {
  MyFrame() {
    box = new QComboBox();
    vbox = new QVBoxLayout();
    tree = new MyList();
    vbox->addWidget(box);
    vbox->addWidget(tree);
    setLayout(vbox);
    setMinimumWidth(500);
  }

  std::optional<std::string> UpdateCombo(QStringList &List) {
    std::cout<<"UpdateCombo"<<std::endl;
    QString currText = box->currentText();
    box->clear();
    box->addItems(List);

    if (List.empty())
      return {};
    if (!List.contains(currText))
      return List.first().toStdString();

    int Index = 0;
    for (; Index < List.size(); Index++) {
      if (List[Index] == currText) {
        break;
      }
    }

    box->setCurrentIndex(Index);
    return currText.toStdString();
  }

  QComboBox *box = nullptr;
  QVBoxLayout *vbox = nullptr;
  MyList *tree = nullptr;
};