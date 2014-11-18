#include "stdafx.h"
#include "BigUInt.h"

#include <algorithm>


BigUInt::BigUInt()
{
}


BigUInt::BigUInt(uint32_t a_uint)
{
	m_quads.resize(1);
	m_quads[0] = a_uint;
}


BigUInt::~BigUInt()
{
}


BigUInt& BigUInt::operator = (const BigUInt& a_uint)
{
	m_quads = a_uint.m_quads;
	return *this;
}


BigUInt& BigUInt::operator+=(const BigUInt& a_uint)
{
	uint32_t maxIndex = std::max(m_quads.size(), a_uint.m_quads.size());

	uint64_t carry = 0;
	for (uint32_t i = 0; i < maxIndex; ++i)
	{
		uint64_t r = carry;
		if (i < a_uint.m_quads.size())
		{
			r += uint64_t(a_uint.m_quads[i]);
		}
		if (i < m_quads.size())
		{
			r += uint64_t(m_quads[i]);
		}
		else
		{
			m_quads.push_back(0);
		}
		m_quads[i] = uint32_t(r);
		carry = r >> 32;
	}
	if (carry > 0)
	{
		m_quads.push_back(uint32_t(carry));
	}
	return *this;
}


BigUInt& BigUInt::operator*= (uint32_t a_uint)
{
	const uint64_t mul(a_uint);
	uint64_t carry = 0;
	for (uint32_t i = 0; i < m_quads.size(); ++i)
	{
		uint64_t r = uint64_t(m_quads[i]) * mul + carry;
		m_quads[i] = uint32_t(r);
		carry = r >> 32;
	}
	if (carry > 0)
	{
		m_quads.push_back(uint32_t(carry));
	}
	return *this;
}