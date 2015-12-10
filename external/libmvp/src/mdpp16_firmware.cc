#include "mdpp16_firmware.h"
#include "flash.h"
#include <QRegularExpression>
#include <quazip/quazip.h>

namespace mvp
{

bool MDPP16Firmware::has_section(uchar section) const
{
  return m_section_map.contains(section);
}

QVector<uchar> MDPP16Firmware::get_section(uchar section) const
{
  if (!is_valid_section(section))
    throw std::runtime_error("MDPP16Firmware::get_section(): invalid section given");

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

MDPP16Firmware from_zip(const QString &zip_filename)
{
  QuaZip zip(zip_filename);
  zip.open(QuaZip::mdUnzip);


}

} // ns mvp
