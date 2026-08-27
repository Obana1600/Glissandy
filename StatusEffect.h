#pragma once

#include "Component.h"
#include "StatusAttribute.h"
#include <array>


class StatusEffect: public Component {
public:	 // メンバ関数

	void Update() override;

	void UpdateImGui() override;

	const std::string &GetName() const {
		return name_;
	}


	void Apply(StatusAttribute status, float duration);

	bool Has(StatusAttribute status) const;


private: // メンバ変数

	std::array<float, kStatusCount> timers_{};

	std::string name_ = "StatusEffect";
};