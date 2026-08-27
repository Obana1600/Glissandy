#pragma once

#include "Component.h"
#include <functional>

class Text2D;


class TextButton: public Component {
public: // メンバ関数

	void Initialize() override;

	void Update() override;

	void UpdateImGui() override;

	const std::string &GetName() const {
		return name_;
	}


	void SetOnClick(std::function<void()> callback);

	void SetNormalTint(const KamataEngine::Vector4 &tint);

	void SetHoverTint(const KamataEngine::Vector4 &tint);

	bool IsHovered() const {
		return isHovered_;
	}


private: // メンバ変数

	std::function<void()> onClick_;
	bool isHovered_ = false;
	bool isPressed_ = false;

	KamataEngine::Vector4 normalTint_ = {0.05f, 0.05f, 0.05f, 1.0f};
	KamataEngine::Vector4 hoverTint_ = {0.0f, 0.25f, 0.25f, 1.0f};
	KamataEngine::Vector4 clickTint_ = {0.0f, 0.5f, 0.5f, 1.0f};

	Text2D *text2D_ = nullptr;

	std::string name_ = "TextButton";
};