#include "file_dialog.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  mvp::FileDialog dialog;
  dialog.show();

  return app.exec();
}
