#pragma once

#include "Component.h"
#include <functional>
#include <memory>
#include <random>

class Health;
class Track;


// ボスの行動
enum class BossAction {
	PlaceRocks, // 岩をレール上に置く
	HomingShot, // 追尾弾を撃つ
	Ripple,		// 波紋を広げる
	Chase,		// レールに乗ってプレイヤーを追う

	NumCount
};



class BossController: public Component {
public: // メンバ関数

	void Initialize() override;

	void Update() override;

	void UpdateImGui() override;

	const std::string &GetName() const override {
		return name_;
	}


	void SetSpawnCallback(std::function<void(std::unique_ptr<GameObject>)> callback) {
		spawnCallback_ = std::move(callback);
	}

	void SetRockCountCallback(std::function<int()> callback) {
		rockCountCallback_ = std::move(callback);
	}

	void SetTrack(const Track *track) {
		track_ = track;
	}

	void SetPlayer(GameObject *player) {
		player_ = player;
	}


private: // インナークラス

	// HP段階ごとに変わる定数
	struct StageParams {
		int rockCount;	 // 一度に置く岩の数
		int rippleCount; // 一度に出す波紋の数
	};


private: // メンバ変数

	int stage_ = 0;
	float hpRateStage_ = 0.5f;

	StageParams stageParams_[2] = {
		{.rockCount = 2, .rippleCount = 2},
		{.rockCount = 5, .rippleCount = 4},
	};

	int maxRockCount_ = 10;

	float idleTimer_ = 0.0f;
	float idleDuration_ = 5.0f;

	bool isActing_ = false;

	BossAction lastAction_ = BossAction::NumCount;

	float windupTime_ = 0.6f;
	float recoverTime_ = 0.5f;
	float windupScale_ = 0.8f;

	std::function<void(std::unique_ptr<GameObject>)> spawnCallback_;
	std::function<int()> rockCountCallback_;

	const Track *track_ = nullptr;
	GameObject *player_ = nullptr;
	Health *health_ = nullptr;

	std::mt19937 rng_{std::random_device{}()};

	std::string name_ = "BossController";


private: // メンバ関数

	BossAction PickAction();

	void PlayActionSequence(std::function<void()> onFire);

	void FirePlaceRocks();

	float Random(float min, float max);
	int RandomInt(int min, int max);
};