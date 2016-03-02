#ifndef UUID_8ac24616_6c87_4efa_8367_a7722f2547c4
#define UUID_8ac24616_6c87_4efa_8367_a7722f2547c4

#include "firmware.h"

namespace mesytec
{
namespace mvp
{

class PortHelper;
class Flash;

class FirmwareWriter: public QObject
{
  Q_OBJECT
  signals:
    void status_message(const QString &);

  public:
    FirmwareWriter(const FirmwareArchive &firmware,
        PortHelper *port_helper,
        Flash *flash,
        QObject *parent = nullptr);

    void write();

    bool do_erase() const   { return m_do_erase; }
    bool do_program() const { return m_do_program; }
    bool do_verify() const  { return m_do_verify; }

    void set_do_erase(bool b)   { m_do_erase = b; }
    void set_do_program(bool b) { m_do_program = b; }
    void set_do_verify(bool b)  { m_do_verify = b; }

  private:
    void write_part(const FirmwarePartPtr &pp,
        uchar section,
        const boost::optional<uchar> &area = boost::none);

    FirmwareArchive m_firmware;
    PortHelper *m_port_helper = nullptr;
    Flash *m_flash = nullptr;

    bool m_do_erase = true;
    bool m_do_program = true;
    bool m_do_verify = false;
};

} // ns mvp
} // ns mesytec

#endif
