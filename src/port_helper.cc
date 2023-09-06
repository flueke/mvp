#include "port_helper.h"
#include "util.h"
#include <QSerialPort>

namespace mesytec
{
namespace mvp
{

static PortInfoProvider default_portinfo_provider = [] {
  auto ports(QSerialPortInfo::availablePorts());

  //ports.erase(std::remove_if(ports.begin(), ports.end(), [](const QSerialPortInfo &info) {
  //        return !info.description().startsWith("mvp");
  //      }), ports.end());

#if 0
  for (const auto &port: ports) {
    qDebug()
      << "port name" << port.portName()
      << "system location" << port.systemLocation()
      << "manufacturer" << port.manufacturer()
      << "serial number" << port.serialNumber()
      << "product identifier" << port.productIdentifier()
      << "vendor identifier" << port.vendorIdentifier()
      << "description" << port.description()
      ;
  }
#endif

  return ports;
};

PortHelper::PortHelper(gsl::not_null<QSerialPort *> port, QObject *parent)
  : QObject(parent)
  , m_port(port)
  , m_portinfo_provider(default_portinfo_provider)
{
}

PortHelper::PortHelper(gsl::not_null<QSerialPort *> port, PortInfoProvider pip,
  QObject *parent)
  : QObject(parent)
  , m_port(port)
  , m_portinfo_provider(pip)
{
}

PortInfoList PortHelper::get_available_ports() const
{
  return m_portinfo_provider();
}

void PortHelper::set_selected_port_name(const QString &name)
{
  m_selected_port_info = QSerialPortInfo(name);
  qDebug() << "set_selected_port_name" << name
    << m_selected_port_info.serialNumber();
}

void PortHelper::open_port()
{
  qDebug() << "open_port(): begin" << m_selected_port_info.portName();

  if (m_selected_port_info.isNull() || m_selected_port_info.portName().isEmpty())
    throw std::runtime_error("No serial port selected");

  auto info   = QSerialPortInfo(m_selected_port_info.portName());
  auto serial = m_selected_port_info.serialNumber();

  if (info.isNull() || info.serialNumber() != serial) {
    // the named port either went away or the serial number changed
    qDebug() << "open_port(): looking for port with serial number" << serial;
    auto ports = get_available_ports();
    auto it    = std::find_if(ports.begin(), ports.end(),
        [&](const QSerialPortInfo &info) { return info.serialNumber() == serial; });

    if (it == ports.end()) {
      m_selected_port_info = QSerialPortInfo();
      emit current_port_name_changed(QString());
      qDebug() << "open_port(): could not find port with serial number" << serial;
      throw std::runtime_error("Could not find port with serial number " + serial.toStdString());
    }

    qDebug() << "open_port(): found port" << it->portName();

    m_selected_port_info = *it;
    emit current_port_name_changed(m_selected_port_info.portName());
  }

  qDebug() << "open_port(): closing port before reopening";
  m_port->close();
  m_port->clearError();

  qDebug() << "open_port(): calling setPortName() on" << m_port
    << "with port =" << info.portName();
  m_port->setPortName(info.portName());
  qDebug() << "open_port(): opening port" << info.portName();
  if (!m_port->open(QIODevice::ReadWrite)) {
    qDebug() << "open_port(): call to open failed" << m_port->errorString();
    throw make_com_error(m_port);
  }

  qDebug()
    << "open_port(): end"
    << "portName ="     << m_port->portName()
    << "isOpen ="       << m_port->isOpen()
    << "errorString ="  << m_port->errorString()
    << "portError ="    << port_error_to_string(m_port->error());
}

void PortHelper::refresh()
{
  emit available_ports_changed(get_available_ports());
}

} // ns mvp
} // ns mesytec

bool operator==(const QSerialPortInfo &a, const QSerialPortInfo &b)
{
  return a.description() == b.description()
    && a.manufacturer() == b.manufacturer()
    && a.portName() == b.portName()
    && a.productIdentifier() == b.productIdentifier()
    && a.serialNumber() == b.serialNumber()
    && a.systemLocation() == b.systemLocation()
    && a.vendorIdentifier() == b.vendorIdentifier()
    ;
}
