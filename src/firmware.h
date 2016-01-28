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

    QString get_filename() const;
    void set_filename(const QString &filename);

    boost::optional<uchar> get_area() const;
    void set_area(const boost::optional<uchar> &area);
    bool has_area() const;

    boost::optional<uchar> get_section() const;
    void set_section(const boost::optional<uchar> &section);
    bool has_section() const;

    ContentsType get_contents();
    void set_contents(const ContentsType &contents);

  protected:
    FirmwarePart(const QString &filename,
        const boost::optional<uchar> &area = boost::none,
        const boost::optional<uchar> &section = boost::none,
        const ContentsType &contents = ContentsType());

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

    /* These methods extract information from the filename. The same
     * information should be inside the files instruction list aswell.
     * TODO: Need something to check the consistency of filename and
     * instruction list. */
#if 0
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
    FirmwareArchive(const QString &filename);

    QString get_filename() const;
    FirmwarePartList get_parts() const;
    void add_part(const FirmwarePartPtr &part);

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
