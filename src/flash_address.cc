#include "flash_address.h"

namespace mesytec
{
namespace mvp
{

QString Address::to_string() const
{
    return QString("A(a0=%1, a1=%2, a2=%3, int=0x%4)")
        .arg(_data[0])
        .arg(_data[1])
        .arg(_data[2])
        .arg(this->to_int(), 6, 16, QLatin1Char('0'));
}

QDebug operator<<(QDebug dbg, const Address &a)
{
  dbg.nospace()
    << "A(a0="  << a[0]
    << ", a1="  << a[1]
    << ", a2="  << a[2]
    << ", int=" << a.to_int() << ", hex=" << QString::number(a.to_int(), 16)
    << ")";
  return dbg.space();
}

} // ns mvp
} // ns mesytec
