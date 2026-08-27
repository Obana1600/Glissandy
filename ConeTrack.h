#pragma once

#include "Track.h"
#include <numbers>


class ConeTrack: public Track {
public: // メンバ関数

	void UpdateImGui() override;

	void AlignTo(const Track &prev) override;

	KamataEngine::Vector3 GetPoint(float s) const override;

	KamataEngine::Vector3 GetTangent(float s) const override;

	KamataEngine::Vector3 GetLaneOffset(float s, float lane) const override;

	float GetLength() const override;

	bool IsLoop() const override;

	const std::string &GetName() const {
		return name_;
	}


	void SetBaseRadius(float baseRadius) {
		baseRadius_ = baseRadius;
	}

	void SetSlope(float slope) {
		slope_ = slope;
	}

	void SetSweepAngle(float sweepAngle) {
		sweepAngle_ = sweepAngle;
	}


	const KamataEngine::Vector3 &GetCenter() const {
		return center_;
	}

	float GetBaseRadius() const {
		return baseRadius_;
	}


private: // メンバ変数

	KamataEngine::Vector3 center_{};
	float baseRadius_ = 100.0f;
	float slope_ = 1.0f;

	float startAngle_ = 0.0f; // 円の回転角
	float sweepAngle_ = 2.0f * std::numbers::pi_v<float>;

	std::string name_ = "ConeTrack";
};