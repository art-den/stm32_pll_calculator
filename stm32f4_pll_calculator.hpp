#pragma once

#include <stdint.h>

template <
	unsigned SysclockFreq,
	unsigned HseFreq,
	unsigned UsbFreq,
	unsigned MinVcoFreq = 100'000'000,
	unsigned MaxVcoFreq = 432'000'000,
	unsigned MinInVcoFreq = 1'000'000,
	unsigned MaxInVcoFreq = 2'000'000
>
class stm32f4_pll_calculator
{
private:
	enum class Type { N, M, P, Q };

	template <Type type, uint16_t q, uint16_t m, uint16_t p>
	static constexpr uint16_t calc_by_q_m_p()
	{
		constexpr unsigned n = (uint64_t)SysclockFreq * (uint64_t)m * (uint64_t)p / (uint64_t)HseFreq;

		constexpr unsigned test_sysfreq = (uint64_t)HseFreq * (uint64_t)n / (m * p);
		if constexpr (test_sysfreq != SysclockFreq)
			return 0;

		constexpr unsigned test_usbfreq = (uint64_t)HseFreq * (uint64_t)n / (m * q);
		if constexpr (test_usbfreq != UsbFreq)
			return 0;

		if constexpr (type == Type::N)
			return n;

		else if constexpr (type == Type::M)
			return m;

		else if constexpr (type == Type::P)
			return p;

		else if constexpr (type == Type::Q)
			return q;

		return 0;
	}

	template <Type type, uint16_t q, uint16_t m>
	static constexpr uint16_t calc_by_q_m()
	{
		constexpr unsigned vco_in_freq = HseFreq / m;
		if constexpr (vco_in_freq < MinInVcoFreq) return 0;
		if constexpr (vco_in_freq > MaxInVcoFreq) return 0;

		constexpr auto res2 = calc_by_q_m_p<type, q, m, 2>();
		if constexpr (res2 != 0) return res2;

		constexpr auto res4 = calc_by_q_m_p<type, q, m, 4>();
		if constexpr (res4 != 0) return res4;

		constexpr auto res6 = calc_by_q_m_p<type, q, m, 6>();
		if constexpr (res6 != 0) return res6;

		return calc_by_q_m_p<type, q, m, 8>();
	}

	template <Type type, uint16_t q, uint16_t m, uint16_t m_max>
	static constexpr uint16_t calc_by_q_m_loop()
	{
		constexpr auto res = calc_by_q_m<type, q, m>();
		if constexpr (res != 0)
			return res;
		else if constexpr (m < m_max)
			return calc_by_q_m_loop<type, q, m + 1, m_max>();
		return 0;
	}

	template <Type type, uint16_t q>
	static constexpr uint16_t calc_by_q()
	{
		constexpr unsigned VcoFreq = UsbFreq * q;
		if constexpr (VcoFreq < MinVcoFreq) return 0;
		if constexpr (VcoFreq > MaxVcoFreq) return 0;

		return calc_by_q_m_loop<type, q, 2, 63>();
	}

	template <Type type, uint16_t q, uint16_t q_max>
	static constexpr uint16_t calc_by_q_loop()
	{
		constexpr auto res = calc_by_q<type, q>();
		if constexpr (res != 0)
			return res;
		else if constexpr (q < q_max)
			return calc_by_q_loop<type, q + 1, q_max>();

		return 0;
	}

	template <Type type>
	static constexpr uint16_t calc()
	{
		return calc_by_q_loop<type, 2, 15>();
	}

public:
	static constexpr uint16_t pll_n = calc<Type::N>();
	static constexpr uint16_t pll_m = calc<Type::M>();
	static constexpr uint16_t pll_p = calc<Type::P>();
	static constexpr uint16_t pll_q = calc<Type::Q>();
};
