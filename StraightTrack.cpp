#include "StraightTrack.h"

#include "DebugUi.h"
#include "Extension.h"
#include <cmath>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void StraightTrack::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// start
		DebugUi::PropatyRow("Start", [&] {
			ImGui::DragFloat3("##start", &start_.x, 1.0f);
		});

		// end
		DebugUi::PropatyRow("End", [&] {
			ImGui::DragFloat3("##end", &end_.x, 1.0f);
		});

		DebugUi::EndTable();
	}

	// 共通部分
	Track::UpdateImGui();
#endif
}


void StraightTrack::AlignTo(const Track &prev) {
	const auto length = prev.GetLength();
	const auto ownLength = GetLength();

	start_ = prev.GetPoint(length);
	end_ = start_ + prev.GetTangent(length) * ownLength;

	const auto prevSpan = (prev.GetLaneCount() - 1) * 0.5f;
	const auto ownSpan = (laneCount_ - 1) * 0.5f;
	startWidth_ = (ownSpan < 1e-6f)
					  ? 0.0f
					  : prev.GetLaneWidthAt(length) * prevSpan / ownSpan;
}


KamataEngine::Vector3 StraightTrack::GetPoint(float s) const {
	return start_ + GetTangent(s) * s;
}


KamataEngine::Vector3 StraightTrack::GetTangent(float s) const {
	(void)s;

	return Normalize(end_ - start_);
}


KamataEngine::Vector3 StraightTrack::GetLaneOffset(float s, float lane) const {
	const Vector3 up = {0.0f, 1.0f, 0.0f};
	auto right = Normalize(Cross(up, GetTangent(s)));

	return right * (lane * GetLaneWidthAt(s));
}


float StraightTrack::GetLength() const {
	return Length(end_ - start_);
}