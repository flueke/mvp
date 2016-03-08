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
  const uchar calibration_section = 3;
  const size_t word_bytes         =  4;

  const size_t channel_bits       =  4;
  const size_t n_channels         = 16;

  const size_t gain_bits          =  4;
  const size_t n_gains            = 10;
  const size_t n_gains_total      = 1u << gain_bits;

  const size_t prediff_bits       =  3;
  const size_t n_prediffs         =  5;
  const size_t n_prediffs_total   = 1u << prediff_bits;

  const size_t offset_bits        = channel_bits + gain_bits + prediff_bits;
  const size_t calib_data_size    = (1u << offset_bits) * word_bytes;

  void format_calibration_data(const gsl::span<uchar> data, QTextStream &out);
} // ns mdpp16
} // ns mvp
} // ns mesytec

#endif
