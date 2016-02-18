#include "instruction_interpreter.h"

namespace mesytec
{
namespace mvp
{

void run_instructions(const InstructionList &instructions, Flash *m_flash, uchar section,
    size_t address_offset)
{
  for (auto instr: instructions) {
    m_flash->write_memory(instr.address + address_offset, section,
        gsl::as_span(instr.data));
  }
}

void print_actions(const InstructionList &instructions)
{
  for (auto instr: instructions) {

    auto str_data = QString::fromLatin1(
        reinterpret_cast<const char *>(instr.data.constData()),
        instr.data.size());

    if (instr.type == Instruction::Type::text) {
      qDebug() << "txt: writing" << instr.data.size() << " bytes"
        << "starting at address" << instr.address
        << ", data =" << str_data;
    } else if (instr.type == Instruction::Type::binary) {
      qDebug() << "hex: writing" << instr.data.size() << " bytes"
        << "starting at address" << instr.address
        << ", data =" << instr.data;
    }
  }
}

QVector<uchar> generate_memory(
    const InstructionList &instructions,
    size_t address_offset,
    size_t min_size)
{
  QVector<uchar> ret;

  for (const auto &instr: instructions) {
    const auto addr = (instr.address + address_offset).to_int();
    const auto len  = instr.data.size();
    const auto size = addr + len;
    const auto old_size = static_cast<size_t>(ret.size());

    if (old_size < size) {
      ret.resize(size);
      std::fill(std::begin(ret) + old_size, std::end(ret), 0xFFu);
    }

    std::copy(std::begin(instr.data), std::end(instr.data),
        std::begin(ret) + addr);
  }

  const auto old_size = static_cast<size_t>(ret.size());

  if (old_size < min_size) {
    ret.resize(min_size);
    std::fill(std::begin(ret) + old_size, std::end(ret), 0xFFu);
  }

  return ret;
}

} // ns mvp
} // ns mesytec
