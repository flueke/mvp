#ifndef UUID_38aac333_3951_4b01_abe7_0c90c425b0ec
#define UUID_38aac333_3951_4b01_abe7_0c90c425b0ec

#include <QEventLoop>
#include <QFutureWatcher>
#include <QMainWindow>
#include <QProgressDialog>
#include <QSerialPortInfo>
#include "flash.h"
#include "port_helper.h"
#include "mdpp16_firmware.h"

class QCloseEvent;
class QLabel;
class QMenu;
class QSerialPort;
class QSpinBox;

namespace Ui
{
  class MVPGui;
}

namespace mvp
{

static const int port_refresh_interval_ms = 1000;

class FlashWidget;

class MVPGui: public QMainWindow
{
  Q_OBJECT

  public:
    explicit MVPGui(QWidget *parent=0);
    virtual ~MVPGui();

  protected:
    virtual void closeEvent(QCloseEvent *event) override;

  private slots:
    // serial port handling
    void on_action_refresh_serial_ports_triggered();
    void on_combo_serial_ports_currentIndexChanged(int index);
    void handle_available_ports_changed(const PortInfoList &);
    void handle_current_port_name_changed(const QString &);

    // firmware
    void on_action_open_firmware_triggered();
    void on_action_firmware_start_triggered();

    // execution
    void handle_future_started();
    void handle_future_finished();

    void append_to_log(const QString &s);

  private:
    void append_to_log_queued(const QString &s);

    uchar get_selected_area() const;

    Ui::MVPGui *ui;

    QObject *m_object_holder;
    Flash *m_flash;
    QSerialPort *m_port;
    PortHelper *m_port_helper;
    QTimer *m_port_refresh_timer;

    QFutureWatcher<void> m_fw;
    QEventLoop m_loop;
    bool m_quit = false;

    FlashWidget *m_flashwidget;
    QProgressBar *m_progressbar;

    MDPP16Firmware m_firmware;
};

} // ns mvp

#endif
