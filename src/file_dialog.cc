#include "file_dialog.h"
#include <QDebug>
#include <QEvent>
#include <QDialogButtonBox>
#include <QPushButton>

namespace mesytec
{
namespace mvp
{
/**
 * QFileDialog specialization which allows to choose a directory or a file.
 */
FileDialog::FileDialog(QWidget *parent)
  : QFileDialog(parent)
{
  setOption(QFileDialog::DontUseNativeDialog);
  setFileMode(QFileDialog::Directory);
  setNameFilter("MVP files (*.mvp *.bin *.key *.hex)");

  auto open_button = get_open_button();

  open_button->installEventFilter(this);
  open_button->disconnect(SIGNAL(clicked()));
  connect(open_button, SIGNAL(clicked()), this, SLOT(handle_open_button_clicked()));
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

void FileDialog::handle_open_button_clicked()
{
  auto selected = selectedFiles().value(0, QString());

  QFileInfo fi(selected);

  if (fi.exists()) {
    done(QDialog::Accepted);
  }
}

} // ns mvp
} // ns mesytec
