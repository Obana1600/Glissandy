#include "PlayerController.h"

#include "DebugUi.h"
#include "GameObject.h"
#include "MusicManager.h"
#include "RailMover.h"
#include "TimeT.h"
#include <algorithm>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void PlayerController::Initialize() {
	// railMoverをセット
	railMover_ = owner_->GetComponent<RailMover>();
}


void PlayerController::Update() {
	if (!railMover_) {
		return;
	}

	auto *input = Input::GetInstance();

	// 加減速
	float speed = railMover_->GetSpeed();
	if (input->PushKey(DIK_W)) {
		speed += acceleration_ * TimeT::GetDeltaTime();
	}
	if (input->PushKey(DIK_S)) {
		speed -= acceleration_ * TimeT::GetDeltaTime();
	}
	railMover_->SetSpeed(std::clamp(speed, minSpeed_, maxSpeed_));

	// レーン移動
	const auto laneIndex = railMover_->GetTargetLaneIndex();

	if (input->TriggerKey(DIK_A)) {
		railMover_->ChangeLane(-1);
	}
	if (input->TriggerKey(DIK_D)) {
		railMover_->ChangeLane(+1);
	}

	// 実際にレーンが変わったときだけ鳴らす
	if (railMover_->GetTargetLaneIndex() != laneIndex) {
		MusicManager::GetInstance()->PlaySe("lane_change");
	}
}


void PlayerController::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// acceleration
		DebugUi::PropatyRow("Acceleration", [&] {
			ImGui::DragFloat("##acceleration", &acceleration_, 0.1f);
		});

		// minSpeed
		DebugUi::PropatyRow("MinSpeed", [&] {
			ImGui::DragFloat("##minSpeed", &minSpeed_, 0.01f);
		});

		// maxSpeed
		DebugUi::PropatyRow("MaxSpeed", [&] {
			ImGui::DragFloat("##maxSpeed", &maxSpeed_, 0.01f);
		});

		DebugUi::EndTable();
	}

	if (railMover_) {
		ImGui::Text("RailMover is assigned☆彡");
	} else {
		ImGui::Text("RailMover is not assigned.");
	}
#endif
}