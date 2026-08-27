#pragma once

#include "Component.h"

class RailMover;


class PlayerController: public Component {
public: // メンバ関数

	void Initialize() override;

	void Update() override;

	void UpdateImGui() override;

	const std::string &GetName() const {
		return name_;
	}


private: // メンバ変数

	float acceleration_ = 10.0f;
	float minSpeed_ = 10.0f;
	float maxSpeed_ = 60.0f;

	RailMover *railMover_ = nullptr;

	std::string name_ = "PlayerController";
};