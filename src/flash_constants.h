#ifndef UUID_33abc8f6_2854_4d53_b87e_98acd08a586b
#define UUID_33abc8f6_2854_4d53_b87e_98acd08a586b

namespace mesytec
{
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
    const uchar ERF = 0x90; // erase selected flash area; 3 dummy address bytes, 1 section byte
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
    const uchar otp_section                 =  0;
    const uchar keys_section                =  2;
    const uchar common_calibration_section  =  3;
    const uchar firmware_section            = 12;
    const uchar access_code[]               = { 0xCD, 0xAB };
    const uchar area_index_max              = 0x03;

    const size_t address_max      = 0xffffff;
    const size_t page_size        = 256;
    const size_t keys_offset      = 2048;
    const size_t max_keys         = 16;

    const QSet<uchar> valid_sections = {{0, 1, 2, 3, 8, 9, 10, 11, 12}};
    const QSet<uchar> non_area_specific_sections = {{0, 1, 2, 3}};

    const int default_timeout_ms =  3000;
    const int erase_timeout_ms   = 60000;
    const int data_timeout_ms    = 10000;
    const int init_timeout_ms    =  1000;
    const int recover_timeout_ms =   100;
  } // ns constants

  namespace keys
  {
    const size_t prefix_offset  = 0x00;
    const size_t prefix_bytes   = 8;
    const size_t sn_offset      = 0x08;
    const size_t sn_bytes       = 4;
    const size_t sw_offset      = 0x0c;
    const size_t sw_bytes       = 2;
    const size_t key_offset     = 0x10;
    const size_t key_bytes      = 4;

    const size_t total_bytes    = key_offset + key_bytes;
  } // ns keys

  namespace otp
  {
    const size_t device_offset  = 0x30;
    const size_t device_bytes   = 8;
    const size_t sn_offset      = 0x38;
    const size_t sn_bytes       = 4;

    const size_t total_bytes    = sn_offset + sn_bytes;
  } // ns otp

  inline bool is_valid_section(uchar section)
  {
    return constants::valid_sections.contains(section);
  }

  inline QList<uchar> get_valid_sections()
  {
    auto ret = constants::valid_sections.toList();
    qSort(ret);
    return ret;
  }

  inline bool is_non_area_specific_section(uchar section)
  {
    if (!is_valid_section(section))
      throw std::runtime_error("invalid section index");

    return constants::non_area_specific_sections.contains(section);
  }

  inline bool is_area_specific_section(uchar section)
  {
    return !is_non_area_specific_section(section);
  }

} // ns mvp
} // ns mesytec

#endif
