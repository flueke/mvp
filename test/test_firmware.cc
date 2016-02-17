#include "tests.h"
#include "firmware.h"
#include "flash.h"

using namespace mesytec::mvp;

namespace
{
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
} // anon ns

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
    { "00_otp.bin", { "Lot's of content here" } },
  };

  FirmwareContentsFileGenerator gen =
      FirmwareContentsFileGeneratorTestImpl(data);

  auto fw = from_firmware_file_generator(gen, "the_filename.mvp");

  QCOMPARE(fw.get_filename(), QString("the_filename.mvp"));
  QCOMPARE(fw.get_parts().size(), 1);

  auto part = fw.get_parts()[0];

  QCOMPARE(part->get_filename(), QString("00_otp.bin"));

  QVERIFY(part->has_section());
  QCOMPARE(*part->get_section(), static_cast<uchar>(0u));

  QVERIFY(!part->has_area());
  QCOMPARE(part->get_contents(), bytearray_to_uchar_vec(data["00_otp.bin"]));
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
    { "12_03_otp.bin",    { "Lot's of content here" } },
    { "12_03-aaa.bin",    { "Lot's of content here" } },
    { "12_3_aaa.hex",     { "Lot's of content here" } },
    { "12_3_a.hex.hex",   { "Lot's of content here" } },
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
    { "012_firmware_stream.bin" , { "The binary salad is tasty!" } },
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

void TestFirmware::test_filename_patterns3()
{
  QMap<QString, QByteArray> data = {
    { "12_0_MDPP16_prototype_FW01.bin", { "The binary salad is tasty!" } },
    { "12_1_MDPP16_prototype_FW01.bin", { "The binary salad is tasty!" } },
    { "1_hardware_descr.hex",           { "Somethings happening here" } },
    { "8_0_area_descr.hex",             { "Somethings happening here" } },
    { "8_1_area_descr.hex",             { "Somethings happening here" } },
    { "mdpp16_sn1337_sw0023.key",       { "Somethings happening here" } },
    { "12_mdpp16_scp_fw0005.bin",       { "This resulted in area=16!" } },
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
  QCOMPARE(parts.size(), 5);

  parts = fw.get_non_area_specific_parts();
  qDebug() << "non area specific parts:";
  output_parts(parts);
  QCOMPARE(parts.size(), 1);

  parts = fw.get_key_parts();
  qDebug() << "key parts:";
  output_parts(parts);
  QCOMPARE(parts.size(), 1);
}

void TestFirmware::test_empty_bin_part()
{
  QMap<QString, QByteArray> data = {
    { "12_0_MDPP16_prototype_FW01.bin", { } },
  };

  FirmwareContentsFileGenerator gen =
      FirmwareContentsFileGeneratorTestImpl(data);

  auto fw = from_firmware_file_generator(gen, "the_filename.mvp");

  QCOMPARE(fw.size(), data.size());

  auto part = fw.get_part(0);

  QCOMPARE(part->get_filename(), QString("12_0_MDPP16_prototype_FW01.bin"));
  QCOMPARE(*part->get_section(), static_cast<uchar>(12u));
  QCOMPARE(*part->get_area(), static_cast<uchar>(0u));
  QVERIFY(part->get_contents().isEmpty());
}
