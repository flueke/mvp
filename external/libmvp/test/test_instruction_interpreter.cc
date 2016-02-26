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

void TestInstructionInterpreter::test_generate_memory()
{
  QString contents = R"(@0x00
>12345678
@0x0a
%50150001
)";

  QTextStream stream(&contents, QIODevice::ReadOnly);
  auto ilist = parse_instruction_file(stream);
  auto mem   = generate_memory(ilist);

  QCOMPARE(mem.size(), 14);

  for (int i=0; i<8; ++i)
    QCOMPARE(mem[i], static_cast<uchar>('1' + i));

  QCOMPARE(mem[0x0a], static_cast<uchar>(0x50u));
  QCOMPARE(mem[0x0b], static_cast<uchar>(0x15u));
  QCOMPARE(mem[0x0c], static_cast<uchar>(0x00u));
  QCOMPARE(mem[0x0d], static_cast<uchar>(0x01u));
}
