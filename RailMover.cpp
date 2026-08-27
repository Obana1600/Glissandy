#include "RailMover.h"

#include "DebugUi.h"
#include "GameObject.h"
#include "TimeT.h"
#include "Track.h"
#include "TrackManager.h"
#include "Tween/EaseFunc.h"
#include <algorithm>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void RailMover::Update() {
	if (!track_) {
		return;
	}

	progress_ += speed_ * TimeT::GetDeltaTime();

	auto length = track_->GetLength();
	while (progress_ >= length) {
		if (track_->IsLoop()) {
			progress_ -= length;

		} else if (track_->GetNext()) {
			progress_ -= length;
			const auto *prev = track_;
			track_ = track_->GetNext(); // 乗り換え
			length = track_->GetLength();
			targetLaneIndex_ = RemapLaneIndex(*prev, *track_, targetLaneIndex_);

		} else {
			progress_ = length;
			speed_ = 0.0f;
			break;
		}
	}

	auto targetLane = track_->GetLaneValue(targetLaneIndex_);

	if (laneTimer_ < laneDuration_) {
		laneTimer_ += TimeT::GetDeltaTime();
		auto t = std::clamp(laneTimer_ / laneDuration_, 0.0f, 1.0f);
		lane_ = Lerp(laneStart_, targetLane, EaseFunc::OutCubic(t));

	} else {
		lane_ = targetLane;
	}

	owner_->SetTranslation(GetPointOnRail());

	if (faceTangent_) {
		auto t = track_->GetTangent(progress_);
		auto yaw = std::atan2(t.x, t.z);
		auto pitch = std::asin(-t.y);
		owner_->SetRotation({pitch, yaw, 0.0f});
	}
}


void RailMover::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// progress
		DebugUi::PropatyRow("Progress", [&] {
			ImGui::Text("%.2f / %.2f", progress_, track_ ? track_->GetLength() : 0.0f);
		});

		// speed
		DebugUi::PropatyRow("Speed", [&] {
			ImGui::DragFloat("##speed", &speed_, 0.1f);
		});

		// targetLaneIndex
		DebugUi::PropatyRow("TargetLaneIndex", [&] {
			if (track_) {
				auto index = targetLaneIndex_;
				if (ImGui::SliderInt("##targetLaneIndex", &index, 0, track_->GetLaneCount() - 1)) {
					SetLaneIndex(index);
				}
			} else {
				ImGui::Text("%d", targetLaneIndex_);
			}
		});

		// lane
		DebugUi::PropatyRow("Lane", [&] {
			ImGui::Text("%.2f", lane_);
		});

		// laneTimer
		DebugUi::PropatyRow("LaneTimer", [&] {
			ImGui::Text("%.2f / %.2f", laneTimer_, laneDuration_);
		});

		// laneDuration
		DebugUi::PropatyRow("LaneDuration", [&] {
			ImGui::DragFloat("##laneDuration", &laneDuration_, 0.01f, 0.0f, FLT_MAX);
		});

		// faceTangent
		DebugUi::PropatyRow("FaceTangent", [&] {
			ImGui::Checkbox("##faceTangent", &faceTangent_);
		});

		DebugUi::EndTable();
	}

	// track
	if (track_) {
		ImGui::Text("Track is assigned! (%s)", track_->GetName().c_str());
	} else {
		ImGui::Text("Track is not assigned.");
	}

	// trackManager
	if (trackManager_) {
		ImGui::Text("TrackManager is assigned!");
	} else {
		ImGui::Text("TrackManager is not assigned.");
	}
#endif
}


KamataEngine::Vector3 RailMover::GetPointOnRail() const {
	if (!track_) {
		return owner_->GetWorldTransform().translation_;
	}

	return track_->GetPoint(progress_) + track_->GetLaneOffset(progress_, lane_);
}


void RailMover::ChangeLane(int direction) {
	SetLaneIndex(targetLaneIndex_ + direction);
}


void RailMover::SetTrack(const Track *track, float progress, int laneIndex) {
	track_ = track;
	progress_ = progress;
	targetLaneIndex_ = track_ ? std::clamp(laneIndex, 0, track_->GetLaneCount() - 1)
							  : laneIndex;

	lane_ = track_ ? track_->GetLaneValue(targetLaneIndex_) : 0.0f;
	laneStart_ = lane_;
	laneTimer_ = laneDuration_;
}


void RailMover::SetLaneIndex(int index) {
	if (!track_) {
		return;
	}

	auto next = std::clamp(index, 0, track_->GetLaneCount() - 1);
	if (next == targetLaneIndex_) {
		return;
	}

	laneStart_ = lane_;
	targetLaneIndex_ = next;
	laneTimer_ = 0.0f;
}


int RailMover::RemapLaneIndex(const Track &from, const Track &to, int index) {
	const auto fromCount = from.GetLaneCount();
	const auto toCount = to.GetLaneCount();

	if (fromCount == toCount) {
		return index;
	}
	if (toCount <= 1) {
		return 0;
	}

	// 中心からの相対位置
	const auto fromHalf = (fromCount - 1) * 0.5f;
	const auto toHalf = (toCount - 1) * 0.5f;
	const auto t = (fromHalf < 1e-6f) ? 0.0f
									  : std::clamp(from.GetLaneValue(index) / fromHalf, -1.0f, 1.0f);

	return std::clamp(static_cast<int>(t * toHalf + toHalf + 0.5f), 0, toCount - 1);
}