#include "instruction_interpreter.h"

namespace mesytec
{
namespace mvp
{

void run_instructions(const InstructionList &instructions, Flash *m_flash)
{
}

void print_actions(const InstructionList &instructions)
{
  for (auto instr: instructions) {
    qDebug() << "writing" << instr.data.size() << " bytes"
      << "starting at address" << instr.address
      << ", data =" << instr.data;
  }
}

} // ns mvp
} // ns mesytec
