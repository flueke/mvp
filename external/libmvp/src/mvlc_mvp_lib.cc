#include "mvlc_mvp_lib.h"
#include <flash_constants.h>
#include <chrono>
#include <stdexcept>

namespace mesytec::mvp
{

using namespace mesytec::mvlc;

std::error_code enable_flash_interface(MVLC &mvlc, u32 moduleBase)
{
    auto logger = mvlc::get_logger("mvlc_mvp_lib");
    logger->info("Enabling flash interface on 0x{:08x}", moduleBase);
    return mvlc.vmeWrite(moduleBase + EnableFlashRegister, 1, vme_amods::A32, VMEDataWidth::D16);
}

std::error_code disable_flash_interface(MVLC &mvlc, u32 moduleBase)
{
    auto logger = mvlc::get_logger("mvlc_mvp_lib");
    logger->info("Disabling flash interface on 0x{:08x}", moduleBase);
    return mvlc.vmeWrite(moduleBase + EnableFlashRegister, 0, vme_amods::A32, VMEDataWidth::D16);
}

std::error_code read_output_fifo(MVLC &mvlc, u32 moduleBase, u32 &dest)
{
    return mvlc.vmeRead(moduleBase + OutputFifoRegister, dest, vme_amods::A32, VMEDataWidth::D16);
}

// FIXME: clear_output_fifo() is dangerous as it might never return!
// FIXME: do not use this function. instead correctly read and parse the
// expected response
std::error_code clear_output_fifo(MVLC &mvlc, u32 moduleBase)
{
    auto logger = mvlc::get_logger("mvlc_mvp_lib");
    logger->debug("Clearing output fifo on 0x{:08x}", moduleBase);

    auto tStart = std::chrono::steady_clock::now();

    size_t cycles = 0;
    //std::vector<u32> readBuffer;

    while (true)
    {
        u32 fifoValue = 0;

        auto ec = mvlc.vmeRead(
            moduleBase + OutputFifoRegister,
            fifoValue, vme_amods::A32, VMEDataWidth::D16);

        //readBuffer.push_back(fifoValue);

        ++cycles;

        if (ec) return ec;

        if (fifoValue & output_fifo_flags::InvalidRead)
            break;

        logger->debug("  clear_output_fifo: 0x{:04x} = 0x{:08x}", OutputFifoRegister, fifoValue);
    }

    //if (cycles > 1)
    //  log_buffer(std::cout, readBuffer, "clear_output_fifo read buffer");
    auto tEnd = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(tEnd - tStart);
    logger->debug("clear_output_fifo() returned after {} read cycles, took {} ms to clear the fifo",
                  cycles, elapsed.count()/1000.0);

    return {};
}

template<typename C>
std::error_code perform_writes(MVLC &mvlc, const C &writes)
{
    for (auto write: writes)
    {
        u32 addr = write.first;
        u32 val = write.second;

        if (auto ec = mvlc.vmeWrite(addr, val, vme_amods::A32, VMEDataWidth::D16))
            return ec;
    }

    return {};
}

std::error_code command_transaction(
    MVLC &mvlc, u32 moduleBase,
    const std::vector<u8> &instruction,
    std::vector<u8> &responseBuffer)
{
    if (auto ec = write_instruction(mvlc, moduleBase, instruction))
        return ec;

    if (auto ec = read_response(mvlc, moduleBase, responseBuffer))
        return ec;

    if (!check_response(instruction, responseBuffer))
        return make_error_code(std::errc::protocol_error);

    return {};
}


std::error_code set_area_index(MVLC &mvlc, u32 moduleBase, unsigned area)
{
    auto logger = mvlc::get_logger("mvlc_mvp_lib");
    logger->info("Setting area index on 0x{:08x} to {}", moduleBase, area);

    std::vector<u8> instr = { 0x20, 0xCD, 0xAB, static_cast<u8>(area) };
    std::vector<u8> response;

    return command_transaction(mvlc, moduleBase, instr, response);
}

std::error_code enable_flash_write(MVLC &mvlc, u32 moduleBase)
{
    auto logger = mvlc::get_logger("mvlc_mvp_lib");
    logger->info("Enabling flash write on 0x{:08x}", moduleBase);

    std::vector<u8> instr = { 0x80, 0xCD, 0xAB };
    std::vector<u8> response;

    return command_transaction(mvlc, moduleBase, instr, response);
}

std::error_code write_instruction(MVLC &mvlc, u32 moduleBase, const std::vector<u8> &instruction)
{
    auto logger = mvlc::get_logger("mvlc_mvp_lib");
    logger->debug("write_instruction: moduleBase=0x{:08x}, instr.size()={}B, instr={:#02x}",
                  moduleBase, instruction.size(), fmt::join(instruction, ", "));

    for (u8 arg: instruction)
    {
        if (auto ec = mvlc.vmeWrite(
                moduleBase + InputFifoRegister, arg,
                vme_amods::A32, VMEDataWidth::D16))
        {
            return ec;
        }
    }

    return {};
}

std::error_code write_instruction(MVLC &mvlc, u32 moduleBase, const gsl::span<unsigned char> instruction)
{
    std::vector<u8> tmp;
    tmp.reserve(instruction.size());
    std::copy(std::begin(instruction), std::end(instruction), std::back_inserter(tmp));
    return write_instruction(mvlc, moduleBase, tmp);
}

std::error_code read_response(MVLC &mvlc, u32 moduleBase, std::vector<u8> &dest)
{
    while (true) // FIXME: dangerous. do time out at some point
    {
        u32 fifoValue = 0;

        if (auto ec = mvlc.vmeRead(
                moduleBase + OutputFifoRegister, fifoValue,
                vme_amods::A32, VMEDataWidth::D16))
        {
            return ec;
        }

        if (fifoValue & output_fifo_flags::InvalidRead)
            break;

        dest.push_back(fifoValue & 0xff);
    }

    auto logger = mvlc::get_logger("mvlc_mvp_lib");
    logger->debug("read_response: moduleBase=0x{:08x}, got {} bytes: {:02x}",
                 moduleBase, dest.size(), fmt::join(dest, ", "));
    return {};
}

bool check_response(const std::vector<u8> &request,
                    const std::vector<u8> &response)
{
    auto logger = mvlc::get_logger("mvlc_mvp_lib");

    if (response.size() < 2)
    {
        logger->warn("short response (size<2)");
        return false;
    }

    if (response.size() < request.size())
    {
        logger->warn("response too short (len={}) for request (len={})",
                     response.size(), request.size());
        return false;
    }

    if (!std::equal(std::begin(request), std::end(request), std::begin(response)))
    {
        logger->warn("request contents != response contents");
        return false;
    }

    u8 codeStart = *(response.end() - 2);
    u8 status    = *(response.end() - 1);

    if (codeStart != 0xff)
    {
        logger->warn("invalid response code start 0x{:02x} (expected 0xff)", codeStart);
        return false;
    }

    if (!(status & 0x01))
    {
        logger->warn("instruction failed (status bit 0 not set): 0x{:02x}", status);
        return false;
    }

    return true;
}

std::error_code set_verbose_mode(MVLC &mvlc, u32 moduleBase, bool verbose)
{
    auto logger = mvlc::get_logger("mvlc_mvp_lib");
    logger->info("Setting verbose mode to {}", verbose);

    u8 veb = verbose ? 0 : 1;

    std::vector<u8> instr = { 0x60, 0xCD, 0xAB, veb };
    std::vector<u8> resp;

    return command_transaction(mvlc, moduleBase, instr, resp);
}

void fill_page_buffer_from_stack_output(std::vector<u8> &pageBuffer, const std::vector<u32> stackOutput)
{
    assert(stackOutput.size() > 3);
    assert(is_stack_buffer(stackOutput.at(0)));
    assert(stackOutput.at(1) == 0x13370001u);

    auto logger = mvlc::get_logger("mvlc_mvp_lib");
    pageBuffer.clear();
    auto view = basic_string_view<u32>(stackOutput.data(), stackOutput.size());

    while (!view.empty())
    {
        u32 word = view[0];

        if (is_stack_buffer(word))
        {
            assert(view.size() >= 2);
            assert(view[1] == 0x13370001u);
            view.remove_prefix(2); // skip over the stack buffer header and the marker
        }
        else if (is_stack_buffer_continuation(word) || is_blockread_buffer(word))
        {
            view.remove_prefix(1); // skip over the header
        }
        else
        {
            view.remove_prefix(1);

            if (word & (output_fifo_flags::InvalidRead))
            {
                logger->debug("fill_page_buffer_from_stack_output: first non-data word: 0x{:08x}", word);
                break;
            }

            pageBuffer.push_back(word & 0xffu);
        }
    }

    if (!view.empty())
    {
        logger->warn("fill_page_buffer_from_stack_output: {} words left in stackOutput data", view.size());
        log_buffer(std::cout, view);
        logger->warn("fill_page_buffer_from_stack_output: end of {} words left in stackOutput data", view.size());
    }
}

std::error_code read_page(
    MVLC &mvlc, u32 moduleBase,
    const FlashAddress &addr, u8 section, unsigned bytesToRead,
    std::vector<u8> &pageBuffer)
{
    if (bytesToRead > PageSize)
        throw std::invalid_argument("read_page: bytesToRead > PageSize");

    auto logger = mvlc::get_logger("mvlc_mvp_lib");

    // Note: the REF instruction does not mirror itself to the output fifo.
    // Instead the page data starts immediately.

    //u8 len = static_cast<u8>(bytesToRead == PageSize ? 0 : bytesToRead);
    //std::vector<u8> instr = { 0xB0, addr[0], addr[1], addr[2], section, len };

    StackCommandBuilder sb;
    sb.addWriteMarker(0x13370001u);
    sb.addVMEWrite(moduleBase + InputFifoRegister, 0xB0, vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, addr[0], vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, addr[1], vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, addr[2], vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, section, vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, bytesToRead == PageSize ? 0 : bytesToRead,
                   vme_amods::A32, VMEDataWidth::D16);

    // Waiting is required, otherwise the response data will start with the
    // InvalidRead flag set.
    sb.addWait(100000);
    // Turn the next vme read into a fake block read. Read one more word than
    // expected to get the first flash interface status word after the payload.
    sb.addSetAccu(bytesToRead + 1);
    // This single read is turned into a block read due to the accu being set.
    sb.addVMERead(moduleBase + OutputFifoRegister, vme_amods::A32, VMEDataWidth::D16);

    std::vector<u32> readBuffer; // stores raw read data

    if (auto ec = mvlc.stackTransaction(sb, readBuffer))
    {
        logger->error("read_page(): mvlc.stackTransaction: {}", ec.message());
        return ec;
    }

    fill_page_buffer_from_stack_output(pageBuffer, readBuffer);

    if (pageBuffer.size() != bytesToRead)
        logger->warn("read_page(): wanted {} bytes, got {} bytes",
                     bytesToRead, pageBuffer.size());

    return {};
}

// Write a full page or less using single vme write commands.
std::error_code write_page(
    MVLC &mvlc, u32 moduleBase,
    const FlashAddress &addr, u8 section,
    const std::vector<u8> &pageBuffer)
{
    if (pageBuffer.empty())
        throw std::invalid_argument("write_page: empty data given");

    if (pageBuffer.size() > PageSize)
        throw std::invalid_argument("write_page: data size > page size");

    auto logger = mvlc::get_logger("mvlc_mvp_lib");

    u8 lenByte = pageBuffer.size() == PageSize ? 0 : pageBuffer.size();

    auto tStart = std::chrono::steady_clock::now();

    std::vector<std::pair<u32, u16>> writes =
    {
        { moduleBase + InputFifoRegister, 0xA0 },
        { moduleBase + InputFifoRegister, addr[0] },
        { moduleBase + InputFifoRegister, addr[1] },
        { moduleBase + InputFifoRegister, addr[2] },
        { moduleBase + InputFifoRegister, section },
        { moduleBase + InputFifoRegister, lenByte },
    };

    if (auto ec = perform_writes(mvlc, writes))
        return ec;

    for (u8 data: pageBuffer)
    {
        if (auto ec = mvlc.vmeWrite(moduleBase + InputFifoRegister, data,
                                    vme_amods::A32, VMEDataWidth::D16))
            return ec;
    }

    if (auto ec = clear_output_fifo(mvlc, moduleBase))
        return ec;

    auto tEnd = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(tEnd - tStart);
    logger->info("write_page(): took {} ms to write {} bytes of data",
                 elapsed.count()/1000.0, pageBuffer.size());

    return {};
}

// Write a full page or less by uploading and executing command stacks
// containing the write commands.
std::error_code write_page2(
    MVLC &mvlc, u32 moduleBase,
    const FlashAddress &addr, u8 section,
    const std::vector<u8> &pageBuffer)
{
    auto logger = mvlc::get_logger("mvlc_mvp_lib");
    static const bool useVerbose = false;

    if (pageBuffer.empty())
        throw std::invalid_argument("write_page2: empty data given");

    if (pageBuffer.size() > PageSize)
        throw std::invalid_argument("write_page2: data size > page size");

    u8 lenByte = pageBuffer.size() == PageSize ? 0 : pageBuffer.size();

    auto tStart = std::chrono::steady_clock::now();

    if (useVerbose)
        if (auto ec = set_verbose_mode(mvlc, moduleBase, true))
            return ec;

    StackCommandBuilder sb;
    sb.addWriteMarker(0x13370001u);
    // EFW - enable flash write
    sb.addVMEWrite(moduleBase + InputFifoRegister, 0x80,  vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, 0xCD,  vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, 0xAB,  vme_amods::A32, VMEDataWidth::D16);
    // WRF - write flash
    sb.addVMEWrite(moduleBase + InputFifoRegister, 0xA0,     vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, addr[0],  vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, addr[1],  vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, addr[2],  vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, section,  vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, lenByte,  vme_amods::A32, VMEDataWidth::D16);

    auto pageIter = pageBuffer.begin();

    logger->info("write_page2(): writing page of size {}", pageBuffer.size());

    while (pageIter != pageBuffer.end())
    {
        while (get_encoded_stack_size(sb) < MirrorTransactionMaxContentsWords / 2 - 2
               && pageIter != pageBuffer.end())
        {
            sb.addVMEWrite(moduleBase + InputFifoRegister, *pageIter++,
                           vme_amods::A32, VMEDataWidth::D16);
        }

        std::vector<u32> stackResponse;

        logger->info("write_page2(): performing stackTransaction with stack of size {}",
                     get_encoded_stack_size(sb));

        if (auto ec = mvlc.stackTransaction(sb, stackResponse))
        {
            logger->error("write_page2(): stackTransaction failed: {}",
                          ec.message());
            return ec;
        }

        logger->trace("write_page(): response from stackTransaction: size={}, data={:08x}",
                      stackResponse.size(), fmt::join(stackResponse, ", "));

        //  Expect the 0xF3 stack frame and the marker word
        if (stackResponse.size() != 2)
            return make_error_code(MVLCErrorCode::UnexpectedResponseSize);

        if (extract_frame_info(stackResponse[0]).flags & frame_flags::AllErrorFlags)
        {
            if (extract_frame_info(stackResponse[0]).flags & frame_flags::Timeout)
                return MVLCErrorCode::NoVMEResponse;

            if (extract_frame_info(stackResponse[0]).flags & frame_flags::SyntaxError)
                return MVLCErrorCode::StackSyntaxError;

            // Note: BusError can not happen as there's no block read in the stack
        }

        sb = {};
        sb.addWriteMarker(0x13370001u);
    }

    assert(pageIter == pageBuffer.end());

    // Read all the response data. Check the response code and status in the
    // final two words.
    if (useVerbose)
    {
        std::vector<u8> response;

        if (auto ec = read_response(mvlc, moduleBase, response))
            return ec;

        if (response.size() < 2)
            throw std::runtime_error("short response");

        u8 codeStart = *(response.end() - 2);
        u8 status    = *(response.end() - 1);

        if (codeStart != 0xff)
        {
            logger->warn("invalid response code start 0x{:02} (expected 0xff)", codeStart);
            throw std::runtime_error("invalid response code");
        }

        if (!(status & 0x01))
        {
            logger->warn("instruction failed (status bit 0 not set): 0x{02x}", status);
            throw std::runtime_error("instruction failed");
        }
    }

    if (auto ec = clear_output_fifo(mvlc, moduleBase))
        return ec;

    if (useVerbose)
        if (auto ec = set_verbose_mode(mvlc, moduleBase, false))
            return ec;

    auto tEnd = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart);
    logger->info("write_page2(): took {} ms to write {} bytes of data",
                 elapsed.count(), pageBuffer.size());

    return {};
}

std::error_code write_page3(
    MVLC &mvlc, u32 moduleBase,
    const FlashAddress &addr, u8 section,
    const std::vector<u8> &pageBuffer)
{
    auto logger = mvlc::get_logger("mvlc_mvp_lib");

    if (pageBuffer.empty())
        throw std::invalid_argument("write_page3: empty data given");

    if (pageBuffer.size() > PageSize)
        throw std::invalid_argument("write_page3: data size > page size");

    u8 lenByte = pageBuffer.size() == PageSize ? 0 : pageBuffer.size();

    auto tStart = std::chrono::steady_clock::now();

    StackCommandBuilder sb;
    sb.addWriteMarker(0x13370001u);
    // EFW - enable flash write
    sb.addVMEWrite(moduleBase + InputFifoRegister, 0x80,  vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, 0xCD,  vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, 0xAB,  vme_amods::A32, VMEDataWidth::D16);
    // WRF - write flash
    sb.addVMEWrite(moduleBase + InputFifoRegister, 0xA0,     vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, addr[0],  vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, addr[1],  vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, addr[2],  vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, section,  vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, lenByte,  vme_amods::A32, VMEDataWidth::D16);

    for (auto dataWord: pageBuffer)
        sb.addVMEWrite(moduleBase + InputFifoRegister, dataWord, vme_amods::A32, VMEDataWidth::D16);

    sb.addWait(100000); // wait for a couple of cycles before returning from the stack

    logger->debug("write_page3(): performing stackTransaction: pageSize={} bytes, stackCommands={}"
                 ", encodedStackSize={} words",
        pageBuffer.size(), sb.commandCount(), get_encoded_stack_size(sb));

    std::vector<u32> stackResponse;

    if (auto ec = mvlc.stackTransaction(sb, stackResponse))
    {
        logger->error("write_page3(): stackTransaction failed: {}", ec.message());
        return ec;
    }

    logger->trace("write_page3(): response from stackTransaction: size={}, data={:#08x}",
                    stackResponse.size(), fmt::join(stackResponse, ", "));

    //  Expect the 0xF3 stack frame and the marker word
    if (stackResponse.size() != 2)
        return make_error_code(MVLCErrorCode::UnexpectedResponseSize);

    if (extract_frame_info(stackResponse[0]).flags & frame_flags::AllErrorFlags)
    {
        if (extract_frame_info(stackResponse[0]).flags & frame_flags::Timeout)
            return MVLCErrorCode::NoVMEResponse;

        if (extract_frame_info(stackResponse[0]).flags & frame_flags::SyntaxError)
            return MVLCErrorCode::StackSyntaxError;

        // Note: BusError can not happen as there's no block read in the stack
    }

    #if 0
    std::vector<u8> flashResponse;
    if (auto ec = read_response(mvlc, moduleBase, flashResponse))
    {
        logger->error("write_page3(): error reading flash response from 0x{:08x}: {}",
            moduleBase, ec.message());
        return ec;
    }

    logger->trace("write_page3(): flash response from 0x{:08x}: size={}, data={:#02x}",
        moduleBase, flashResponse.size(), fmt::join(flashResponse, ", "));
    #else
    // TODO: get rid of this, handle it above with read_response() and check_response()
    if (auto ec = clear_output_fifo(mvlc, moduleBase))
        return ec;

    #endif

    auto tEnd = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart);
    logger->info("write_page3(): took {} ms to write {} bytes of data",
                 elapsed.count(), pageBuffer.size());

    return {};
}

std::error_code write_page4(
    MVLC &mvlc, u32 moduleBase,
    const FlashAddress &addr, u8 section,
    const std::vector<u8> &pageBuffer)
{
    auto logger = mvlc::get_logger("mvlc_mvp_lib");

    if (pageBuffer.empty())
        throw std::invalid_argument("write_page4: empty data given");

    if (pageBuffer.size() > PageSize)
        throw std::invalid_argument("write_page4: data size > page size");

    u8 lenByte = pageBuffer.size() == PageSize ? 0 : pageBuffer.size();

    auto tStart = std::chrono::steady_clock::now();

    #if 0
    for (int i=0; i<10; ++i)
    {
        u32 statusValue = 0u;
        if (auto ec = mvlc.vmeRead(moduleBase + StatusRegister, statusValue, vme_amods::A32, VMEDataWidth::D16))
            return ec;
        spdlog::warn("write_page4(): polled flash StatusRegister: {:#04x} (poll1)", statusValue);
        if (statusValue != 0)
            break;
    }
    #endif

    static const std::vector<u8> EfwRequest = { mesytec::mvp::opcodes::EFW, 0xCD, 0xAB };
    static const unsigned ExpectedFlashResponseSize = 5;
    static const u32 StackReferenceMarker = 0x13370001u;
    // Response structure: 0xF3 stack frame header, reference marker word, data
    // words from vme reads.
    static const unsigned ExpectedStackResponseSize = 2 + ExpectedFlashResponseSize;


    StackCommandBuilder sb;
    // Initial marker so that stackTransaction() has a reference word.
    sb.addWriteMarker(StackReferenceMarker);

    // EFW - enable flash write. This is written back to the output fifo by the
    // flash interface.
    for (auto op: EfwRequest)
        sb.addVMEWrite(moduleBase + InputFifoRegister, op,  vme_amods::A32, VMEDataWidth::D16);

    // WRF - write flash. This is not written back to the output fifo.
    sb.addVMEWrite(moduleBase + InputFifoRegister, opcodes::WRF, vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, addr[0],      vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, addr[1],      vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, addr[2],      vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, section,      vme_amods::A32, VMEDataWidth::D16);
    sb.addVMEWrite(moduleBase + InputFifoRegister, lenByte,      vme_amods::A32, VMEDataWidth::D16);

    // Add the actual page data to the stack.
    for (auto dataWord: pageBuffer)
        sb.addVMEWrite(moduleBase + InputFifoRegister, dataWord, vme_amods::A32, VMEDataWidth::D16);

    // Wait for a couple of cycles before continuing with the stack (max value
    // is 24 bit). Without the wait the output fifo will be in an invalid state
    // as the commands from the input fifo are still being processed by the
    // flash interface.
    sb.addWait(100000);

    // Accu loop: read the statusregister until it's 0, meaning "flash output fifo not empty".
    sb.addReadToAccu(moduleBase + StatusRegister, vme_amods::A32, VMEDataWidth::D16);
    sb.addCompareLoopAccu(AccuComparator::EQ, 0);

    // Now read the flash response from the flash output fifo.
    for (unsigned i=0; i<ExpectedFlashResponseSize; ++i)
        sb.addVMERead(moduleBase + OutputFifoRegister, vme_amods::A32, VMEDataWidth::D16);

    logger->debug("write_page4(): performing stackTransaction: pageSize={} bytes, stackCommands={}"
                  ", encodedStackSize={} words",
                  pageBuffer.size(), sb.commandCount(), get_encoded_stack_size(sb));

    std::vector<u32> stackResponse;

    if (auto ec = mvlc.stackTransaction(sb, stackResponse))
    {
        logger->error("write_page4(): stackTransaction failed: {}", ec.message());
        return ec;
    }

    logger->debug("write_page4(): response from stackTransaction: size={}, data={:#08x}",
                    stackResponse.size(), fmt::join(stackResponse, ", "));

    if (stackResponse.size() != ExpectedStackResponseSize)
    {
        logger->error("write_page4(): stack response too short! got {} words, expected {} words",
            stackResponse.size(), ExpectedStackResponseSize);
        return make_error_code(std::errc::protocol_error);
    }

    if (extract_frame_info(stackResponse[0]).flags & frame_flags::AllErrorFlags)
    {
        if (extract_frame_info(stackResponse[0]).flags & frame_flags::Timeout)
            return MVLCErrorCode::NoVMEResponse;

        if (extract_frame_info(stackResponse[0]).flags & frame_flags::SyntaxError)
            return MVLCErrorCode::StackSyntaxError;

        // Note: BusError can not happen as there's no block read in the stack
    }

    if (stackResponse[1] != StackReferenceMarker)
    {
        logger->error("write_page4(): stack response does not start with the reference marker");
        return MVLCErrorCode::StackReferenceMismatch;
    }

    std::vector<u8> flashResponse;
    std::copy(std::begin(stackResponse) + 2, std::end(stackResponse), std::back_inserter(flashResponse));

    if (!check_response(EfwRequest, flashResponse))
    {
        logger->error("write_page4(): flash check_response() failed");
        return make_error_code(std::errc::protocol_error);
    }

    //if (auto ec = clear_output_fifo(mvlc, moduleBase))
    //    return ec;

#if 0
    //  Expect the 0xF3 stack frame and the marker word
    if (stackResponse.size() != 2)
        return make_error_code(MVLCErrorCode::UnexpectedResponseSize);

    if (extract_frame_info(stackResponse[0]).flags & frame_flags::AllErrorFlags)
    {
        if (extract_frame_info(stackResponse[0]).flags & frame_flags::Timeout)
            return MVLCErrorCode::NoVMEResponse;

        if (extract_frame_info(stackResponse[0]).flags & frame_flags::SyntaxError)
            return MVLCErrorCode::StackSyntaxError;

        // Note: BusError can not happen as there's no block read in the stack
    }

    #if 0
    std::vector<u8> flashResponse;
    if (auto ec = read_response(mvlc, moduleBase, flashResponse))
    {
        logger->error("write_page4(): error reading flash response from 0x{:08x}: {}",
            moduleBase, ec.message());
        return ec;
    }

    logger->trace("write_page4(): flash response from 0x{:08x}: size={}, data={:#02x}",
        moduleBase, flashResponse.size(), fmt::join(flashResponse, ", "));
    #elif 0
    // TODO: get rid of this, handle it above with read_response() and check_response()
    if (auto ec = clear_output_fifo(mvlc, moduleBase))
        return ec;

    #else
    // Manually read the StatusRegister to figure out how it works

    for (int i=0; i<10; ++i)
    {
        u32 statusValue = 0u;
        if (auto ec = mvlc.vmeRead(moduleBase + StatusRegister, statusValue, vme_amods::A32, VMEDataWidth::D16))
            return ec;
        spdlog::warn("write_page4(): polled flash StatusRegister: {:#04x} (poll2)", statusValue);
        if (statusValue != 0)
            break;
    }

    if (auto ec = clear_output_fifo(mvlc, moduleBase))
        return ec;

    for (int i=0; i<10; ++i)
    {
        u32 statusValue = 0u;
        if (auto ec = mvlc.vmeRead(moduleBase + StatusRegister, statusValue, vme_amods::A32, VMEDataWidth::D16))
            return ec;
        spdlog::warn("write_page4(): polled flash StatusRegister: {:#04x} (poll3)", statusValue);
        if (statusValue != 0)
            break;
    }

    #endif
#endif

    auto tEnd = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(tEnd - tStart);
    logger->info("write_page4(): took {} ms to write {} bytes of data",
                 elapsed.count()/1000.0, pageBuffer.size());

    return {};
}

std::error_code erase_section(
    MVLC &mvlc, u32 moduleBase, u8 index)
{
    auto logger = mvlc::get_logger("mvlc_mvp_lib");
    if (auto ec = enable_flash_write(mvlc, moduleBase))
        return ec;

    std::vector<u8> instr = { 0x90, 0, 0, 0, index };
    std::vector<u8> response;

    if (auto ec = write_instruction(mvlc, moduleBase, instr))
        return ec;

    if (auto ec = read_response(mvlc, moduleBase, response))
        return ec;

    logger->info("Response from erase instruction: {:02x}", fmt::join(response, ", "));

    if (instr != response)
    {
        throw std::runtime_error(fmt::format(
                "Unexpected response from erase command: {:02x}",
                fmt::join(response, ", ")));
    }

    auto tStart = std::chrono::steady_clock::now();
    u32 outputFifoValue = 0u;
    u32 loops = 0u;

    // Poll until InvalidRead flag is set
    logger->info("Polling until InvalidRead is set");
    do
    {
        if (auto ec = read_output_fifo(mvlc, moduleBase, outputFifoValue))
            return ec;

        ++loops;
    } while (!(outputFifoValue & output_fifo_flags::InvalidRead));

    logger->info("Done polling until InvalidRead is set, loops={}", loops);
    loops = 0u;

    // Now poll until InvalidRead is not set anymore
    logger->info("Polling until InvalidRead is cleared");
    do
    {
        if (auto ec = read_output_fifo(mvlc, moduleBase, outputFifoValue))
            return ec;

        ++loops;
    } while (outputFifoValue & output_fifo_flags::InvalidRead);

    logger->info("Done polling until InvalidRead is cleared, loops={}", loops);
    loops = 0u;

    // outputFifofValue should now contain the flash response code 0xff
    if (outputFifoValue != 0xff)
        throw std::runtime_error(fmt::format(
                "Invalid flash response code 0x{:02x}, expected 0xff", outputFifoValue));

    // Read the flash response status
    if (auto ec = mvlc.vmeRead(moduleBase + OutputFifoRegister, outputFifoValue,
                               vme_amods::A32, VMEDataWidth::D16))
        return ec;

    if (!(outputFifoValue & FlashInstructionSuccess))
        throw std::runtime_error(fmt::format("Flash instruction not successful, code = 0x{:02x}",
                                             outputFifoValue));

    auto tEnd = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart);

    logger->info("flash response status ok, erasing took {} ms", elapsed.count());

    return {};
}

}
