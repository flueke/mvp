#include "util.h"
#include <QSerialPort>

namespace mvp
{

ComError make_com_error(gsl::not_null<QIODevice *> device,
    bool clear_serial_port_error)
{
  qDebug() << "begin make_com_error()";
  if (auto port = qobject_cast<QSerialPort *>(device.get())) {
    qDebug() << "make_com_error(): errorString" << port->errorString() << "code" << port_error_to_string(port->error());
    auto finally = gsl::finally([&] { if (clear_serial_port_error) port->clearError(); });
    return ComError(port->errorString(), port->error());
  }

  return ComError(device->errorString());
}

} // ns mvp
