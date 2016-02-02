#include "tests.h"
#include "firmware.h"
#include "flash.h"

using namespace mesytec::mvp;

void TestFirmware::test_print_section_sizes()
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

    QVector<uchar> get_file_contents()
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

void TestFirmware::test_from_firmware_file_generator_simple()
{
  QMap<QString, QByteArray> data = {
    { "00-otp.bin", { "Lot's of content here" } },
  };

  FirmwareContentsFileGenerator gen =
      FirmwareContentsFileGeneratorTestImpl(data);

  auto fw = from_firmware_file_generator(gen, "the_filename.mvp");

  QCOMPARE(fw.get_filename(), QString("the_filename.mvp"));
  QCOMPARE(fw.get_parts().size(), 1);

  auto part = fw.get_parts()[0];

  QCOMPARE(part->get_filename(), QString("00-otp.bin"));

  QVERIFY(part->has_section());
  QCOMPARE(*part->get_section(), static_cast<uchar>(0u));

  QVERIFY(!part->has_area());
  QCOMPARE(part->get_contents(), bytearray_to_uchar_vec(data["00-otp.bin"]));
}

void TestFirmware::test_from_firmware_file_generator_empty()
{
  QMap<QString, QByteArray> data = {};

  FirmwareContentsFileGenerator gen =
      FirmwareContentsFileGeneratorTestImpl(data);

  QCOMPARE(from_firmware_file_generator(gen).size(), 0);
}

void TestFirmware::test_filename_patterns()
{
  QMap<QString, QByteArray> data = {
    { "12-03_otp.bin",    { "Lot's of content here" } },
    { "12-03-aaa.bin",    { "Lot's of content here" } },
    { "12_3_aaa.hex",     { "Lot's of content here" } },
    { "12-3_a.hex.hex",   { "Lot's of content here" } },
  };

  FirmwareContentsFileGenerator gen =
      FirmwareContentsFileGeneratorTestImpl(data);

  auto fw = from_firmware_file_generator(gen, "the_filename.mvp");

  QCOMPARE(fw.size(), data.size());

  for (auto part: fw.get_parts()) {
    QVERIFY(part->has_section());
    QCOMPARE(*part->get_section(), static_cast<uchar>(12u));

    QVERIFY(part->has_area());
    QCOMPARE(*part->get_area(), static_cast<uchar>(3u));
  }
}

void TestFirmware::test_filename_patterns2()
{
  QMap<QString, QByteArray> data = {
    { "012-firmware-stream.bin" , { "The binary salad is tasty!" } },
  };

  FirmwareContentsFileGenerator gen =
      FirmwareContentsFileGeneratorTestImpl(data);

  auto fw = from_firmware_file_generator(gen, "the_filename.mvp");

  QCOMPARE(fw.size(), data.size());

  for (auto part: fw.get_parts()) {
    QVERIFY(part->has_section());
    QCOMPARE(*part->get_section(), static_cast<uchar>(12u));

    QVERIFY(!part->has_area());
  }
}

void output_parts(const FirmwarePartList &parts)
{
  for (auto pp: parts) {
    qDebug() << pp->get_filename()
      << "area" << pp->has_area()
      << (pp->has_area() ? *pp->get_area() : 255u)
      << "section" << pp->has_section()
      << (pp->has_section() ? *pp->get_section() : 255u)
      ;
  }
}

void TestFirmware::test_filename_patterns3()
{
  QMap<QString, QByteArray> data = {
    { "12_0_MDPP16_prototype_FW01.bin", { "The binary salad is tasty!" } },
    { "12_1_MDPP16_prototype_FW01.bin", { "The binary salad is tasty!" } },
    { "1_hardware_descr.hex",           { "Somethings happening here" } },
    { "8_0_area_descr.hex",             { "Somethings happening here" } },
    { "8_1_area_descr.hex",             { "Somethings happening here" } },
    { "mdpp16_sn1337_sw0023.key",       { "Somethings happening here" } },
  };

  FirmwareContentsFileGenerator gen =
      FirmwareContentsFileGeneratorTestImpl(data);

  auto fw = from_firmware_file_generator(gen, "the_filename.mvp");

  QCOMPARE(fw.size(), data.size());

  auto parts = fw.get_parts();
  qDebug() << "all parts:";
  output_parts(parts);
  QCOMPARE(parts.size(), data.size());

  parts = fw.get_area_specific_parts();
  qDebug() << "area specific parts:";
  output_parts(parts);
  QCOMPARE(parts.size(), 4);

  parts = fw.get_non_area_specific_parts();
  qDebug() << "non area specific parts:";
  output_parts(parts);
  QCOMPARE(parts.size(), 1);

  parts = fw.get_key_parts();
  qDebug() << "key parts:";
  output_parts(parts);
  QCOMPARE(parts.size(), 1);
}

#if 0
void TestFirmware::test_from_firmware_file_generator_duplicate_section()
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
#endif

#if 0
void TestFirmware::test_from_firmware_file_generator_section_size()
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
#endif
