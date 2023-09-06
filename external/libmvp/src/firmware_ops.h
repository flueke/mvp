#ifndef UUID_8ac24616_6c87_4efa_8367_a7722f2547c4
#define UUID_8ac24616_6c87_4efa_8367_a7722f2547c4

#include "firmware.h"

namespace mesytec
{
namespace mvp
{

class FlashInterface;

class FirmwareWriter: public QObject
{
  Q_OBJECT
  signals:
    void status_message(const QString &);

  public:
    FirmwareWriter(const FirmwareArchive &firmware,
        FlashInterface *flash,
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
    FlashInterface *m_flash = nullptr;

    bool m_do_erase = true;
    bool m_do_program = true;
    bool m_do_verify = false;
};

typedef QList<Key> KeyList;

class KeysInfo
{
  public:
    KeysInfo() {}
    KeysInfo(const OTP &otp, const KeyMap &device_keys, const KeyList &firmware_keys);

    /** True if erasing is needed to store the keys contained in the firmware
     * archive. */
    bool need_to_erase() const;

    /* Returns the list of keys contained in the firmware archive matching the
     * OTP info.
     *
     * Note: only keys matching the devices OTP information are returned. Use
     * get_mismatched_firmware_keys() to get any additional keys contained in the
     * firmware file which do not match the OTP.*/
    KeyList get_firmware_keys() const { return m_firmware_keys; }

    /* Returns a list of keys contained in the firmware archive and not present
     * on the device. */
    KeyList get_new_firmware_keys() const;

    /* Returns the slot -> key mapping of keys present on the device. */
    KeyMap  get_device_keys() const { return m_device_keys; }

    /* Get the devices OTP info. */
    OTP get_otp() const { return m_otp; }

    bool is_valid() const { return m_otp.is_valid(); }

    /* Returns a list of the keys contained in the firmware file that did not
     * match the devices OTP information (e.g. device serial number mismatch).
     */
    KeyList get_mismatched_firmware_keys() const { return m_mismatched_keys; }

  private:
    OTP m_otp;
    KeyList m_firmware_keys;
    KeyList m_mismatched_keys;
    KeyMap m_device_keys;
};

class KeysHandler: public QObject
{
  Q_OBJECT
  signals:
    void status_message(const QString &);

  public:
    KeysHandler(
        const FirmwareArchive &firmware,
        gsl::not_null<FlashInterface *> flash,
        QObject *parent = nullptr);

    KeysInfo get_keys_info();
    FirmwarePartList get_key_parts_to_write();
    void write_keys();

  private:
    FirmwareArchive m_firmware;
    FlashInterface *m_flash = nullptr;
    bool m_keys_info_read = false;
    KeysInfo m_keys_info;
};

Key key_from_firmware_part(const FirmwarePart &part);

} // ns mvp
} // ns mesytec

#endif
