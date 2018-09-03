#pragma once

#include <fstream>
#include <type_traits>

template<class T>
inline void SwitchEndianness(T& var) {
	static_assert(std::is_integral<T>::value, "Only integral types allowed");
	static_assert(sizeof(T) % 2 == 0, "endianness switch for non-exponential sizes not implemented");
	if constexpr (sizeof(T) == 2) {
		var = _byteswap_ushort(var);
	}
	else if constexpr (sizeof(T) == 4) {
		var = _byteswap_ulong(var);
	}
	else if constexpr (sizeof(T) == 8) {
		var = _byteswap_uint64(var);
	}
	else {
		static_assert(true, "default endianness not implemented");
	}
	
}

template<typename T>
inline void Shl(T& var, int dist) {
	static_assert(std::is_integral<T>::value, "Only integral types allowed");
	static_assert(sizeof(T) % 2 == 0, "roll for non-exponential sizes not implemented");
	if constexpr (sizeof(T) == 2) {
		var = _rotl16(var, dist);
	}
	else if constexpr (sizeof(T) == 4) {
		var = _rotl(var, dist);
	}
	else if constexpr (sizeof(T) == 8) {
		var = _rotl64(var, dist);
	}
	else {
		static_assert(true, "default shl not implemented");
	}
}

/* saturated unsigned arithmetics, i.e non-overflow unsigned */
template<typename T>
struct SatUAr {
	T t;
	SatUAr(const T& t) : t(t) {}

	operator T() { return t; }
	operator const T() const { return t; }
	SatUAr& operator=(const T& t) { this->t = t; }
	friend SatUAr operator+ (const SatUAr& lhs, const SatUAr& rhs)
		{ T val = lhs.t + rhs.t; return SatUAr(val | -(val < lhs.t)); }
	friend SatUAr operator+ (const SatUAr& lhs, const T& rhs)
		{ T val = lhs.t + rhs; return SatUAr(val | -(val < lhs.t)); }
	friend SatUAr operator+ (const T& lhs, const SatUAr& rhs)
		{ T val = lhs + rhs.t; return SatUAr(val | -(val < lhs)); }
	friend SatUAr operator- (const SatUAr& lhs, const SatUAr& rhs)
		{ T val = lhs.t - rhs.t; return SatUAr(val & -(lhs.t > val)); }
	friend SatUAr operator- (const SatUAr& lhs, const T& rhs)
		{ T val = lhs.t - rhs; return SatUAr(val & -(lhs.t > val)); }
	friend SatUAr operator- (const T& lhs, const SatUAr& rhs)
		{ T val = lhs - rhs.t; return SatUAr(val & -(lhs > val)); }
};


class DefRoster;
class DefText;
void PrintAllRosterTables(DefRoster* roster);
void PrintAllNicknames(DefRoster* roster, DefText* text);
void PrintAllTrainerNames(DefRoster* roster, DefText* text);

