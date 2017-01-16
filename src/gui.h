#ifndef UUID_cfaa97f6_7609_496a_8cbd_4f9ac2229047
#define UUID_cfaa97f6_7609_496a_8cbd_4f9ac2229047

#include <QEventLoop>
#include <QFutureWatcher>
#include <QMainWindow>
#include <QProgressDialog>
#include <QSerialPortInfo>
#include "flash.h"
#include "port_helper.h"
#include "firmware.h"

class QCloseEvent;
class QLabel;
class QMenu;
class QSerialPort;
class QSpinBox;

namespace Ui
{
  class MVPGui;
}

namespace mesytec
{
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
    // firmware
    void _on_start_button_clicked();
    void _on_firmware_file_changed(const QString &);

    void write_firmware();
    void handle_keys();

    // execution
    void handle_future_started();
    void handle_future_finished();

    // misc
    void append_to_log(const QString &s);

    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();

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

    FirmwareArchive m_firmware;
};

} // ns mvp
} // ns mesytec

#endif
