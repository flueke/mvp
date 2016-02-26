#include "tests.h"
#include "instruction_file.h"

using namespace mesytec::mvp;

void TestInstructionFile::test_valid()
{
  QString contents = R"(@0x00
>SCP Firmware for MDPP-16

@0x100
>MDPP16
  # This is a comment
@0x108

%50150001
@0x110
%0023  
@   0x118  
% 31456721
@0x120
%3F

# The end
)";

  QTextStream stream(&contents, QIODevice::ReadOnly);

  try {
    auto result = parse_instruction_file(stream);

    QCOMPARE(result.size(), 6);

    QCOMPARE(result[0].type, Instruction::Type::text);
    QCOMPARE(result[0].address.to_int(), 0x0u);
    QCOMPARE(result[0].to_string(), QString("SCP Firmware for MDPP-16"));

    QCOMPARE(result[1].type, Instruction::Type::text);
    QCOMPARE(result[1].address.to_int(), 0x100u);
    QCOMPARE(result[1].to_string(), QString("MDPP16"));

    QCOMPARE(result[2].type, Instruction::Type::binary);
    QCOMPARE(result[2].address.to_int(), 0x108u);
    QCOMPARE(result[2].data, (QVector<uchar>{ 0x50, 0x15, 0x00, 0x01 }));

    QCOMPARE(result[3].type, Instruction::Type::binary);
    QCOMPARE(result[3].address.to_int(), 0x110u);
    QCOMPARE(result[3].data, (QVector<uchar>{ 0x00, 0x23 }));

    QCOMPARE(result[4].type, Instruction::Type::binary);
    QCOMPARE(result[4].address.to_int(), 0x118u);
    QCOMPARE(result[4].data, (QVector<uchar>{ 0x31, 0x45, 0x67, 0x21 }));

    QCOMPARE(result[5].type, Instruction::Type::binary);
    QCOMPARE(result[5].address.to_int(), 0x120u);
    QCOMPARE(result[5].data, (QVector<uchar>{ 0x3f }));
  } catch (const InstructionFileParseError &e) {
    qDebug() << e.to_string();
    throw;
  }
}

void TestInstructionFile::test_invalid_binary()
{
  {
    // invalid character
    QString contents =
R"(@0x108
%50x150001
)";

    QTextStream stream(&contents, QIODevice::ReadOnly);
    QVERIFY_EXCEPTION_THROWN(
        parse_instruction_file(stream),
        InstructionFileParseError);
  }

  {
    // length%2 != 0
    QString contents =
R"(@0x108
%5015000
)";

    QTextStream stream(&contents, QIODevice::ReadOnly);
    QVERIFY_EXCEPTION_THROWN(
        parse_instruction_file(stream),
        InstructionFileParseError);
  }

  {
    // wsp only
    QString contents =
R"(@0x108
%   
)";

    QTextStream stream(&contents, QIODevice::ReadOnly);
    QVERIFY_EXCEPTION_THROWN(
        parse_instruction_file(stream),
        InstructionFileParseError);
  }
}

void TestInstructionFile::test_invalid_structure()
{
  {
    // empty file
    QString contents =
R"(
# Just a comment
)";

    QTextStream stream(&contents, QIODevice::ReadOnly);
    QVERIFY_EXCEPTION_THROWN(
        parse_instruction_file(stream),
        InstructionFileParseError);
  }

  {
    // missing data at end of file
    QString contents =
R"(
@0x100
>MDPP16
  # This is a comment
@0x108
)";

    QTextStream stream(&contents, QIODevice::ReadOnly);
    QVERIFY_EXCEPTION_THROWN(
        parse_instruction_file(stream),
        InstructionFileParseError);
  }
}
