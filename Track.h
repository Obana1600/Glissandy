#pragma once

#include "KamataEngine.h"
#include <cstdint>
#include <memory>
#include <vector>


class Track {
public: // メンバ関数

	virtual ~Track() = default;

	virtual void UpdateImGui();

	virtual void AlignTo(const Track &) {};

	virtual KamataEngine::Vector3 GetPoint(float s) const = 0;

	virtual KamataEngine::Vector3 GetTangent(float s) const = 0;

	virtual KamataEngine::Vector3 GetLaneOffset(float s, float lane) const = 0;

	virtual float GetLength() const = 0;

	virtual bool IsLoop() const = 0;

	virtual const std::string &GetName() const = 0;


	void DrawDebug() const;

	// レールの見た目を作り直す。
	void BuildRail();

	// レールの描画
	void DrawRail(const KamataEngine::Camera &camera) const;

	float GetLaneValue(int index) const;

	float GetLaneWidthAt(float s) const;


	int GetLaneCount() const {
		return laneCount_;
	}

	const Track *GetNext() const {
		return next_;
	}

	Track *GetNext() {
		return next_;
	}

	void SetLaneCount(int laneCount) {
		laneCount_ = laneCount;
	}

	void SetStartWidth(float width) {
		startWidth_ = width;
	}

	void SetEndWidth(float width) {
		endWidth_ = width;
	}

	void SetNext(Track *next) {
		next_ = next;
	}


protected: // メンバ変数

	// レーン数
	int laneCount_ = 1;

	// レーン間の幅
	float startWidth_ = 2.0f;
	float endWidth_ = 2.0f;

	// 次のtrack
	Track *next_ = nullptr;

	// rail
	std::vector<std::unique_ptr<KamataEngine::WorldTransform>> railTransforms_;
	int railCount_ = 0;

	int railDivision_ = 72;
	float railWidth_ = 0.8f;
	float railThickness_ = 0.25f;
	float railOffsetY_ = -1.33f;

	KamataEngine::Model *railModel_ = nullptr;
	KamataEngine::ObjectColor railColor_;
	KamataEngine::Vector4 railColorValue_ = {0.5f, 0.5f, 0.5f, 1.0f};
	bool isRailReady_ = false;

	// 継ぎ目のV字を埋めるための伸び代
	static constexpr float kRailOverlap_ = 1.03f;

	// debug
	int debugDivision_ = 32;
	KamataEngine::Vector4 debugColorCenter_ = {0.5f, 0.5f, 0.5f, 1.0f};
	KamataEngine::Vector4 debugColorLane_ = {1.0f, 1.0f, 1.0f, 1.0f};


protected: // メンバ関数

	// ボックス1つをp0〜p1に合わせる
	void SetupRailSegment(KamataEngine::WorldTransform &transform, const KamataEngine::Vector3 &p0, const KamataEngine::Vector3 &p1) const;
};