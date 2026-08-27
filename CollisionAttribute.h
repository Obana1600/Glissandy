#pragma once

#include <cstdint>


enum class CollisionAttribute : uint32_t {
	None = 0,
	Player = 1 << 0,
	Enemy = 1 << 1, 
	PlayerBullet = 1 << 2,
	EnemyBullet = 1 << 3,
	Obstacle = 1 << 4,
	All = 0xFFFFFFFF,
};


constexpr CollisionAttribute operator~(CollisionAttribute a) {
	return static_cast<CollisionAttribute>(~static_cast<uint32_t>(a));
}


constexpr CollisionAttribute operator|(CollisionAttribute a, CollisionAttribute b) {
	return static_cast<CollisionAttribute>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}


constexpr CollisionAttribute operator&(CollisionAttribute a, CollisionAttribute b) {
	return static_cast<CollisionAttribute>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}


constexpr CollisionAttribute &operator|=(CollisionAttribute &a, CollisionAttribute b) {
	a = a | b;
	return a;
}


constexpr CollisionAttribute &operator&=(CollisionAttribute &a, CollisionAttribute b) {
	a = a & b;
	return a;
}


constexpr bool Any(CollisionAttribute a) {
	return a != CollisionAttribute::None;
}