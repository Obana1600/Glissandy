#pragma once

#include "Collider.h"
#include "CollisionAttribute.h"
#include "Geometry.h"
#include <cstdint>
#include <utility>
#include <vector>


class CollisionManager {
public: // メンバ関数

	void Clear();

	void Register(Collider *collider);

	void CheckAllCollisions();

	void DrawAllColliders() const;

	void UpdateImGui();

	Collider *RayCast(const Segment &segment, CollisionAttribute targetAttribute, float *outDistance) const;


	bool IsShowCollider() const {
		return isShowCollider_;
	}

	void SetIsShowCollider(bool value) {
		isShowCollider_ = value;
	}


private: // メンバ変数

	std::vector<Collider *> colliders_;

	std::vector<std::pair<Collider *, Collider *>> currentPairs_;
	std::vector<std::pair<Collider *, Collider *>> prevPairs_;
	std::vector<std::pair<Collider *, Collider *>> enterPairs_;
	std::vector<std::pair<Collider *, Collider *>> stayPairs_;
	std::vector<std::pair<Collider *, Collider *>> exitPairs_;

	bool isShowCollider_ = true;


private: // メンバ関数

	bool IsRegistered(const Collider *collider) const;
};