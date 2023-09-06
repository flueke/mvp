#ifndef __MESYTEC_MVLC_MVP_LIB_H__
#define __MESYTEC_MVLC_MVP_LIB_H__

#include <mesytec-mvlc/mesytec-mvlc.h>
#include <gsl/gsl-lite.hpp>
#include <stdexcept>

namespace mesytec::mvp
{

using u8 = mvlc::u8;
using u16 = mvlc::u16;
using u32 = mvlc::u32;
using MVLC = mvlc::MVLC;

using FlashAddress = std::array<u8, 3>;

static const u16 EnableFlashRegister = 0x6200;
static const u16 InputFifoRegister = 0x6202;
static const u16 OutputFifoRegister = 0x6204;
static const u16 StatusRegister = 0x6206;
static const size_t PageSize = 256;
static const size_t SectorSize = mvlc::util::Kilobytes(64);
static const size_t PagesPerSector = SectorSize / PageSize;
static const size_t FlashAddressBits = 24;
static const size_t FlashMaxAddress = (1u << FlashAddressBits) - 1;

namespace output_fifo_flags
{
    static const u32 ReadProgFull = 1u << 10;
    static const u32 InvalidRead = 1u << 9;
    static const u32 FlashEmpty = 1u << 8;
    static const u32 AnyFlag = ReadProgFull | InvalidRead | FlashEmpty;
    static const u32 DataMask = 0xff;
};

static const u8 FlashInstructionSuccess = 0x01;

namespace status_register_flags
{
    static const u32 FlashOutputFifoEmpty = 1u << 0;
};

inline void log_page_buffer(const std::vector<u8> &page)
{
    int col = 0;

    for (auto val: page)
    {
        if (col > 0 && (col % 16 == 0))
            std::cout << std::endl;

        std::cout << fmt::format("{:02x} ", val);
        ++col;
    }

    std::cout << std::endl;
}

inline FlashAddress flash_address_from_byte_offset(u32 byteOffset)
{
    if (byteOffset > FlashMaxAddress)
        throw std::invalid_argument("byteOffset exceeds FlashMaxAddress");

    FlashAddress addr =
    {
        static_cast<u8>((byteOffset & 0x0000ff) >>  0),
        static_cast<u8>((byteOffset & 0x00ff00) >>  8),
        static_cast<u8>((byteOffset & 0xff0000) >> 16),
    };

    return addr;
}

std::error_code enable_flash_interface(MVLC &mvlc, u32 moduleBase);
std::error_code disable_flash_interface(MVLC &mvlc, u32 moduleBase);
std::error_code read_output_fifo(MVLC &mvlc, u32 moduleBase, unsigned bytesToRead, std::vector<u32> &dest);
std::error_code clear_output_fifo(MVLC &mvlc, u32 moduleBase);
std::error_code set_area_index(MVLC &mvlc, u32 moduleBase, unsigned areaIndex);
std::error_code enable_flash_write(MVLC &mvlc, u32 moduleBase);
std::error_code set_verbose_mode(MVLC &mvlc, u32 moduleBase, bool verbose);

std::error_code write_instruction(MVLC &mvlc, u32 moduleBase, const std::vector<u8> &instruction);
std::error_code write_instruction(MVLC &mvlc, u32 moduleBase, const gsl::span<unsigned char> instruction);
std::error_code read_response(MVLC &mvlc, u32 moduleBase, std::vector<u8> &dest);
bool check_response(const std::vector<u8> &request,
                    const std::vector<u8> &response);

// Note: bytesToRead=0 is used to read a full page of 256 bytes.
// Note: bytesToRead <= 256, the value 0 is the same as 256 (full page)
std::error_code read_page(
    MVLC &mvlc, u32 moduleBase,
    const FlashAddress &addr, u8 section, unsigned bytesToRead,
    std::vector<u8> &pageBuffer);

// Write a full page or less using single vme write commands.
std::error_code write_page(
    MVLC &mvlc, u32 moduleBase,
    const FlashAddress &addr, u8 section,
    const std::vector<u8> &pageBuffer);

// Write a full page or less by uploading and executing command stacks
// containing the write commands. Each command stack uses the max size that can
// be uploaded in a single MirrorTransaction.
std::error_code write_page2(
    MVLC &mvlc, u32 moduleBase,
    const FlashAddress &addr, u8 section,
    const std::vector<u8> &pageBuffer);

// Write a full page or less by uploading all data into a single command stack
// and executing that stack. The MirrorTransactionMaxWords limit does not apply
// as the stack upload is split into multiple parts internally.
std::error_code write_page3(
    MVLC &mvlc, u32 moduleBase,
    const FlashAddress &addr, u8 section,
    const std::vector<u8> &pageBuffer);

// Same as write_page3() but uses the stack accu to poll the flash status
// register for '!flash_empty', meaning there is an answer from the flash
// interface. After the accu loop the stack reads the expected number of
// response words from the output fifo. This is the final stack response that
// has to be parsed by software.
// During development/debugging an additional check can be done:
// clear_output_fifo() should return after one cycle if the stack words
// correctly.
std::error_code write_page4(
    MVLC &mvlc, u32 moduleBase,
    const FlashAddress &addr, u8 section,
    const std::vector<u8> &pageBuffer);

std::error_code erase_section(
    MVLC &mvlc, u32 moduleBase, u8 index);

void fill_page_buffer_from_stack_output(
    std::vector<u8> &pageBuffer, const std::vector<u32> stackOutput);

}

#endif /* __MESYTEC_MVLC_MVP_LIB_H__ */
