/* 
 
Compile-time utilities to calculate PLL-coefficients for STM32 mcu

Copyright (c) 2020 Artyomov Denis (denis.artyomov@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once

#include <stdint.h>

template <
	unsigned SysclockFreq,
	unsigned HseFreq,
	unsigned UsbFreq,
	unsigned MinVcoFreq = 100'000'000,
	unsigned MaxVcoFreq = 432'000'000,
	unsigned MinInVcoFreq = 1'000'000,
	unsigned MaxInVcoFreq = 2'000'000,
	unsigned MinN = 50,
	unsigned MaxN = 432
>
class stm32f4_pll_calculator
{
private:
	enum class Factor { N, M, P, Q };

	static constexpr uint16_t calc_by_q_m_p(Factor type, uint16_t q, uint16_t m, uint16_t p)
	{
		unsigned n = (uint64_t)SysclockFreq * (uint64_t)m * (uint64_t)p / (uint64_t)HseFreq;
		if (n < MinN) return 0;
		if (n > MaxN) return 0;

		unsigned test_sysfreq = (uint64_t)HseFreq * (uint64_t)n / (m * p);
		if (test_sysfreq != SysclockFreq)
			return 0;

		unsigned test_usbfreq = (uint64_t)HseFreq * (uint64_t)n / (m * q);
		if (test_usbfreq != UsbFreq)
			return 0;

		return
			(type == Factor::N) ? n :
			(type == Factor::M) ? m :
			(type == Factor::P) ? p :
			(type == Factor::Q) ? q :
			0;
	}

	static constexpr uint16_t calc_by_q_m(Factor type, uint16_t q, uint16_t m)
	{
		unsigned vco_in_freq = HseFreq / m;
		if (vco_in_freq < MinInVcoFreq) return 0;
		if (vco_in_freq > MaxInVcoFreq) return 0;

		auto res2 = calc_by_q_m_p(type, q, m, 2);
		if (res2 != 0) return res2;

		auto res4 = calc_by_q_m_p(type, q, m, 4);
		if (res4 != 0) return res4;

		auto res6 = calc_by_q_m_p(type, q, m, 6);
		if (res6 != 0) return res6;

		return calc_by_q_m_p(type, q, m, 8);
	}

	static constexpr uint16_t calc_by_q_m_loop(Factor type, uint16_t q, uint16_t m, uint16_t m_max)
	{
		auto res = calc_by_q_m(type, q, m);
		if (res != 0)
			return res;
		else if (m < m_max)
			return calc_by_q_m_loop(type, q, m + 1, m_max);
		return 0;
	}

	static constexpr uint16_t calc_by_q(Factor type, uint16_t q)
	{
		unsigned VcoFreq = UsbFreq * q;
		if (VcoFreq < MinVcoFreq) return 0;
		if (VcoFreq > MaxVcoFreq) return 0;

		return calc_by_q_m_loop(type, q, 2, 63);
	}

	static constexpr uint16_t calc_by_q_loop(Factor type, uint16_t q, uint16_t q_max)
	{
		auto res = calc_by_q(type, q);
		if (res != 0) 
			return res;
		else if (q < q_max) 
			return calc_by_q_loop(type, q + 1, q_max);
		return 0;
	}

	static constexpr uint16_t calc(Factor type)
	{
		return calc_by_q_loop(type, 2, 15);
	}

public:

	// Division factor for the main PLL (PLL) input clock
	static constexpr uint16_t pll_m = calc(Factor::M);

	// Main PLL (PLL) multiplication factor for VCO
	static constexpr uint16_t pll_n = calc(Factor::N);

	// Main PLL (PLL) division factor for main system clock
	static constexpr uint16_t pll_p = calc(Factor::P);

	// Main PLL (PLL) division factor for USB OTG FS, and SDIO clocks
	static constexpr uint16_t pll_q = calc(Factor::Q);

	static_assert(
		pll_m && pll_n && pll_p && pll_q, 
		"Combination of SysclockFreq, HseFreq and UsbFreq is incorrect"
	);
};
