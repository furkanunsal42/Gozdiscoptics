#pragma once
#include "FDTD.h"

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool>>
inline FDTD::SourceExcitation::SourceExcitation(const T& constant)
{
	fdtd_excitation_glsl = std::to_string(constant);
}

///////////////////			OPERATORS			///////////////////

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>>>
inline FDTD::SourceExcitation operator+(const FDTD::SourceExcitation& a, const T& b) 
{
	return FDTD::SourceExcitation(a.fdtd_excitation_glsl + std::to_string(b));
}

