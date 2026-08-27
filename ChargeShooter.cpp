#include "ChargeShooter.h"

#include "Bullet.h"
#include "CollisionManager.h"
#include "DebugUi.h"
#include "Extension.h"
#include "GameObject.h"
#include "Geometry.h"
#include "MusicManager.h"
#include "Prefab.h"
#include "TimeT.h"
#include <algorithm>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void ChargeShooter::Update() {
#ifdef USE_IMGUI
	if (DebugUi::IsCapturingMouse()) {
		StopCharge();
		return;
	}
#endif

	if (!collisionMgr_ || !camera_) {
		return;
	}

	// チャージ
	auto pressed = Input::GetInstance()->IsPressMouse(0);
	if (pressed) {
		// 押し始めにチャージ音を鳴らす
		if (!wasPressed_) {
			chargeVoiceHandle_ = MusicManager::GetInstance()->PlaySe("charge");
			isChargeSePlaying_ = true;
		}

		chargeTimer_ += TimeT::GetDeltaTime();
	}

	// レイキャスト
	float range = GetRange();
	auto forward = GetForward();

	Segment segment{};
	segment.origin = camera_->translation_;
	segment.diff = forward * range;

	float dist = 0.0f;
	hasTarget_ = collisionMgr_->RayCast(segment, CollisionAttribute::Enemy | CollisionAttribute::Obstacle, &dist) != nullptr;

	// 離したら発射
	if (!pressed && wasPressed_) {
		// 音を止める
		StopChargeSe();

		if (chargeTimer_ >= minChargeTime_) {
			auto aimPoint = segment.origin + forward * (hasTarget_ ? dist : range);
			Fire(aimPoint);
		}

		chargeTimer_ = 0.0f;
	}
	wasPressed_ = pressed;
}




void ChargeShooter::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// chargeTimer
		DebugUi::PropatyRow("ChargeTimer", [&] {
			ImGui::Text("%.2f / %.2f", chargeTimer_, maxChargeTime_);
		});

		// chargeRate
		DebugUi::PropatyRow("ChargeRate", [&] {
			ImGui::Text("%.2f", GetChargeRate());
		});

		// minChargeTime
		DebugUi::PropatyRow("MinChargeTime", [&] {
			ImGui::DragFloat("##minChargeTime", &minChargeTime_, 0.01f, 0.0f, FLT_MAX);
		});

		// maxChargeTime
		DebugUi::PropatyRow("MaxChargeTime", [&] {
			ImGui::DragFloat("##maxChargeTime", &maxChargeTime_, 0.01f, 0.001f, FLT_MAX);
		});

		// minRange
		DebugUi::PropatyRow("MinRange", [&] {
			ImGui::DragFloat("##minRange", &minRange_);
		});

		// maxRange
		DebugUi::PropatyRow("MaxRange", [&] {
			ImGui::DragFloat("##maxRange", &maxRange_);
		});

		// minDamage
		DebugUi::PropatyRow("MinDamage", [&] {
			ImGui::DragInt("##minDamage", &minDamage_, 1, 0, INT_MAX);
		});

		// maxDamage
		DebugUi::PropatyRow("MaxDamage", [&] {
			ImGui::DragInt("##maxDamage", &maxDamage_, 1, 0, INT_MAX);
		});

		// bulletSpeed
		DebugUi::PropatyRow("BulletSpeed", [&] {
			ImGui::DragFloat("##bulletSpeed", &bulletSpeed_);
		});

		// wasPressed
		DebugUi::PropatyRow("WasPressed", [&] {
			ImGui::Text("%s", wasPressed_ ? "true" : "false");
		});

		// hasTarget
		DebugUi::PropatyRow("HasTarget", [&] {
			ImGui::Text("%s", hasTarget_ ? "true" : "false");
		});

		DebugUi::EndTable();
	}

	// camera
	if (camera_) {
		ImGui::Text("Camera is assigned!");
	} else {
		ImGui::Text("Camera is not assigned.");
	}

	// collisionMgr
	if (collisionMgr_) {
		ImGui::Text("CollisionManager is assigned!");
	} else {
		ImGui::Text("CollisionManager is not assigned.");
	}
#endif
}


void ChargeShooter::SetSpawnCallback(std::function<void(std::unique_ptr<GameObject>)> callback) {
	spawnCallback_ = std::move(callback);
}


void ChargeShooter::StopCharge() {
	StopChargeSe();

	chargeTimer_ = 0.0f;
	wasPressed_ = false;
}


float ChargeShooter::GetChargeRate() const {
	return std::clamp(chargeTimer_ / maxChargeTime_, 0.0f, 1.0f);
}


void ChargeShooter::StopChargeSe() {
	if (!isChargeSePlaying_) {
		return;
	}

	MusicManager::GetInstance()->StopSe(chargeVoiceHandle_);
	isChargeSePlaying_ = false;
}


void ChargeShooter::Fire(const KamataEngine::Vector3 &aimPoint) {
	MusicManager::GetInstance()->PlaySe("shot");

	std::unique_ptr<GameObject> newBullet = Prefab::CreatePlayerBullet();

	auto spawnPos = owner_->GetWorldPosition();

	// bulletに流し込む
	if (auto *bullet = newBullet->GetComponent<Bullet>()) {
		bullet->SetRange(GetRange());
		bullet->SetDamage(GetDamage());
		bullet->SetVelocity(Normalize(aimPoint - spawnPos) * bulletSpeed_);
	}
	newBullet->SetTranslation(spawnPos);

	// 弾のスポーン
	if (spawnCallback_) {
		spawnCallback_(std::move(newBullet));
	}
}


KamataEngine::Vector3 ChargeShooter::GetForward() const {
	return Vector3(camera_->matView.m[0][2], camera_->matView.m[1][2], camera_->matView.m[2][2]);
}


float ChargeShooter::GetRange() const {
	return Lerp(minRange_, maxRange_, GetChargeRate());
}


int ChargeShooter::GetDamage() const {
	return static_cast<int>(Lerp(static_cast<float>(minDamage_), static_cast<float>(maxDamage_), GetChargeRate()));
}