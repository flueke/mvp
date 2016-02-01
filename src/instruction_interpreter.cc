#include "instruction_interpreter.h"

namespace mesytec
{
namespace mvp
{

void run_instructions(const InstructionList &instructions, Flash *m_flash, uchar subindex,
    int address_offset)
{
  for (auto instr: instructions) {
    m_flash->write_memory(instr.address + address_offset, subindex,
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

} // ns mvp
} // ns mesytec
