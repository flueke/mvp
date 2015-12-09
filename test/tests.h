#ifndef UUID_ef942d6f_6b14_42df_9cca_dddfabf57972
#define UUID_ef942d6f_6b14_42df_9cca_dddfabf57972

#include <QtTest/QtTest>

class TestFlash: public QObject
{
  Q_OBJECT
private slots:
  void test_address();
};

class TestQtExceptionPtr: public QObject
{
  Q_OBJECT
private slots:
  void test();
};

class GUITestBase: public QObject {};

class TestFileDialog: public GUITestBase
{
  Q_OBJECT
private slots:
  void test();
};

#endif
