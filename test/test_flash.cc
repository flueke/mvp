#include "tests.h"
#include "flash.h"

using namespace mesytec::mvp;

template<typename T>
void test_constructors()
{
  {
    T a;
    QVERIFY(a.a0() == 0);
    QVERIFY(a.a1() == 0);
    QVERIFY(a.a2() == 0);

    QVERIFY(a[0] == 0);
    QVERIFY(a[1] == 0);
    QVERIFY(a[2] == 0);
  }

  {
    T a(42, 98, 254);
    QVERIFY(a.a0() == 42);
    QVERIFY(a.a1() == 98);
    QVERIFY(a.a2() == 254);

    QVERIFY(a[0] == 42);
    QVERIFY(a[1] == 98);
    QVERIFY(a[2] == 254);
  }

  {
    T a(42, 98, 254);
    T b(a);
    QVERIFY(a.a0() == b.a0()); 
    QVERIFY(a.a1() == b.a1());
    QVERIFY(a.a2() == b.a2());

    QVERIFY(a[0] == b[0]);
    QVERIFY(a[1] == b[1]);
    QVERIFY(a[2] == b[2]);

    QVERIFY(a == b);
    QVERIFY(!(a != b));
  }
}

template<typename T>
void test_increment()
{
  {
    T a;
    QVERIFY((a++)[0] == 0);
    QVERIFY(a[0] == 1);
    QVERIFY(a == T(1));
  }

  {
    T a(0xff, 0, 0);
    QVERIFY(a == T(0x0000ff));
    ++a;
    QVERIFY(a == T(0x000100));
    QVERIFY(a[0] == 0);
    QVERIFY(a[1] == 1);
    QVERIFY(a[2] == 0);
  }

  {
    T a(0xff, 0xff, 0);
    QVERIFY(a == T(0x00ffff));
    ++a;
    QVERIFY(a == T(0x010000));
    QVERIFY(a[0] == 0);
    QVERIFY(a[1] == 0);
    QVERIFY(a[2] == 1);
  }

  {
    T a(0xfffffe);
  }
}

void TestFlash::test_address()
{
  test_constructors<Address>();

  test_increment<Address>();
}

void TestFlash::test_key_from_flash_memory()
{
  // valid
  {
    std::string str_data = "MDPP16  \x11\x12\x13\x14\x15\x16  \x23\x24\x25\x26";
    std::vector<uchar> data;

    std::transform(std::begin(str_data), std::end(str_data), std::back_inserter(data),
        [](char c) { return static_cast<uchar>(c); });

    Key key = Key::from_flash_memory(gsl::as_span(data));

    QCOMPARE(key.get_prefix(), QString("MDPP16  "));
    QCOMPARE(key.get_sn(),  0x11121314u);
    QCOMPARE(key.get_sw(),  static_cast<uint16_t>(0x1516u));
    QCOMPARE(key.get_key(), 0x23242526u);
  }

  // more data but valid
  {
    std::string str_data = "MDPP16  \x11\x12\x13\x14\x15\x16  \x23\x24\x25\x26\x42\x42";
    std::vector<uchar> data;

    std::transform(std::begin(str_data), std::end(str_data), std::back_inserter(data),
        [](char c) { return static_cast<uchar>(c); });

    Key key = Key::from_flash_memory(gsl::as_span(data));

    QCOMPARE(key.get_prefix(), QString("MDPP16  "));
    QCOMPARE(key.get_sn(),  0x11121314u);
    QCOMPARE(key.get_sw(),  static_cast<uint16_t>(0x1516u));
    QCOMPARE(key.get_key(), 0x23242526u);

    qDebug() << "key:" << key.to_string();
  }

  // not enough data
  {
    std::string str_data = "MDPP16  \x11\x12\x13\x14\x15\x16  \x23\x24\x25";
    std::vector<uchar> data;

    std::transform(std::begin(str_data), std::end(str_data), std::back_inserter(data),
        [](char c) { return static_cast<uchar>(c); });

    QVERIFY_EXCEPTION_THROWN(
      Key::from_flash_memory(gsl::as_span(data)),
      KeyError);
  }
}

void TestFlash::test_key_constructor()
{
  {
    Key k("ABCDEFGH", 1u, 1u, 0xffffffffu);

    QCOMPARE(k.get_prefix(), QString("ABCDEFGH"));
    QCOMPARE(k.get_sn(), 1u);
    QCOMPARE(k.get_sw(), static_cast<uint16_t>(1u));
    QCOMPARE(k.get_key(), 0xffffffffu);
  }

  {
    QVERIFY_EXCEPTION_THROWN(
        Key("ABCDEFGHI", 1u, 1u, 0xffffffffu),
        KeyError);
  }

  {
    QVERIFY_EXCEPTION_THROWN(
        Key("ABCDEFG", 1u, 1u, 0xffffffffu),
        KeyError);
  }
}

void TestFlash::test_key_to_string()
{
  {
    Key k("ABCDEFGH", 1u, 1u, 0xffffffffu);

    QCOMPARE(k.to_string(), QString("Key(sn=ABCDEFGH00000001, sw=0001, key=FFFFFFFF)"));
  }
}
