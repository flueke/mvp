#ifndef __MESYTEC_MVLC_MVLC_ERROR_H__
#define __MESYTEC_MVLC_MVLC_ERROR_H__

#include <system_error>
#include "mesytec-mvlc/mesytec-mvlc_export.h"

namespace mesytec
{
namespace mvlc
{

/* Lower level MVLC specific error codes. In addition to this the specific
 * implementations (USB, UDP) use their own detailed error codes. */
enum class MVLCErrorCode
{
    NoError,
    IsConnected,
    IsDisconnected,
    ShortWrite,
    ShortRead,
    MirrorEmptyRequest,  // size of the request < 1
    MirrorEmptyResponse, // size of the mirror response < 1
    MirrorShortResponse,
    MirrorNotEqual,
    MirrorMaxTriesExceeded,
    InvalidBufferHeader,
    ReadResponseMaxWaitExceeded,
    UnexpectedResponseSize, // wanted N words, got M words
    CommandArgOutOfRange,
    InvalidPipe,
    NoVMEResponse,
    VMEBusError,
    HostLookupError,
    EmptyHostname,
    BindLocalError,
    SocketError,
    SocketReadTimeout,
    SocketWriteTimeout,
    UDPPacketChannelOutOfRange,
    UDPDataWordCountExceedsPacketSize, // dataWordCount in header0 exceeds received packet length
    StackCountExceeded,
    StackMemoryExceeded,
    ImmediateStackReservedMemoryExceeded,
    StackSyntaxError,
    StackSegmentSizeExceeded,
    Stack0IsReserved,
    MirrorTransactionMaxWordsExceeded,
    InvalidStackHeader,
    NonBlockAddressMode,

    // Readout setup releated (e.g. mvlc_daq.cc)
    TimerCountExceeded,
    ReadoutSetupError,

    UnexpectedBufferHeader, // mvlc_dialog.cc

    // Returned by the ETH implementation on connect if it detects that any of
    // the triggers are enabled.
    InUse,

    // USB specific error code to indicate that the FTDI chip configuration is
    // not correct.
    USBChipConfigError,

    // Added for mvlc_apiv2.cc
    SuperCommandTimeout,
    StackCommandTimeout,
    ShortSuperFrame,
    SuperFormatError,
    StackFormatError,
    SuperReferenceMismatch,
    StackReferenceMismatch,
};

MESYTEC_MVLC_EXPORT std::error_code make_error_code(MVLCErrorCode error);

/* The higher level error condition used to categorize the errors coming from
 * the MVLC logic code and the low level implementations. */
enum class ErrorType
{
    Success,
    ConnectionError,
    Timeout,
    ShortTransfer,
    ProtocolError,
    VMEError
};

MESYTEC_MVLC_EXPORT std::error_condition make_error_condition(ErrorType et);

} // end namespace mvlc
} // end namespace mesytec

namespace std
{
    template<> struct is_error_code_enum<mesytec::mvlc::MVLCErrorCode>: true_type {};

    template<> struct is_error_condition_enum<mesytec::mvlc::ErrorType>: true_type {};
} // end namespace std

namespace mesytec
{
namespace mvlc
{

inline bool is_vme_error(const std::error_code &ec)
{
    return ec == ErrorType::VMEError;
}

inline bool is_connection_error(const std::error_code &ec)
{
    return ec == ErrorType::ConnectionError;
}

inline bool is_protocol_error(const std::error_code &ec)
{
    return ec == ErrorType::ProtocolError;
}

inline bool is_timeout(const std::error_code &ec)
{
    return ec == ErrorType::Timeout;
}

} // end namespace mvlc
} // end namespace mesytec

#endif /* __MESYTEC_MVLC_MVLC_ERROR_H__ */
