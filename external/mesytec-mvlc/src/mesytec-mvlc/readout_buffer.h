#ifndef __MESYTEC_MVLC_UTIL_READOUT_BUFFER_H__
#define __MESYTEC_MVLC_UTIL_READOUT_BUFFER_H__

#include <cassert>
#include <vector>

#include "mesytec-mvlc/mesytec-mvlc_export.h"
#include "mesytec-mvlc/mvlc_constants.h"
#include "mesytec-mvlc/util/string_view.hpp"

namespace mesytec
{
namespace mvlc
{

class MESYTEC_MVLC_EXPORT ReadoutBuffer
{
    public:
        // BufferType value which can be used to indicate the end of the buffer
        // stream.
        static const s32 EndOfStream = -1;

        explicit ReadoutBuffer(size_t capacity = 0)
            : m_buffer(capacity)
        { }

        s32 type() const { return m_type; }
        void setType(s32 t) { m_type = t; }
        void setType(ConnectionType t) { setType(static_cast<s32>(t)); }

        size_t bufferNumber() const { return m_number; }
        void setBufferNumber(size_t number) { m_number = number; }

        size_t capacity() const { return m_buffer.size(); }
        size_t used() const { return m_used; }
        size_t free() const { return capacity() - m_used; }

        bool empty() const { return used() == 0; }

        void ensureFreeSpace(size_t freeSpace)
        {
            if (free() < freeSpace)
                m_buffer.resize(m_used + freeSpace);
            assert(free() >= freeSpace);
        }

        void clear() { m_used = 0u; }

        void use(size_t bytes)
        {
            assert(m_used + bytes <= capacity());
            m_used += bytes;
        }

        void setUsed(size_t bytes)
        {
            assert(bytes <= capacity());
            m_used = bytes;
        }

        const std::vector<u8> &buffer() const { return m_buffer; }
        std::vector<u8> &buffer() { return m_buffer; }

        const u8 *data() const { return buffer().data(); }
        u8 *data() { return buffer().data(); }

        nonstd::basic_string_view<const u8> viewU8() const
        {
            return nonstd::basic_string_view<const u8>(m_buffer.data(), m_used);
        }

        nonstd::basic_string_view<const u32> viewU32() const
        {
            return nonstd::basic_string_view<const u32>(
                reinterpret_cast<const u32 *>(m_buffer.data()),
                m_used / sizeof(u32));
        }

        template<typename T>
        void push_back(const T &t)
        {
            static_assert(std::is_trivial<T>::value, "T must be a trivial type");
            ensureFreeSpace(sizeof(t));
            auto begin = reinterpret_cast<const u8 *>(&t);
            auto end = begin + sizeof(t);
            std::copy(begin, end, data() + used());
            use(sizeof(t));
        }

    private:
        s32 m_type = static_cast<s32>(ConnectionType::ETH);
        size_t m_number = 0;
        std::vector<u8> m_buffer;
        size_t m_used = 0;
};

} // end namespace mvlc
} // end namespace mesytec

#endif /* __MESYTEC_MVLC_UTIL_READOUT_BUFFER_H__ */
