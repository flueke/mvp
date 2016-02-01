#ifndef UUID_4beceb76_c0b3_4e40_8917_8ee9a4b262ac
#define UUID_4beceb76_c0b3_4e40_8917_8ee9a4b262ac

#include <boost/optional.hpp>
#include <functional>
#include <QDir>
#include <QVector>

#include "instruction_file.h"

namespace mesytec
{
namespace mvp
{

class FirmwarePart
{
  public:
    typedef QVector<uchar> ContentsType;

    virtual ~FirmwarePart() {};

    QString get_filename() const
    { return m_filename; }

    void set_filename(const QString &filename)
    { m_filename = filename; }

    boost::optional<uchar> get_area() const
    { return m_area; }

    void set_area(const boost::optional<uchar> &area)
    { m_area = area; }

    bool has_area() const
    { return bool(m_area); }

    boost::optional<uchar> get_section() const
    { return m_section; }

    void set_section(const boost::optional<uchar> &section)
    { m_section = section; }

    bool has_section() const
    { return bool(m_section); }

    ContentsType get_contents() const
    { return m_contents; }

    void set_contents(const ContentsType &contents)
    { m_contents = contents; }

  protected:
    FirmwarePart(const QString &filename,
        const boost::optional<uchar> &area = boost::none,
        const boost::optional<uchar> &section = boost::none,
        const ContentsType &contents = ContentsType())
      : m_filename(filename)
      , m_area(area)
      , m_section(section)
      , m_contents(contents)
  {}

  private:
    QString m_filename;
    boost::optional<uchar> m_area;
    boost::optional<uchar> m_section;
    ContentsType m_contents;
};

class BinaryFirmwarePart: public FirmwarePart
{
  public:
    BinaryFirmwarePart(const QString &filename,
        const boost::optional<uchar> &area = boost::none,
        const boost::optional<uchar> &section = boost::none,
        const ContentsType &contents = ContentsType())
      : FirmwarePart(filename, area, section, contents)
    {}
};

class InstructionFirmwarePart: public FirmwarePart
{
  public:
    InstructionFirmwarePart(const QString &filename,
        const boost::optional<uchar> &area = boost::none,
        const boost::optional<uchar> &section = boost::none,
        const ContentsType &contents = ContentsType())
      : FirmwarePart(filename, area, section, contents)
    {}

    InstructionList get_instructions() const;
};

class KeyFirmwarePart: public InstructionFirmwarePart
{
  public:
    KeyFirmwarePart(const QString &filename,
        const ContentsType &contents = ContentsType())
      : InstructionFirmwarePart(filename, boost::none, boost::none, contents)
    {}

#if 0
    /* These methods extract information from the filename. The same
     * information should be inside the files instruction list aswell.
     * TODO: Need something to check the consistency of filename and
     * instruction list. */
    QString get_device_name() const;
    QVector<uchar> get_serial_number() const;
    QVector<uchar> get_software_number() const;

    // TODO: need to interpret the instruction list to be able to calculate this
    uchar get_checksum() const;
#endif
};

typedef std::shared_ptr<FirmwarePart> FirmwarePartPtr;
typedef QList<FirmwarePartPtr> FirmwarePartList;

class FirmwareArchive
{
  public:
    FirmwareArchive(const QString &filename = QString())
      : m_filename(filename)
    {}

    QString get_filename() const
    { return m_filename; }

    FirmwarePartList get_parts() const
    { return m_parts; }

    FirmwarePartPtr get_part(int idx) const
    { return m_parts.value(idx); }

    void add_part(const FirmwarePartPtr &part)
    { m_parts.push_back(part); }

    int size() const
    { return m_parts.size(); }

  private:
    QString m_filename;
    FirmwarePartList m_parts;
};

class FirmwareContentsFile
{
  public:
    virtual ~FirmwareContentsFile() {}
    virtual QString get_filename() const = 0;
    virtual QVector<uchar> get_file_contents() = 0;
};

typedef std::function<FirmwareContentsFile * (void)> FirmwareContentsFileGenerator;

FirmwareArchive from_firmware_file_generator(FirmwareContentsFileGenerator &gen,
    const QString &archive_filename = QString());
FirmwareArchive from_dir(const QDir &dir);
FirmwareArchive from_zip(const QString &zip_filename);

} // ns mvp
} // ns mesytec

#endif
