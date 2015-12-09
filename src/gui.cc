#include "gui.h"
#include "ui_gui.h"
#include "util.h"
#include "file_dialog.h"

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
  , m_progressbar(new QProgressBar)
{
  m_object_holder->setObjectName("object holder");

  ui->setupUi(this);
  ui->pb_open_file->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
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

#if 1
  connect(m_flash, &Flash::statusbyte_received, this, [=](const uchar &ss) {
    append_to_log(QString("statusbyte(bin)=%1, inst_success=%2, area=%3, dipsw=%4")
      .arg(ss, 0, 2)
      .arg(bool(ss & status::inst_success))
      .arg(get_area(ss))
      .arg(get_dipswitch(ss)));
  }, Qt::QueuedConnection);
#endif

#if 0
  connect(m_flash, &Flash::instruction_written, this, [=](const QVector<uchar> &data) {
      qDebug() << "instruction written:" << format_bytes(data);
      append_to_log(QString("instruction %1 written")
        .arg(opcodes::op_to_string.value(*data.begin(), QString::number(*data.begin(), 16))));
  }, Qt::QueuedConnection);
#endif

  connect(m_port_helper, SIGNAL(available_ports_changed(const PortInfoList &)),
      this, SLOT(handle_available_ports_changed(const PortInfoList &)), Qt::QueuedConnection);

  connect(m_port_helper, SIGNAL(current_port_name_changed(const QString &)),
      this, SLOT(handle_current_port_name_changed(const QString &)), Qt::QueuedConnection);

  on_action_refresh_serial_ports_triggered();
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

  QString filename = file_dialog.get_selected_file_or_dir();

  if (filename.isEmpty())
    return;

  QFileInfo fi(filename);

  settings.setValue("directories/firmware", fi.path());

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
}

void MVPGui::on_combo_serial_ports_currentIndexChanged(int index)
{
  auto name = ui->combo_serial_ports->itemData(index).toString();
  m_port_helper->set_selected_port_name(name);
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

  auto area_index     = get_selected_area();
  auto do_erase       = true;
  auto do_blankcheck  = false;
  auto do_program     = true;
  auto do_verify      = true;

  if (!(do_erase || do_blankcheck || do_program || do_verify)) {
    return;
  }

  if ((do_program || do_verify) && m_firmware_buffer.isEmpty()) {
    append_to_log("Error: empty firmware data");
    return;
  }

  ThreadMover tm(m_object_holder, 0);
  auto f_result = run_in_thread<void>([&] {
      m_port_helper->open_port();
      m_flash->ensure_clean_state();
      m_flash->set_area_index(area_index);

      if (do_erase)
        m_flash->erase_firmware();

      if (do_blankcheck) {
        auto result = m_flash->blankcheck_firmware();
        if (!result)
          return;
      }

      if (do_program)
        m_flash->write_firmware(m_firmware_buffer);

      if (do_verify)
        m_flash->verify_firmware(m_firmware_buffer);

    }, m_object_holder);

  m_fw.setFuture(f_result);

  if (!m_fw.isFinished()) {
    m_progressbar->setVisible(true);
    m_loop.exec();
    m_progressbar->setVisible(false);
  }

  try {
    m_fw.waitForFinished();
  } catch (const std::exception &e) {
    append_to_log(QString(e.what()));
  }

#if 0

  auto port_variant = ui->combo_serial_ports->currentData();

  if (!m_port->isOpen() && !port_variant.isValid()) {
    append_to_log("Error: no serial connection");
    return;
  }

  auto port_name  = port_variant.toString();
  int area_index  = ui->combo_area->currentIndex();
  bool do_erase   = ui->cb_erase->isChecked();
  bool do_program = ui->cb_program->isChecked();
  bool do_verify  = ui->cb_verify->isChecked();
  bool do_blankcheck = ui->cb_blankcheck->isChecked();

  if (!(do_erase || do_program || do_verify || do_blankcheck)) {
    return;
  }

  if ((do_program || do_verify) && !m_flash->has_program_data()) {
    append_to_log("Error: no firmware file loaded");
    return;
  }

  {
    ThreadMover tm(m_flash, 0);

    m_fw.setFuture(QtConcurrent::run([&] {
      try {
        ThreadMover tm(m_flash, QThread::currentThread());

        if (!m_port->isOpen()) {
          m_port->setPortName(port_name);
          if (!m_port->open(QIODevice::ReadWrite)) {
            throw std::runtime_error("Error opening serial port");
          }
        }

        m_flash->set_area_index(area_index);

        if (do_erase) {
          m_flash->erase_firmware();
        }

        if (do_blankcheck) {
          m_flash->blankcheck_firmware();
        }

        if (do_program && do_verify) {
          m_flash->program_and_verify();
        } else if (do_program) {
          //m_flash->program();
          m_flash->program_verbose_test();
        } else if (do_verify) {
          //m_flash->verify();
          m_flash->verify_verbose_test();
        }
      } catch (...) {
        throw QtExceptionPtr(std::current_exception());
      }
    }));

    m_progressbar->setVisible(true);
    m_loop.exec();
    m_progressbar->setVisible(false);

    try {
      m_fw.future().waitForFinished();
    } catch (const std::exception &e) {
      auto errstr(QString("Error: ") + e.what());
      append_to_log(errstr);
    }
  }
#endif
}

void MVPGui::handle_future_started()
{
  ui->gb_input->setEnabled(false);
}

void MVPGui::handle_future_finished()
{
  m_loop.quit();
  ui->gb_input->setEnabled(true);
  if (m_quit)
    close();
}

void MVPGui::handle_available_ports_changed(const PortInfoList &ports)
{
  ui->combo_serial_ports->clear();

  auto msg = QString("Available ports: ");

  for (auto &info: ports) {
    ui->combo_serial_ports->addItem(info.portName());
    msg += QString("%1(%2), ").arg(info.portName()).arg(info.serialNumber());
  }
  QSignalBlocker b(ui->combo_serial_ports);
  ui->combo_serial_ports->setCurrentIndex(0); // FIXME: keep the currently used port selected

  append_to_log(msg);
}

void MVPGui::handle_current_port_name_changed(const QString &port_name)
{
  QSignalBlocker b(ui->combo_serial_ports);
  ui->combo_serial_ports->setCurrentText(port_name);
}


uchar MVPGui::get_selected_area() const
{
  return static_cast<uchar>(ui->combo_area->currentIndex());
}

} // ns mvp
