#pragma once

#include "Component.h"
#include "ParticleManager.h"


class ParticleEmitter: public Component {
public: // メンバ関数

	void Initialize() override;

	void Update() override;

	void UpdateImGui() override;

	const std::string &GetName() const {
		return name_;
	}


	void Play();

	void Stop();

	void Burst();

	const EmitParams &GetParams() const {
		return params_;
	}

	void SetParams(const EmitParams &params) {
		params_ = params;
	}

	void SetBurstCount(int burstCount) {
		burstCount_ = burstCount;
	}

	void SetRatePerSecond(float ratePerSecond) {
		ratePerSecond_ = ratePerSecond;
	}

	void SetOffset(const KamataEngine::Vector3 &offset) {
		offset_ = offset;
	}

	// 1つのオブジェクトに複数挿すとInspectorの見出しが同じになるので、名前で見分ける
	void SetName(const std::string &name) {
		name_ = name;
	}

	void SetParticleManager(ParticleManager *particleMgr) {
		particleMgr_ = particleMgr;
	}


private: // メンバ関数

	// 放出位置と方向を所有者へ追従させる
	void UpdateTransform();


private: // メンバ変数

	EmitParams params_{};

	int burstCount_ = 1;
	float duration_ = 0.0f;
	float ratePerSecond_ = 0.0f;
	bool loop_ = false;

	bool isPlaying_ = false;
	float elapsed_ = 0.0f;
	float emitAccumlator_ = 0.0f;

	KamataEngine::Vector3 offset_{};
	KamataEngine::Vector3 baseDirection_{};
	bool useOwnerRotation_ = false;

	ParticleManager *particleMgr_ = nullptr;

	std::string name_ = "ParticleEmitter";
};