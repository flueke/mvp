#include "flash_address.h"

namespace mesytec
{
namespace mvp
{

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
