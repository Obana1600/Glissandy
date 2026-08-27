#pragma once

#include "Component.h"
#include "TextRenderer.h"


class Text3D: public Component {
public: // メンバ関数

	void Initialize();

	void Draw(const KamataEngine::Camera &camera) const override;

	void UpdateImGui() override;

	const std::string &GetName() const override {
		return name_;
	}

	DrawPass GetDrawPass() const override {
		return DrawPass::Text3D;
	}


	void SetText(const std::string &text);

	void SetRectSize(const KamataEngine::Vector2 &size);


private: // メンバ変数

	static constexpr float kFontSizePerUnit = 12.0f;

	KamataEngine::Vector2 rectSize_ = {20.0f, 10.0f};
	KamataEngine::Vector2 pivot_ = {0.5f, 0.5f};

	TextBase text_{};
	TextRenderer renderer_{};

	bool showRect_ = true;
	bool showPivot_ = true;

	std::string name_ = "Text3D";


private: // メンバ関数

	KamataEngine::Matrix4x4 MakeWvp(const KamataEngine::Camera &camera) const;
};