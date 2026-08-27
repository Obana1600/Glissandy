#include "BossController.h"

#include "DebugUi.h"
#include "GameObject.h"
#include "Health.h"
#include "MusicManager.h"
#include "Prefab.h"
#include "RailMover.h"
#include "TimeT.h"
#include "Track.h"
#include <Tween/TweenManager.h>
#include <algorithm>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;


namespace {

	constexpr BossAction kEnabledActions[] = {
		BossAction::PlaceRocks,
	};

} // namespace



void BossController::Initialize() {
	health_ = owner_->GetComponent<Health>();
}


void BossController::Update() {
	// 死んでいる、もしくわ退場演出中なら行動しない
	if (owner_->IsDead() || (health_ && health_->GetHp() <= 0)) {
		return;
	}

	// HP段階
	if (health_ && health_->GetMaxHp() > 0) {
		const auto rate = static_cast<float>(health_->GetHp()) / static_cast<float>(health_->GetMaxHp());
		stage_ = (rate <= hpRateStage_) ? 1 : 0;
	}

	// 行動中は次を抽選しない
	if (isActing_) {
		return;
	}

	idleTimer_ += TimeT::GetDeltaTime();
	if (idleTimer_ < idleDuration_) {
		return;
	}
	idleTimer_ = 0.0f;

	const auto action = PickAction();
	lastAction_ = action;

	switch (action) {
		case BossAction::PlaceRocks:
			PlayActionSequence([this] { FirePlaceRocks(); });
			break;

		default:
			// 未実装の行動
			break;
	}
}


void BossController::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// stage
		DebugUi::PropatyRow("Stage", [&] {
			ImGui::Text("%d", stage_);
		});

		// hpRateStage
		DebugUi::PropatyRow("HpRateStage", [&] {
			ImGui::DragFloat("##hpRateStage", &hpRateStage_, 0.01f, 0.0f, 1.0f);
		});

		// isActing
		DebugUi::PropatyRow("IsActing", [&] {
			ImGui::Text("%s", isActing_ ? "true" : "false");
		});

		// idleTimer
		DebugUi::PropatyRow("IdleTimer", [&] {
			ImGui::Text("%.2f / %.2f", idleTimer_, idleDuration_);
		});

		// idleDuration
		DebugUi::PropatyRow("IdleDuration", [&] {
			ImGui::DragFloat("##idleDuration", &idleDuration_, 0.01f, 0.0f, FLT_MAX);
		});

		// windupTime
		DebugUi::PropatyRow("WindupTime", [&] {
			ImGui::DragFloat("##windupTime", &windupTime_, 0.01f, 0.0f, FLT_MAX);
		});

		// recoverTime
		DebugUi::PropatyRow("RecoverTime", [&] {
			ImGui::DragFloat("##recoverTime", &recoverTime_, 0.01f, 0.0f, FLT_MAX);
		});

		// windupScale
		DebugUi::PropatyRow("WindupScale", [&] {
			ImGui::DragFloat("##windupScale", &windupScale_, 0.01f, 0.01f, FLT_MAX);
		});

		// rockCount
		DebugUi::PropatyRow("RockCount", [&] {
			ImGui::DragInt("##rockCount", &stageParams_[stage_].rockCount, 1, 0, 64);
		});

		// rippleCount
		DebugUi::PropatyRow("RippleCount", [&] {
			ImGui::DragInt("##rippleCount", &stageParams_[stage_].rippleCount, 1, 0, 64);
		});

		// maxRockCount
		DebugUi::PropatyRow("MaxRockCount", [&] {
			ImGui::DragInt("##maxRockCount", &maxRockCount_, 1, 0, 64);
		});

		// aliveRockCount
		DebugUi::PropatyRow("AliveRockCount", [&] {
			if (rockCountCallback_) {
				ImGui::Text("%d / %d", rockCountCallback_(), maxRockCount_);
			} else {
				ImGui::Text("-");
			}
		});

		// 行動
		DebugUi::PropatyRow("PlaceRocks", [&] {
			if (ImGui::Button("fire")) {
				FirePlaceRocks();
			}
		});

		DebugUi::EndTable();
	}

	// track
	if (track_) {
		ImGui::Text("Track is assigned!");
	} else {
		ImGui::Text("Track is not assigned.");
	}

	// player
	if (player_) {
		ImGui::Text("Player is assigned!");
	} else {
		ImGui::Text("Player is not assigned.");
	}

	// health
	if (health_) {
		ImGui::Text("Health is assigned!");
	} else {
		ImGui::Text("Health is not assigned.");
	}

	// spawnCallback
	if (spawnCallback_) {
		ImGui::Text("SpawnCallback is assigned!");
	} else {
		ImGui::Text("SpawnCallback is not assigned.");
	}

	// rockCountCallback
	if (rockCountCallback_) {
		ImGui::Text("RockCountCallback is assigned!");
	} else {
		ImGui::Text("RockCountCallback is not assigned.");
	}
#endif
}


BossAction BossController::PickAction() {
	constexpr int count = static_cast<int>(std::size(kEnabledActions));

	// 選べるものが1つしかないなら抽選しない
	if (count <= 1) {
		return kEnabledActions[0];
	}

	// 直前と同じものは避ける
	for (int i = 0; i < count; i++) {
		const auto action = kEnabledActions[RandomInt(0, count - 1)];
		if (action != lastAction_) {
			return action;
		}
	}

	return kEnabledActions[0];
}


void BossController::PlayActionSequence(std::function<void()> onFire) {
	isActing_ = true;

	auto *obj = owner_;
	const auto baseScale = obj->GetScale();

	// 予備動作
	auto windup = TweenManager::Create(obj, &GameObject::GetScale, &GameObject::SetScale, baseScale * windupScale_, windupTime_)
					  .SetEase(EaseType::InQuad)
					  .OnComplete(std::move(onFire));

	// 硬直
	auto recover = TweenManager::Create(obj, &GameObject::GetScale, &GameObject::SetScale, baseScale, recoverTime_)
					   .SetEase(EaseType::OutBack);

	TweenManager::ToSequence()
		.SetTarget(obj)
		.Append(std::move(windup))
		.Append(std::move(recover))
		.OnComplete([this] {
			isActing_ = false;
		});
}


void BossController::FirePlaceRocks() {
	if (!spawnCallback_ || !track_) {
		return;
	}

	// 岩の上限を設ける
	const auto alive = rockCountCallback_ ? rockCountCallback_() : 0;
	const auto count = (std::min)(stageParams_[stage_].rockCount, (std::max)(0, maxRockCount_ - alive));
	if (count <= 0) {
		return;
	}

	MusicManager::GetInstance()->PlaySe("boss_place_rock");

	const auto length = track_->GetLength();
	const auto laneCount = track_->GetLaneCount();

	for (int i = 0; i < count; i++) {
		auto rock = Prefab::CreateRock();

		// レール上のランダムな点に置く
		if (auto *mover = rock->GetComponent<RailMover>()) {
			mover->SetTrack(track_, Random(0.0f, length), RandomInt(0, laneCount - 1));
		}

		spawnCallback_(std::move(rock));
	}
}


float BossController::Random(float min, float max) {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(rng_);
}


int BossController::RandomInt(int min, int max) {
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng_);
}