#pragma once

#include "Component.h"
#include "TextRenderer.h"
#include <array>


class Text2D: public Component {
public: // メンバ関数

	void Initialize() override;

	void Draw(const KamataEngine::Camera &) const override;

	void UpdateImGui() override;

	const std::string &GetName() const override {
		return name_;
	}

	DrawPass GetDrawPass() const override {
		return DrawPass::Text2D;
	}


	void SetText(const std::string &text);

	void SetCornerColor(const std::array<KamataEngine::Vector4, 4> &color);

	void SetTint(const KamataEngine::Vector4 &tint);

	void SetRectSize(const KamataEngine::Vector2 &size);

	void SetFontSize(float fontSize);

	std::array<KamataEngine::Vector2, 2> GetScreenRect() const;


private: // メンバ変数

	KamataEngine::Vector2 rectSize_ = {1280.0f, 720.0f};
	KamataEngine::Vector2 pivot_ = {0.5f, 0.5f};
	KamataEngine::Vector2 anchorMin_ = {0.5f, 0.5f};
	KamataEngine::Vector2 anchorMax_ = {0.5f, 0.5f};

	// テキスト描画
	TextBase text_{};
	TextRenderer renderer_{};

	// imgui
	bool showRect_ = true;
	bool showPivot_ = true;

	std::string name_ = "Text2D";

	static inline const KamataEngine::Matrix4x4 kOrthoMat_ =
		KamataEngine::MathUtility::MakeOrthographicMatrix(
			0, KamataEngine::WinApp::kWindowHeight, KamataEngine::WinApp::kWindowWidth, 0, -1000, 1000
		);


private: // メンバ関数

	KamataEngine::Matrix4x4 MakeWvp() const;
};