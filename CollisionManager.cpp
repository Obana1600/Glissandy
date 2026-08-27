#include "CollisionManager.h"

#include "Collision.h"
#include "DebugUi.h"
#include <cassert>
#include <limits>
#include <algorithm>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void CollisionManager::Clear() {
	colliders_.clear();
}


void CollisionManager::Register(Collider *collider) {
	if (!collider) {
		assert(false && "コライダーがnullptrです");
		return;
	}

	colliders_.push_back(collider);
}


void CollisionManager::CheckAllCollisions() {
	// 前フレームの結果をprevと入れ替える
	std::swap(prevPairs_, currentPairs_);

	currentPairs_.clear();
	enterPairs_.clear();
	stayPairs_.clear();
	exitPairs_.clear();

	const auto count = colliders_.size();
	for (size_t i = 0; i < count; i++) {
		auto *a = colliders_[i];
		if (!a->IsActive()) {
			continue;
		}
		auto aOBB = a->GetOBB();

		for (size_t j = i + 1; j < count; j++) {
			auto *b = colliders_[j];
			if (!b->IsActive()) {
				continue;
			}

			// フィルタリング
			if (!Any(a->GetAttribute() & b->GetMask())) {
				continue;
			}
			if (!Any(b->GetAttribute() & a->GetMask())) {
				continue;
			}

			// 形状判定
			if (!IsCollision(aOBB, b->GetOBB())) {
				continue;
			}

			// ペアの登録
			auto key = std::less<Collider *>{}(a, b) ? std::pair(a, b)
													 : std::pair(b, a);
			currentPairs_.emplace_back(key);
		}
	}

	// 並び変える
	std::sort(currentPairs_.begin(), currentPairs_.end());

	std::set_difference(currentPairs_.begin(), currentPairs_.end(), prevPairs_.begin(), prevPairs_.end(), std::back_inserter(enterPairs_));
	std::set_intersection(currentPairs_.begin(), currentPairs_.end(), prevPairs_.begin(), prevPairs_.end(), std::back_inserter(stayPairs_));
	std::set_difference(prevPairs_.begin(), prevPairs_.end(), currentPairs_.begin(), currentPairs_.end(), std::back_inserter(exitPairs_));

	// 各コールバックを発火
	for (auto pair : enterPairs_) {
		pair.first->OnCollisionEnter(pair.second);
		pair.second->OnCollisionEnter(pair.first);
	}
	for (auto pair : stayPairs_) {
		pair.first->OnCollisionStay(pair.second);
		pair.second->OnCollisionStay(pair.first);
	}
	for (auto pair : exitPairs_) {
		if (!IsRegistered(pair.first) || !IsRegistered(pair.second)) {
			continue;
		}

		pair.first->OnCollisionExit(pair.second);
		pair.second->OnCollisionExit(pair.first);
	}
}


void CollisionManager::DrawAllColliders() const {
	if (!isShowCollider_) {
		return;
	}

	for (auto &collider : colliders_) {
		collider->DrawDebug();
	}
}




void CollisionManager::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable("CollisionManager")) {

		// colliderCount
		DebugUi::PropatyRow("ColliderCount", [&] {
			ImGui::Text("%zu", colliders_.size());
		});

		// activeCount
		DebugUi::PropatyRow("ActiveCount", [&] {
			const auto active = std::count_if(colliders_.begin(), colliders_.end(), [](const Collider *collider) {
				return collider->IsActive();
			});
			ImGui::Text("%zd", active);
		});

		// hitPairCount
		DebugUi::PropatyRow("HitPairCount", [&] {
			ImGui::Text("%zu", currentPairs_.size());
		});

		// enter / stay / exit
		DebugUi::PropatyRow("Enter/Stay/Exit", [&] {
			ImGui::Text("%zu / %zu / %zu", enterPairs_.size(), stayPairs_.size(), exitPairs_.size());
		});

		// isShowCollider
		DebugUi::PropatyRow("ShowCollider", [&] {
			ImGui::Checkbox("##isShowCollider", &isShowCollider_);
		});

		DebugUi::EndTable();
	}
#endif
}


Collider *CollisionManager::RayCast(const Segment &segment, CollisionAttribute targetAttribute, float *outDistance) const {
	Collider *nearest = nullptr;
	float nearestT = std::numeric_limits<float>::infinity();

	for (auto *collider : colliders_) {
		if (!collider->IsActive()) {
			continue;
		}

		// 探したい相手に絞る
		if (!Any(collider->GetAttribute() & targetAttribute)) {
			continue;
		}

		// 交差判定を行う
		float t = 0.0f;
		if (!RaycastOBB(collider->GetOBB(), segment, &t)) {
			continue;
		}

		if (t < nearestT) {
			nearestT = t;
			nearest = collider;
		}
	}

	if (nearest && outDistance) {
		*outDistance = nearestT * Length(segment.diff);
	}

	return nearest;
}


bool CollisionManager::IsRegistered(const Collider *collider) const {
	return std::find(colliders_.begin(), colliders_.end(), collider) != colliders_.end();
}