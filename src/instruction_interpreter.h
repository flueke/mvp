#ifndef UUID_088ea7d6_2dc0_46a5_ae7e_bd2daf73f88d
#define UUID_088ea7d6_2dc0_46a5_ae7e_bd2daf73f88d

#include "instruction_file.h"

namespace mesytec
{
namespace mvp
{

class Flash;

void run_instructions(const InstructionList &instructions, Flash *m_flash, uchar subindex,
    size_t address_offset = 0);
void print_actions(const InstructionList &instructions);

QVector<uchar> generate_memory(
      const InstructionList &instructions,
      size_t address_offset = 0,
      size_t min_size = 0);

} // ns mvp
} // ns mesytec

#endif
