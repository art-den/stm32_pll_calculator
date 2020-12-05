# Compile-time utilities to calculate PLL-coefficients for STM32 mcu

```cpp
#include <stdio.h>

#include "stm32f4_pll_calculator.hpp"

int main()
{
	// desired sysclock frequency
	constexpr unsigned SysClockFreq = 96'000'000; 

	using pll_calc = stm32f4_pll_calculator<
		SysClockFreq,
		25'000'000, // HSE (external quartz frequency)
		48'000'000  // UBS clock frequency
	>;

	printf(
		"N = %d, M = %d, P = %d, Q = %d\n", 
		pll_calc::pll_n, // Main PLL (PLL) multiplication factor for VCO
		pll_calc::pll_m, // Division factor for the main PLL (PLL) input clock
		pll_calc::pll_p, // Main PLL (PLL) division factor for main system clock
		pll_calc::pll_q  // Main PLL (PLL) division factor for USB OTG FS, and SDIO clocks
	);
}

```
