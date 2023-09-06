#ifndef __MESYTEC_MVLC_LOGGING_H__
#define __MESYTEC_MVLC_LOGGING_H__

#include <string>
#include <spdlog/spdlog.h>

#include "mesytec-mvlc/mesytec-mvlc_export.h"

namespace mesytec
{
namespace mvlc
{

// Creates a new named, thread safe logger or returns the existing instance.
std::shared_ptr<spdlog::logger> MESYTEC_MVLC_EXPORT
    get_logger(const std::string &name);

//
// Creates a new named logger or returns an existing instance. If a new logger
// is created the provided sinks are attached to the new instance.
std::shared_ptr<spdlog::logger> MESYTEC_MVLC_EXPORT
    create_logger(const std::string &name, const std::vector<spdlog::sink_ptr> &sinks = {});

void MESYTEC_MVLC_EXPORT
    set_global_log_level(spdlog::level::level_enum level);

MESYTEC_MVLC_EXPORT std::vector<std::string> list_logger_names();

template<typename View>
void log_buffer(const std::shared_ptr<spdlog::logger> &logger,
                const spdlog::level::level_enum &level,
                const View &buffer,
                const std::string &header,
                size_t numWordsBegin = 0)
{
    if (!logger->should_log(level))
        return;

    if (numWordsBegin == 0)
        numWordsBegin = buffer.size();

    numWordsBegin = std::min(numWordsBegin, buffer.size());

    if (numWordsBegin == buffer.size())
        logger->log(level, "begin buffer '{}' (size={})", header, buffer.size());
    else
        logger->log(level, "begin buffer '{}' (size={}, first {} words)", header, buffer.size(), numWordsBegin);

    for (size_t i=0; i<numWordsBegin; ++i)
        logger->log(level, "  0x{:008X}", buffer[i]);

    auto wordsLeft = buffer.size() - numWordsBegin;

    if (wordsLeft == 0)
        logger->log(level, "end buffer '{}' (size={})", header, buffer.size());
    else
        logger->log(level, "end buffer '{}' (size={}, {} words not logged)", header, buffer.size(), wordsLeft);

}

} // end namespace mvlc
} // end namespace mesytec

#endif /* __MESYTEC_MVLC_LOGGING_H__ */
