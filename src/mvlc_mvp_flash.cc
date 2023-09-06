#include "mvlc_mvp_flash.h"
#include "mvlc_mvp_lib.h"

using namespace mesytec::mvlc;
using namespace mesytec::mvp;

namespace mesytec::mvp
{

MvlcMvpFlash::~MvlcMvpFlash()
{
    if (isFlashEnabled_)
        disable_flash_interface(mvlc_, vmeAddress_);
}

void MvlcMvpFlash::setMvlc(MVLC &mvlc)
{
    mvlc_ = mvlc;
    isFlashEnabled_ = false;
    m_write_enabled = false;
}

MVLC MvlcMvpFlash::getMvlc() const
{
    return MVLC();
}

void MvlcMvpFlash::setVmeAddress(u32 vmeAddress)
{
    vmeAddress_ = vmeAddress;
    isFlashEnabled_ = false;
    m_write_enabled = false;
}

u32 MvlcMvpFlash::getVmeAddress() const
{
    return vmeAddress_;
}

void MvlcMvpFlash::maybe_enable_flash_interface()
{
    if (!isFlashEnabled_)
    {
        if (auto ec = enable_flash_interface(mvlc_, vmeAddress_))
            throw std::system_error(ec);
        isFlashEnabled_ = true;
    }
}

void MvlcMvpFlash::write_instruction(const gsl::span<uchar> data, int timeout_ms)
{
    (void) timeout_ms;
    maybe_enable_flash_interface();

    if (auto ec = mesytec::mvp::write_instruction(mvlc_, vmeAddress_, data))
        throw std::system_error(ec);

    emit instruction_written(span_to_qvector(data));
}

void MvlcMvpFlash::read_response(gsl::span<uchar> dest, int timeout_ms)
{
    (void) timeout_ms;
    maybe_enable_flash_interface();

    std::vector<u8> tmpDest;
    if (auto ec = mesytec::mvp::read_response(mvlc_, vmeAddress_, tmpDest))
        throw std::system_error(ec);

    std::copy(std::begin(tmpDest), std::begin(tmpDest) + std::min(tmpDest.size(), dest.size()), std::begin(dest));

    emit response_read(span_to_qvector(dest));
}

void MvlcMvpFlash::write_page(const Address &address, uchar section, const gsl::span<uchar> data, int timeout_ms)
{
    (void) timeout_ms;
    maybe_enable_flash_interface();
    maybe_set_verbose(false);
    maybe_enable_write();

    std::vector<u8> pageBuffer;
    pageBuffer.reserve(data.size());
    std::copy(std::begin(data), std::end(data), std::back_inserter(pageBuffer));
    if (auto ec = mesytec::mvp::write_page4(mvlc_, vmeAddress_, address.data() , section, pageBuffer))
        throw std::system_error(ec);

    emit data_written(span_to_qvector(data));
}

void MvlcMvpFlash::read_page(const Address &address, uchar section, gsl::span<uchar> dest, int timeout_ms)
{
    (void) timeout_ms;
    maybe_enable_flash_interface();
    maybe_set_verbose(false);

    std::vector<u8> pageBuffer;
    pageBuffer.reserve(dest.size());
    if (auto ec = mesytec::mvp::read_page(mvlc_, vmeAddress_, address.data(), section,
        dest.size(), pageBuffer))
        throw std::system_error(ec);

    std::copy(std::begin(pageBuffer), std::begin(pageBuffer) + std::min(pageBuffer.size(), dest.size()), std::begin(dest));
}

void MvlcMvpFlash::recover(size_t tries)
{
    std::exception_ptr last_nop_exception;

    for (auto n=0u; n<tries; ++n)
    {
        try
        {
            nop();
            return;
        }
        catch(const std::exception& e)
        {
            last_nop_exception = std::current_exception();
            clear_output_fifo(mvlc_, vmeAddress_);
        }
    }

    if (last_nop_exception)
        std::rethrow_exception(last_nop_exception);
    else
        throw std::runtime_error("NOP recovery failed for an unknown reason");
}

void MvlcMvpFlash::erase_section(uchar section)
{
    maybe_enable_flash_interface();
    maybe_enable_write();
    if (auto ec = mesytec::mvp::erase_section(mvlc_, vmeAddress_, section))
        throw std::system_error(ec);
}

}
