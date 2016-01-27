#include "tests.h"
#include "instruction_file.h"

using namespace mesytec::mvp;

void TestInstructionFile::test()
{
  QString contents = R"(@0x00
>SCP Firmware for MDPP-16
@0x100
MDPP16
@0x108
%50150001
@0x110
%0023
@0x118
%31456721
@0x120
%3F
)";

  QTextStream stream(&contents, QIODevice::ReadOnly);
  try {
  auto result = parse_instruction_file(stream);
  } catch (const InstructionFileParseError &e) {
    qDebug() << e.to_string();
    throw;
  }
}
