
#include "Client.hpp"
#include "FileSystem.hpp"
#include <QApplication>
#include <QScopedPointer>
#include <QWidget>

#include "MyWindow.hpp"

int main(int argc, char **argv) {
  QScopedPointer<QApplication> app(new QApplication(argc, argv));

  GrpcTransferContext context;

  QScopedPointer<QMainWindow> ww(new MyWindow(context));
  ww->showNormal();
  app->exec();
}