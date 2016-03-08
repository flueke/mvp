#include "tests.h"
#include "firmware_ops.h"

using namespace mesytec::mvp;

void TestFirmwareOps::test_keysinfo()
{
  // empty
  {
    auto ki = KeysInfo(
        OTP("MXYZ1234", 0x03160001),
        KeyMap(),
        KeyList());

    QVERIFY(!ki.need_to_erase());
    QCOMPARE(ki.get_device_keys().size(), 0);
    QCOMPARE(ki.get_firmware_keys().size(), 0);
    QCOMPARE(ki.get_new_firmware_keys().size(), 0);
  }

  // some matches, no device keys
  {
    const KeyList fw_keys = {
      { "MXYZ1234", 0x03160001, 0x0001, 0x42434445 }, // match
      { "ABCD1234", 0x03160001, 0x0001, 0x42434445 },
      { "MXYZ1234", 0x03160002, 0x0001, 0x42434445 },
      { "MXYZ1234", 0x03160001, 0x0010, 0xdeadbeef }, // match
    };

    auto ki = KeysInfo(
        OTP("MXYZ1234", 0x03160001),
        KeyMap(),
        fw_keys);

    QVERIFY(!ki.need_to_erase());
    QCOMPARE(ki.get_device_keys().size(), 0);
    QCOMPARE(ki.get_firmware_keys().size(), 2);
    QCOMPARE(ki.get_new_firmware_keys().size(), 2);
  }

  // fw matches and device keys; no new keys
  {
    const KeyList fw_keys = {
      { "MXYZ1234", 0x03160001, 0x0001, 0x42434445 }, // match & on device
      { "MXYZ1234", 0x03160001, 0x0002, 0x42434445 }, // match
      { "MXYZ1234", 0x03160001, 0x0003, 0x42434445 }, // match
      { "ABCD1234", 0x03160001, 0x0001, 0x42434445 },
      { "MXYZ1234", 0x03160002, 0x0001, 0x42434445 },
      { "MXYZ1234", 0x03160001, 0x0010, 0xdeadbeef }, // match & on device
    };

    const KeyMap dev_keys = {
      { 0, { "MXYZ1234", 0x03160001, 0x0001, 0x42434445 } }, // in firmware
      { 3, { "MXYZ1234", 0x03160001, 0x0010, 0xdeadbeef } }, // in firmware
      { 8, { "MXYZ1234", 0x03160001, 0x0020, 0xbeefdead } }, // device only
    };

    auto ki = KeysInfo(
        OTP("MXYZ1234", 0x03160001),
        dev_keys,
        fw_keys);

    QVERIFY(!ki.need_to_erase());
    QCOMPARE(ki.get_device_keys().size(), 3);
    QCOMPARE(ki.get_firmware_keys().size(), 4);
    QCOMPARE(ki.get_new_firmware_keys().size(), 2);
  }

  // fw matches and device keys; new keys
  {
    const KeyList fw_keys = {
      { "MXYZ1234", 0x03160001, 0x0001, 0x42434445 }, // match & on device
      { "MXYZ1234", 0x03160001, 0x0002, 0x42434445 }, // match
      { "MXYZ1234", 0x03160001, 0x0003, 0x42434445 }, // match
      { "ABCD1234", 0x03160001, 0x0001, 0x42434445 },
      { "MXYZ1234", 0x03160002, 0x0001, 0x42434445 },
      { "MXYZ1234", 0x03160001, 0x0010, 0xdeadbeef }, // match & on device
    };

    const KeyMap dev_keys = {
      { 0, { "MXYZ1234", 0x03160001, 0x0001, 0x42434445 } }, // in firmware
      { 3, { "MXYZ1234", 0x03160001, 0x0010, 0xdeadbeef } }, // in firmware
      { 8, { "MXYZ1234", 0x03160001, 0x0020, 0xbeefdead } }, // device only
    };

    auto ki = KeysInfo(
        OTP("MXYZ1234", 0x03160001),
        dev_keys,
        fw_keys);

    QVERIFY(!ki.need_to_erase());
    QCOMPARE(ki.get_device_keys().size(), 3);
    QCOMPARE(ki.get_firmware_keys().size(), 4);
    QCOMPARE(ki.get_new_firmware_keys().size(), 2);
  }

  // device key limit
  {
    const KeyList fw_keys = {
      { "MXYZ1234", 0x03160001, 0x0001, 0x42434445 }, // match & on device
      { "MXYZ1234", 0x03160001, 0x0042, 0x42434445 }, // match
    };

    const KeyMap dev_keys = {
      {  0, { "MXYZ1234", 0x03160001, 0x0001, 0x42434445 } }, // in firmware
      {  1, { "MXYZ1234", 0x03160001, 0x0002, 0xdeadbeef } }, // device only
      {  2, { "MXYZ1234", 0x03160001, 0x0003, 0xdeadbeef } }, // device only
      {  3, { "MXYZ1234", 0x03160001, 0x0004, 0xdeadbeef } }, // device only
      {  4, { "MXYZ1234", 0x03160001, 0x0005, 0xdeadbeef } }, // device only
      {  5, { "MXYZ1234", 0x03160001, 0x0006, 0xdeadbeef } }, // device only
      {  6, { "MXYZ1234", 0x03160001, 0x0007, 0xdeadbeef } }, // device only
      {  7, { "MXYZ1234", 0x03160001, 0x0008, 0xdeadbeef } }, // device only
      {  8, { "MXYZ1234", 0x03160001, 0x0009, 0xdeadbeef } }, // device only
      {  9, { "MXYZ1234", 0x03160001, 0x000a, 0xdeadbeef } }, // device only
      { 10, { "MXYZ1234", 0x03160001, 0x000b, 0xdeadbeef } }, // device only
      { 11, { "MXYZ1234", 0x03160001, 0x000c, 0xdeadbeef } }, // device only
      { 12, { "MXYZ1234", 0x03160001, 0x000d, 0xdeadbeef } }, // device only
      { 13, { "MXYZ1234", 0x03160001, 0x000e, 0xdeadbeef } }, // device only
      { 14, { "MXYZ1234", 0x03160001, 0x000f, 0xdeadbeef } }, // device only
      { 15, { "MXYZ1234", 0x03160001, 0x0010, 0xdeadbeef } }, // device only
    };

    auto ki = KeysInfo(
        OTP("MXYZ1234", 0x03160001),
        dev_keys,
        fw_keys);

    QVERIFY(ki.need_to_erase());
    QCOMPARE(ki.get_device_keys().size(), 16);
    QCOMPARE(ki.get_firmware_keys().size(), 2);
    QCOMPARE(ki.get_new_firmware_keys().size(), 1);
  }
}
