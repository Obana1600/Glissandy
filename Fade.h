#pragma once

#include "KamataEngine.h"
#include <memory>
#include <string>


class Fade {
public: // メンバ関数

	void Initialize();

	void Update();

	void Draw() const;

	void UpdateImGui();


	void Start(const KamataEngine::Vector4 &target, float duration);

	void SetColor(const KamataEngine::Vector4 &color);

	bool IsFinished() const;

	const KamataEngine::Vector4 &GetColor() const {
		return currentColor_;
	}

	const std::string &GetName() const {
		return name_;
	}

	void SetName(const std::string &name) {
		name_ = name;
	}


private: // メンバ変数

	std::unique_ptr<KamataEngine::Sprite> sprite_;
	const std::string textureName_ = "white1x1.png";

	KamataEngine::Vector4 currentColor_ = {1.0f, 1.0f, 1.0f, 0.0f};
	KamataEngine::Vector4 startColor_{};
	KamataEngine::Vector4 targetColor_{};

	float duration_ = 0.0f;
	float timer_ = 0.0f;

	// imgui
	float inputDuration_ = 0.0f;

	std::string name_ = "Fade";
};