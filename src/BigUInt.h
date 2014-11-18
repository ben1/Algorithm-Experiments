#pragma once

#include <vector>


class BigUInt
{
public:
	BigUInt();
	BigUInt(uint32_t a_uint);
	~BigUInt();

	BigUInt& operator=(const BigUInt& a_uint);
	BigUInt& operator+=(const BigUInt& a_uint);
	BigUInt& operator*=(uint32_t a_uint);

	uint32_t GetNumQuads() const { return m_quads.size(); }
	uint32_t GetQuad(uint32_t a_index) const { return m_quads[a_index]; }

private:

	std::vector<uint32_t> m_quads;
};

