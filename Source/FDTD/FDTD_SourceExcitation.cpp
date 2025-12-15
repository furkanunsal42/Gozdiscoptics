#include "FDTD.h"

FDTD::SourceExcitation::SourceExcitation(const std::string& glsl) :
	fdtd_excitation_glsl(glsl) { }

FDTD::SourceExcitation::SourceExcitation(std::string&& glsl) :
	fdtd_excitation_glsl(glsl) { }

FDTD::SourceExcitation& FDTD::SourceExcitation::operator=(const std::string& fdtd_excitation_glsl)
{
	this->fdtd_excitation_glsl = fdtd_excitation_glsl;
	return *this;
}

FDTD::SourceExcitation& FDTD::SourceExcitation::operator=(std::string&& fdtd_excitation_glsl)
{
	this->fdtd_excitation_glsl = fdtd_excitation_glsl;
	return *this;
}

///////////////////			OPERATORS			///////////////////

std::ostream& operator<<(std::ostream& stream, const FDTD::SourceExcitation& object)
{
	return stream << object.fdtd_excitation_glsl;
}

FDTD::SourceExcitation operator+(const FDTD::SourceExcitation& a, const FDTD::SourceExcitation& b)
{
	return FDTD::SourceExcitation(a.fdtd_excitation_glsl + " + " + b.fdtd_excitation_glsl);
}

FDTD::SourceExcitation operator-(const FDTD::SourceExcitation& a, const FDTD::SourceExcitation& b)
{
	return FDTD::SourceExcitation(a.fdtd_excitation_glsl + " - " + b.fdtd_excitation_glsl);
}

FDTD::SourceExcitation operator/(const FDTD::SourceExcitation& a, const FDTD::SourceExcitation& b)
{
	return FDTD::SourceExcitation(a.fdtd_excitation_glsl + " / " + b.fdtd_excitation_glsl);
}

FDTD::SourceExcitation operator*(const FDTD::SourceExcitation& a, const FDTD::SourceExcitation& b)
{
	return FDTD::SourceExcitation(a.fdtd_excitation_glsl + " * " + b.fdtd_excitation_glsl);
}