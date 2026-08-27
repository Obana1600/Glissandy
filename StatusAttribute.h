#pragma once

#include <bit>
#include <cstdint>
#include <string>


enum class StatusAttribute : uint32_t {
	None = 0,
	Stun = 1 << 0,
};

constexpr size_t kStatusCount = 1;


constexpr StatusAttribute operator~(StatusAttribute a) {
	return static_cast<StatusAttribute>(~static_cast<uint32_t>(a));
}


constexpr StatusAttribute operator|(StatusAttribute a, StatusAttribute b) {
	return static_cast<StatusAttribute>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}


constexpr StatusAttribute operator&(StatusAttribute a, StatusAttribute b) {
	return static_cast<StatusAttribute>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}


constexpr StatusAttribute &operator|=(StatusAttribute &a, StatusAttribute b) {
	a = a | b;
	return a;
}


constexpr StatusAttribute &operator&=(StatusAttribute &a, StatusAttribute b) {
	a = a & b;
	return a;
}


constexpr bool Any(StatusAttribute a) {
	return a != StatusAttribute::None;
}


constexpr size_t StatusIndex(StatusAttribute status) {
	return static_cast<size_t>(std::countr_zero(static_cast<uint32_t>(status))); // 最下位ビットからのカウント
}