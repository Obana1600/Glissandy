#pragma once

#include "Component.h"
#include <functional>


class Bullet: public Component {
public: // メンバ関数

	void Update() override;

	void UpdateImGui() override;

	const std::string &GetName() const override {
		return name_;
	}


	GameObject *GetOwner() {
		return owner_;
	}

	void SetVelocity(const KamataEngine::Vector3 &velocity) {
		velocity_ = velocity;
	}

	void SetRange(float range) {
		range_ = range;
	}

	int GetDamage() const {
		return damage_;
	}

	void SetDamage(int damage) {
		damage_ = damage;
	}

	void SetOnExpire(std::function<void()> onExpire) {
		onExpire_ = std::move(onExpire);
	}


private: // メンバ変数

	KamataEngine::Vector3 velocity_{};
	float traveled_ = 0.0f; // 飛距離
	float range_ = 100.0f;	// 最大飛距離
	int damage_ = 1;

	std::function<void()> onExpire_;

	std::string name_ = "Bullet";
};