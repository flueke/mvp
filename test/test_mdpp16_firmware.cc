#include "tests.h"
#include "mdpp16_firmware.h"
#include "flash.h"

using namespace mvp;

void TestMDPP16Firmware::test()
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

void TestMDPP16Firmware::test_from_dir()
{
  /* Needs a QDir as input.
   * Test: no files, multi files, duplicate sections
   */
}

void TestMDPP16Firmware::test_from_zip()
{
}
