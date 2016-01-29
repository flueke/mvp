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

class TestFirmware: public QObject
{
  Q_OBJECT
  private slots:
    void test_basics();
    void test_print_section_sizes();
    void test_from_firmware_file_generator_simple();
    void test_from_firmware_file_generator_empty();
    void test_filename_patterns();
};

class TestInstructionFile: public QObject
{
  Q_OBJECT
  private slots:
    void test_valid();
    void test_invalid_binary();
    void test_invalid_text();
    void test_invalid_structure();
};

#endif
