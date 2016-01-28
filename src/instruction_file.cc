#include "instruction_file.h"

/*
 * Types:
 * @ Address
 * Address (flash.h)
 * > Text
 * QString
 * % Binary given in hex
 * QVector<uchar>
 */

namespace
{
using namespace mesytec::mvp;

Address parse_address(int line_number, QString line)
{
  const auto orig_line(line);

  if (!line.startsWith('@')) {
    throw InstructionFileParseError(line_number, orig_line,
        "Expected an address line starting with '@'");
  }

  bool ok = false;
  uint32_t addr = line.remove(0, 1).toUInt(&ok, 0);

  if (!ok) {
    throw InstructionFileParseError(line_number, orig_line,
        "Error parsing address value");
  }

  return Address(addr);
}

typedef std::pair<Instruction::Type, Instruction::data_type> TypeDataPair;

TypeDataPair parse_data(int line_number, QString line)
{
  const auto orig_line(line);
  TypeDataPair ret;

  if (line.startsWith('>')) {

    if (line.midRef(1).trimmed().isEmpty())
      throw InstructionFileParseError(line_number, orig_line,
          "Empty text data");

    ret.first = Instruction::Type::text;

    std::transform(std::begin(line)+1, std::end(line), std::back_inserter(ret.second),
        [](QChar qc) { return qc.toLatin1(); });

    ret.second.push_back('\0');

  } else if (line.startsWith('%')) {
    ret.first = Instruction::Type::binary;

    line = line.remove(0, 1).trimmed();

    if (line.isEmpty())
      throw InstructionFileParseError(line_number, orig_line,
          "Empty hex data");

    /* Skip '%' char, take 2 chars and parse them as a hex number. */
    for (int i=0; i<line.size(); i+=2) {

      auto substr = line.mid(i, 2);

      if (substr.size() != 2) {
        throw InstructionFileParseError(line_number, orig_line,
            "Invalid hex value length (expected length % 2 == 0)");
      }

      bool ok = false;
      uint32_t val = substr.toUInt(&ok, 16);

      if (!ok) {
        throw InstructionFileParseError(line_number, orig_line,
            "Error parsing hex value");
      }

      ret.second.push_back(static_cast<uchar>(val));
    }
  } else {
    throw InstructionFileParseError(line_number, orig_line,
        "Expected a data line starting with either '>' or '%'");
  }

  return ret;
}

} // anon ns

namespace mesytec
{
namespace mvp
{

QString Instruction::to_string() const
{
  if (type != Type::text)
    throw std::runtime_error("Can not convert non-string type instruction to string");

  return QString::fromLatin1(reinterpret_cast<const char *>(data.data()));
}

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

    if (line.isEmpty() || line.trimmed().startsWith('#'))
      continue;

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
          ret.push_back(instruction);
          expectation = exp_address;
        }
        break;
    }
  } while (!line.isNull());

  if (expectation == exp_data)
    throw InstructionFileParseError(line_number, line,
        "Expected instruction data, got EOF");

  if (ret.isEmpty())
    throw InstructionFileParseError(0, QString(), "Empty instruction file");

  return ret;
}

} // ns mvp
} // ns mesytec
