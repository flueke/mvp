#include "firmware_ops.h"
#include "flash.h"
#include "instruction_interpreter.h"

namespace mesytec
{
namespace mvp
{

FirmwareWriter::FirmwareWriter(const FirmwareArchive &firmware,
    PortHelper *port_helper, Flash *flash, QObject *parent)
  : QObject(parent)
  , m_firmware(firmware)
  , m_port_helper(port_helper)
  , m_flash(flash)
{}

void FirmwareWriter::write()
{
  auto non_area_specific_parts = m_firmware.get_non_area_specific_parts();
  auto area_specific_parts = m_firmware.get_area_specific_parts();
  const auto selected_area = m_flash->read_area_index();

  m_flash->set_verbose(false);

  emit status_message("Writing non area-specific parts...");

  for (auto pp: non_area_specific_parts) {
    write_part(pp, *pp->get_section());
  }

  emit status_message("Writing area-specific parts...");

  for (auto pp: area_specific_parts) {
    auto area = pp->has_area() ? *pp->get_area() : selected_area;
    write_part(pp, *pp->get_section(), area);
  }

  emit status_message(QString("Restoring area index to %1")
      .arg(selected_area));
  m_flash->set_area_index(selected_area);
}

void FirmwareWriter::write_part(const FirmwarePartPtr &pp,
    uchar section,
    const boost::optional<uchar> &area)
{
  emit status_message(QString("File %1, section %2, contents size=%3")
      .arg(pp->get_filename())
      .arg(section)
      .arg(pp->get_contents_size())
      );

  if (bool(area)) {
    emit status_message(QString("Selecting area %1").arg(*area));
    m_flash->set_area_index(*area);
  }

  if (section != constants::otp_section) {

    if (do_erase()) {
      emit status_message(QString("Erasing section %1").arg(section));
      m_flash->erase_section(section);
    }
  } else if (section == constants::otp_section) {
    emit status_message("Not erasing OTP section");
  }

  if (is_binary_part(pp)) {
    auto contents = pp->get_contents();

    if (contents.isEmpty()) {
      emit status_message(QString("File %1: empty file -> erase only")
          .arg(pp->get_filename()));

    } else if (do_program()) {
      emit status_message(QString("File %1: writing %2 bytes of data")
          .arg(pp->get_filename()).arg(contents.size()));

      m_flash->write_memory({0, 0, 0}, section, gsl::as_span(contents));
    }

    if (!contents.isEmpty() && do_verify()) {
      emit status_message(QString("File %1: verifying memory")
          .arg(pp->get_filename()));


      auto res = m_flash->verify_memory({0, 0, 0}, section, gsl::as_span(contents));
      if (!res) throw FlashVerificationError(res);
    }
  } else if (is_instruction_part(pp) && !is_key_part(pp)) {

    auto instructions = std::dynamic_pointer_cast<InstructionFirmwarePart>(pp)
      ->get_instructions();

    if (section == constants::otp_section) {

      auto mem = generate_memory(instructions);

      emit status_message(QString("File %1: OTP: generated %2 bytes of memory")
          .arg(pp->get_filename())
          .arg(mem.size()));

      if (do_program()) {
        emit status_message(QString("File %1: writing %2 bytes of data")
            .arg(pp->get_filename()).arg(mem.size()));

        m_flash->write_memory({0, 0, 0}, section, gsl::as_span(mem));
      }

      if (do_verify()) {
        emit status_message(QString("File %1: verifying memory")
            .arg(pp->get_filename()));

        auto res = m_flash->verify_memory({0, 0, 0}, section, gsl::as_span(mem));

        qDebug() << res.to_string();

        if (!res) throw FlashVerificationError(res);
      }

    } else {
      if (do_program()) {
        emit status_message(QString("File %1: executing %2 instructions")
            .arg(pp->get_filename())
            .arg(instructions.size()));

        run_instructions(instructions, m_flash, section);
      }

      if (do_verify()) {
        emit status_message(QString("File %1: verifying memory")
            .arg(pp->get_filename()));

        auto mem = generate_memory(instructions);

        auto res = m_flash->verify_memory({0, 0, 0}, section, gsl::as_span(mem));
        qDebug() << res.to_string();
        if (!res) throw FlashVerificationError(res);
      }
    }
  }
}

KeysInfo::KeysInfo(
    const OTP &otp,
    const KeyMap &device_keys,
    const KeyList &firmware_keys)
  : m_otp(otp)
  , m_device_keys(device_keys)
{
  std::copy_if(std::begin(firmware_keys), std::end(firmware_keys),
      std::back_inserter(m_firmware_keys),
      [&](const Key &key) {
      return key_matches_otp(key, otp);
      });
}

bool KeysInfo::need_to_erase() const
{
  return static_cast<size_t>(get_device_keys().size() + get_new_firmware_keys().size()) > constants::max_keys;
}

KeyList KeysInfo::get_new_firmware_keys() const
{
  const auto dev_keys = get_device_keys().values();

  auto finder = [&](const Key &k) {
    return std::find(std::begin(dev_keys), std::end(dev_keys), k) == std::end(dev_keys);
  };

  auto ret = KeyList();

  std::copy_if(std::begin(m_firmware_keys), std::end(m_firmware_keys),
      std::back_inserter(ret), finder);

  return ret;
}

KeysHandler::KeysHandler(
    const FirmwareArchive &firmware,
    gsl::not_null<PortHelper *> port_helper,
    gsl::not_null<Flash *> flash,
    QObject *parent)
  : m_firmware(firmware)
  , m_port_helper(port_helper)
  , m_flash(flash)
{
}

KeysInfo KeysHandler::get_keys_info()
{
  if (m_keys_info_read)
    return m_keys_info;

  KeyList fw_keys;

  auto key_parts = m_firmware.get_key_parts();

  for (const auto &key_part: key_parts) {
    fw_keys.push_back(key_from_firmware_part(*key_part));
  }

  const auto otp      = m_flash->read_otp();
  const auto dev_keys = m_flash->read_keys();

  m_keys_info = KeysInfo(otp, dev_keys, fw_keys);
  m_keys_info_read = true;

  return m_keys_info;
}

FirmwarePartList KeysHandler::get_key_parts_to_write()
{
  auto new_keys = get_keys_info().get_new_firmware_keys();

  FirmwarePartList ret;

  for (const auto &key: new_keys) {
    for (const auto &key_part: m_firmware.get_key_parts()) {
      const auto fw_key = key_from_firmware_part(*key_part);

      if (fw_key == key) {
        ret.push_back(key_part);
        break;
      }
    }
  }

  return ret;
}

void KeysHandler::write_keys()
{
  const auto key_parts = get_key_parts_to_write();

  if (static_cast<size_t>(key_parts.size()) > constants::max_keys) {
    throw std::runtime_error("Firmware keys exceed maximum number of device keys.");
  }

  const auto ki = get_keys_info();

  qDebug() << "write_keys: #key_parts =" << key_parts.size()
    << "need_to_erase" << ki.need_to_erase();

  if (ki.need_to_erase()) {
    emit status_message("Erasing keys section");
    m_flash->erase_section(constants::keys_section);
  }

  auto free_slots = m_flash->get_free_key_slots().toList();

  /* This should not happen as we'd either have more than max_keys keys or
   * erase and get_free_key_slots() would not work properly. */
  if (key_parts.size() > free_slots.size()) {
    throw std::runtime_error("Too many keys to write");
  }

  qSort(free_slots);

  qDebug() << "write_keys: free slots =" << free_slots;

  auto it_slots = std::begin(free_slots);
  auto it_parts = std::begin(key_parts);

  for (; it_parts != std::end(key_parts); ++it_parts, ++it_slots) {
    const auto &key_part = dynamic_cast<const KeyFirmwarePart &>(**it_parts);

    const auto offset = *it_slots * constants::keys_offset;

    qDebug() << "write_keys: writing key to slot" << *it_slots
      << ", offset =" << offset;

    run_instructions(key_part.get_instructions(), m_flash, constants::keys_section, offset);
  }
}

Key key_from_firmware_part(const FirmwarePart &part)
{
  const auto &key_part = dynamic_cast<const KeyFirmwarePart &>(part);
  auto mem = generate_memory(key_part.get_instructions());

  return Key::from_flash_memory(mem);
}

} // ns mvp
} // ns mesytec
