#include "gui.h"
#include "ui_gui.h"
#include "util.h"
#include "file_dialog.h"
#include "mdpp16_firmware.h"
#include "flash_widget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QIcon>
#include <QProgressBar>
#include <QSerialPort>
#include <QtConcurrent>
#include <QtDebug>
#include <QThread>
#include <QDateTime>
#include <QSignalBlocker>
#include <QCloseEvent>
#include <QMetaObject>

#include <utility>

namespace mvp
{

MVPGui::MVPGui(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MVPGui)
  , m_object_holder(new QObject)
  , m_flash(new Flash(m_object_holder))
  , m_port(new QSerialPort(m_object_holder))
  , m_port_helper(new PortHelper(m_port, m_object_holder))
  , m_port_refresh_timer(new QTimer(m_object_holder))
  , m_flashwidget(new FlashWidget)
  , m_progressbar(new QProgressBar)
{
  m_object_holder->setObjectName("object holder");

  ui->setupUi(this);

  auto layout = qobject_cast<QBoxLayout *>(centralWidget()->layout());
  layout->insertWidget(0, m_flashwidget);

  ui->statusbar->addPermanentWidget(m_progressbar);
  ui->statusbar->setSizeGripEnabled(false);
  ui->logview->document()->setMaximumBlockCount(10000);

  m_progressbar->setVisible(false);

  m_flash->set_port(m_port);

  connect(&m_fw, SIGNAL(started()), this, SLOT(handle_future_started()));
  connect(&m_fw, SIGNAL(finished()), this, SLOT(handle_future_finished()));
  connect(&m_fw, SIGNAL(finished()), m_progressbar, SLOT(reset()));

  connect(m_flash, SIGNAL(progress_range_changed(int, int)),
      m_progressbar, SLOT(setRange(int, int)), Qt::QueuedConnection);

  connect(m_flash, SIGNAL(progress_changed(int)),
      m_progressbar, SLOT(setValue(int)), Qt::QueuedConnection);

  connect(m_flash, &Flash::progress_text_changed, this, [=](const QString &text) {
      append_to_log(text);
      }, Qt::QueuedConnection);

  connect(m_flash, &Flash::statusbyte_received, this, [=](const uchar &ss) {
    if (!bool(ss & status::inst_success)) {
      append_to_log(QString("statusbyte(bin)=%1, inst_success=%2, area=%3, dipsw=%4")
                    .arg(ss, 0, 2)
                    .arg(bool(ss & status::inst_success))
                    .arg(get_area(ss))
                    .arg(get_dipswitch(ss)));
    }
  }, Qt::QueuedConnection);

#if 0
  connect(m_flash, &Flash::instruction_written, this, [=](const QVector<uchar> &data) {
      qDebug() << "instruction written:" << format_bytes(data);
      append_to_log(QString("instruction %1 written")
        .arg(opcodes::op_to_string.value(*data.begin(), QString::number(*data.begin(), 16))));
  }, Qt::QueuedConnection);
#endif

  connect(m_port_refresh_timer, SIGNAL(timeout()),
    m_port_helper, SLOT(refresh()), Qt::QueuedConnection);

  connect(m_port_helper, SIGNAL(available_ports_changed(const PortInfoList &)),
      m_flashwidget, SLOT(set_available_ports(const PortInfoList &)), Qt::QueuedConnection);

  connect(m_flashwidget, SIGNAL(serial_port_refresh_requested()),
      m_port_helper, SLOT(refresh()), Qt::QueuedConnection);

  connect(m_flashwidget, SIGNAL(serial_port_changed(const QString &)),
      m_port_helper, SLOT(set_selected_port_name(const QString &)), Qt::QueuedConnection);

  auto ports = m_port_helper->get_available_ports();

  if (ports.size()) {
    m_port_helper->set_selected_port_name(ports[0].portName());
  }

  on_action_refresh_serial_ports_triggered();

  m_port_refresh_timer->setInterval(port_refresh_interval_ms);
  m_port_refresh_timer->start();
}

MVPGui::~MVPGui()
{
  delete m_object_holder;
}

void MVPGui::on_action_refresh_serial_ports_triggered()
{
  m_port_helper->refresh();
}

void MVPGui::on_action_open_firmware_triggered()
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

  qDebug() << "on_action_open_firmware_triggered(): filename ="
           << filename;

  if (filename.isEmpty())
    return;

  QFileInfo fi(filename);

  qDebug() << "on_action_open_firmware_triggered():"
           << "filename =" << filename
           << "absoluteDir =" << fi.absoluteDir()
           << "absolutePath =" << fi.absolutePath()
           << "path =" << fi.path();

  settings.setValue("directories/firmware", fi.path());

  ThreadMover tm(m_object_holder, 0);

  auto f_result = run_in_thread<MDPP16Firmware>([&] {
    auto firmware = fi.isDir()
                    ? from_dir(filename)
                    : from_zip(filename);

    qDebug() << "Firmware object created from" << fi.filePath();

    // TODO: move this code somewhere into libmvp
    if (!firmware.has_required_sections())
      throw std::runtime_error("Firmware: missing required sections (8 and/or 12)");

    return firmware;
  }, m_object_holder);

#if 0
    qDebug() << "Firmware contains at least sections 8 and 12";

    qDebug() << "Firmware: open port";
    m_port_helper->open_port();

    qDebug() << "Firmware: ensure clean state";
    m_flash->ensure_clean_state();

    qDebug() << "Firmware: set area index" << area_index;
    m_flash->set_area_index(area_index);

    for (auto sec: get_valid_sections()) {
      if (!firmware.has_section(sec))
        continue;

      qDebug() << "Firmware: working on section" << sec;

      qDebug() << "Firmware: erase";

      m_flash->erase_subindex(sec);

      auto content = firmware.get_section(sec);
      qDebug() << "Firmware: write memory" << content.size();
      m_flash->write_memory({0, 0, 0}, sec, gsl::as_span(content));
      qDebug() << "Firmware: verify memory" << content.size();
      m_flash->verify_memory({0, 0, 0}, sec, gsl::as_span(content));

      qDebug() << "Firmware: done with section" << sec;
    }
  }, m_object_holder);
#endif

  m_fw.setFuture(f_result);

  if (!m_fw.isFinished()) {
    m_progressbar->setVisible(true);
    m_loop.exec();
    m_progressbar->setVisible(false);
  }

  try {
    m_firmware = f_result.result();
    //ui->le_filename->setText(filename);
  } catch (const std::exception &e) {
    m_firmware.clear();
    //ui->le_filename->clear();
    append_to_log(QString(e.what()));
  }


#if 0
  QFile f(filename);

  if (!f.open(QIODevice::ReadOnly)) {
    ui->statusbar->showMessage(QString("Error opening %1: %2")
        .arg(filename).arg(f.errorString()));
    return;
  }

  if (f.bytesAvailable() > constants::firmware_max_size) {
    ui->statusbar->showMessage(QString("Error: %1 exceeds firmware max size (%2 > %3)")
        .arg(filename).arg(f.bytesAvailable()).arg(constants::firmware_max_size));
    return;
  }

  QVector<uchar> buf(f.bytesAvailable());

  {
    ThreadMover tm(&f, 0);

    m_fw.setFuture(run_in_thread<void>([&] {
          auto sz = f.read(reinterpret_cast<char *>(buf.data()), buf.size());
          if (sz < 0) {
            throw std::runtime_error(QString("Error reading from %1: %2")
                .arg(filename).arg(f.errorString()).toStdString());
          }
        }, &f));

    if (!m_fw.isFinished())
      m_loop.exec();
  }

  try {
    m_fw.waitForFinished();
    m_firmware_buffer = std::move(buf);

    auto pad = pad_to_page_size(m_firmware_buffer);
    append_to_log(QString("Padding data to page size (%1 bytes)")
                  .arg(pad));

    ui->le_filename->setText(filename);
  } catch (const std::exception &e) {
    auto errstr(QString("Error: ") + e.what());
    append_to_log(errstr);
  }
#endif
}

void MVPGui::on_combo_serial_ports_currentIndexChanged(int index)
{
  //auto name = ui->combo_serial_ports->itemData(index).toString();
  //m_port_helper->set_selected_port_name(name);
}

void MVPGui::closeEvent(QCloseEvent *event)
{
  if (!m_fw.isRunning()) {
    event->accept();
  } else {
    if (!m_quit)
      append_to_log("Quitting after the current operation finishes");

    m_quit = true;
    event->ignore();
  }
}

void MVPGui::append_to_log(const QString &s)
{
  auto str(QString("%1: %2")
      .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
      .arg(s));

  ui->logview->append(str);
  qDebug() << s;
}

void MVPGui::on_action_firmware_start_triggered()
{
  if (m_fw.isRunning()) {
    append_to_log("Error: operation in progress");
    return;
  }

  if (m_firmware.is_empty()) {
    append_to_log("Error: no or empty firmware loaded");
    return;
  }

  if (!m_firmware.has_required_sections()) {
    append_to_log("Error: missing required section(s)");
    return;
  }

  auto area_index = get_selected_area();
  ThreadMover tm(m_object_holder, 0);

  auto f_result = run_in_thread<void>([&] {
    qDebug() << "Firmware: open port";
    m_port_helper->open_port();

    qDebug() << "Firmware: ensure clean state";
    m_flash->ensure_clean_state();

    qDebug() << "Firmware: set area index" << area_index;
    m_flash->set_area_index(area_index);

    for (auto sec: get_valid_sections()) {
      if (!m_firmware.has_section(sec))
        continue;

      qDebug() << "Firmware: working on section" << sec;

      qDebug() << "Firmware: erase";

      append_to_log_queued(QString("Erasing section %1").arg(sec));
      m_flash->erase_subindex(sec);

      auto content = m_firmware.get_section(sec);
      qDebug() << "Firmware: write memory" << content.size();

      append_to_log_queued(QString("Writing %1 bytes to section %2")
                           .arg(content.size()).arg(sec));

      m_flash->write_memory({0, 0, 0}, sec, gsl::as_span(content));

      qDebug() << "Firmware: verify memory" << content.size();
      append_to_log_queued(QString("Verify: reading %1 bytes from section %2")
                           .arg(content.size()).arg(sec));

      m_flash->verify_memory({0, 0, 0}, sec, gsl::as_span(content));

      append_to_log_queued(QString("Section %1 done").arg(sec));

      qDebug() << "Firmware: done with section" << sec;
    }
  }, m_object_holder);

  m_fw.setFuture(f_result);

  if (!m_fw.isFinished()) {
    m_progressbar->setVisible(true);
    m_loop.exec();
    m_progressbar->setVisible(false);
  }

  try {
    m_fw.waitForFinished();
    auto ss = m_flash->get_last_status();
    auto area = get_area(ss);
    auto dips = get_dipswitch(ss);

    append_to_log(
          QString("Firmware from %1 written to area %2.")
          .arg(m_flashwidget->get_firmware_file())
          .arg(area));

    append_to_log(
          QString("Boot area on power cycle is %1 (dipswitches)")
          .arg(dips));

  } catch (const std::exception &e) {
    append_to_log(QString(e.what()));
  }
}

void MVPGui::handle_future_started()
{
  //ui->gb_input->setEnabled(false);
  //ui->pb_firmware_start->setEnabled(false);
}

void MVPGui::handle_future_finished()
{
  m_loop.quit();
  //ui->gb_input->setEnabled(true);
  //ui->pb_firmware_start->setEnabled(true);
  if (m_quit)
    close();
}

void MVPGui::handle_available_ports_changed(const PortInfoList &ports)
{
#if 0
  QSignalBlocker b(ui->combo_serial_ports);

  ui->combo_serial_ports->clear();

  for (auto &info: ports) {
    ui->combo_serial_ports->addItem(
      info.portName() + " - " + info.serialNumber(),
      info.portName());
  }

  ui->combo_serial_ports->addItem(QString());

  handle_current_port_name_changed(m_port_helper->get_selected_port_name());
#endif
}

void MVPGui::handle_current_port_name_changed(const QString &port_name)
{
#if 0

  int idx = ui->combo_serial_ports->findData(port_name);

  if (idx < 0)
    idx = ui->combo_serial_ports->count() - 1;

  QSignalBlocker b(ui->combo_serial_ports);
  ui->combo_serial_ports->setCurrentIndex(idx);
#endif
}


uchar MVPGui::get_selected_area() const
{
#if 0
  return static_cast<uchar>(ui->combo_area->currentIndex());
#endif
}

void MVPGui::append_to_log_queued(const QString &s)
{
  QMetaObject::invokeMethod(
        this,
        "append_to_log",
        Qt::QueuedConnection,
        Q_ARG(QString, s));
}

} // ns mvp
