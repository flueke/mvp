#include "mdpp16.h"
#include <QTextStream>
#include <QDebug>
#include <QString>
#include <boost/endian/conversion.hpp>
#include <boost/format.hpp>
#include <iostream>

namespace mesytec
{
namespace mvp
{
namespace mdpp16
{

void format_calibration_data(const gsl::span<uchar> data, QTextStream &out)
{
  for (size_t channel=0; channel<n_channels; ++channel) {

    for (size_t gain=0; gain<n_gains_total; ++gain) {
      if (gain >= n_gains)
        continue;

      out << "c=" << channel << " g=" << gain << ": ";

      for (size_t prediff=0; prediff<n_prediffs_total; ++prediff) {
        if (prediff >= n_prediffs)
          continue;

        size_t word_offset = (prediff | (gain << 3) | (channel << 7));
        size_t byte_offset = word_offset * word_bytes;

        //uint32_t value = boost::endian::big_to_native(
        //    *(reinterpret_cast<uint32_t *>(data.data() + byte_offset)));

        uint32_t value = (
            *(reinterpret_cast<uint32_t *>(data.data() + byte_offset)));

        out << value;

        if (prediff == n_prediffs-1)
          out << endl;
        else
          out << " ";

        /*
        qDebug()
          << "c"  << channel
          << "g"  << gain
          << "p"  << prediff
          << "wo" << word_offset
          << "bo" << byte_offset << QString::fromStdString((boost::format("%|1$04x|") % byte_offset).str())
          << "v"  << value << QString::fromStdString((boost::format("%|1$08x|") % value).str())
          ;
          */
      }
    }
  }
}

} // ns mdpp16
} // ns mvp
} // ns mesytec
