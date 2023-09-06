#ifndef __MESYTEC_MVLC_MVLC_READOUT_PARSER_UTIL_H__
#define __MESYTEC_MVLC_MVLC_READOUT_PARSER_UTIL_H__

#include <atomic>
#include <ostream>

#include "mesytec-mvlc/mesytec-mvlc_export.h"
#include "mesytec-mvlc/mvlc_readout_parser.h"
#include "mesytec-mvlc/readout_buffer_queues.h"
#include "mesytec-mvlc/util/protected.h"

namespace mesytec
{
namespace mvlc
{
namespace readout_parser
{

// Driver function intended to run the readout parser in its own thread.
// Readout buffers are taken from the bufferQueues and are passed to the readout
// parser. Once a buffer has been processed it is re-enqueued on the empty
// buffer queue.

// Call like this:
//  parserThread = std::thread(
//      run_readout_parser,
//      std::ref(parserState),
//      std::ref(parserCounters),
//      std::ref(snoopQueues),
//      std::ref(parserCallbacks));
//
// To terminate set the atomic 'quit' to true.
void MESYTEC_MVLC_EXPORT run_readout_parser(
    readout_parser::ReadoutParserState &state,
    Protected<readout_parser::ReadoutParserCounters> &counters,
    ReadoutBufferQueues &bufferQueues,
    readout_parser::ReadoutParserCallbacks &parserCallbacks,
    std::atomic<bool> &quit);

MESYTEC_MVLC_EXPORT std::ostream &print_counters(
    std::ostream &out, const ReadoutParserCounters &counters);

} // end namespace readout_parser
} // end namespace mvlc
} // end namespace mesytec

#endif /* __MESYTEC_MVLC_MVLC_READOUT_PARSER_UTIL_H__ */
