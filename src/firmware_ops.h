#ifndef UUID_8ac24616_6c87_4efa_8367_a7722f2547c4
#define UUID_8ac24616_6c87_4efa_8367_a7722f2547c4

#include "firmware.h"

namespace mesytec
{
namespace mvp
{

class PortHelper;
class Flash;

/*
 * Options:
 * - do_verify
 *
 * XXX:
 * Not sure if the port helper is needed here
 */

class FirmwareWriter: public QObject
{
  Q_OBJECT
  signals:
    void status_message(const QString &);

  public:
    FirmwareWriter(const FirmwareArchive &firmware = FirmwareArchive(),
        PortHelper *port_helper = nullptr,
        Flash *flash = nullptr,
        QObject *parent= nullptr);

    void write();

  private:
    void write_part(const FirmwarePartPtr &pp,
        uchar section,
        const boost::optional<uchar> &area = boost::none);

    FirmwareArchive m_firmware;
    PortHelper *m_port_helper;
    Flash *m_flash;
};

} // ns mvp
} // ns mesytec

#endif
