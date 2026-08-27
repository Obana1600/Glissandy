#pragma once

#include "Component.h"
#include <memory>
#include <string>


class SpriteRenderer: public Component {
public: // メンバ関数

	void Initialize() override;

	void Update() override;

	void Draw(const KamataEngine::Camera &camera) const override;

	void UpdateImGui() override;

	const std::string &GetName() const override {
		return name_;
	}

	DrawPass GetDrawPass() const override {
		return DrawPass::Sprite;
	}


	// getter -------------------------------------------------------------------------------------

	const KamataEngine::Vector2 &GetSize() const {
		return size_;
	}

	const KamataEngine::Vector4 &GetColor() const {
		return color_;
	}

	// --------------------------------------------------------------------------------------------


	// setter -------------------------------------------------------------------------------------

	void SetTextureName(const std::string &textureName) {
		textureName_ = textureName;
	}

	void SetSize(const KamataEngine::Vector2 &size) {
		size_ = size;
	}

	void SetColor(const KamataEngine::Vector4 &color);

	void SetAnchorPoint(const KamataEngine::Vector2 &anchorPoint);

	// --------------------------------------------------------------------------------------------


private: // メンバ変数

	std::unique_ptr<KamataEngine::Sprite> sprite_;

	std::string textureName_ = "white1x1.png";

	KamataEngine::Vector2 size_ = {100.0f, 100.0f};
	KamataEngine::Vector2 anchorPoint_ = {0.5f, 0.5f};
	KamataEngine::Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	std::string name_ = "SpriteRenderer";
};