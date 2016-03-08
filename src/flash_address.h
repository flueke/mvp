#ifndef UUID_3c4e97fa_29aa_40c6_8389_28c546f52900
#define UUID_3c4e97fa_29aa_40c6_8389_28c546f52900

#include "flash_constants.h"

namespace mesytec
{
namespace mvp
{
  class Address
  {
    public:
      Address() = default;

      Address(uchar a0, uchar a1, uchar a2): _data({{a0, a1, a2}}) {}

      Address(const Address &o): _data(o._data) {}

      explicit Address(uint32_t a) { set_value(a); }

      uchar a0() const { return _data[0]; }
      uchar a1() const { return _data[1]; }
      uchar a2() const { return _data[2]; }

      void set_value(uint32_t a)
      {
        //qDebug() << "Address::set_value() value =" << a;

        if (a > constants::address_max)
          throw std::out_of_range("address range exceeded");

        _data = {{
          gsl::narrow_cast<uchar>((a & 0x0000ff)),
          gsl::narrow_cast<uchar>((a & 0x00ff00) >> 8),
          gsl::narrow_cast<uchar>((a & 0xff0000) >> 16)
        }};
      }

      uchar operator[](size_t idx) const
      {
        if (idx >= size())
          throw std::out_of_range("address index out of range");
        return _data[idx];
      }

      Address &operator++()
      {
        if (*this == Address(constants::address_max))
          throw std::overflow_error("address range exceeded");

        if (++_data[0] == 0)
          if (++_data[1] == 0)
            ++_data[2];

        return *this;
      }

      Address operator++(int)
      {
        auto ret(*this);
        operator++();
        return ret;
      }

      bool operator==(const Address &o) const
      { return _data == o._data; }

      bool operator!=(const Address &o) const
      { return !operator==(o); }

      constexpr size_t size() const { return 3; }

      uint32_t to_int() const
      { return _data[0] | _data[1] << 8 | _data[2] << 16; }

      bool operator>(const Address &o) const
      { return to_int() > o.to_int(); }

      bool operator<(const Address &o) const
      { return to_int() < o.to_int(); }

      Address operator+(const Address &other) const
      {
        return Address(to_int() + other.to_int());
      }

      Address operator-(const Address &other) const
      {
        return Address(to_int() - other.to_int());
      }

      Address &operator+=(int i)
      {
        set_value(to_int() + i);
        return *this;
      }

      Address operator+(int n) const
      {
        Address a(*this);
        a += n;
        return a;
      }

    private:
      std::array<uchar, 3> _data = {{0, 0, 0}};
  };

  QDebug operator<<(QDebug dbg, const Address &a);

} // ns mvp
} // ns mesytec

#endif
