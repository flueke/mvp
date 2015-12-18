#include "tests.h"
#include "mdpp16_firmware.h"
#include "flash.h"

using namespace mvp;

void TestMDPP16Firmware::test_basics()
{
  MDPP16Firmware fw;

  for (auto sec: {4,5,6,7,13}) {
    QVERIFY_EXCEPTION_THROWN(fw.has_section(sec), std::runtime_error);
    QVERIFY_EXCEPTION_THROWN(fw.set_section(sec, {1,2,3,4}), std::runtime_error);
    QVERIFY_EXCEPTION_THROWN(fw.get_section(sec), std::runtime_error);
  }

  for (auto sec: constants::section_max_sizes.keys()) {
    QVERIFY(!fw.has_section(sec));

    const auto sz = get_section_max_size(sec);

    QVERIFY_EXCEPTION_THROWN(fw.set_section(sec, QVector<uchar>(sz+1)),
                             std::runtime_error);

    QVERIFY(!fw.has_section(sec));

    fw.set_section(sec, QVector<uchar>(sz));

    QVERIFY(fw.has_section(sec));
    QVERIFY(static_cast<size_t>(fw.get_section(sec).size()) == sz);

    QVERIFY_EXCEPTION_THROWN(fw.set_section(sec, QVector<uchar>(sz)),
                             std::runtime_error);
  }
}

void TestMDPP16Firmware::test_print_section_sizes()
{
  QTextStream out(stdout);

  out << "Section max sizes" << endl;
  out << qSetFieldWidth(8);
  out << "Section" << "Sectors" << "Bytes" << endl;

  for (auto sec: constants::section_max_sizes.keys()) {
    const auto bytes = get_section_max_size(sec);
    const auto sectors = bytes / constants::sector_size;

    out << sec << sectors << bytes << endl;
  }
}

QVector<uchar> bytearray_to_uchar_vec(const QByteArray &data)
{
  QVector<uchar> ret;
  ret.reserve(data.size());
  for (auto c: data)
    ret.push_back(static_cast<uchar>(c));
  return ret;
}

class FirmwareContentsFileTestImpl: public FirmwareContentsFile
{
  public:
    FirmwareContentsFileTestImpl(const QString &filename = QString(),
                                 const QByteArray &contents = QByteArray())
      : m_filename(filename)
      , m_contents(bytearray_to_uchar_vec(contents))
    {}

    QString get_filename() const
    { return m_filename; }

    QVector<uchar> read_file_contents()
    { return m_contents; }

    void set(const QString &filename, const QByteArray &contents)
    {
      m_filename = filename;
      m_contents = bytearray_to_uchar_vec(contents);
    }

  private:
    QString m_filename;
    QVector<uchar> m_contents;
};

class FirmwareContentsFileGeneratorTestImpl
{
  public:
    FirmwareContentsFileGeneratorTestImpl(const QMap<QString, QByteArray> &data = {})
      : m_data(data)
      , m_iter(m_data.begin())
    {}

    FirmwareContentsFile *operator()()
    {
      if (m_iter == m_data.end())
        return nullptr;

      m_fw_file.set(m_iter.key(), m_iter.value());
      ++m_iter;
      return &m_fw_file;
    }

    void set_data(const QMap<QString, QByteArray> &data)
    {
      m_data = data;
      m_iter = m_data.begin();
    }

  private:
    QMap<QString, QByteArray> m_data;
    QMap<QString, QByteArray>::const_iterator m_iter;
    FirmwareContentsFileTestImpl m_fw_file;
};

void TestMDPP16Firmware::test_from_firmware_file_generator_simple()
{
  QMap<QString, QByteArray> data = {
    { "00-otp.bin", { "Lot's of content here" } },
  };

  FirmwareContentsFileGenerator gen =
      FirmwareContentsFileGeneratorTestImpl(data);

  auto fw = from_firmware_file_generator(gen);

  QVERIFY(fw.has_section(0));
  for (auto sec: {1, 2, 3, 8, 9, 10, 11, 12}) {
    QVERIFY(!fw.has_section(sec));
  }

  QCOMPARE(fw.get_section(0), bytearray_to_uchar_vec(data["00-otp.bin"]));
}

void TestMDPP16Firmware::test_from_firmware_file_generator_empty()
{
  QMap<QString, QByteArray> data = {};

  FirmwareContentsFileGenerator gen =
      FirmwareContentsFileGeneratorTestImpl(data);

  QVERIFY_EXCEPTION_THROWN(
        from_firmware_file_generator(gen),
        std::runtime_error);
}

void TestMDPP16Firmware::test_from_firmware_file_generator_duplicate_section()
{
  QMap<QString, QByteArray> data = {
    { "01-first.bin",   { "01-Lot's of content here" } },
    { "02-second.bin",  { "02-Lot's of content here" } },
    { "1-again",    { "1-Lot's of content here" } },
  };

  FirmwareContentsFileGenerator gen =
      FirmwareContentsFileGeneratorTestImpl(data);

  QVERIFY_EXCEPTION_THROWN(
        from_firmware_file_generator(gen),
        std::runtime_error);
}


void TestMDPP16Firmware::test_from_firmware_file_generator_section_size()
{
  // exactly max size
  {
    QMap<QString, QByteArray> data = {
      { "09sec9",   QByteArray(get_section_max_size(9), 0xff) }
    };

    FirmwareContentsFileGenerator gen =
        FirmwareContentsFileGeneratorTestImpl(data);

    auto fw = from_firmware_file_generator(gen);

    QCOMPARE(static_cast<size_t>(fw.get_section(9).size()),
             get_section_max_size(9));

    QCOMPARE(fw.get_section(9),
             QVector<uchar>(get_section_max_size(9), 0xff));
  }

  // > max size
  {
    QMap<QString, QByteArray> data = {
      { "09sec9.foobar",   QByteArray(get_section_max_size(9)+1, 0xff) }
    };

    FirmwareContentsFileGenerator gen =
        FirmwareContentsFileGeneratorTestImpl(data);

    QVERIFY_EXCEPTION_THROWN(
          from_firmware_file_generator(gen),
          std::runtime_error);
  }
}
