#ifndef __MDPP16_FIRMWARE_H__
#define __MDPP16_FIRMWARE_H__

#include <QMap>
#include <QVector>
#include <QDir>
#include <functional>

namespace mvp
{

class MDPP16Firmware
{
  public:
    bool has_section(uchar section) const;
    QVector<uchar> get_section(uchar section) const;
    void set_section(uchar section, const QVector<uchar> &data);
    QList<uchar> get_present_section_numbers() const;
    bool has_required_sections() const;

    void clear()
    { m_section_map.clear(); }

    bool is_empty() const
    { return m_section_map.size() == 0; }

  private:
    QMap<uchar, QVector<uchar>> m_section_map;
};

class FirmwareContentsFile
{
  public:
    virtual ~FirmwareContentsFile() {}
    virtual QString get_filename() const = 0;
    virtual QVector<uchar> read_file_contents() = 0;
};

typedef std::function<FirmwareContentsFile * (void)> FirmwareContentsFileGenerator;

MDPP16Firmware from_firmware_file_generator(FirmwareContentsFileGenerator &gen);
MDPP16Firmware from_dir(const QDir &dir);
MDPP16Firmware from_zip(const QString &zip_filename);

} // ns mvp

#endif /* __MDPP16_FIRMWARE_H__ */
