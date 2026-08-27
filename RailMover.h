#pragma once

#include "Component.h"

class Track;
class TrackManager;


class RailMover: public Component {
public: // メンバ関数

	void Update() override;

	void UpdateImGui() override;

	const std::string &GetName() const override {
		return name_;
	}


	void ChangeLane(int direction);

	void SetTrack(const Track *track, float progress = 0.0f, int laneIndex = 0);

	void SetTrackManager(const TrackManager *trackManager) {
		trackManager_ = trackManager;
	}

	float GetSpeed() const {
		return speed_;
	}

	void SetSpeed(float speed) {
		speed_ = speed;
	}

	float GetProgress() const {
		return progress_;
	}

	const Track *GetTrack() const {
		return track_;
	}

	void SetProgress(float progress) {
		progress_ = progress;
	}

	int GetTargetLaneIndex() const {
		return targetLaneIndex_;
	}

	float GetLane() const {
		return lane_;
	}

	KamataEngine::Vector3 GetPointOnRail() const;


private: // メンバ変数

	float progress_ = 0.0f;
	float speed_ = 0.0f;

	int targetLaneIndex_ = 0;
	float lane_ = 0.0f;
	float laneStart_ = 0.0f;
	float laneTimer_ = 0.0f;
	float laneDuration_ = 0.5f;

	bool faceTangent_ = true;

	const Track *track_ = nullptr;
	static inline const TrackManager *trackManager_ = nullptr;

	std::string name_ = "RailMover";


private: // メンバ関数

	void SetLaneIndex(int index);

	static int RemapLaneIndex(const Track &from, const Track &to, int index);
};