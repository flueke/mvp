#include "flash.h"
#include <boost/endian/conversion.hpp>
#include <boost/format.hpp>

namespace mesytec
{
namespace mvp
{

size_t get_default_mem_read_chunk_size()
{
#ifdef Q_OS_WIN
  // Workaround for Qt versions starting from 5.12: reading larger amounts of
  // data (e.g. a full 256 byte page) in one go from the serial port leads to a
  // read timeout.
  // Dividing the page into smaller  chunks fixes the problem.
  // See https://bugreports.qt.io/browse/QTBUG-93865 for more info.
  const size_t chunk_size = constants::page_size / 8;
#else
  const size_t chunk_size = constants::page_size;
#endif
  return chunk_size;
}

using constants::access_code;

FlashInterface::~FlashInterface() {}

void FlashInterface::read_response(QVector<uchar> &buf, size_t len, int timeout_ms)
{
  buf.clear();
  buf.resize(len);
  auto span = gsl::span(buf);
  read_response(span, timeout_ms);
}

QVector<uchar> FlashInterface::read_page(const Address &addr, uchar section,
  size_t len, int timeout_ms)
{
  QVector<uchar> ret(len);
  read_page(addr, section, gsl::span(ret), timeout_ms);
  return ret;
}

void FlashInterface::ensure_response_ok(
  const gsl::span<uchar> &instruction,
  const gsl::span<uchar> &response)
{
  if (response.size() < 2) {
    throw FlashInstructionError(instruction, response, "short response (size<2)");
  }

  if (!std::equal(std::begin(instruction), std::end(instruction),
        std::begin(response))) {
    throw FlashInstructionError(instruction, response,
      "response contents do not equal instruction contents");
  }

  try {
    const gsl::span<uchar> response_code{std::begin(response) + (response.size() - 2),
          std::end(response)};
    m_last_status = *(response_code.begin()+1);
    emit statusbyte_received(m_last_status);
    ensure_response_code_ok(response_code);
  } catch (const std::runtime_error &e) {
    m_write_enabled = false; // write enable is unset on error
    throw FlashInstructionError(instruction, response, e.what());
  }
}

void FlashInterface::ensure_response_code_ok(const gsl::span<uchar> &response_code) const
{
  if (response_code.size() != 2)
    throw std::runtime_error("invalid response code size (expected size=2)");

  if (0xff != *std::begin(response_code))
    throw std::runtime_error("invalid response code start (expected 0xff)");

  if (!(status::inst_success & (*(std::begin(response_code)+1))))
    throw std::runtime_error("instruction failed");
}

void SerialPortFlash::recover(size_t tries)
{
  std::exception_ptr last_nop_exception;

  qDebug() << "begin recover(): tries =" << tries;

  for (size_t n=0; n<tries; ++n) {
    try {
      auto data = read_available(constants::recover_timeout_ms);
      qDebug() << "recover(): read_available():" << format_bytes(data);
    } catch (const ComError &e) {
      qDebug() << "ignoring ComError from read_available()";
    }

    try {
      nop();
      return;
    } catch (const std::exception &e) {
      last_nop_exception = std::current_exception();
      qDebug() << "Flash::recover(): exception from NOP" << e.what();
    }
  }

  if (last_nop_exception)
    std::rethrow_exception(last_nop_exception);
  else
    throw std::runtime_error("NOP recovery failed for an unknown reason");
}

void FlashInterface::ensure_clean_state()
{
  // FIXME: leftover bytes need to be logged
  qDebug() << "begin ensure_clean_state";
  recover();
  qDebug() << "end ensure_clean_state";
}


void FlashInterface::write_memory(const Address &start, uchar section,
  const gsl::span<uchar> data)
{
  Address addr(start);
  size_t remaining = data.size();
  size_t offset    = 0;

  emit progress_range_changed(0, std::max(static_cast<int>(remaining / constants::page_size), 1));
  int progress = 0;

  while (remaining) {
    emit progress_changed(progress++);
    auto len = std::min(constants::page_size, remaining);
    write_page(addr, section, gsl::span(data.data() + offset, len));

    remaining -= len;
    addr      += len;
    offset    += len;
  }
}

QVector<uchar> FlashInterface::read_memory(const Address &start, uchar section,
  size_t len, size_t chunk_size, EarlyReturnFun early_return_fun)
{
  qDebug() << "read_memory: start =" << start
           << ", section =" << section
           << ", len =" << len
           << ", chunk_size =" << chunk_size
           << ", early return function set =" << bool(early_return_fun);

  QVector<uchar> ret(len);

  Address addr(start);
  size_t remaining = len;
  size_t offset    = 0;


  emit progress_range_changed(0, std::max(static_cast<int>(remaining / chunk_size), 1));
  int progress = 0;

  set_verbose(false);

  while (remaining) {
    emit progress_changed(progress++);

    auto rl = std::min(chunk_size, remaining);
    auto page_span = gsl::span(ret.data() + offset, rl);
    read_page(addr, section, page_span);

    offset    += rl;

    if (early_return_fun && early_return_fun(addr, section, page_span)) {
      ret.resize(offset);
      return ret;
    }

    remaining -= rl;
    addr      += rl;
  }

  return ret;
}

VerifyResult FlashInterface::verify_memory(const Address &start, uchar section,
  const gsl::span<uchar> data)
{
  set_verbose(false);

  auto fun = [&](const Address &addr, uchar, const gsl::span<uchar> page) {
    auto res = std::mismatch(page.begin(), page.end(), data.begin() + addr.to_int());
    return res.first != page.end();
  };

  auto mem = read_memory(start, section, data.size(), get_default_mem_read_chunk_size(), fun);
  auto res = std::mismatch(mem.begin(), mem.end(), data.begin());

  if (res.first == mem.end())
    return VerifyResult();

  return VerifyResult(res.second - data.begin(), *res.second, *res.first);
}

void FlashInterface::nop()
{
  m_wbuf = { opcodes::NOP };
  write_instruction(m_wbuf);
  read_response(m_rbuf, size_t(3));
  ensure_response_ok(m_wbuf, m_rbuf);
}

void FlashInterface::set_area_index(uchar area_index)
{
  m_wbuf = { opcodes::SAI, access_code[0], access_code[1], area_index };

  write_instruction(m_wbuf);
  read_response(m_rbuf, m_wbuf.size() + size_t(2));
  ensure_response_ok(m_wbuf, m_rbuf);
}

uchar FlashInterface::read_area_index()
{
  m_wbuf = { opcodes::RAI };

  write_instruction(m_wbuf);
  read_response(m_rbuf, size_t(4));
  ensure_response_ok(m_wbuf, m_rbuf);
  return m_rbuf[1];
}

void FlashInterface::set_verbose(bool verbose)
{
  qDebug() << "set_verbose:" << verbose;
  uchar veb = verbose ? 0 : 1;
  m_wbuf    = { opcodes::VEB, access_code[0], access_code[1], veb };
  write_instruction(m_wbuf);
  read_response(m_rbuf, size_t(6));
  ensure_response_ok(m_wbuf, m_rbuf);
  m_verbose = verbose;
}

void FlashInterface::boot(uchar area_index)
{
  m_wbuf = { opcodes::BFP, access_code[0], access_code[1], area_index };
  write_instruction(m_wbuf);
  read_response(m_rbuf, size_t(6));
  ensure_response_ok(m_wbuf, m_rbuf);
}

void FlashInterface::enable_write()
{
  qDebug() << "begin enable_write";

  m_wbuf = { opcodes::EFW, access_code[0], access_code[1] };
  write_instruction(m_wbuf);
  read_response(m_rbuf, size_t(5));
  ensure_response_ok(m_wbuf, m_rbuf);
  m_write_enabled = true;

  qDebug() << "end enable_write: set write_enable flag";
}

void FlashInterface::erase_section(uchar index)
{
  maybe_enable_write();
  m_wbuf = { opcodes::ERF, 0, 0, 0, index };
  write_instruction(m_wbuf);
  read_response(m_rbuf, 7, constants::erase_timeout_ms);
  ensure_response_ok(m_wbuf, m_rbuf);
}

uchar FlashInterface::read_hardware_id()
{
  m_wbuf = { opcodes::RDI };
  write_instruction(m_wbuf);
  read_response(m_rbuf, size_t(4));
  ensure_response_ok(m_wbuf, m_rbuf);
  return m_rbuf[1];
}

void SerialPortFlash::write_page(const Address &addr, uchar section,
  const gsl::span<uchar> data, int timeout_ms)
{
  //if (addr[0] != 0)
  //  throw std::invalid_argument("write_page: address is not page aligned (a0!=0)");

  const auto sz = data.size();

  if (sz == 0)
    throw std::invalid_argument("write_page: empty data given");

  if (sz > constants::page_size)
    throw std::invalid_argument("write_page: data size > page size");

  const bool use_verbose = false;

  maybe_set_verbose(use_verbose);
  maybe_enable_write();

  uchar len_byte(sz == constants::page_size ? 0 : sz); // 256 encoded as 0
  m_wbuf = { opcodes::WRF, addr[0], addr[1], addr[2], section, len_byte };

  //qDebug() << "WRF: addr=" << addr << ", si =" << section << ", len =" << len_byte;

  write_instruction(m_wbuf);
  write(data, timeout_ms);

  emit data_written(span_to_qvector(data));

  //qDebug() << "WRF data written:" << span_to_qvector(data);

  if (use_verbose) {
    try {
      //QVector<uchar> rbuf(use_verbose ? 4 + data.size() : 4);
      //read(gsl::span(rbuf));
      auto rbuf = read_available();
      qDebug() << "write_page: try read yielded" << rbuf;
    } catch (const std::exception &e) {
      qDebug() << "write_page: try read raised" << e.what();
    }
  }
}

void SerialPortFlash::read_page(const Address &addr, uchar section,
  gsl::span<uchar> dest, int timeout_ms)
{
  //if (addr[0] != 0)
  //  throw std::invalid_argument("read_page: address is not page aligned (a0!=0)");

  qDebug() << "read_page: addr =" << addr << ", section =" << static_cast<uint32_t>(section)
    << ", dest.size() =" << dest.size() << ", timeout_ms =" << timeout_ms;

  auto len = dest.size();

  if (len == 0)
    throw std::invalid_argument("read_page: len == 0");

  if (len > constants::page_size)
    throw std::invalid_argument("read_page: len > page size");

  maybe_set_verbose(false);

  uchar len_byte(len == constants::page_size ? 0 : len); // 256 encoded as 0

  m_wbuf = { opcodes::REF, addr[0], addr[1], addr[2], section, len_byte };
  write_instruction(m_wbuf);
  read(dest, timeout_ms);
}

void SerialPortFlash::write_instruction(const gsl::span<uchar> data, int timeout_ms)
{
  write(data, timeout_ms);

  uchar opcode(*std::begin(data));

  if (opcode != opcodes::WRF && m_write_enabled) {
    // any instruction except WRF (and EFW) unsets write enable
    qDebug() << "clearing cached write_enable flag (instruction ="
      << op_to_string(opcode) << "!= WRF)";
    m_write_enabled = false;
  }

  //qDebug() << "instruction written:" << format_bytes(span_to_qvector(data));

  emit instruction_written(span_to_qvector(data));
}

void SerialPortFlash::read_response(gsl::span<uchar> dest, int timeout_ms)
{
  read(dest, timeout_ms);
  emit response_read(span_to_qvector(dest));
}

void SerialPortFlash::write(const gsl::span<uchar> data, int timeout_ms)
{
  qint64 res = m_port->write(reinterpret_cast<const char *>(data.data()), data.size());

  if (res != static_cast<qint64>(data.size())
      || !m_port->waitForBytesWritten(timeout_ms)) {
    throw make_com_error(m_port);
  }
}

void SerialPortFlash::read(gsl::span<uchar> dest, int timeout_ms)
{
  const auto len = dest.size();

  if (len == 0)
    throw std::invalid_argument("read: destination size == 0");

  size_t bytes_read = 0;

  while (bytes_read < len) {
    if (!m_port->bytesAvailable() && !m_port->waitForReadyRead(timeout_ms))
      throw make_com_error(m_port);

    qint64 to_read = std::min(m_port->bytesAvailable(), static_cast<qint64>(len - bytes_read));
    qint64 res     = m_port->read(reinterpret_cast<char *>(dest.begin()) + bytes_read, to_read);

    if (res != to_read)
      throw make_com_error(m_port);

    bytes_read += res;
  }
}

QVector<uchar> SerialPortFlash::read_available(int timeout_ms)
{
  if (!m_port->bytesAvailable() && !m_port->waitForReadyRead(timeout_ms))
    throw make_com_error(m_port);

  QVector<uchar> ret(m_port->bytesAvailable());

  qint64 res = m_port->read(reinterpret_cast<char *>(ret.data()), ret.size());

  if (res != ret.size())
    throw make_com_error(m_port);

  if (m_port->bytesAvailable()) {
    qDebug() << "read_available: there are still" << m_port->bytesAvailable() << " bytes available";
  }

  return ret;
}

Key::Key(const QString &prefix, uint32_t sn, uint16_t sw, uint32_t key)
  : m_prefix(prefix)
  , m_sn(sn)
  , m_sw(sw)
  , m_key(key)
{
  if (m_prefix.size() != keys::prefix_bytes)
    throw KeyError("Invalid prefix size");
}

Key Key::from_flash_memory(const gsl::span<uchar> data)
{
  if (data.size() < keys::total_bytes)
    throw KeyError("Key::from_flash_memory: given data is too short");

  Key ret;

  std::transform(
      std::begin(data) + keys::prefix_offset,
      std::begin(data) + keys::prefix_offset + keys::prefix_bytes,
      std::back_inserter(ret.m_prefix),
      [](uchar c) { return static_cast<char>(c); });

  ret.m_sn  = boost::endian::big_to_native(*(reinterpret_cast<uint32_t *>(data.data() + keys::sn_offset)));
  ret.m_sw  = boost::endian::big_to_native(*(reinterpret_cast<uint16_t *>(data.data() + keys::sw_offset)));
  ret.m_key = boost::endian::big_to_native(*(reinterpret_cast<uint32_t *>(data.data() + keys::key_offset)));

  return ret;
}

QString Key::to_string() const
{
  auto fmt = boost::format("Key(sn=%|1$|%|2$08X|, sw=%|3$04X|, key=%|4$08X|)")
    % get_prefix().toStdString() % get_sn() % get_sw() % get_key();

  return QString::fromStdString(fmt.str());
}

bool Key::operator==(const Key &o) const
{
  return get_prefix() == o.get_prefix()
    && get_sn() == o.get_sn()
    && get_sw() == o.get_sw()
    && get_key() == o.get_key();
}

OTP::OTP(const QString &device, uint32_t sn)
  : m_device(device)
  , m_sn(sn)
{
  if (m_device.size() != otp::device_bytes)
    throw OTPError("Invalid device name length");
}

OTP OTP::from_flash_memory(const gsl::span<uchar> data)
{
  if (data.size() < otp::total_bytes)
    throw OTPError("OTP::from_flash_memory: given data is too short");

  OTP ret;

  std::transform(
      std::begin(data) + otp::device_offset,
      std::begin(data) + otp::device_offset + otp::device_bytes,
      std::back_inserter(ret.m_device),
      [](uchar c) { return static_cast<char>(c); });

  ret.m_sn  = boost::endian::big_to_native(*(reinterpret_cast<uint32_t *>(data.data() + otp::sn_offset)));

  return ret;
}

QString OTP::to_string() const
{
  auto fmt = boost::format("OTP(dev=%|1$|, sn=%|2$08X|)")
    % get_device().toStdString() % get_sn();

  return QString::fromStdString(fmt.str());
}

VerifyResult FlashInterface::blankcheck_section(uchar section, size_t size)
{
  emit progress_text_changed(QString("Blankchecking section %1 (sz=%2)")
      .arg(static_cast<int>(section))
      .arg(size));

  set_verbose(false);

  auto fun = [&](const Address &, uchar, const gsl::span<uchar> page) {
    auto it = std::find_if(page.begin(), page.end(), [](uchar c) { return c != 0xff; });
    return it != page.end();
  };

  auto mem = read_memory({0, 0, 0}, section, size, get_default_mem_read_chunk_size(), fun);
  auto it  = std::find_if(mem.begin(), mem.end(), [](uchar c) { return c != 0xff; });
  auto ret = (it == mem.end() ? VerifyResult() : VerifyResult(it - mem.begin(), 0xff, *it));

  emit progress_text_changed(QString("Blankcheck result for section %1: %2")
      .arg(static_cast<int>(section))
      .arg(ret.to_string()));

  return ret;
}

KeyMap FlashInterface::read_keys()
{
  KeyMap ret;

  for (size_t i=0; i<constants::max_keys; ++i) {

    auto addr = Address(i * constants::keys_offset);
    auto mem  = read_memory(addr, constants::keys_section, keys::total_bytes, get_default_mem_read_chunk_size());
    auto it   = std::find_if(mem.begin(), mem.end(), [](uchar c) { return c != 0xff; });

    if (it == mem.end())
      continue;

    auto key = Key::from_flash_memory(mem);

    ret[i] = key;
  }

  return ret;
}

QSet<size_t> FlashInterface::get_used_key_slots()
{
  auto keymap = read_keys();
  auto keys = keymap.keys();

  return QSet<size_t>(std::begin(keys), std::end(keys));
}

QSet<size_t> FlashInterface::get_free_key_slots()
{
  auto used = get_used_key_slots();
  QSet<size_t> all_slots;

  for (size_t i=0; i<constants::max_keys; ++i)
    all_slots.insert(i);

  return all_slots.subtract(used);
}

OTP FlashInterface::read_otp()
{
#ifdef Q_OS_WIN
  // Workaround for Qt versions starting from 5.12: reading larger amounts of
  // data (e.g. a full 256 byte page) in one go leads to a read timeout.
  // Dividing the page into smaller  chunks fixes the problem.
  // See https://bugreports.qt.io/browse/QTBUG-93865 for more info.
  const size_t chunk_size = constants::page_size / 4;
#else
  const size_t chunk_size = constants::page_size;
#endif

  auto mem = read_memory({0, 0, 0}, constants::otp_section, otp::total_bytes, chunk_size);

  qDebug() << "read_otp() data:\n" << format_bytes(mem);

  return OTP::from_flash_memory(mem);
}

size_t pad_to_page_size(QVector<uchar> &data)
{
  auto rest = data.size() % constants::page_size;
  if (rest) {
    const auto pad = constants::page_size - rest;

    for (size_t i=0; i<pad; ++i)
      data.push_back(0xff);

    return pad;
  }
  return 0;
}

} // ns mvp
} // ns mesytec
