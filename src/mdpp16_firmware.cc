#include "mdpp16_firmware.h"
#include "flash.h"
#include <QRegularExpression>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

namespace mvp
{

bool MDPP16Firmware::has_section(uchar section) const
{
  if (!is_valid_section(section))
    throw std::runtime_error("MDPP16Firmware::has_section(): invalid section given");

  return m_section_map.contains(section);
}

QVector<uchar> MDPP16Firmware::get_section(uchar section) const
{
  if (!has_section(section))
    throw std::runtime_error("MDPP16Firmware::get_section(): section not set");

  return m_section_map.value(section);
}

void MDPP16Firmware::set_section(uchar section, const QVector<uchar> &data)
{
  if (has_section(section))
    throw std::runtime_error("MDPP16Firmware::set_section(): section already set");

  if (static_cast<size_t>(data.size()) > get_section_max_size(section))
    throw std::runtime_error("MDPP16Firmware::set_section(): section max size exceeded");

  m_section_map.insert(section, data);
}

static const QString filename_pattern = "^(\\d+).*\\.bin$";

MDPP16Firmware from_dir(const QDir &dir)
{
  MDPP16Firmware ret;

  QRegularExpression re(filename_pattern);

  for (auto fi: dir.entryInfoList()) {
    auto match = re.match(fi.fileName());
    if (!match.hasMatch())
      continue;

    qDebug() << "reading" << fi.filePath();

    const auto section = static_cast<uchar>(match.captured(1).toUInt());

    QFile file(fi.filePath());

    if (!file.open(QIODevice::ReadOnly))
      throw std::runtime_error("Error opening file for reading");

    QVector<uchar> data;

    for (auto c: file.readAll())
      data.push_back(static_cast<uchar>(c));

    ret.set_section(section, data);
  }

  return ret;
}

std::runtime_error make_zip_error(const QString &msg, const QuaZip &zip)
{
  auto m = QString("zip: %1 (error=%2)")
    .arg(msg)
    .arg(zip.getZipError());

  return std::runtime_error(m.toStdString());
}

MDPP16Firmware from_zip(const QString &zip_filename)
{
  QuaZip zip(zip_filename);

  if (!zip.open(QuaZip::mdUnzip))
    throw make_zip_error("open", zip);

  MDPP16Firmware ret;

  QRegularExpression re(filename_pattern);

  for (bool more=zip.goToFirstFile(); more; more=zip.goToNextFile()) {

    if (zip.getZipError() != UNZ_OK)
      throw make_zip_error("loop through files", zip);

    auto match = re.match(zip.getCurrentFileName());

    if (!match.hasMatch())
      continue;

    const auto section = static_cast<uchar>(match.captured(1).toUInt());

    QuaZipFile file(&zip);

    if (!file.open(QIODevice::ReadOnly))
      throw make_zip_error("open current file", zip);

    QVector<uchar> data;

    for (auto c: file.readAll())
      data.push_back(static_cast<uchar>(c));

    ret.set_section(section, data);
  }

  return ret;
}

} // ns mvp
