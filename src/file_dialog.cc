#include "file_dialog.h"
#include <QDebug>
#include <QEvent>
#include <QDialogButtonBox>
#include <QPushButton>

namespace mvp
{
FileDialog::FileDialog(QWidget *parent)
  : QFileDialog(parent)
{
  setOption(QFileDialog::DontUseNativeDialog);
  setFileMode(QFileDialog::Directory);
  setNameFilter("ZIP files (*.zip)");

  get_open_button()->installEventFilter(this);
}

bool FileDialog::eventFilter(QObject *watched, QEvent *event)
{
  auto open_button = get_open_button();

  if (open_button == watched
    && event->type() == QEvent::EnabledChange) {
    open_button->setEnabled(true);
  }

  return QFileDialog::eventFilter(watched, event);
}

QPushButton *FileDialog::get_open_button() const
{
  return findChild<QDialogButtonBox *>()->button(
    QDialogButtonBox::Open);
}

} // ns mvp
