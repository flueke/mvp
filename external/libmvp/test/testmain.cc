#include "tests.h"
<<<<<<< HEAD
=======

>>>>>>> df6c852b433bf9b9a0c9c392154008a69a1c9a11
#include <memory>
#include <QtTest/QtTest>

int main(int argc, char *argv[])
{
  {
    std::list<std::shared_ptr<QObject>> tests = {
      std::make_shared<TestQtExceptionPtr>(),
      std::make_shared<TestFlash>(),
      std::make_shared<TestMDPP16Firmware>()
    };

#ifdef RUN_GUI_TESTS

    tests.push_back(std::make_shared<TestFileDialog>());

    QApplication app(argc, argv);

#endif // RUN_GUI_TESTS

    for (auto obj: tests) {
      QTest::qExec(obj.get(), argc, argv);
    }
  }

  return 0;
}
