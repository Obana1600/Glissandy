#include "SceneBase.h"

#include "CursorLock.h"
#include "DebugUi.h"
#include "MusicManager.h"
#include "SceneManager.h"

using namespace KamataEngine;



void SceneBase::InitializeInternal() {
	camera_.Initialize();
	camera_.farZ = kFarZ_;

	guiEditor_ = std::make_unique<GuiEditor>();

	// 明転で始める
	transitionFade_ = std::make_unique<Fade>();
	transitionFade_->Initialize();
	transitionFade_->SetName("TransitionFade");
	transitionFade_->SetColor(kTransitionColor_);
	transitionFade_->Start(kTransitionClearColor_, kTransitionDuration_);

	isTransitioning_ = false;

#ifdef USE_IMGUI
	debugCamera_.SetFarZ(kFarZ_);

	CursorLock::SetLocked(!isActiveDebug_);
#else
	CursorLock::SetLocked(true);
#endif
}


void SceneBase::UpdateCamera() {
#ifdef USE_IMGUI
	if (!DebugUi::IsCapturingKeyboard()) {
		if (Input::GetInstance()->TriggerKey(kDebugActiveKey_)) {
			isActiveDebug_ = !isActiveDebug_;
			AxisIndicator::GetInstance()->SetVisible(isActiveDebug_);
			CursorLock::SetLocked(!isActiveDebug_);
		}
	}

	if (isActiveDebug_) {
		if (!DebugUi::IsCapturingMouse()) {
			debugCamera_.Update();
		}

		camera_.matView = debugCamera_.GetCamera().matView;
		camera_.matProjection = debugCamera_.GetCamera().matProjection;

		camera_.TransferMatrix();
		return;
	}
#endif

	UpdateGameCamera();
}


void SceneBase::UpdateGameCamera() {
	camera_.UpdateMatrix();
}


void SceneBase::RequestChangeScene(SceneType scene) {
	if (isTransitioning_) {
		return;
	}

	nextSceneType_ = scene;
	isTransitioning_ = true;

	transitionFade_->Start(kTransitionColor_, kTransitionDuration_);
}


void SceneBase::UpdateTransition() {
	if (!transitionFade_) {
		return;
	}

	transitionFade_->Update();

	if (!isTransitioning_ || !transitionFade_->IsFinished()) {
		return;
	}

	isTransitioning_ = false;
	SceneManager::GetInstance()->ChangeScene(nextSceneType_);
}


void SceneBase::DrawTransition() const {
	if (!transitionFade_) {
		return;
	}

	Sprite::PreDraw();
	transitionFade_->Draw();
	Sprite::PostDraw();
}


void SceneBase::UpdateEditor() {
	if (transitionFade_) {
		transitionFade_->UpdateImGui();
	}

	MusicManager::GetInstance()->UpdateImGui();

	if (!guiEditor_) {
		return;
	}

	guiEditor_->Update(camera_);
}