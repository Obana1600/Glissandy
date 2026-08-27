#pragma once

#include "Track.h"


class StraightTrack: public Track {
public: // メンバ関数

	void UpdateImGui() override;

	void AlignTo(const Track &prev) override;

	KamataEngine::Vector3 GetPoint(float s) const override;

	KamataEngine::Vector3 GetTangent(float s) const override;

	KamataEngine::Vector3 GetLaneOffset(float s, float lane) const override;

	float GetLength() const override;

	bool IsLoop() const override {
		return kIsLoop_;
	};

	const std::string &GetName() const override {
		return name_;
	}



	const KamataEngine::Vector3 &GetStart() const {
		return start_;
	}

	const KamataEngine::Vector3 &GetEnd() const {
		return end_;
	}

	void SetStart(const KamataEngine::Vector3 &start) {
		start_ = start;
	}

	void SetEnd(const KamataEngine::Vector3 &end) {
		end_ = end;
	}


private: // メンバ変数

	KamataEngine::Vector3 start_{};
	KamataEngine::Vector3 end_{};

	const bool kIsLoop_ = false;

	std::string name_ = "StraightTrack";
};