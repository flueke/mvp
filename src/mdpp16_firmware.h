#ifndef __MDPP16_FIRMWARE_H__
#define __MDPP16_FIRMWARE_H__

#include <QMap>
#include <QVector>
#include <QDir>

namespace mvp
{

class MDPP16Firmware
{
  public:
    bool has_section(uchar section) const;
    QVector<uchar> get_section(uchar section) const;
    void set_section(uchar section, const QVector<uchar> &data);

  private:
    QMap<uchar, QVector<uchar>> m_section_map;
};

MDPP16Firmware from_dir(const QDir &dir);
MDPP16Firmware from_zip(const QString &zip_filename);

} // ns mvp

#endif /* __MDPP16_FIRMWARE_H__ */
