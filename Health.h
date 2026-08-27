#pragma once

#include "Component.h"
#include <functional>


class Health: public Component {
public: // メンバ関数

	void UpdateImGui() override;

	const std::string &GetName() const override {
		return name_;
	}


	void TakeDamage(int damage);

	void SetOnDeath(std::function<void()> callback);

	bool IsAlive() const;

	void SetMaxHp(int maxHp, bool restoreHp);

	void SetAutoDestroy(bool autoDestroy) {
		autoDestroy_ = autoDestroy;
	}

	bool IsAutoDestroy() const {
		return autoDestroy_;
	}


	int GetHp() const {
		return hp_;
	}

	int GetMaxHp() const {
		return maxHp_;
	}


private: // メンバ変数

	int maxHp_ = 1;
	int hp_ = maxHp_;

	std::function<void()> onDeath_;

	bool autoDestroy_ = true;

	std::string name_ = "Health";
};