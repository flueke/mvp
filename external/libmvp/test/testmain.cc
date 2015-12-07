#include "tests.h"

#ifdef RUN_GUI_TESTS
#include "file_dialog.h"
#endif

#include <memory>
#include <QtTest/QtTest>

int main(int argc, char *argv[])
{
  {
    std::list<std::shared_ptr<QObject>> tests = {
      std::make_shared<TestQtExceptionPtr>(),
      std::make_shared<TestFlash>()
    };

    for (auto obj: tests) {
      QTest::qExec(obj.get(), argc, argv);
    }
  }

#ifdef RUN_GUI_TESTS
  QApplication app(argc, argv);
  mvp::FileDialog dialog;
  dialog.show();

  return app.exec();
#else
  return 0;
#endif // RUN_GUI_TESTS
}
