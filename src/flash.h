#ifndef UUID_1b258d8d_e521_438d_b374_64a974cf2e1e
#define UUID_1b258d8d_e521_438d_b374_64a974cf2e1e

#include <QMap>
#include <QString>
#include <QDebug>
#include <gsl.h>
#include "util.h"

namespace mvp
{
  namespace opcodes
  {
    const uchar NOP = 0x00; // no op
    const uchar RES = 0x10; // reset (currently no effect)
    const uchar SAI = 0x20; // set area index
    const uchar RAI = 0x30; // read area index
    const uchar UFA = 0x40; // unprotect factory areas
    const uchar RDI = 0x50; // read ID
    const uchar VEB = 0x60; /* set verbose on/off
                             * If set to on (value=0) WRF won't return
                             * anything, REF will return only data.
                             * If set to off (value=1) WRF and REF will return
                             * the data plus the statusbytes. */
    const uchar BFP = 0x70; // set FPGA boot area index and boot
    const uchar EFW = 0x80; // enable flash write/erase;
                            // cleared after any op except WRF; cleared on error
    const uchar ERF = 0x90; // erase selected flash area; 3 dummy address bytes, 1 subindex byte
    const uchar WRF = 0xA0; // write flash or OTP
    const uchar REF = 0xB0; // read flash or OTP

    const QMap<uchar, QString> op_to_string_data = {
      { NOP, "NOP" },
      { RES, "RES" },
      { SAI, "SAI" },
      { RAI, "RAI" },
      { UFA, "UFA" },
      { RDI, "RDI" },
      { VEB, "VEB" },
      { BFP, "BFP" },
      { EFW, "EFW" },
      { ERF, "ERF" },
      { WRF, "WRF" },
      { REF, "REF" },
    };
  } // ns opcodes

  inline QString op_to_string(uchar op)
  {
    return opcodes::op_to_string_data.value(op,
        QString::number(static_cast<int>(op), 16));
  }

  namespace status
  {
    const uchar inst_success  = 0x01;
    const uchar area          = (0x01 << 1 | 0x01 << 2);
    const uchar dipswitch     = (0x01 << 3 | 0x01 << 4);
  } // ns status

  inline int get_area(uchar statusbyte)
  {
    return (statusbyte & status::area) >> 1;
  }

  inline int get_dipswitch(uchar statusbyte)
  {
    return (statusbyte & status::dipswitch) >> 3;
  }

  namespace constants
  {
    const uchar firmware_subindex = 0x0C;
    const uchar access_code[]     = { 0xCD, 0xAB };
    const uchar area_index_max    = 0x03;

    const size_t address_max    = 0xffffff;
    const size_t sector_size       = 64 * 1024;
    const size_t subsector_size    =  4 * 1024;
    const size_t firmware_sectors  = 51;
    const size_t page_size      = 256;

    // 51 sectors * 64 * 1024 = 3342336
    const size_t firmware_max_size = firmware_sectors * sector_size;

    const QMap<uchar, size_t> section_max_sizes = {
      {  0, 63 },
      {  1, sector_size },
      {  2, sector_size },
      {  3, sector_size * 8 },
      {  8, subsector_size },
      {  9, sector_size },
      { 10, sector_size },
      { 11, sector_size * 6 },
      { 12, firmware_max_size }
    };

    const int default_timeout_ms =  3000;
    const int erase_timeout_ms   = 60000;
    const int data_timeout_ms    = 10000;
    const int init_timeout_ms    =  1000;
    const int recover_timeout_ms =   100;
  } // ns constants


  inline bool is_valid_section(uchar section)
  {
    return constants::section_max_sizes.contains(section);
  }

  inline size_t get_section_max_size(uchar section)
  {
    if (!is_valid_section(section))
      throw std::runtime_error("invalid section index");

    return constants::section_max_sizes.value(section);
  }

  class Address
  {
    public:
      Address() = default;

      Address(uchar a0, uchar a1, uchar a2): _data({a0, a1, a2}) {}

      Address(const Address &o): _data(o._data) {}

      explicit Address(uint32_t a) { set_value(a); }

      uchar a0() const { return _data[0]; }
      uchar a1() const { return _data[1]; }
      uchar a2() const { return _data[2]; }

      void set_value(uint32_t a)
      {
        //qDebug() << "Address::set_value() value =" << a;

        if (a > constants::address_max)
          throw std::out_of_range("address range exceeded");

        _data = {
          gsl::narrow_cast<uchar>((a & 0x0000ff)),
          gsl::narrow_cast<uchar>((a & 0x00ff00) >> 8),
          gsl::narrow_cast<uchar>((a & 0xff0000) >> 16)
        };
      }

      uchar operator[](size_t idx) const
      {
        if (idx >= size())
          throw std::out_of_range("address index out of range");
        return _data[idx];
      }

      Address &operator++()
      {
        if (*this == Address(constants::address_max))
          throw std::overflow_error("address range exceeded");

        if (++_data[0] == 0)
          if (++_data[1] == 0)
            ++_data[2];

        return *this;
      }

      Address operator++(int)
      {
        auto ret(*this);
        operator++();
        return ret;
      }

      bool operator==(const Address &o) const
      { return _data == o._data; }

      bool operator!=(const Address &o) const
      { return !operator==(o); }

      constexpr size_t size() const { return 3; }

      uint32_t to_int() const
      { return _data[0] | _data[1] << 8 | _data[2] << 16; }

      bool operator>(const Address &o) const
      { return to_int() > o.to_int(); }

      bool operator<(const Address &o) const
      { return to_int() < o.to_int(); }

      Address operator+(const Address &other) const
      {
        return Address(to_int() + other.to_int());
      }

      Address operator-(const Address &other) const
      {
        return Address(to_int() - other.to_int());
      }

      Address &operator+=(int i)
      {
        set_value(to_int() + i);
        return *this;
      }

    private:
      std::array<uchar, 3> _data = {{0, 0, 0}};
  };

  QDebug operator<<(QDebug dbg, const Address &a);

  class InstructionError: public std::runtime_error
  {
    public:
      InstructionError(const gsl::span<uchar> &instruction, const gsl::span<uchar> &response,
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
      virtual void erase_subindex(uchar index);
      uchar read_hardware_id();

      void write_page(const Address &address, uchar subindex,
        const gsl::span<uchar> data, int timeout_ms = constants::data_timeout_ms);

      void read_page(const Address &address, uchar subindex, gsl::span<uchar> dest,
        int timeout_ms = constants::data_timeout_ms);

      QVector<uchar> read_page(const Address &address, uchar subindex, size_t len,
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

  class Canceled: public std::runtime_error
  {
    public:
      Canceled(): std::runtime_error("Canceled") {}
  };

  class Flash: public BasicFlash
  {
    Q_OBJECT
    signals:
      void progress_range_changed(int, int);
      void progress_changed(int);
      void progress_text_changed(const QString &);

    public:

      using BasicFlash::BasicFlash;

      void recover(size_t tries=10);
      void ensure_clean_state();

      void write_memory(const Address &start, uchar subindex,
        const gsl::span<uchar> data);

      typedef std::function<bool (
          const Address &, uchar, const gsl::span<uchar>)>
        EarlyReturnFun;

      QVector<uchar> read_memory(const Address &start, uchar subindex,
        size_t len, EarlyReturnFun f = nullptr);

      VerifyResult verify_memory(const Address &start, uchar subindex,
        const gsl::span<uchar> data);

      virtual void erase_subindex(uchar index);
      void erase_firmware();
      void write_firmware(const gsl::span<uchar> data);

      VerifyResult verify_firmware(const gsl::span<uchar> data);
      VerifyResult blankcheck_firmware();
  };

  size_t pad_to_page_size(QVector<uchar> &data);

} // ns mvp

#endif
