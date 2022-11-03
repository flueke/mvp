#include "flash_widget.h"
#include "ui_flash_widget.h"
#include "file_dialog.h"
#include <QSettings>
#include <QStandardPaths>
#include <QtDebug>

namespace mesytec
{
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
  return ui->combo_serial_ports->currentData().toString();
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
  if (ports == m_prevPortInfoList)
    return;

  const auto current_port = get_serial_port();

  QSignalBlocker b(ui->combo_serial_ports);

  ui->combo_serial_ports->clear();

  for (auto &info: ports) {
    if (!info.serialNumber().isEmpty()) {
      ui->combo_serial_ports->addItem(
        info.portName() + " - " + info.serialNumber(),
        info.portName());
    } else {
      ui->combo_serial_ports->addItem(
          info.portName(),
          info.portName());
    }
  }

  int idx = 0;

  if (!current_port.isEmpty()) {
    idx = ui->combo_serial_ports->findData(current_port);
    idx = idx >= 0 ? idx : 0;
  }

  if (ports[idx].serialNumber().isEmpty())
  {
    // The previously selected port does not have a serial number so it cannot
    // be a mesytec device. Look for the first port that has a serial number and
    // select that instead.
    auto it = std::find_if(std::begin(ports), std::end(ports),
     [] (const QSerialPortInfo &port) { return !port.serialNumber().isEmpty(); });
    if (it != std::end(ports))
      idx = it - std::begin(ports);
  }

  ui->combo_serial_ports->setCurrentIndex(idx);

  m_prevPortInfoList = ports;

  emit serial_port_changed(get_serial_port());
}

void FlashWidget::set_firmware_file(const QString &filename)
{
  ui->le_filename->setText(filename);
  emit firmware_file_changed(filename);
}

void FlashWidget::set_area_select_enabled(bool b)
{
    ui->combo_area->setEnabled(b);
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

} // ns mvp
} // ns mesytec
