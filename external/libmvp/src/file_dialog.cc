#include "file_dialog.h"
#include <QDebug>

namespace mvp
{
FileDialog::FileDialog(QWidget *parent)
  : QFileDialog(parent)
{
  setOption(QFileDialog::DontUseNativeDialog);
  setFileMode(QFileDialog::Directory);
  setNameFilter("ZIP files (*.zip)");

  //for (auto child: findChildren<QWidget *>()) {
  //    qDebug() << child;
  //}
}

} // ns mvp
