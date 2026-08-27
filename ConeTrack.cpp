#include "ConeTrack.h"

#include "DebugUi.h"
#include <numbers>
#include <cmath>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;





void ConeTrack::UpdateImGui() {
#ifdef USE_IMGUI
	constexpr float toDegrees = 180.0f / std::numbers::pi_v<float>;
	constexpr float toRadians = std::numbers::pi_v<float> / 180.0f;

	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// center
		DebugUi::PropatyRow("Center", [&] {
			ImGui::DragFloat3("##center", &center_.x, 1.0f);
		});

		// baseRadius
		DebugUi::PropatyRow("BaseRadius", [&] {
			ImGui::DragFloat("##baseRadius", &baseRadius_, 1.0f, 0.1f, FLT_MAX);
		});

		// slope
		DebugUi::PropatyRow("Slope", [&] {
			float degrees = slope_ * toDegrees;
			if (ImGui::DragFloat("##slope", &degrees, 0.5f, -89.0f, 89.0f)) {
				slope_ = degrees * toRadians;
			}
		});

		// startAngle
		DebugUi::PropatyRow("StartAngle", [&] {
			float degrees = startAngle_ * toDegrees;
			if (ImGui::DragFloat("##startAngle", &degrees, 0.5f)) {
				startAngle_ = degrees * toRadians;
			}
		});

		// sweepAngle
		DebugUi::PropatyRow("SweepAngle", [&] {
			float degrees = sweepAngle_ * toDegrees;
			if (ImGui::DragFloat("##sweepAngle", &degrees, 0.5f, 0.0f, 360.0f)) {
				sweepAngle_ = degrees * toRadians;
			}
		});

		DebugUi::EndTable();
	}

	// 共通部分
	Track::UpdateImGui();
#endif
}


void ConeTrack::AlignTo(const Track &prev) {
	const auto length = prev.GetLength();
	const auto end = prev.GetPoint(length);
	auto dir = prev.GetTangent(length);

	dir.y = 0.0f;
	if (Length(dir) < 1e-6f) {
		return;
	}
	dir = Normalize(dir);

	center_ = end + dir * baseRadius_;

	const auto outward = end - center_;
	startAngle_ = std::atan2(outward.z, outward.x);
}


KamataEngine::Vector3 ConeTrack::GetPoint(float s) const {
	auto theta = startAngle_ + s / baseRadius_;
	return center_ + (Vector3(std::cos(theta), 0.0f, std::sin(theta)) * baseRadius_);
}


KamataEngine::Vector3 ConeTrack::GetTangent(float s) const {
	auto theta = startAngle_ + s / baseRadius_;
	return Vector3(-std::sin(theta), 0.0f, std::cos(theta));
}


KamataEngine::Vector3 ConeTrack::GetLaneOffset(float s, float lane) const {
	auto theta = startAngle_ + s / baseRadius_;
	auto outward = Vector3(std::cos(theta), 0.0f, std::sin(theta));
	auto w = lane * GetLaneWidthAt(s);
	return outward * w + Vector3(0.0f, 1.0f, 0.0f) * (w * std::tan(slope_));
}


float ConeTrack::GetLength() const {
	return sweepAngle_ * baseRadius_;
}


bool ConeTrack::IsLoop() const {
	return sweepAngle_ >= 2.0f * std::numbers::pi_v<float>;
}