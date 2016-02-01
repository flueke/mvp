#include "tests.h"
#include "instruction_interpreter.h"

using namespace mesytec::mvp;

void TestInstructionInterpreter::test_print()
{
  InstructionList ilist = {
    Instruction(Instruction::Type::text, {0, 0, 0},
        { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!' })
  };

  print_actions(ilist);
}
