#ifndef FLOATING_POINT_H_
#define FLOATING_POINT_H_
#include <stdint.h>

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

class FloatingPoint
{
private:
    uint32_t value;
    uint32_t sign_with_mantissa;

    uint32_t get_mantissa() const
    {
        return ((~(1<<31)) & sign_with_mantissa);
    }

    uint8_t get_value_len(uint32_t v) const
    {
        if(v == 0) return 1;
        uint8_t len = 0;
        uint32_t temp = v;
        while(temp > 0)
        {
            ++len;
            temp /= 10;
        }
        return len;
    }


    uint8_t get_value_len() const
    {
        return get_value_len(value);
    }

    uint8_t get_digit(uint8_t pos) const
    {
        uint32_t temp = value;
        while(pos-- > 0)
        {
            temp /= 10;
        }
        return temp % 10;
    }

    void positive_mantissa_string(char* buff, uint32_t buff_size, uint32_t& i) const;
    void negative_mantissa_string(char* buff, uint32_t buff_size, uint32_t& i) const;

    void normalize_value()
    {
        while(value != 0 && (value % 10) == 0)
        {
            value /= 10;
        }
        if (value == 0)
        {
            sign_with_mantissa = (1 << 30);
        }
    }

    int32_t numbers_alignment(const FloatingPoint& second,
    						  uint32_t& max_offset_value,
							  uint32_t& min_offset_value) const;

public:
    FloatingPoint(const char* c);

    FloatingPoint(uint8_t sign, uint32_t mantissa, uint32_t value) :
            value(value), sign_with_mantissa(((sign ? 1 : 0) << 31) | mantissa)
    {
        normalize_value();
    }

    FloatingPoint(const FloatingPoint& floatingPoint) :
            value(floatingPoint.value), sign_with_mantissa(floatingPoint.sign_with_mantissa) {}

    void operator=(const FloatingPoint& floatingPoint)
    {
        this->value = floatingPoint.value;
        this->sign_with_mantissa = floatingPoint.sign_with_mantissa;
    }

    uint8_t get_sign() const
    {
        return sign_with_mantissa >> 31;
    }

    void to_string(char* buff, uint32_t buff_size) const;

    bool operator==(const FloatingPoint& second) const
    {
        return (this->sign_with_mantissa == second.sign_with_mantissa && this->value == second.value);
    }

    bool operator!=(const FloatingPoint& second) const
    {
        return !(*this == second);
    }

    bool operator>(const FloatingPoint& second) const;

    FloatingPoint negative() const
    {
        return FloatingPoint(!this->get_sign(), this->get_mantissa(), this->value);
    }

    FloatingPoint absolute() const
    {
        return FloatingPoint(0, this->get_mantissa(), this->value);
    }

    FloatingPoint operator+(const FloatingPoint& second) const;

    FloatingPoint operator-(const FloatingPoint& second) const
    {
        return (*this) + second.negative();
    }

    FloatingPoint operator*(const FloatingPoint& second) const;

    FloatingPoint operator/(const FloatingPoint& second) const;
};

#endif
