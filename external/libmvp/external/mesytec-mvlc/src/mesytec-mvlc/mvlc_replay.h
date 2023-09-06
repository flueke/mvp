#ifndef __MESYTEC_MVLC_MVLC_REPLAY_H__
#define __MESYTEC_MVLC_MVLC_REPLAY_H__

#include <future>

#include "mesytec-mvlc/mesytec-mvlc_export.h"

#include "mesytec-mvlc/mvlc_readout_parser.h"
#include "mesytec-mvlc/mvlc_replay_worker.h"

namespace mesytec
{
namespace mvlc
{

class MESYTEC_MVLC_EXPORT MVLCReplay
{
    public:
        MVLCReplay(MVLCReplay &&other);
        MVLCReplay &operator=(MVLCReplay &&other);

        MVLCReplay(MVLCReplay &other) = delete;
        MVLCReplay &operator=(MVLCReplay &other) = delete;

        ~MVLCReplay();

        std::error_code start();
        std::error_code stop();
        std::error_code pause();
        std::error_code resume();

        bool finished();

        ReplayWorker::State workerState() const;
        // FIXME: waitableState() == Idle and finished() which checks the
        // internal buffer queue should return the same result at the same
        // point in time. This is not true right now and misleading.
        WaitableProtected<ReplayWorker::State> &waitableState();
        ReplayWorker::Counters workerCounters();
        readout_parser::ReadoutParserCounters parserCounters();
        const CrateConfig &crateConfig() const;

        ReplayWorker &replayWorker();
        std::thread &parserThread();
        std::atomic<bool> &parserQuit();

    private:
        MVLCReplay();

        struct Private;
        std::unique_ptr<Private> d;

        friend MVLCReplay make_mvlc_replay(
            const std::string &listfileFilename,
            readout_parser::ReadoutParserCallbacks parserCallbacks,
            int crateIndex,
            void *userContext);

        friend MVLCReplay make_mvlc_replay(
            const std::string &listfileArchiveName,
            const std::string &listfileArchiveMemberName,
            readout_parser::ReadoutParserCallbacks parserCallbacks,
            int crateIndex,
            void *userContext);

        friend MVLCReplay make_mvlc_replay(
            listfile::ReadHandle *lfh,
            readout_parser::ReadoutParserCallbacks parserCallbacks,
            int crateIndex,
            void *userContext);

        friend void init_common(MVLCReplay &rdo, int crateIndex, void *userContext);
};

MVLCReplay MESYTEC_MVLC_EXPORT make_mvlc_replay(
    const std::string &listfileArchiveName,
    readout_parser::ReadoutParserCallbacks parserCallbacks,
    int crateIndex = 0,
    void *userContext = nullptr);

MVLCReplay MESYTEC_MVLC_EXPORT make_mvlc_replay(
    const std::string &listfileArchiveName,
    const std::string &listfileArchiveMemberName,
    readout_parser::ReadoutParserCallbacks parserCallbacks,
    int crateIndex = 0,
    void *userContext = nullptr);

MVLCReplay MESYTEC_MVLC_EXPORT make_mvlc_replay(
    listfile::ReadHandle *lfh,
    readout_parser::ReadoutParserCallbacks parserCallbacks,
    int crateIndex = 0,
    void *userContext = nullptr);

} // end namespace mvlc
} // end namespace mesytec

#endif /* __MESYTEC_MVLC_MVLC_REPLAY_H__ */
