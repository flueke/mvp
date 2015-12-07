#include "tests.h"
#include "flash.h"

using namespace mvp;

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
