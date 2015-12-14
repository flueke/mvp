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

class TestFileDialog: public QObject
{
  Q_OBJECT
  private slots:
    void test();
};

class TestMDPP16Firmware: public QObject
{
  Q_OBJECT
  private slots:
    void test();
    void test_print_section_sizes();
    void test_from_dir();
    void test_from_zip();
};

#endif
