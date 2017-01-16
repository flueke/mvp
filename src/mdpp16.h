#ifndef UUID_ff7816fc_f3c7_440c_b9b7_28fc156bdfc0
#define UUID_ff7816fc_f3c7_440c_b9b7_28fc156bdfc0

#include <gsl.h>
#include <QtGlobal>

class QTextStream;

namespace mesytec
{
namespace mvp
{
namespace mdpp16
{
  const size_t n_channels   = 16;
  const size_t channel_bits =  4;
  const size_t word_bytes   =  4;

  // offset values organized like this:
  // offsets[channel][gain][prediff]
  // offsets[16][10][5]
  // empty slots need to be skipped
  namespace offsets
  {
    const size_t gain_bits          =  4;
    const size_t n_gains_used       = 10;
    const size_t n_gains_total      = 1u << gain_bits;

    const size_t prediff_bits       =  3;
    const size_t n_prediffs_used    =  5;
    const size_t n_prediffs_total   = 1u << prediff_bits;

    const size_t total_bits         = channel_bits + gain_bits + prediff_bits;
    const size_t total_bytes        = (1u << total_bits) * word_bytes;
  }

  // prediff values organized like this:
  // prediffs[channel][#prediff]
  // prediffs[16][3]
  // empty slots need to be skipped
  namespace prediffs
  {
    const size_t prediff_bits     = 2;
    const size_t n_prediffs_used  = 4;
    const size_t n_prediffs_total = 1u << prediff_bits;
    const size_t total_bits       = channel_bits + prediff_bits;
    const size_t total_bytes      = (1u << total_bits) * word_bytes;
  }

  const size_t calib_data_size = offsets::total_bytes + prediffs::total_bytes;

  void format_calibration_data(const gsl::span<uchar> data, QTextStream &out);
  void format_offsets(const gsl::span<uchar> data, QTextStream &out);
  void format_prediffs(const gsl::span<uchar> data, QTextStream &out);

} // ns mdpp16
} // ns mvp
} // ns mesytec

#endif
