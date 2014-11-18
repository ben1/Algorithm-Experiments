#pragma once

#include "BigUInt.h"


bool ConvertBase(const std::string& a_decimal, std::string& a_hex)
{
	const char HEX_CHAR[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	BigUInt result(0);
	BigUInt powTen(1);
	BigUInt add;

	// accumulate the binary result as we add each digit from the end of the decimal string to the beginning
	for (int d = a_decimal.size() - 1; d >= 0; --d)
	{
		add = powTen;
		uint32_t numeral = a_decimal[d] - '0';
		if (numeral > 9)
		{
			return false;
		}
		add *= numeral;
		result += add;
		powTen *= 10;
	}

	// write out the hex string from the beginning
	a_hex.clear();
	bool leadingZeroes = true;
	for (int q = result.GetNumQuads() - 1; q >= 0; --q)
	{
		uint32_t quad = result.GetQuad(q);

		int hexDigit = 7;
		if (leadingZeroes)
		{
			for (; hexDigit >= 0; --hexDigit)
			{
				if (quad >> (hexDigit << 2) > 0)
				{
					leadingZeroes = false;
					break;
				}
			}
		}

		for (int h = hexDigit; h >= 0; --h)
		{
			a_hex.push_back(HEX_CHAR[(quad >> (h << 2)) & 0xF]);
		}
	}

	return true;
}