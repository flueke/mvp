#ifndef UUID_bed5a8e3_946b_4d71_ac48_22b2b463961c
#define UUID_bed5a8e3_946b_4d71_ac48_22b2b463961c

#include <QList>
#include <QObject>
#include <QSerialPortInfo>
#include <gsl.h>
#include <functional>

namespace mesytec
{
namespace mvp
{

typedef QList<QSerialPortInfo> PortInfoList;
typedef std::function<PortInfoList ()> PortInfoProvider;

class PortHelper: public QObject
{
  Q_OBJECT
  signals:
    void available_ports_changed(const PortInfoList &);
    void current_port_name_changed(const QString &);

  public:
    PortHelper(gsl::not_null<QSerialPort *> port, QObject *parent=nullptr);

    PortHelper(gsl::not_null<QSerialPort *> port,
      PortInfoProvider pip, QObject *parent=nullptr);

    PortInfoProvider get_portinfo_provider() const
    { return m_portinfo_provider; }

    void set_portinfo_provider(PortInfoProvider provider)
    { m_portinfo_provider = provider; }

    /** Returns the list of available serial ports. */
    PortInfoList get_available_ports() const;

    QString get_selected_port_name() const
    { return m_selected_port_info.portName(); }

    /** Makes sure the currently selected port name still exists and is open.
     * On error searches for a port with the same serial number as the old port.
     * If that fails an exception is raised. */
    void open_port();

  public slots:
    void refresh();

    /** Sets the port name the user wants to use. */
    void set_selected_port_name(const QString &name);

  private:
    QSerialPort *m_port;
    QSerialPortInfo m_selected_port_info;
    PortInfoProvider m_portinfo_provider;
};

} // ns mvp
} // ns mesytec

#endif
