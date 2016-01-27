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

  connect(m_flashwidget, SIGNAL(serial_port_refresh_requested()),
      m_port_helper, SLOT(refresh()), Qt::QueuedConnection);

  connect(m_port_helper, SIGNAL(available_ports_changed(const PortInfoList &)),
      m_flashwidget, SLOT(set_available_ports(const PortInfoList &)), Qt::QueuedConnection);

  connect(m_flashwidget, SIGNAL(serial_port_changed(const QString &)),
      m_port_helper, SLOT(set_selected_port_name(const QString &)), Qt::QueuedConnection);

  connect(m_flashwidget, SIGNAL(firmware_file_changed(const QString &)),
      this, SLOT(_on_firmware_file_changed(const QString &)));

  connect(m_flashwidget, SIGNAL(start_button_clicked()),
      this, SLOT(_on_start_button_clicked()));

  auto ports = m_port_helper->get_available_ports();

  if (ports.size()) {
    m_port_helper->set_selected_port_name(ports[0].portName());
  }

  m_flashwidget->set_available_ports(ports);

  m_port_refresh_timer->setInterval(port_refresh_interval_ms);
  m_port_refresh_timer->start();
}

MVPGui::~MVPGui()
{
  delete m_object_holder;
}

void MVPGui::_on_start_button_clicked()
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

  auto area_index = m_flashwidget->get_area_index();
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

void MVPGui::_on_firmware_file_changed(const QString &filename)
{
  ThreadMover tm(m_object_holder, 0);

  auto f_result = run_in_thread<MDPP16Firmware>([&] {
    QFileInfo fi(filename);

    auto firmware = fi.isDir()
                    ? from_dir(filename)
                    : from_zip(filename);

    qDebug() << "Firmware object created from" << fi.filePath();

    // TODO: move this code somewhere into libmvp
    if (!firmware.has_required_sections())
      throw std::runtime_error("Firmware: missing required sections (8 and/or 12)");

    return firmware;
  }, m_object_holder);

  m_fw.setFuture(f_result);

  if (!m_fw.isFinished()) {
    m_progressbar->setVisible(true);
    m_loop.exec();
    m_progressbar->setVisible(false);
  }

  try {
    m_firmware = f_result.result();
  } catch (const std::exception &e) {
    m_firmware.clear();
    append_to_log(QString(e.what()));
  }
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

void MVPGui::handle_future_started()
{
  m_flashwidget->set_input_box_enabled(false);
  m_flashwidget->set_start_button_enabled(false);
}

void MVPGui::handle_future_finished()
{
  m_loop.quit();
  m_flashwidget->set_input_box_enabled(true);
  m_flashwidget->set_start_button_enabled(true);
  if (m_quit)
    close();
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
