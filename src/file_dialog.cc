#include "file_dialog.h"

namespace mvp
{
FileDialog::FileDialog(QWidget *parent)
  : QFileDialog(parent)
{
  setOption(QFileDialog::DontUseNativeDialog);
  setFileMode(QFileDialog::Directory);
  setNameFilter("ZIP files (*.zip)");
}

} // ns mvp
