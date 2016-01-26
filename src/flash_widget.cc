#include "flash_widget.h"
#include "ui_flash_widget.h"
#include "file_dialog.h"
#include <QSettings>
#include <QStandardPaths>
#include <QtDebug>


namespace mvp
{

FlashWidget::FlashWidget(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::FlashWidget)
{
  ui->setupUi(this);

  connect(ui->pb_refresh_serial_ports, SIGNAL(clicked()),
      this, SIGNAL(serial_port_refresh_requested()));

  connect(ui->combo_area, SIGNAL(currentIndexChanged(int)),
      this, SIGNAL(area_index_changed(int)));

  connect(ui->pb_start, SIGNAL(clicked()),
      this, SIGNAL(start_button_clicked()));

  ui->pb_open_file->setIcon(style()->standardIcon(
        QStyle::SP_DialogOpenButton));
}

bool FlashWidget::is_start_button_enabled() const
{
  return ui->pb_start->isEnabled();
}

void FlashWidget::set_start_button_enabled(bool b)
{
  ui->pb_start->setEnabled(b);
}

bool FlashWidget::is_input_box_enabled() const
{
  return ui->gb_input->isEnabled();
}

void FlashWidget::set_input_box_enabled(bool b)
{
  ui->gb_input->setEnabled(b);
}

QString FlashWidget::get_serial_port() const
{
  auto index = ui->combo_serial_ports->currentIndex();
  return ui->combo_serial_ports->itemData(index).toString();
}

QString FlashWidget::get_firmware_file() const
{
  return ui->le_filename->text();
}

int FlashWidget::get_area_index() const
{
  return ui->combo_area->currentIndex();
}

void FlashWidget::set_available_ports(const PortInfoList &ports)
{
  const auto current_port = get_serial_port();

  QSignalBlocker b(ui->combo_serial_ports);

  ui->combo_serial_ports->clear();

  for (auto &info: ports) {
    ui->combo_serial_ports->addItem(
      info.portName() + " - " + info.serialNumber(),
      info.portName());
  }

  ui->combo_serial_ports->addItem(QString());

  if (!current_port.isEmpty()) {
    handle_current_port_name_changed(current_port);
  }
}

void FlashWidget::set_firmware_file(const QString &filename)
{
  ui->le_filename->setText(filename);
  emit firmware_file_changed(filename);
}

void FlashWidget::on_pb_open_file_clicked()
{
  QString dir = QStandardPaths::standardLocations(
        QStandardPaths::DocumentsLocation).value(0, QString());

  QSettings settings;

  dir = settings.value("directories/firmware", dir).toString();
  FileDialog file_dialog;
  file_dialog.setDirectory(dir);

  if (file_dialog.exec() != QDialog::Accepted)
    return;

  const QString filename = file_dialog.get_selected_file_or_dir();

  if (!filename.isEmpty()) {
    QFileInfo fi(filename);
    settings.setValue("directories/firmware", fi.path());
  }

  set_firmware_file(filename);
}

void FlashWidget::handle_current_port_name_changed(const QString &port_name)
{
  qDebug() << "FlashWidget::handle_current_port_name_changed" << port_name;

  int idx = ui->combo_serial_ports->findData(port_name);

  if (idx < 0)
    idx = ui->combo_serial_ports->count() - 1;

  QSignalBlocker b(ui->combo_serial_ports);
  ui->combo_serial_ports->setCurrentIndex(idx);

  emit serial_port_changed(get_serial_port());
}

} // ns mvp
