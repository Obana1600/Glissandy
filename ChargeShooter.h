#pragma once

#include "Component.h"
#include <cstdint>
#include <functional>
#include <memory>

class CollisionManager;



class ChargeShooter: public Component {
public: // メンバ関数

	void Update() override;

	void UpdateImGui() override;

	const std::string &GetName() const override {
		return name_;
	}


	void SetSpawnCallback(std::function<void(std::unique_ptr<GameObject>)> callback);

	void SetCamera(const KamataEngine::Camera *camera) {
		camera_ = camera;
	}

	void SetCollisionManager(const CollisionManager *collisionMgr) {
		collisionMgr_ = collisionMgr;
	}


	void StopCharge();

	float GetChargeRate() const;

	bool HasTargetInRange() const {
		return hasTarget_;
	}


private: // メンバ変数

	float chargeTimer_ = 0.0f;
	float minChargeTime_ = 0.5f;
	float maxChargeTime_ = 2.0f;

	float minRange_ = 50.0f;
	float maxRange_ = 3000.0f; // 最大チャージは実質無限
	int minDamage_ = 5;
	int maxDamage_ = 100;
	float bulletSpeed_ = 1000.0f;

	bool wasPressed_ = false;
	bool hasTarget_ = false;

	// チャージ音
	uint32_t chargeVoiceHandle_ = 0;
	bool isChargeSePlaying_ = false;

	std::function<void(std::unique_ptr<GameObject>)> spawnCallback_;

	const KamataEngine::Camera *camera_ = nullptr;
	const CollisionManager *collisionMgr_ = nullptr;

	std::string name_ = "ChargeShooter";


private: // メンバ関数

	void StopChargeSe();

	void Fire(const KamataEngine::Vector3 &aimPoint);

	KamataEngine::Vector3 GetForward() const;
	float GetRange() const;
	int GetDamage() const;
};