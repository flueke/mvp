#ifndef UUID_088ea7d6_2dc0_46a5_ae7e_bd2daf73f88d
#define UUID_088ea7d6_2dc0_46a5_ae7e_bd2daf73f88d

#include "instruction_file.h"

namespace mesytec
{
namespace mvp
{

class Flash;

void run_instructions(const InstructionList &instructions, Flash *m_flash, uchar subindex,
    int address_offset = 0);
void print_actions(const InstructionList &instructions);

} // ns mvp
} // ns mesytec

#endif
