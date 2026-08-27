#pragma once

#include "KamataEngine.h"
#include "SceneBase.h"
#include "SceneFactory.h"
#include <memory>


class SceneManager {
public: // 静的メンバ関数

	static SceneManager *GetInstance();


public: // メンバ関数

	void Initialize();

	void Finalize();

	void Update();

	void Draw() const;

	void UpdateImGui();

	void ChangeScene(SceneType scene);


	void RequestExit() {
		isExitRequested_ = true;
	}

	bool IsExitRequested() const {
		return isExitRequested_;
	}

	std::string GetCurrentSceneName() const {
		return currentScene_ ? currentScene_->GetName()
							 : "CurrentScene is not assigned.";
	}


private: // メンバ変数

	std::unique_ptr<SceneBase> currentScene_;
	std::unique_ptr<SceneBase> nextScene_;

	bool isExitRequested_ = false;

	// デバッグ時の開始シーン
	SceneType currentSceneType_ = SceneType::Game;


private: // メンバ関数

	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(const SceneManager &) = delete;
	SceneManager operator=(const SceneManager &) = delete;
};