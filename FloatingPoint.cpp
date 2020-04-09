#include "FloatingPoint.h"

FloatingPoint::FloatingPoint(const char* c)
{
	this->value = 0;
	this->sign_with_mantissa = (1 << 30) - 1;
	bool was_dot = false;
	bool was_num = false;
	bool has_set_mantissa = false;
	for(;*c != 0; ++c)
	{
		if(*c == '-')
		{
			this->sign_with_mantissa |= 1 << 31;
			continue;
		}
		if(*c == '.')
		{
			was_dot = true;
			has_set_mantissa = was_num;
			continue;
		}
		if(*c == '0')
		{
			if(!has_set_mantissa && was_num)
			{
				++(this->sign_with_mantissa);
			}
			if(!has_set_mantissa && was_dot)
			{
				--(this->sign_with_mantissa);
			}
		}
		if(*c > '0' && *c <= '9')
		{
			if(!was_num && was_dot)
			{
				has_set_mantissa = true;
			}
			was_num = true;
			if(!has_set_mantissa)
			{
				++(this->sign_with_mantissa);
			}
		}
		if(((value * 10)/10) == value)
		{
			value *= 10;
			value += *c - '0';
		}
	}
	normalize_value();
}


void FloatingPoint::positive_mantissa_string(char* buff, uint32_t buff_size, uint32_t& i) const
{
	uint32_t shift = get_mantissa() - (1<<30);
	uint32_t value_len = get_value_len() - 1;
	for(uint32_t j = 0; i < buff_size; ++i)
	{
		if(value_len >= j)
		{
			buff[i] = get_digit(value_len - j) + '0';
		}
		else
		{
			buff[i] = '0';
		}
		if(j >= shift && j >= value_len)
		{
			return;
		}
		if(j == shift)
		{
			buff[++i] = '.';
		}
		++j;
	}
}

void FloatingPoint::negative_mantissa_string(char* buff, uint32_t buff_size, uint32_t& i) const
{
	buff[i++] = '0';
	buff[i++] = '.';
	uint32_t shift = (1<<30) - get_mantissa() - 1;
	uint32_t value_len = get_value_len() - 1;
	for(uint32_t j = 0; i < buff_size; ++i)
	{
		if(j < shift)
		{
			buff[i] = '0';
		}
		else
		{
			buff[i] = get_digit(value_len + shift - j) + '0';
			if(j >= (value_len + shift))
			{
				return;
			}
		}
		++j;
	}
}

int32_t FloatingPoint::numbers_alignment(const FloatingPoint& second, uint32_t& max_offset_value, uint32_t& min_offset_value) const
{
    int32_t offset1 = int32_t(this->get_value_len() - 1) - int32_t(this->get_mantissa() - (1<<30));
    int32_t offset2 = int32_t(second.get_value_len() - 1) - int32_t(second.get_mantissa() - (1<<30));

    int32_t max_offset = max(offset1, offset2);
    int32_t min_offset = min(offset1, offset2);
    max_offset_value = (offset1 > offset2) ? this->value : second.value;
    min_offset_value = (offset1 <= offset2) ? this->value : second.value;

    while(min_offset < max_offset)
    {
        if( ((min_offset_value*10)/10) != min_offset_value)
        {
            max_offset_value /= 10;
            --max_offset;
        }
        else
        {
            min_offset_value *= 10;
            ++min_offset;
        }
    }
    return min_offset;
}

void FloatingPoint::to_string(char* buff, uint32_t buff_size) const
{
    uint32_t i = 0;
    if(value == 0)
    {
        buff[i++] = '0';
        buff[i] = '\0';
        return;
    }

    if(get_sign())
    {
        ++i;
        buff[0] = '-';
    }
    if(get_mantissa() >> 30)
    {
    	positive_mantissa_string(buff, buff_size, i);
    }
    else
    {
    	negative_mantissa_string(buff, buff_size, i);
    }
    buff[++i] = '\0';
}

bool FloatingPoint::operator>(const FloatingPoint& second) const
{
    bool greaterSign = (this->get_sign() == 0 && second.get_sign() != 0);
    bool greaterValue = (this->get_mantissa() > second.get_mantissa() ||
                        (this->get_mantissa() == second.get_mantissa() &&
                         this->value > second.value));

    return (greaterSign ||
            (this->get_sign() == 0 && greaterValue) ||
            (this->get_sign() != 0 && !greaterValue && *this != second));
}

FloatingPoint FloatingPoint::operator+(const FloatingPoint& second) const
{
	uint8_t sign;
	uint32_t mantissa;
	uint32_t value;
	uint32_t value1;
	uint32_t value2;
	int32_t offset = numbers_alignment(second, value1, value2);

	if(this->get_sign() == second.get_sign())
	{
		sign = get_sign();
		value = value1 + value2;
	}
	else
	{
		sign = (this->absolute() > second.absolute()) ? this->get_sign() : second.get_sign();
		uint32_t max_value = max(value1, value2);
		uint32_t min_value = min(value1, value2);
		value = max_value - min_value;
	}
	mantissa = int32_t(get_value_len(value)) - 1 - offset + (1<<30);
	return FloatingPoint(sign, mantissa, value);
}

FloatingPoint FloatingPoint::operator*(const FloatingPoint& second) const
{
    uint8_t sign = (this->get_sign() + second.get_sign()) % 2;
    uint32_t value = this->value * second.value;
    int32_t offset1 = int32_t(this->get_value_len() - 1) - int32_t(this->get_mantissa() - (1<<30));
    int32_t offset2 = int32_t(second.get_value_len() - 1) - int32_t(second.get_mantissa() - (1<<30));
    int32_t offset = offset1 + offset2;
    uint32_t mantissa = int32_t(get_value_len(value)) - 1 - offset + (1<<30);
    return FloatingPoint(sign, mantissa, value);
}

FloatingPoint FloatingPoint::operator/(const FloatingPoint& second) const
{
    uint8_t sign = (this->get_sign() + second.get_sign()) % 2;
    int32_t offset1 = int32_t(this->get_value_len() - 1) - int32_t(this->get_mantissa() - (1<<30));
    int32_t offset2 = int32_t(second.get_value_len() - 1) - int32_t(second.get_mantissa() - (1<<30));
    uint32_t value = this->value;
    while(((value*10)/10) == value)
    {
        ++offset1;
        value *= 10;
    }
    value /= second.value;
    int32_t offset = offset1 - offset2;
    uint32_t mantissa = int32_t(get_value_len(value)) - 1 - offset + (1<<30);
    return FloatingPoint(sign, mantissa, value);
}

