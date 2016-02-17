#include "flash.h"

namespace mesytec
{
namespace mvp
{

using constants::access_code;

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

void BasicFlash::nop()
{
  m_wbuf = { opcodes::NOP };
  write_instruction(m_wbuf);
  read_response(m_rbuf, size_t(3));
  ensure_response_ok(m_wbuf, m_rbuf);
}

void BasicFlash::set_area_index(uchar area_index)
{
  m_wbuf = { opcodes::SAI, access_code[0], access_code[1], area_index };

  write_instruction(m_wbuf);
  read_response(m_rbuf, m_wbuf.size() + size_t(2));
  ensure_response_ok(m_wbuf, m_rbuf);
}

uchar BasicFlash::read_area_index()
{
  m_wbuf = { opcodes::RAI };

  write_instruction(m_wbuf);
  read_response(m_rbuf, size_t(4));
  ensure_response_ok(m_wbuf, m_rbuf);
  return m_rbuf[1];
}

void BasicFlash::set_verbose(bool verbose)
{
  qDebug() << "set_verbose:" << verbose;
  uchar veb = verbose ? 0 : 1;
  m_wbuf    = { opcodes::VEB, access_code[0], access_code[1], veb };
  write_instruction(m_wbuf);
  read_response(m_rbuf, size_t(6));
  ensure_response_ok(m_wbuf, m_rbuf);
  m_verbose = verbose;
}

void BasicFlash::boot(uchar area_index)
{
  m_wbuf = { opcodes::BFP, access_code[0], access_code[1], area_index };
  write_instruction(m_wbuf);
  read_response(m_rbuf, size_t(6));
  ensure_response_ok(m_wbuf, m_rbuf);
}

void BasicFlash::enable_write()
{
  qDebug() << "begin enable_write";

  m_wbuf = { opcodes::EFW, access_code[0], access_code[1] };
  write_instruction(m_wbuf);
  read_response(m_rbuf, size_t(5));
  ensure_response_ok(m_wbuf, m_rbuf);
  m_write_enabled = true;

  qDebug() << "end enable_write: set write_enable flag";
}

void BasicFlash::erase_subindex(uchar index)
{
  maybe_enable_write();
  m_wbuf = { opcodes::ERF, 0, 0, 0, index };
  write_instruction(m_wbuf);
  read_response(m_rbuf, 7, constants::erase_timeout_ms);
  ensure_response_ok(m_wbuf, m_rbuf);
}

uchar BasicFlash::read_hardware_id()
{
  m_wbuf = { opcodes::RDI };
  write_instruction(m_wbuf);
  read_response(m_rbuf, size_t(4));
  ensure_response_ok(m_wbuf, m_rbuf);
  return m_rbuf[1];
}

void BasicFlash::write_page(const Address &addr, uchar subindex,
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
  m_wbuf = { opcodes::WRF, addr[0], addr[1], addr[2], subindex, len_byte };

  //qDebug() << "WRF: addr=" << addr << ", si =" << subindex << ", len =" << len_byte;

  write_instruction(m_wbuf);
  write(data, timeout_ms);

  //qDebug() << "WRF data written:" << span_to_qvector(data);

  if (use_verbose) {
    try {
      //QVector<uchar> rbuf(use_verbose ? 4 + data.size() : 4);
      //read(gsl::as_span(rbuf));
      auto rbuf = read_available();
      qDebug() << "write_page: try read yielded" << rbuf;
    } catch (const std::exception &e) {
      qDebug() << "write_page: try read raised" << e.what();
    }
  }
}

void BasicFlash::read_page(const Address &addr, uchar subindex,
  gsl::span<uchar> dest, int timeout_ms)
{
  //if (addr[0] != 0)
  //  throw std::invalid_argument("read_page: address is not page aligned (a0!=0)");

  auto len = dest.size();

  if (len == 0)
    throw std::invalid_argument("read_page: len == 0");

  if (len > constants::page_size)
    throw std::invalid_argument("read_page: len > page size");

  maybe_set_verbose(false);

  uchar len_byte(len == constants::page_size ? 0 : len); // 256 encoded as 0

  m_wbuf = { opcodes::REF, addr[0], addr[1], addr[2], subindex, len_byte };
  write_instruction(m_wbuf);
  read(dest, timeout_ms);
}

QVector<uchar> BasicFlash::read_page(const Address &addr, uchar subindex,
  size_t len, int timeout_ms)
{
  QVector<uchar> ret(len);
  read_page(addr, subindex, gsl::as_span(ret), timeout_ms);
  return ret;
}

void BasicFlash::write_instruction(const gsl::span<uchar> data, int timeout_ms)
{
  write(data, timeout_ms);

  uchar opcode(*std::begin(data));

  if (opcode != opcodes::WRF && m_write_enabled) {
    // any instruction except WRF (and EFW) unsets write enable
    qDebug() << "clearing cached write_enable flag (instruction ="
      << op_to_string(opcode) << "!= WRF)";
    m_write_enabled = false;
  }

  qDebug() << "instruction written:" << format_bytes(span_to_qvector(data));

  emit instruction_written(span_to_qvector(data));
}

void BasicFlash::read_response(gsl::span<uchar> dest, int timeout_ms)
{
  read(dest, timeout_ms);
  emit response_read(span_to_qvector(dest));
}

void BasicFlash::read_response(QVector<uchar> &buf, size_t len, int timeout_ms)
{
  buf.clear();
  buf.resize(len);
  auto span = gsl::as_span(buf);
  read_response(span, timeout_ms);
}

void BasicFlash::write(const gsl::span<uchar> data, int timeout_ms)
{
  qint64 res = m_port->write(reinterpret_cast<const char *>(data.data()), data.size());

  if (res != static_cast<qint64>(data.size())
      || !m_port->waitForBytesWritten(timeout_ms)) {
    throw make_com_error(m_port);
  }
}

void BasicFlash::read(gsl::span<uchar> dest, int timeout_ms)
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

QVector<uchar> BasicFlash::read_available(int timeout_ms)
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

void BasicFlash::ensure_response_ok(
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

void BasicFlash::ensure_response_code_ok(const gsl::span<uchar> &response_code) const
{
  if (response_code.size() != 2)
    throw std::runtime_error("invalid response code size (expected size=2)");

  if (0xff != *std::begin(response_code))
    throw std::runtime_error("invalid response code start (expected 0xff)");

  if (!(status::inst_success & (*(std::begin(response_code)+1))))
    throw std::runtime_error("instruction failed");
}

void Flash::recover(size_t tries)
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

void Flash::ensure_clean_state()
{
  // FIXME: leftover bytes need to be logged
  qDebug() << "begin ensure_clean_state";
  recover();
  qDebug() << "end ensure_clean_state";
}

void Flash::write_memory(const Address &start, uchar subindex, const gsl::span<uchar> data)
{
  Address addr(start);
  size_t remaining = data.size();
  size_t offset    = 0;

  emit progress_range_changed(0, std::max(static_cast<int>(remaining / constants::page_size), 1));
  int progress = 0;

  while (remaining) {
    emit progress_changed(progress++);
    auto len = std::min(constants::page_size, remaining);
    write_page(addr, subindex, gsl::as_span(data.data() + offset, len));

    remaining -= len;
    addr      += len;
    offset    += len;
  }
}

QVector<uchar> Flash::read_memory(const Address &start, uchar subindex,
  size_t len, EarlyReturnFun early_return_fun)
{
  qDebug() << "read_memory: start =" << start
           << ", si =" << subindex
           << ", len =" << len
           << ", early return function set =" << bool(early_return_fun);

  QVector<uchar> ret(len);

  Address addr(start);
  size_t remaining = len;
  size_t offset    = 0;

  emit progress_range_changed(0, std::max(static_cast<int>(remaining / constants::page_size), 1));
  int progress = 0;

  set_verbose(false);

  while (remaining) {
    emit progress_changed(progress++);

    auto rl = std::min(constants::page_size, remaining);
    auto page_span = gsl::as_span(ret.data() + offset, rl);
    read_page(addr, subindex, page_span);

    offset    += rl;

    if (early_return_fun && early_return_fun(addr, subindex, page_span)) {
      ret.resize(offset);
      return ret;
    }

    remaining -= rl;
    addr      += rl;
  }

  return ret;
}

VerifyResult Flash::verify_memory(const Address &start, uchar subindex, const gsl::span<uchar> data)
{
  set_verbose(false);

  auto fun = [&](const Address &addr, uchar, const gsl::span<uchar> page) {
    auto res = std::mismatch(page.begin(), page.end(), data.begin() + addr.to_int());
    return res.first != page.end();
  };

  auto mem = read_memory(start, subindex, data.size(), fun);
  auto res = std::mismatch(mem.begin(), mem.end(), data.begin());

  if (res.first == mem.end())
    return VerifyResult();

  return VerifyResult(res.second - data.begin(), *res.second, *res.first);
}

void Flash::erase_subindex(uchar index)
{
  emit progress_range_changed(0, 0);
  BasicFlash::erase_subindex(index);
}

void Flash::erase_firmware()
{
  emit progress_text_changed("Erasing firmware");
  erase_subindex(constants::firmware_subindex);
}

void Flash::write_firmware(const gsl::span<uchar> data)
{
  emit progress_text_changed("Writing firmware");
  set_verbose(false);
  write_memory({0, 0, 0}, constants::firmware_subindex, data);
}

VerifyResult Flash::verify_firmware(const gsl::span<uchar> data)
{
  emit progress_text_changed("Verifying firmware");
  set_verbose(false);
  auto ret = verify_memory({0, 0, 0}, constants::firmware_subindex, data);

  emit progress_text_changed(QString("Verify firmware: %1").arg(ret.to_string()));

  return ret;
}

VerifyResult Flash::blankcheck_firmware()
{
  emit progress_text_changed("Blankcheck");

  set_verbose(false);

  auto fun = [&](const Address &, uchar, const gsl::span<uchar> page) {
    auto it = std::find_if(page.begin(), page.end(), [](uchar c) { return c != 0xff; });
    return it != page.end();
  };

  auto mem = read_memory({0, 0, 0}, constants::firmware_subindex,
    constants::firmware_max_size, fun);

  auto it  = std::find_if(mem.begin(), mem.end(), [](uchar c) { return c != 0xff; });
  auto ret = (it == mem.end() ? VerifyResult() : VerifyResult(it - mem.begin(), 0xff, *it));

  emit progress_text_changed(QString("Blankcheck: %1").arg(ret.to_string()));

  return ret;
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
