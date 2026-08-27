#include "FollowCamera.h"

#include "CursorLock.h"
#include "DebugUi.h"
#include "GameObject.h"
#include "TimeT.h"
#include <algorithm>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void FollowCamera::Update() {
	if (!target_ || !camera_) {
		return;
	}

	UpdateLook();
	UpdateFollow();
}


void FollowCamera::UpdateLook() {
	// アングルを動かしてよいか
	auto canLock = CursorLock::IsLocked();

	// マウスでアングルを回す
	auto move = Input::GetInstance()->GetMouseMove();
	auto rowX = canLock ? static_cast<float>(move.lX) : 0.0f;
	auto rowY = canLock ? static_cast<float>(move.lY) : 0.0f;

	smoothedMove_.x = Lerp(smoothedMove_.x, rowX, smoothing_);
	smoothedMove_.y = Lerp(smoothedMove_.y, rowY, smoothing_);

	angle_.y += smoothedMove_.x * sensitivity_;
	angle_.x += smoothedMove_.y * sensitivity_;
	angle_.x = std::clamp(angle_.x, -pitchLimit_, pitchLimit_);
}


void FollowCamera::UpdateFollow() {
	// 追従の基準
	const auto &targetRot = target_->GetWorldTransform().rotation_;
	auto yaw = targetRot.y + angle_.y;
	auto pitch = angle_.x;

	// オフセット補正
	auto rotMat = MakeRotateXMatrix(pitch) * MakeRotateYMatrix(yaw);
	auto desired = target_->GetWorldPosition() + TransformNormal(offset_, rotMat);

	// 線形に追従
	auto t = 1 - std::pow(1 - followRate_, TimeT::GetDeltaTime() * 60.0f);
	camera_->translation_.x = Lerp(camera_->translation_.x, desired.x, t);
	camera_->translation_.y = Lerp(camera_->translation_.y, desired.y, t);
	camera_->translation_.z = Lerp(camera_->translation_.z, desired.z, t);

	camera_->rotation_ = {pitch, yaw, 0.0f};
	camera_->UpdateMatrix();
}


void FollowCamera::UpdateImGui() {
#ifdef USE_IMGUI
	if (target_) {
		ImGui::Text("Target is assigned!");

	} else {
		ImGui::Text("Target is not assigned.");
	}

	if (camera_) {
		ImGui::Text("Camera is assigned!");

	} else {
		ImGui::Text("Camera is not assigned.");
	}

	// テーブル
	if (DebugUi::BeginTable("FollowCamera")) {

		// offset
		DebugUi::PropatyRow("Offset", [&] {
			ImGui::DragFloat3("##offset", &offset_.x, 0.01f);
		});

		// followRate
		DebugUi::PropatyRow("FollowRate", [&] {
			ImGui::DragFloat("##followRate", &followRate_, 0.001f);
		});

		// angle
		DebugUi::PropatyRow("Angle", [&] {
			ImGui::DragFloat2("##angle", &angle_.x, 0.01f);
		});

		// sensivity
		DebugUi::PropatyRow("Sensitivity", [&] {
			ImGui::DragFloat("##sentivity", &sensitivity_, 0.0001f, 0.0f, 1.0f);
		});

		// pitchLimit
		DebugUi::PropatyRow("PitchLimit", [&] {
			ImGui::DragFloat("##pitchLimit", &pitchLimit_, 0.01f);
		});

		DebugUi::EndTable();
	}
#endif
}