#include "tests.h"
#include "util.h"
#include <QtConcurrent>

using mvp::QtExceptionPtr;

void TestQtExceptionPtr::test()
{
  auto f = QtConcurrent::run([&]() {
      try {
        throw std::runtime_error("fourtytwo!");
      } catch (...) {
        throw QtExceptionPtr(std::current_exception());
      }
    });

  try {
    f.waitForFinished();
    QVERIFY(false);
  } catch (const std::exception &e) {
    QCOMPARE("fourtytwo!", e.what());
  }
}
