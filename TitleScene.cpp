#include "TitleScene.h"

#include "CursorLock.h"
#include "DebugUi.h"
#include "MusicManager.h"
#include "Prefab.h"
#include "SceneFactory.h"
#include "SceneManager.h"
#include "Text/Text2D.h"
#include "TextButton.h"

using namespace KamataEngine;



void TitleScene::Initialize() {
	InitializeInternal();

	CursorLock::SetLocked(false);


	// camera -------------------------------------------------------------------------------------

	camera_.translation_ = {0.0f, 0.0f, 0.0f};
	camera_.UpdateMatrix();

	// --------------------------------------------------------------------------------------------


	// skydome ------------------------------------------------------------------------------------

	skydome_ = Prefab::CreateSkydome();

	// --------------------------------------------------------------------------------------------


	// ui -----------------------------------------------------------------------------------------

	logo_ = Prefab::CreateTitleLogo();
	logo_->SetTranslation(kLogoPosition_);

	startButton_ = Prefab::CreateTextButton("開始ssandy", kButtonRectSize_, [this] {
		RequestChangeScene(SceneType::Game);
	});
	startButton_->SetTranslation(kStartButtonPosition_);

	exitButton_ = Prefab::CreateTextButton("終了ssandy", kButtonRectSize_, [] {
		SceneManager::GetInstance()->RequestExit();
	});
	exitButton_->SetTranslation(kExitButtonPosition_);

	// --------------------------------------------------------------------------------------------


	// music --------------------------------------------------------------------------------------

	MusicManager::GetInstance()->PlayBgm("bgm_title");

	// --------------------------------------------------------------------------------------------
}


void TitleScene::Update() {

	// input ------------------------------------

	if (!DebugUi::IsCapturingKeyboard() && Input::GetInstance()->TriggerKey(kExitKey_)) {
		SceneManager::GetInstance()->RequestExit();
	}

	// ------------------------------------------


	// object -----------------------------------

	skydome_->Update();
	logo_->Update();
	startButton_->Update();
	exitButton_->Update();

	// ------------------------------------------


	// transition -------------------------------

	UpdateTransition();

	ApplyTransitionTint();

	// ------------------------------------------


	// gui --------------------------------------

	guiEditor_->Clear();

	guiEditor_->Register(logo_.get());
	guiEditor_->Register(startButton_.get());
	guiEditor_->Register(exitButton_.get());

	UpdateEditor();

	// ------------------------------------------
}


void TitleScene::Draw() const {

	Model::PreDraw(); // 3D描画ここから =============================================================

	skydome_->Draw(camera_, DrawPass::Model);

	Model::PostDraw(); // 3D描画ここまで ============================================================



	Sprite::PreDraw(); // 2d描画ここから ============================================================

	logo_->Draw(camera_, DrawPass::Sprite);

	Sprite::PostDraw(); // 2d描画ここまで ===========================================================



	// transition ---------------------------------------------------------------------------------

	DrawTransition();

	// --------------------------------------------------------------------------------------------


	// ui -----------------------------------------------------------------------------------------

	startButton_->Draw(camera_, DrawPass::Text2D);
	exitButton_->Draw(camera_, DrawPass::Text2D);

	// --------------------------------------------------------------------------------------------
}


void TitleScene::ApplyTransitionTint() {
	if (!transitionFade_ || !isTransitioning_) {
		return;
	}

	// フェードのアルファだけ文字の色を変える
	const auto brightness = 1.0f - transitionFade_->GetColor().w;
	const Vector4 tint = {brightness, brightness, brightness, 1.0f};

	for (auto *object : {startButton_.get(), exitButton_.get()}) {
		// 暗転中はクリックを受け付けない
		if (auto *button = object->GetComponent<TextButton>()) {
			button->SetEnabled(!isTransitioning_);
		}

		if (auto *text = object->GetComponent<Text2D>()) {
			text->SetTint(tint);
		}
	}
}