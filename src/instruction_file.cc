#include "instruction_file.h"

/*
 * Types:
 * @ Address
 * Address (flash.h)
 * > Text
 * QString
 * % Hex
 * QVector<uchar>
 */

namespace
{
using namespace mesytec::mvp;

Address parse_address(int line_number, QString line)
{
  if (!line.startsWith('@')) {
    throw InstructionFileParseError(line_number, line,
        "Expected an address line starting with '@'");
  }

  bool ok = false;
  uint32_t addr = line.remove(0, 1).toUInt(&ok);

  if (!ok) {
    throw InstructionFileParseError(line_number, line,
        "Error parsing address value");
  }

  return Address(addr);
}

typedef std::pair<Instruction::Type, Instruction::data_type> TypeDataPair;

TypeDataPair parse_data(int line_number, QString line)
{
  TypeDataPair ret;

  if (line.startsWith('>')) {
    ret.first = Instruction::Type::text;

    std::transform(std::begin(line), std::end(line), std::back_inserter(ret.second),
        [](QChar qc) { return qc.toLatin1(); });

  } else if (line.startsWith('%')) {
    ret.first = Instruction::Type::binary;

    /* Skip '%' char, take 2 chars and parse them as a hex number. */
    for (int i=1; i<line.size(); i+=2) {

      auto substr = line.mid(i, 2);

      if (substr.size() != 2) {
        throw InstructionFileParseError(line_number, line,
            "Invalid hex value length");
      }

      bool ok = false;
      uint32_t val = substr.toUInt(&ok, 16);

      if (!ok) {
        throw InstructionFileParseError(line_number, line,
            "Error parsing hex value");
      }

      ret.second.push_back(static_cast<uchar>(val));
    }
  } else {
    throw InstructionFileParseError(line_number, line,
        "Expected a data line starting with either '>' or '%'");
  }

  return ret;
}

} // anon ns

namespace mesytec
{
namespace mvp
{

QVector<Instruction> parse_instruction_file(QTextStream &stream)
{
  enum Expectation { exp_address, exp_data };

  QVector<Instruction> ret;
  Expectation expectation = exp_address;
  Instruction instruction;
  QString line;
  int line_number = 0;

  do {
    line = stream.readLine();

    if (line.isNull())
      break;

    ++line_number;

    switch (expectation) {
      case exp_address:
        instruction.address = parse_address(line_number, line);
        expectation = exp_data;
        break;

      case exp_data:
        {
          auto p = parse_data(line_number, line);
          instruction.type = p.first;
          instruction.data = p.second;
          expectation = exp_address;
        }
        break;
    }
  } while (!line.isNull());

  return ret;
}

} // ns mvp
} // ns mesytec
