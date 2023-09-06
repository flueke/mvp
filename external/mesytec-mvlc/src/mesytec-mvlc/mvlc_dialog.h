/* mesytec-mvlc - driver library for the Mesytec MVLC VME controller
 *
 * Copyright (C) 2020-2023 mesytec GmbH & Co. KG <info@mesytec.com>
 *
 * Author: Florian Lüke <f.lueke@mesytec.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef __MESYTEC_MVLC_MVLC_DIALOG_H__
#define __MESYTEC_MVLC_MVLC_DIALOG_H__

#include <chrono>
#include <functional>
#include <system_error>
#include <vector>

#include "mesytec-mvlc/mesytec-mvlc_export.h"
#include "mesytec-mvlc/mvlc_basic_interface.h"
#include "mesytec-mvlc/mvlc_buffer_validators.h"
#include "mesytec-mvlc/mvlc_command_builders.h"
#include "mesytec-mvlc/mvlc_stack_errors.h"
#include "mesytec-mvlc/util/logging.h"
#include "mesytec-mvlc/util/protected.h"

// Higher level MVLC dialog (request/response) layer. Builds on top of the
// MVLCBasicInterface abstraction.

namespace mesytec
{
namespace mvlc
{

std::error_code check_mirror(const std::vector<u32> &request, const std::vector<u32> &response);

class MESYTEC_MVLC_EXPORT MVLCDialog_internal
{
    public:
        // Maximum time spent in readResponse() until a non-error-notification
        // response buffer is received.
        constexpr static auto ReadResponseMaxWait = std::chrono::milliseconds(60000);

        // Max number of retries for superTransaction() in case of
        // SocketReadTimeout or SocketWriteTimeout errors.
        // When running into one of the above timeout cases the whole
        // transaction is retried a maximum of MirrorMaxRetries times.
        constexpr static unsigned MirrorMaxRetries = 3;

        explicit MVLCDialog_internal(MVLCBasicInterface *mvlc);

        // MVLC register access
        std::error_code readRegister(u16 address, u32 &value);
        std::error_code writeRegister(u16 address, u32 value);
#if 0 // disabled for now. need to test if this is implemented in the firmware and working.
        std::error_code readRegisterBlock(u16 address, u16 words,
                                          std::vector<u32> &dest);
#endif

        // Higher level VME access
        // Note: Stack0 is used for the VME commands and the stack is written
        // starting from offset 0 into stack memory.

        std::error_code vmeRead(
            u32 address, u32 &value, u8 amod, VMEDataWidth dataWidth);

        std::error_code vmeWrite(
            u32 address, u32 value, u8 amod, VMEDataWidth dataWidth);

        // Note: The data from the block read is currently returned as is
        // including the stack frame (0xF3), stack continuation (0xF9) and
        // block frame (0xF5) headers. The flags, except for the Continue flag,
        // of any of these headers are not interpreted by this method.
        std::error_code vmeBlockRead(
            u32 address, u8 amod, u16 maxTransfers, std::vector<u32> &dest);

        std::error_code vmeMBLTSwapped(
            u32 address, u16 maxTransfers, std::vector<u32> &dest);

        // Command stack uploading

        std::error_code uploadStack(
            u8 stackOutputPipe,
            u16 stackMemoryOffset,
            const std::vector<StackCommand> &commands,
            std::vector<u32> &responseDest);

        inline std::error_code uploadStack(
            u8 stackOutputPipe,
            u16 stackMemoryOffset,
            const StackCommandBuilder &stack,
            std::vector<u32> &responseDest)
        {
            return uploadStack(stackOutputPipe, stackMemoryOffset, stack.getCommands(), responseDest);
        }

        // Immediate stack execution
        std::error_code execImmediateStack(u16 stackMemoryOffset, std::vector<u32> &responseDest);

        // Lower level utilities

        // Read a full response buffer into dest. The buffer header is passed
        // to the BufferHeaderValidator and MVLCErrorCode::InvalidBufferHeader
        // is returned if the validation fails (in this case the data will
        // still be available in the dest buffer for inspection).
        //
        // If no non-error buffer is received within ReadResponseMaxWait the
        // method returns MVLCErrorCode::UnexpectedBufferHeader
        //
        // Note: internally buffers are read from the MVLC until a
        // non-stack_error_notification type buffer is read. All error
        // notifications received up to that point are saved and can be queried
        // using getStackErrorNotifications().
        std::error_code readResponse(BufferHeaderValidator bhv, std::vector<u32> &dest);

        // Sends the given cmdBuffer to the MVLC then reads and verifies the
        // mirror response. The buffer must start with CmdBufferStart and end
        // with CmdBufferEnd, otherwise the MVLC cannot interpret it.
        std::error_code superTransaction(
            const std::vector<u32> &cmdBuffer, std::vector<u32> &responseDest);

        std::error_code superTransaction(
            const SuperCommandBuilder &superBuilder, std::vector<u32> &dest)
        {
            return superTransaction(make_command_buffer(superBuilder), dest);
        }

        // Sends the given stack data (which must include upload commands),
        // reads and verifies the mirror response, and executes the stack.
        // Notes:
        // - Stack0 is used and offset 0 into stack memory is assumed.
        // - Stack responses consisting of multiple frames (0xF3 followed by
        //   0xF9 frames) are supported. The stack frames will all be copied to
        //   the responseDest vector.
        // - Any stack error notifications read while attempting to read an
        //   actual stack response are available via
        //   getStackErrorNotifications().
        std::error_code stackTransaction(const std::vector<u32> &stackUploadData,
                                         std::vector<u32> &responseDest);

        // Low level read accepting any of the known buffer types (see
        // is_known_buffer_header()). Does not do any special handling for
        // stack error notification buffers as is done in readResponse().
        std::error_code readKnownBuffer(std::vector<u32> &dest);

        // Returns the response buffer used internally by readRegister(),
        // readRegisterBlock(), writeRegister(), vmeWrite() and
        // vmeRead().
        // The buffer will contain the last data received from the MVLC.
        std::vector<u32> getResponseBuffer() const { return m_responseBuffer; }

        StackErrorCounters getStackErrorCounters() const
        {
            return m_stackErrorCounters.copy();
        }

        Protected<StackErrorCounters> &getProtectedStackErrorCounters()
        {
            return m_stackErrorCounters;
        }

        void clearStackErrorCounters()
        {
            m_stackErrorCounters.access().ref() = {};
        }

    private:
        std::error_code doWrite(const std::vector<u32> &buffer);
        std::error_code readWords(u32 *dest, size_t count, size_t &wordsTransferred);

        void logBuffer(const std::vector<u32> &buffer, const std::string &info);

        MVLCBasicInterface *m_mvlc = nullptr;
        u16 m_referenceWord = 1;
        std::vector<u32> m_responseBuffer;
        mutable Protected<StackErrorCounters> m_stackErrorCounters;
        std::shared_ptr<spdlog::logger> m_logger;
};

} // end namespace mvlc
} // end namespace mesytec

#endif /* __MESYTEC_MVLC_MVLC_DIALOG_H__ */
