#include "SceneManager.h"

#include "DebugUi.h"
#include "Tween/TweenManager.h"

using namespace KamataEngine;



SceneManager *SceneManager::GetInstance() {
	static SceneManager instance;
	return &instance;
}


void SceneManager::Initialize() {
#ifdef USE_IMGUI
	ChangeScene(currentSceneType_);
#else
	ChangeScene(SceneType::Title);
#endif
}


void SceneManager::Finalize() {
	currentScene_.reset();
	nextScene_.reset();

	TweenManager::GetInstance()->Clear();
}


void SceneManager::Update() {
	if (nextScene_) {
		currentScene_ = std::move(nextScene_);
		TweenManager::GetInstance()->Clear();

		currentScene_->Initialize();
	}

	if (currentScene_) {
		currentScene_->Update();
	}
}


void SceneManager::Draw() const {
	if (!currentScene_) {
		assert(false && "現在のシーンがありません");
		return;
	}

	currentScene_->Draw();
}


void SceneManager::UpdateImGui() {
#ifdef USE_IMGUI
	// currentSceneName
	ImGui::Text("Current Scene: %s", currentScene_->GetName().c_str());

	ImGui::Spacing();

	// changeScene
	if (ImGui::TreeNode("ChengeScene")) {
		ImGui::Indent();

		// テーブル
		if (DebugUi::BeginTable("ChangeScene")) {

			// title
			DebugUi::PropatyRow("Title", [&] {
				if (ImGui::RadioButton("##title", currentSceneType_ == SceneType::Title)) {
					currentSceneType_ = SceneType::Title;
					ChangeScene(currentSceneType_);
				}
			});

			// game
			DebugUi::PropatyRow("Game", [&] {
				if (ImGui::RadioButton("##game", currentSceneType_ == SceneType::Game)) {
					currentSceneType_ = SceneType::Game;
					ChangeScene(currentSceneType_);
				}
			});

			DebugUi::EndTable();
		}

		ImGui::Unindent();
		ImGui::TreePop();
	}
#endif
}


void SceneManager::ChangeScene(SceneType scene) {
	nextScene_ = CreateScene(scene);
}