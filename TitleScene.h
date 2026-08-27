#pragma once

#include "GameObject.h"
#include "SceneBase.h"
#include <memory>


class TitleScene: public SceneBase {
public: // メンバ関数

	void Initialize() override;

	void Update() override;

	void Draw() const override;

	const std::string &GetName() const override {
		return name_;
	}


private: // メンバ変数

	// skydome
	std::unique_ptr<GameObject> skydome_;

	// ui
	std::unique_ptr<GameObject> logo_;
	std::unique_ptr<GameObject> startButton_;
	std::unique_ptr<GameObject> exitButton_;

	// 終了キー
	static constexpr BYTE kExitKey_ = DIK_ESCAPE;

	// レイアウト
	const KamataEngine::Vector3 kLogoPosition_ = {0.0f, 100.0f, 0.0f};
	const KamataEngine::Vector2 kButtonRectSize_ = {360.0f, 90.0f};
	const KamataEngine::Vector3 kStartButtonPosition_ = {0.0f, -140.0f, 0.0f};
	const KamataEngine::Vector3 kExitButtonPosition_ = {0.0f, -240.0f, 0.0f};

	std::string name_ = "Title";


private: // メンバ関数

	void ApplyTransitionTint();
};