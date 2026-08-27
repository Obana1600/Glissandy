#pragma once

#include "KamataEngine.h"

class GameObject;


class FollowCamera {
public: // メンバ関数

	void Update();

	void UpdateImGui();


	void SetCamera(KamataEngine::Camera *camera) {
		camera_ = camera;
	}

	void SetTarget(const GameObject *target) {
		target_ = target;
	}


private: // メンバ変数

	KamataEngine::Camera *camera_ = nullptr;
	const GameObject *target_ = nullptr;

	KamataEngine::Vector3 offset_ = {0.0f, 5.0f, -20.0f};
	float followRate_ = 0.1f;

	KamataEngine::Vector2 smoothedMove_{};
	float smoothing_ = 0.5f;

	KamataEngine::Vector2 angle_{};
	float sensitivity_ = 0.001f;
	float pitchLimit_ = 1.0f;


private: // メンバ関数

	void UpdateLook();

	void UpdateFollow();
};