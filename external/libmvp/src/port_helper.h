#ifndef UUID_bed5a8e3_946b_4d71_ac48_22b2b463961c
#define UUID_bed5a8e3_946b_4d71_ac48_22b2b463961c

#include <QList>
#include <QObject>
#include <QSerialPortInfo>
#include <gsl.h>

namespace mvp
{

typedef QList<QSerialPortInfo> PortInfoList;

class PortHelper: public QObject
{
  Q_OBJECT
  signals:
    void available_ports_changed(const PortInfoList &);
    void current_port_name_changed(const QString &);

  public:
    PortHelper(gsl::not_null<QSerialPort *> port, QObject *parent=nullptr);

    /** Returns the list of available serial ports. */
    PortInfoList get_available_ports();

    /** Sets the port name the user wants to use. */
    void set_selected_port_name(const QString &name);

    /** Makes sure the currently selected port name still exists and is open.
     * On error searches for a port with the same serial number as the old port. */
    void open_port();

  private:
    QSerialPort *m_port;
    QSerialPortInfo m_selected_port_info;
};

} // ns mvp

#endif
