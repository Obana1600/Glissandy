#pragma once

#include "Component.h"
#include <cstdint>
#include <memory>

class ChargeShooter;


class Reticle: public Component {
public: // メンバ関数

	void Initialize() override;

	void Update() override;

	void Draw(const KamataEngine::Camera &) const override;

	void UpdateImGui() override;

	const std::string &GetName() const {
		return name_;
	}

	DrawPass GetDrawPass() const override {
		return DrawPass::Sprite;
	}


private: // メンバ変数

	std::unique_ptr<KamataEngine::Sprite> sprite_;
	uint32_t textureHandle_ = 0;

	KamataEngine::Vector2 minSize_ = {128.0f, 128.0f};
	KamataEngine::Vector2 maxSize_ = {64.0f, 64.0f};
	KamataEngine::Vector4 defaultColor_ = {1.0f, 1.0f, 1.0f, 1.0f};
	KamataEngine::Vector4 targetColor_ = {1.0f, 0.0f, 0.0f, 1.0f};

	const ChargeShooter *chargeShooter_ = nullptr;

	std::string name_ = "Reticle";
};