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
      //return true;
      return key_matches_otp(key, otp);
      });
}

bool KeysInfo::need_to_erase() const
{
  throw std::runtime_error("not implemented");
}

KeyList KeysInfo::get_new_firmware_keys() const
{
  return KeyList();
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
  KeyList fw_keys;

  auto key_parts = m_firmware.get_key_parts();

  for (const auto &key_part: key_parts) {
    auto instructions = std::dynamic_pointer_cast<KeyFirmwarePart>(key_part)
      ->get_instructions();

    auto mem = generate_memory(instructions);

    fw_keys.push_back(Key::from_flash_memory(mem));
  }

  const auto otp      = m_flash->read_otp();
  const auto dev_keys = m_flash->read_keys();

  return KeysInfo(otp, dev_keys, fw_keys);
}

void KeysHandler::write_keys()
{
  throw std::runtime_error("not implemented");
}

} // ns mvp
} // ns mesytec
