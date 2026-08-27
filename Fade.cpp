#include "Fade.h"

#include "DebugUi.h"
#include "TimeT.h"
#include "Tween/EaseFunc.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void Fade::Initialize() {
	auto textureHandle = TextureManager::Load(textureName_);

	sprite_ = std::unique_ptr<Sprite>(Sprite::Create(textureHandle, {0.0f, 0.0f}));
	sprite_->SetSize({WinApp::kWindowWidth, WinApp::kWindowHeight});
	sprite_->SetColor(currentColor_);
}


void Fade::Update() {
	if (IsFinished()) {
		return;
	}

	timer_ += TimeT::GetDeltaTime();
	auto t = std::clamp(timer_ / duration_, 0.0f, 1.0f);
	auto eased = EaseFunc::OutCubic(t);

	currentColor_ = {
		Lerp(startColor_.x, targetColor_.x, eased),
		Lerp(startColor_.y, targetColor_.y, eased),
		Lerp(startColor_.z, targetColor_.z, eased),
		Lerp(startColor_.w, targetColor_.w, eased)
	};

	sprite_->SetColor(currentColor_);
}


void Fade::Draw() const {
	if (currentColor_.w <= 0.0f) {
		return;
	}

	sprite_->Draw();
}




void Fade::UpdateImGui() {
#ifdef USE_IMGUI
	ImGui::Begin(name_.c_str());

	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// simulate
		DebugUi::PropatyRow("Simulate", [&] {
			if (ImGui::Button("Start")) {
				Start(targetColor_, inputDuration_);
			}
			ImGui::SameLine();
			if (ImGui::Button("SetColor")) {
				SetColor(targetColor_);
			} }, false);

		// currnetColor
		DebugUi::PropatyRow("CurrentColor", [&] {
			ImGui::ColorEdit4("##currentColor", &currentColor_.x, ImGuiColorEditFlags_NoInputs);
		});

		// startColor
		DebugUi::PropatyRow("StartColor", [&] {
			ImGui::ColorEdit4("##startColor", &startColor_.x, ImGuiColorEditFlags_NoInputs);
		});

		// targetColor
		DebugUi::PropatyRow("TargetColor", [&] {
			ImGui::ColorEdit4("##targetColor", &targetColor_.x, ImGuiColorEditFlags_NoInputs);
		});

		// timer
		DebugUi::PropatyRow("Timer", [&] {
			ImGui::Text("%.2f / %.2f", timer_, duration_);
		});

		// inputDuration
		DebugUi::PropatyRow("Duration", [&] {
			ImGui::DragFloat("##inputDuration", &inputDuration_, 0.01f, 0.0f, FLT_MAX);
		});

		DebugUi::EndTable();
	}

	ImGui::End();
#endif
}


void Fade::Start(const KamataEngine::Vector4 &target, float duration) {
	startColor_ = currentColor_;
	targetColor_ = target;

	if (duration <= 0.0f) {
		SetColor(target);
		return;
	}

	duration_ = duration;
	timer_ = 0.0f;
}


void Fade::SetColor(const KamataEngine::Vector4 &color) {
	startColor_ = color;
	currentColor_ = color;
	targetColor_ = color;

	duration_ = 0.0f;
	timer_ = 0.0f;

	if (sprite_) {
		sprite_->SetColor(color);
	}
}


bool Fade::IsFinished() const {
	return timer_ >= duration_;
}