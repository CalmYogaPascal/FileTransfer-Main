#pragma once
#include "Client.hpp"
#include "MyFrame.hpp"
#include "MyListItem.hpp"

#include <exception>
#include <stdexcept>

#include <QMainWindow>
#include <QMenuBar>
#include <QSplitter>
#include <QStringList>
#include <QTimer>

std::string what(const std::exception_ptr &eptr = std::current_exception()) {
  if (!eptr) {
    throw std::bad_exception();
  }

  try {
    std::rethrow_exception(eptr);
  } catch (const std::exception &e) {
    return e.what();
  } catch (const std::string &e) {
    return e;
  } catch (const char *e) {
    return e;
  } catch (...) {
    return "who knows";
  }
}

class MyWindow : public QMainWindow {
public:
  void OnConnect(bool) {
    std::cout << "OnConnect" << std::endl;
    grpc_context->GetConnectedList(
        [this](std::exception_ptr exc, std::set<std::string> List) {
          if (exc)
            std::cout << what(exc) << std::endl;
          std::cout << "Success";
          connectedList.clear();
          for (const auto &str : List) {
            connectedList.append(QString::fromStdString(str));
          }
          frame1->UpdateCombo(connectedList);
          frame2->UpdateCombo(connectedList);
        });
  }

  void OnReconnect(bool) { grpc_context->Connect("localhost:50051"); }
  void leftChangeIndex(int) {
    QStringList copy = connectedList;
    grpc_context->GetConnectedList(
        [this, copy](std::exception_ptr, std::set<std::string> List) {
          connectedList.clear();
          for (const auto &str : List) {
            connectedList.append(QString::fromStdString(str));
          }
          bool shouldReset = true;
          if (copy.size() == connectedList.size()) {
            shouldReset = false;
            for (int Index = 0; Index < copy.size(); Index++) {
              if (copy[Index] != connectedList[Index]) {
                shouldReset = true;
                break;
              }
            }
          }

          if (shouldReset) {
            frame1->UpdateCombo(connectedList);
            frame2->UpdateCombo(connectedList);
          }
        });
  }
  void LindexActivated(int) {
    if (frame1->box->count() == 0)
      frame1->tree->clear();

    auto client = frame1->box->currentText().toStdString();
    std::cout<<"Lft one: "<<client<<std::endl;

    grpc_context->GetFileMap(
        client, [this](std::exception_ptr, FsRequestType set) {
          frame1->tree->clear();
          for (const auto &str : set) {
            QListWidgetItem* Item = new QListWidgetItem;
            MyListItem* widget = new MyListItem(this);
            widget->SetFileInfo(str);
            Item->setSizeHint(widget->sizeHint());
            frame1->tree->addItem(Item);
            frame1->tree->setItemWidget(Item,widget);
          }
        });
  }
  void RindexActivated(int) {
    if (frame2->box->count() == 0)
      frame2->tree->clear();

    auto client = frame2->box->currentText().toStdString();
    std::cout<<"Rght one: "<<client<<std::endl;

    grpc_context->GetFileMap(
        client, [this](std::exception_ptr, FsRequestType set) {
          frame2->tree->clear();
          for (const auto &str : set) {
            QListWidgetItem* Item = new QListWidgetItem;
            MyListItem* widget = new MyListItem(this);
            widget->SetFileInfo(str);
            Item->setSizeHint(widget->sizeHint());
            std::cout<<widget->sizeHint().width()<<" "<<widget->sizeHint().height()<<std::endl;
            frame2->tree->addItem(Item);
            frame2->tree->setItemWidget(Item,widget);
          }
        });
  }

  MyWindow(GrpcTransferContext &InGrpc_context) {
    splitter = new QSplitter(this);
    frame1 = new MyFrame();
    frame2 = new MyFrame();
    connect(frame1->box, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &MyWindow::leftChangeIndex);
    connect(frame2->box, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &MyWindow::leftChangeIndex);

    connect(frame1->box, qOverload<int>(&QComboBox::activated), this,
            &MyWindow::LindexActivated);
    connect(frame2->box, qOverload<int>(&QComboBox::activated), this,
            &MyWindow::RindexActivated);
    splitter->addWidget(frame1);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(frame2);
    // connect(frame1,&QListWidget::)
    setCentralWidget(splitter);
    grpc_context = &InGrpc_context;
    setStyleSheet("QMainWindow::separator {"
                  "background: yellow;"
                  "width: 10px; /* when vertical */"
                  "height: 10px; /* when horizontal */"
                  "}"
                  "QMainWindow::separator : hover"
                  "{"
                  "background:"
                  "   red;"
                  "}");

    QMenuBar *bar = new QMenuBar();
    {
      QMenu *FileMenu = bar->addMenu("File");
      FileMenu->addSection("Connection");
      auto Connection = FileMenu->addAction("Connect");
      connect(Connection, &QAction::triggered, this, &MyWindow::OnConnect);
      auto REConnection = FileMenu->addAction("Reconnect");
      connect(REConnection, &QAction::triggered, this, &MyWindow::OnReconnect);
      FileMenu->addAction("Disconnect");
    }
    setMenuBar(bar);

    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, [this]() { grpc_context->poll(); });

    timer->start(std::chrono::milliseconds(500));

    setMinimumWidth(1280);
    setMinimumHeight(720);
  };

protected:
  void closeEvent(QCloseEvent *) { grpc_context->Disconnect(); }

private:
  QStringList connectedList;
  QTimer *timer = nullptr;
  GrpcTransferContext *grpc_context = nullptr;
  QSplitter *splitter = nullptr;
  MyFrame *frame1;
  MyFrame *frame2;
};