#ifndef UUID_2ad9be0a_5a8e_4afb_96eb_23b0d31362eb
#define UUID_2ad9be0a_5a8e_4afb_96eb_23b0d31362eb

#include <QWidget>
#include "port_helper.h"

namespace Ui
{
  class FlashWidget;
}

namespace mvp
{

class FlashWidget: public QWidget
{
  Q_OBJECT
  signals:
    void serial_port_changed(const QString &);
    void serial_port_refresh_requested();
    void firmware_file_changed(const QString &);
    void area_index_changed(int);
    void start_button_clicked();

  public:
    explicit FlashWidget(QWidget *parent=0);

    bool is_start_button_enabled() const;
    bool is_input_box_enabled() const;
    QString get_serial_port() const;
    QString get_firmware_file() const;
    int get_area_index() const;

  public slots:
    void set_start_button_enabled(bool b=true);
    void set_input_box_enabled(bool b=true);
    void set_available_ports(const PortInfoList &ports);
    void set_firmware_file(const QString &);

  private slots:
    void on_pb_open_file_clicked();

  private:
    void handle_current_port_name_changed(const QString &port_name);

    Ui::FlashWidget *ui;
};

} // ns mvp

#endif
