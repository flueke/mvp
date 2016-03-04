#ifndef UUID_1b258d8d_e521_438d_b374_64a974cf2e1e
#define UUID_1b258d8d_e521_438d_b374_64a974cf2e1e

#include <QMap>
#include <QString>
#include <QDebug>
#include <gsl.h>
#include "flash_address.h"
#include "util.h"

namespace mesytec
{
namespace mvp
{
  class FlashInstructionError: public std::runtime_error
  {
    public:
      FlashInstructionError(const gsl::span<uchar> &instruction, const gsl::span<uchar> &response,
          const QString &message = QString("instruction error"))
        : std::runtime_error(message.toStdString())
        , m_instruction(span_to_qvector(instruction))
        , m_response(span_to_qvector(response))
      {}

      const QVector<uchar> &instruction() const { return m_instruction; }
      const QVector<uchar> &response() const { return m_response; }
      uchar statusbyte() const { return response().size() ? *(std::end(m_response) - 1) : 0; }

      const QString to_string() const
      {
        return QString("%1: instr=%2, resp=%3")
            .arg(what())
            .arg(format_bytes(m_instruction))
            .arg(format_bytes(m_response));
      }

      QVector<uchar> m_instruction;
      QVector<uchar> m_response;
  };

  class BasicFlash: public QObject
  {
    Q_OBJECT
    signals:
      void instruction_written(const QVector<uchar> &);
      void response_read(const QVector<uchar> &);
      void statusbyte_received(const uchar &);
      void data_written(const QVector<uchar> &);

    public:
      BasicFlash(QObject *parent=nullptr)
        : QObject(parent)
      {}

      BasicFlash(gsl::not_null<QIODevice *> device, QObject *parent=nullptr)
        : QObject(parent)
        , m_port(device)
      {}

      void set_port(gsl::not_null<QIODevice *> device) { m_port = device; }
      QIODevice *get_port() const { return m_port; }

      // instructions
      void nop();
      void set_area_index(uchar area_index);
      uchar read_area_index();
      void set_verbose(bool verbose);
      void boot(uchar area_index);
      void enable_write();
      virtual void erase_section(uchar index);
      uchar read_hardware_id();

      void write_page(const Address &address, uchar section,
        const gsl::span<uchar> data, int timeout_ms = constants::data_timeout_ms);

      void read_page(const Address &address, uchar section, gsl::span<uchar> dest,
        int timeout_ms = constants::data_timeout_ms);

      QVector<uchar> read_page(const Address &address, uchar section, size_t len,
        int timeout_ms = constants::data_timeout_ms);

      QVector<uchar> read_available(
        int timeout_ms = constants::default_timeout_ms);

      uchar get_last_status() const
      { return m_last_status; }

      // state carrying
      /** Sets verbose mode to the given value. Only writes to the port if
       * verbose mode is changed. */
      void maybe_set_verbose(bool verbose)
      {
        if (m_verbose != verbose)
          set_verbose(verbose);
      }

      bool get_verbose_mode() const { return m_verbose; }

      /** Enables write mode if it is not already enabled. */
      void maybe_enable_write()
      {
        if (!m_write_enabled)
          enable_write();
      }

      bool is_write_enabled() const { return m_write_enabled; }

    protected:
      void write(const gsl::span<uchar> data,
        int timeout_ms = constants::default_timeout_ms);

      void read(gsl::span<uchar> dest,
        int timeout_ms = constants::default_timeout_ms);

      void write_instruction(const gsl::span<uchar> data,
        int timeout_ms = constants::default_timeout_ms);

      void read_response(gsl::span<uchar> dest,
        int timeout_ms = constants::default_timeout_ms);

      void read_response(QVector<uchar> &buf, size_t len,
        int timeout_ms = constants::default_timeout_ms);

      void ensure_response_ok(
        const gsl::span<uchar> &instruction,
        const gsl::span<uchar> &response);

      void ensure_response_code_ok(
        const gsl::span<uchar> &response_code) const;

    private:
      QIODevice *m_port     = 0;
      bool m_verbose        = true;
      bool m_write_enabled  = false;
      uchar m_last_status   = 0;
      QVector<uchar> m_wbuf;
      QVector<uchar> m_rbuf;
  };

  struct VerifyResult
  {
    VerifyResult() = default;
    VerifyResult(size_t offset_, uchar expected_, uchar actual_)
      : offset(offset_)
      , expected(expected_)
      , actual(actual_)
    {}

    operator bool() const
    { return expected == actual; }

    QString to_string() const
    {
      if (*this)
        return "success";

      return QString("failed: offset=%1, expected=0x%2, actual=0x%3")
        .arg(offset)
        .arg(expected, 0, 16)
        .arg(actual, 0, 16);
    }

    size_t offset  = 0;
    uchar expected = 0;
    uchar actual   = 0;
  };

  class FlashVerificationError: public std::runtime_error
  {
    public:
      FlashVerificationError(const VerifyResult &result,
          const QString &message = QString("verification error"))
        : std::runtime_error(message.toStdString())
        , m_result(result)
      {}

      const VerifyResult &result() const { return m_result; }

      const QString to_string() const
      {
        return QString("%1: %2")
          .arg(what())
          .arg(m_result.to_string());
      }

    private:
      VerifyResult m_result;
  };

  class Canceled: public std::runtime_error
  {
    public:
      Canceled(): std::runtime_error("Canceled") {}
  };

  class KeyError: public std::runtime_error
  {
    public:
      KeyError(const std::string &msg = std::string("key error")):
        std::runtime_error(msg)
    {}
  };

  class Key
  {
    public:
      Key() {}
      Key(const QString &prefix, uint32_t sn, uint16_t sw, uint32_t key);

      QString  get_prefix() const { return m_prefix; }
      uint32_t get_sn() const { return m_sn; }
      uint16_t get_sw() const { return m_sw; }
      uint32_t get_key() const { return m_key; }

      QString to_string() const;

      bool operator==(const Key &o);
      bool operator!=(const Key &o)
      { return !(*this == o); }

      static Key from_flash_memory(const gsl::span<uchar> data);

    private:
      QString  m_prefix;
      uint32_t m_sn  = 0;
      uint16_t m_sw  = 0;
      uint32_t m_key = 0;
  };

  class OTPError: public std::runtime_error
  {
    public:
      OTPError(const std::string &msg = std::string("otp error")):
        std::runtime_error(msg)
    {}
  };

  class OTP
  {
    public:
      OTP() {}
      OTP(const QString &device, uint32_t sn);

      QString get_device() const { return m_device; }
      uint32_t get_sn() const { return m_sn; }

      QString to_string() const;

      static OTP from_flash_memory(const gsl::span<uchar> data);

    private:
      QString  m_device;
      uint32_t m_sn;
  };

  inline bool key_matches_otp(const Key &key, const OTP &otp)
  {
    return key.get_prefix() == otp.get_device()
      && key.get_sn() == otp.get_sn();
  }

  typedef QMap<size_t, Key> KeyMap;

  class Flash: public BasicFlash
  {
    Q_OBJECT
    signals:
      void progress_range_changed(int, int);
      void progress_changed(int);
      void progress_text_changed(const QString &);

    public:
      static const size_t default_recover_tries = 3;

      using BasicFlash::BasicFlash;

      void recover(size_t tries=default_recover_tries);
      void ensure_clean_state();

      void write_memory(const Address &start, uchar section,
        const gsl::span<uchar> data);

      typedef std::function<bool (
          const Address &, uchar, const gsl::span<uchar>)>
        EarlyReturnFun;

      QVector<uchar> read_memory(const Address &start, uchar section,
        size_t len, EarlyReturnFun f = nullptr);

      VerifyResult verify_memory(const Address &start, uchar section,
        const gsl::span<uchar> data);

      virtual void erase_section(uchar index);
      VerifyResult blankcheck_section(uchar section, size_t size);

      KeyMap read_keys();
      QSet<size_t> get_used_key_slots();
      QSet<size_t> get_free_key_slots();

      OTP read_otp();
  };

  size_t pad_to_page_size(QVector<uchar> &data);

} // ns mvp
} // ns mesytec

#endif
