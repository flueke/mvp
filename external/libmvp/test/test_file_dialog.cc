#include "file_dialog.h"
#include "tests.h"

#include <QApplication>
#include <QStandardPaths>

void TestFileDialog::test()
{
  const QString dir = QStandardPaths::standardLocations(
        QStandardPaths::DocumentsLocation).value(0, QString());

  mvp::FileDialog dialog;

  dialog.setDirectory(dir);
  dialog.show();
}
