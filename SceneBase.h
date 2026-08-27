#pragma once

#include "Fade.h"
#include "GuiEditor.h"
#include "KamataEngine.h"
#include <string>

enum class SceneType;



class SceneBase {
public: // メンバ関数

	virtual ~SceneBase() = default;

	virtual void Initialize() = 0;

	virtual void Update() = 0;

	virtual void Draw() const = 0;

	virtual const std::string &GetName() const = 0;


	// シーン遷移の要求
	void RequestChangeScene(SceneType scene);


protected: // メンバ変数

	KamataEngine::Camera camera_;

	// 最大深度
	static constexpr float kFarZ_ = 3000.0f;

	std::unique_ptr<GuiEditor> guiEditor_;

	// シーン遷移のフェード
	std::unique_ptr<Fade> transitionFade_;
	SceneType nextSceneType_{};
	bool isTransitioning_ = false;

	const KamataEngine::Vector4 kTransitionColor_ = {0.0f, 0.0f, 0.0f, 1.0f};
	const KamataEngine::Vector4 kTransitionClearColor_ = {0.0f, 0.0f, 0.0f, 0.0f};
	const float kTransitionDuration_ = 1.0f;

#ifdef USE_IMGUI
	KamataEngine::DebugCamera debugCamera_{KamataEngine::WinApp::kWindowWidth, KamataEngine::WinApp::kWindowHeight};
	bool isActiveDebug_ = true;
	static constexpr BYTE kDebugActiveKey_ = DIK_1;
#endif


protected: // メンバ関数

	void InitializeInternal();

	void UpdateCamera();
	virtual void UpdateGameCamera();

	// フェードの更新
	void UpdateTransition();

	// フェードの描画
	void DrawTransition() const;

	void UpdateEditor();
};