#include "tests.h"

#include <memory>
#include <QtTest/QtTest>

int main(int argc, char *argv[])
{
  std::list<std::shared_ptr<QObject>> tests = {
    std::make_shared<TestQtExceptionPtr>(),
    std::make_shared<TestFlash>()
  };

  for (auto obj: tests) {
    QTest::qExec(obj.get(), argc, argv);
  }
}
