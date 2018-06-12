#include "mdpp16.h"
#include <QTextStream>
#include <QString>
#include <boost/endian/conversion.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <QDebug>

namespace mesytec
{
namespace mvp
{

namespace mdpp16
{

void format_calibration_data(const gsl::span<uchar> data, QTextStream &out)
{
  out << "offsets:" << endl;
  format_offsets(data, out);

  out << "prediffs:" << endl;
  format_prediffs({ std::begin(data) + offsets::total_bytes, std::end(data) }, out);
}

void format_offsets(const gsl::span<uchar> data, QTextStream &out)
{
  using namespace offsets;

  Expects(data.size() >= total_bytes);

  for (size_t channel=0; channel<n_channels; ++channel) {

    for (size_t gain=0; gain<n_gains_total; ++gain) {
      if (gain >= n_gains_used)
        continue;

      out << "c=" << qSetFieldWidth(2) << channel << qSetFieldWidth(0)
        << " g=" << gain << ": ";

      for (size_t prediff=0; prediff<n_prediffs_total; ++prediff) {
        if (prediff >= n_prediffs_used)
          continue;

        size_t word_offset = (prediff | (gain << 3) | (channel << 7));
        size_t byte_offset = word_offset * word_bytes;

        uint32_t value = boost::endian::little_to_native(
            *(reinterpret_cast<uint32_t *>(data.data() + byte_offset)));

        out << qSetFieldWidth(6) << value << qSetFieldWidth(0);

        if (prediff == n_prediffs_used-1)
          out << endl;
        else
          out << " ";
      }
    }
  }
}

void format_prediffs(const gsl::span<uchar> data, QTextStream &out)
{
  using namespace prediffs;

  Expects(data.size() >= total_bytes);

  for (size_t channel=0; channel<n_channels; ++channel) {

    out << "c=" << qSetFieldWidth(2) << channel << qSetFieldWidth(0) << ": ";

    for (size_t prediff=0; prediff<n_prediffs_total; ++prediff) {
      if (prediff >= n_prediffs_used)
        continue;

      size_t word_offset = (prediff | (channel << 2));
      size_t byte_offset = word_offset * word_bytes;

      uint32_t value = boost::endian::little_to_native(
          *(reinterpret_cast<uint32_t *>(data.data() + byte_offset)));

      out << qSetFieldWidth(6) << value << qSetFieldWidth(0);

      if (prediff == n_prediffs_used-1)
        out << endl;
      else
        out << " ";
    }
  }
}

} // ns mdpp16

namespace mdpp32
{

void format_calibration_data(const gsl::span<uchar> data, QTextStream &out)
{
  out << "offsets:" << endl;
  format_offsets(data, out);

  qDebug() << __PRETTY_FUNCTION__ << "offsets::total_bytes_needed =" << offsets::total_bytes_needed;

  out << "prediffs:" << endl;
  format_prediffs({ std::begin(data) + offsets::total_bytes_needed, std::end(data) }, out);
}

void format_offsets(const gsl::span<uchar> data, QTextStream &out)
{
    using namespace offsets;

    qDebug() << __PRETTY_FUNCTION__ << "offsets::total_bytes_needed =" << total_bytes_needed;

    Expects(data.size() >= total_bytes_needed);

    for (size_t channel = 0; channel < n_channels; channel++)
    {
        size_t channel_block_start = channel_block_words * channel;
        size_t word_offset = channel_block_start;

        for (size_t gain = 0; gain < n_gains; gain++)
        {
            out << "c=" << qSetFieldWidth(2) << channel
                << qSetFieldWidth(0) << " g=" << gain << ": ";

            for (size_t prediff = 0; prediff < n_prediffs; prediff++, word_offset++)
            {
                size_t byte_offset = word_offset * word_bytes;

                //qDebug() << __PRETTY_FUNCTION__
                //    << "c=" << channel << "g=" << gain << "p=" << prediff
                //    << "word offset" << word_offset
                //    << "byte offset" << byte_offset;

                Expects(byte_offset < data.size());

                uint32_t value = boost::endian::little_to_native(
                    *(reinterpret_cast<uint32_t *>(data.data() + byte_offset)));

                out << qSetFieldWidth(6) << value << qSetFieldWidth(0);

                if (prediff == n_prediffs - 1)
                    out << endl;
                else
                    out << " ";
            }
        }
    }
}

void format_prediffs(const gsl::span<uchar> data, QTextStream &out)
{
    using namespace prediffs;

    qDebug() << __PRETTY_FUNCTION__ << "prediffs::total_bytes_needed =" << total_bytes_needed
        << "data.size()=" << data.size();

    Expects(data.size() >= total_bytes_needed);

    for (size_t channel=0; channel < n_channels; channel++)
    {
        size_t channel_block_start = channel_block_words * channel;
        size_t word_offset = channel_block_start;

        out << "c=" << qSetFieldWidth(2) << channel << qSetFieldWidth(0) << ": ";

        for (size_t prediff = 0; prediff < n_prediffs; prediff++, word_offset++)
        {
            size_t byte_offset = word_offset * word_bytes;

            //qDebug() << __PRETTY_FUNCTION__
            //    << "c=" << channel << "p=" << prediff
            //    << "word offset" << word_offset
            //    << "byte offset" << byte_offset;

            Expects(byte_offset < data.size());

            uint32_t value = boost::endian::little_to_native(
                *(reinterpret_cast<uint32_t *>(data.data() + byte_offset)));

            out << qSetFieldWidth(6) << value << qSetFieldWidth(0);

            if (prediff == n_prediffs - 1)
                out << endl;
            else
                out << " ";
        }
    }
}

} // ns mdpp32

} // ns mvp
} // ns mesytec
