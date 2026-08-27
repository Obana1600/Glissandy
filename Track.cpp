#include "Track.h"

#include "DebugUi.h"
#include "ModelManager.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;


namespace {

	// レール用の1x1x1の白いボックス
	constexpr const char *kRailModelName = "rail";

} // namespace


void Track::UpdateImGui() {
#ifdef USE_IMGUI
	// レーン
	if (DebugUi::BeginTable("Track")) {

		// laneCount
		DebugUi::PropatyRow("LaneCount", [&] {
			ImGui::DragInt("##laneCount", &laneCount_, 1, 1, 16);
		});

		// startWidth
		DebugUi::PropatyRow("StartWidth", [&] {
			ImGui::DragFloat("##startWidth", &startWidth_, 0.1f, 0.0f, FLT_MAX);
		});

		// endWidth
		DebugUi::PropatyRow("EndWidth", [&] {
			ImGui::DragFloat("##endWidth", &endWidth_, 0.1f, 0.0f, FLT_MAX);
		});

		// length
		DebugUi::PropatyRow("Length", [&] {
			ImGui::Text("%.2f", GetLength());
		});

		// next
		DebugUi::PropatyRow("Next", [&] {
			ImGui::Text("%s", next_ ? next_->GetName().c_str() : "-");
		});

		DebugUi::EndTable();
	}

	// レールの見た目
	ImGui::SeparatorText("Rail");
	if (DebugUi::BeginTable("Rail")) {

		// division
		DebugUi::PropatyRow("Division", [&] {
			ImGui::DragInt("##railDivision", &railDivision_, 1, 1, 256);
		});

		// width
		DebugUi::PropatyRow("Width", [&] {
			ImGui::DragFloat("##railWidth", &railWidth_, 0.01f, 0.0f, FLT_MAX);
		});

		// thickness
		DebugUi::PropatyRow("Thickness", [&] {
			ImGui::DragFloat("##railThickness", &railThickness_, 0.01f, 0.0f, FLT_MAX);
		});

		// offsetY
		DebugUi::PropatyRow("OffsetY", [&] {
			ImGui::DragFloat("##railOffsetY", &railOffsetY_, 0.01f);
		});

		// color
		DebugUi::PropatyRow("Color", [&] {
			if (ImGui::ColorEdit4("##railColor", &railColorValue_.x, ImGuiColorEditFlags_NoInputs)) {
				railColor_.SetColor(railColorValue_);
			}
		});

		// railCount
		DebugUi::PropatyRow("RailCount", [&] {
			ImGui::Text("%d", railCount_);
		});

		DebugUi::EndTable();
	}

	// デバッグ描画
	ImGui::SeparatorText("Debug");
	if (DebugUi::BeginTable("Debug")) {

		// division
		DebugUi::PropatyRow("Division", [&] {
			ImGui::DragInt("##debugDivision", &debugDivision_, 1, 1, 256);
		});

		// colorCenter
		DebugUi::PropatyRow("ColorCenter", [&] {
			ImGui::ColorEdit4("##debugColorCenter", &debugColorCenter_.x, ImGuiColorEditFlags_NoInputs);
		});

		// colorLane
		DebugUi::PropatyRow("ColorLane", [&] {
			ImGui::ColorEdit4("##debugColorLane", &debugColorLane_.x, ImGuiColorEditFlags_NoInputs);
		});

		DebugUi::EndTable();
	}
#endif
}


void Track::DrawDebug() const {
	auto length = GetLength();
	auto step = length / debugDivision_;

	// 各レール
	for (int i = 0; i < laneCount_; i++) {
		auto lane = GetLaneValue(i);

		for (int j = 0; j < debugDivision_; j++) {
			auto s0 = j * step;
			auto s1 = (j + 1) * step;
			auto p0 = GetPoint(s0) + GetLaneOffset(s0, lane);
			auto p1 = GetPoint(s1) + GetLaneOffset(s1, lane);

			PrimitiveDrawer::GetInstance()->DrawLine3d(p0, p1, debugColorLane_);
		}
	}

	// 中心線
	for (int i = 0; i < debugDivision_; i++) {
		auto p0 = GetPoint(i * step);
		auto p1 = GetPoint((i + 1) * step);

		PrimitiveDrawer::GetInstance()->DrawLine3d(p0, p1, debugColorCenter_);
	}
}


void Track::BuildRail() {
	if (!isRailReady_) {
		railModel_ = ModelManager::GetInstance()->Load(kRailModelName);
		railColor_.Initialize();
		railColor_.SetColor(railColorValue_);
		isRailReady_ = true;
	}

	const auto division = (std::max)(railDivision_, 1);
	const auto needed = laneCount_ * division;

	while (static_cast<int>(railTransforms_.size()) < needed) {
		auto transform = std::make_unique<WorldTransform>();
		transform->Initialize();
		railTransforms_.push_back(std::move(transform));
	}
	railCount_ = needed;

	const auto step = GetLength() / division;
	const Vector3 offset = {0.0f, railOffsetY_, 0.0f};

	int index = 0;
	for (int i = 0; i < laneCount_; i++) {
		const auto lane = GetLaneValue(i);

		for (int j = 0; j < division; j++) {
			const auto s0 = j * step;
			const auto s1 = (j + 1) * step;
			const auto p0 = GetPoint(s0) + GetLaneOffset(s0, lane) + offset;
			const auto p1 = GetPoint(s1) + GetLaneOffset(s1, lane) + offset;

			SetupRailSegment(*railTransforms_[index], p0, p1);
			index++;
		}
	}
}


void Track::DrawRail(const KamataEngine::Camera &camera) const {
	if (!railModel_) {
		return;
	}

	for (int i = 0; i < railCount_; i++) {
		railModel_->Draw(*railTransforms_[i], camera, &railColor_);
	}
}


void Track::SetupRailSegment(KamataEngine::WorldTransform &transform, const KamataEngine::Vector3 &p0, const KamataEngine::Vector3 &p1) const {
	Vector3 dir = p1 - p0;
	const auto length = Length(dir);

	transform.translation_ = (p0 + p1) * 0.5f;

	if (length < 1e-6f) {
		transform.scale_ = {};
		transform.rotation_ = {};

	} else {
		Normalize(dir);

		transform.scale_ = {railWidth_, railThickness_, length * kRailOverlap_};
		transform.rotation_ = {std::asin(-dir.y), std::atan2(dir.x, dir.z), 0.0f};
	}

	transform.matWorld_ =
		MakeScaleMatrix(transform.scale_) *
		MakeRotateXMatrix(transform.rotation_.x) *
		MakeRotateYMatrix(transform.rotation_.y) *
		MakeRotateZMatrix(transform.rotation_.z) *
		MakeTranslateMatrix(transform.translation_);

	transform.TransferMatrix();
}


float Track::GetLaneValue(int index) const {
	return index - (laneCount_ - 1) * 0.5f;
}


float Track::GetLaneWidthAt(float s) const {
	const auto length = GetLength();
	if (length < 1e-6f) {
		return startWidth_;
	}

	const auto t = std::clamp(s / length, 0.0f, 1.0f);
	return Lerp(startWidth_, endWidth_, t);
}