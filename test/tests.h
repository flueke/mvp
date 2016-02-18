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
    void test_from_firmware_file_generator_simple();
    void test_from_firmware_file_generator_empty();
    void test_filename_patterns();
    void test_filename_patterns2();
    void test_filename_patterns3();
    void test_empty_bin_part();
};

class TestInstructionFile: public QObject
{
  Q_OBJECT
  private slots:
    void test_valid();
    void test_invalid_binary();
    void test_invalid_structure();
};

class TestInstructionInterpreter: public QObject
{
  Q_OBJECT
  private slots:
    void test_print();
    void test_generate_memory();
};

#endif
