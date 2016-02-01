#include "firmware.h"
#include "flash.h"
#include <QRegularExpression>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

namespace
{

std::runtime_error make_zip_error(const QString &msg, const QuaZip &zip)
{
  auto m = QString("archive: %1 (error=%2)")
    .arg(msg)
    .arg(zip.getZipError());

  return std::runtime_error(m.toStdString());
}

boost::optional<uchar> convert_to_uchar(const QString &str)
{
  bool ok = false;
  auto n  = str.toUInt(&ok, 10);

  if (!ok)
    return boost::none;

  return static_cast<uchar>(n);
}

} // anon ns

namespace mesytec
{
namespace mvp
{

#if 0
bool Firmware::has_section(uchar section) const
{
  if (!is_valid_section(section))
    throw std::runtime_error("Firmware::has_section(): invalid section given");

  return m_section_map.contains(section);
}

QVector<uchar> Firmware::get_section(uchar section) const
{
  if (!has_section(section))
    throw std::runtime_error("Firmware::get_section(): section not set");

  return m_section_map.value(section);
}

void Firmware::set_section(uchar section, const QVector<uchar> &data)
{
  if (has_section(section))
    throw std::runtime_error("Firmware::set_section(): section already set");

  if (static_cast<size_t>(data.size()) > get_section_max_size(section))
    throw std::runtime_error("Firmware::set_section(): section max size exceeded");

  m_section_map.insert(section, data);
}

QList<uchar> Firmware::get_present_section_numbers() const
{
  return m_section_map.keys();
}

bool Firmware::has_required_sections() const
{
  return get_present_section_numbers().size() > 0;
}

#endif

InstructionList InstructionFirmwarePart::get_instructions() const
{
  auto contents = get_contents();

  auto data = QByteArray::fromRawData(
      reinterpret_cast<const char *>(contents.constData()),
      contents.size());

  QTextStream stream(data);
  return parse_instruction_file(stream);
}

class DirFirmwareFile: public FirmwareContentsFile
{
  public:
    DirFirmwareFile(const QFileInfo &info = QFileInfo())
      : m_fi(info)
    {}

    QString get_filename() const override
    { return m_fi.fileName();}

    QVector<uchar> get_file_contents() override
    {
      QFile file(m_fi.filePath());

      if (!file.open(QIODevice::ReadOnly))
        throw std::runtime_error("Error opening file for reading");

      QVector<uchar> data;

      for (auto c: file.readAll())
        data.push_back(static_cast<uchar>(c));

      return data;
    }

  private:
    QFileInfo m_fi;
};

class ZipFirmwareFile: public FirmwareContentsFile
{
  public:
    ZipFirmwareFile(QuaZip *zip)
      : m_zip(zip)
    {}

    QString get_filename() const override
    { return m_zip->getCurrentFileName(); }

    QVector<uchar> get_file_contents() override
    {
      QuaZipFile file(m_zip);

      if (!file.open(QIODevice::ReadOnly))
        throw make_zip_error("open current file", *m_zip);

      QVector<uchar> data;

      for (auto c: file.readAll())
        data.push_back(static_cast<uchar>(c));

      return data;
    }

  private:
    QuaZip *m_zip;
};

static const QVector<QRegularExpression> filename_regexps = {
  QRegularExpression(R"(^(?<section>\d+).+(?<area>\d+).+\.(?<extension>bin|hex)$)"),
  QRegularExpression(R"(^(?<section>\d+).+\.(?<extension>bin|hex)$)"),
  QRegularExpression(R"(^.+\.(?<extension>key)$)")
};

#if 0
static const QString pat_section_area = QStringLiteral(
    R"(^(?<section>\d+).+(?<area>\d+)\w+\.(?<extension>bin|hex)$)");

static const QString pat_section = QStringLiteral(
    R"(^(?<section>\d+).*\w+\.(?<extension>bin|hex)$)");
#endif

FirmwareArchive from_firmware_file_generator(FirmwareContentsFileGenerator &gen,
    const QString &archive_filename)
{
  FirmwareArchive ret(archive_filename);

  while (auto fw_file = gen()) {
    const auto fn = fw_file->get_filename();

    QRegularExpressionMatch match;

    for (auto re: filename_regexps) {
      match = re.match(fn);

      if (match.hasMatch()) {
        qDebug() << "re match found: fn =" << fn << ", re =" << re;
        break;
      }
    }

    if (!match.hasMatch()) {
      qDebug() << "no regexp match for fn =" << fn << ", skipping";
      continue;
    }

    const auto s_section  = match.captured("section");
    const auto s_area     = match.captured("area");
    const auto ext        = match.captured("extension");

    qDebug()
      << "fn =" << fn << ":"
      << "section =" << s_section
      << ", area =" << s_area
      << ", ext =" << ext;

    FirmwarePartPtr part;

    if (ext == "key") {
      part = std::make_shared<KeyFirmwarePart>(fn);
    } else if (ext == "bin") {
      part = std::make_shared<BinaryFirmwarePart>(fn);
    } else if (ext == "hex") {
      part = std::make_shared<InstructionFirmwarePart>(fn);
    } else {
      qDebug() << "skipping unknown file type, fn =" << fn;
    }

    if (part) {
      part->set_contents(fw_file->get_file_contents());
      part->set_section(convert_to_uchar(s_section));
      part->set_area(convert_to_uchar(s_area));
      ret.add_part(part);
    }
  }

  return ret;
#if 0
  Firmware ret;
  QRegularExpression re(section_filename_pattern);

  while (auto fw_file = gen()) {
    auto match = re.match(fw_file->get_filename());

    if (!match.hasMatch())
      continue;

    const auto section = static_cast<uchar>(match.captured(1).toUInt());
    ret.set_section(section, fw_file->get_file_contents());
  }

  if (ret.is_empty())
    throw std::runtime_error("No section contents found in firmware");

  return ret;
#endif
}

class ZipFirmwareFileGenerator
{
  public:
    ZipFirmwareFileGenerator(QuaZip *zip)
      : m_zip(zip)
      , m_zip_fw_file(zip)
    {}

    FirmwareContentsFile* operator()()
    {
      if (m_first_file) {
        m_first_file = false;

        if (!m_zip->goToFirstFile()
          || m_zip->getZipError() != UNZ_OK) {
          throw make_zip_error("goToFirstFile", *m_zip);
        }

        return &m_zip_fw_file;
      }

      if (!m_zip->goToNextFile()) {
        if (m_zip->getZipError() != UNZ_OK) {
          throw make_zip_error("goToNextFile", *m_zip);
        }
        return nullptr;
      }

      return &m_zip_fw_file;
    }

  private:
    QuaZip *m_zip;
    ZipFirmwareFile m_zip_fw_file;
    bool m_first_file = true;
};

class DirFirmwareFileGenerator
{
  public:
    DirFirmwareFileGenerator(const QDir &dir)
      : m_fileinfo_list(dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
      , m_iter(m_fileinfo_list.begin())
    {
      qDebug() << "DirFirmwareFileGenerator:"
               << "info list size =" << m_fileinfo_list.size()
               << "dir.filePath() =" << dir.absolutePath();
    }

    FirmwareContentsFile *operator()()
    {
      qDebug() << "DirFirmwareFileGenerator::operator()";

      if (m_iter == m_fileinfo_list.end())
        return nullptr;

      m_dir_fw_file = std::make_shared<DirFirmwareFile>(*m_iter++);

      qDebug() << "DirFirmwareFileGenerator::operator(): returning DirFirmwareFile"
               << m_dir_fw_file->get_filename();

      return m_dir_fw_file.get();
    }

  private:
    QFileInfoList m_fileinfo_list;
    QFileInfoList::iterator m_iter;
    std::shared_ptr<DirFirmwareFile> m_dir_fw_file;
};

FirmwareArchive from_zip(const QString &zip_filename)
{
  QuaZip zip(zip_filename);

  if (!zip.open(QuaZip::mdUnzip))
    throw make_zip_error("open", zip);

  FirmwareContentsFileGenerator gen = ZipFirmwareFileGenerator(&zip);

  return from_firmware_file_generator(gen, zip_filename);
}

FirmwareArchive from_dir(const QDir &dir)
{
  FirmwareContentsFileGenerator gen = DirFirmwareFileGenerator(dir);
  return from_firmware_file_generator(gen, dir.path());
}

} // ns mvp
} // ns mesytec
