#include "Reticle.h"

#include "ChargeShooter.h"
#include "DebugUi.h"
#include "GameObject.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void Reticle::Initialize() {
	textureHandle_ = TextureManager::Load("reticle.png");

	sprite_ = std::unique_ptr<Sprite>(Sprite::Create(textureHandle_, {0.0f, 0.0f}));
	sprite_->SetPosition({WinApp::kWindowWidth / 2.0f, WinApp::kWindowHeight / 2.0f});
	sprite_->SetAnchorPoint({0.5f, 0.5f});

	if (auto *chargeShooter = owner_->GetComponent<ChargeShooter>()) {
		chargeShooter_ = chargeShooter;
	}
}


void Reticle::Update() {
	if (!chargeShooter_) {
		return;
	}

	auto t = chargeShooter_->GetChargeRate();
	auto sizeX = Lerp(minSize_.x, maxSize_.x, t);
	auto sizeY = Lerp(minSize_.y, maxSize_.y, t);
	sprite_->SetSize({sizeX, sizeY});

	if (chargeShooter_->HasTargetInRange()) {
		sprite_->SetColor(targetColor_);
	} else {
		sprite_->SetColor(defaultColor_);
	}
}


void Reticle::Draw(const KamataEngine::Camera &) const {
	sprite_->Draw();
}


void Reticle::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// position
		DebugUi::PropatyRow("Position", [&] {
			auto position = sprite_->GetPosition();
			if (ImGui::DragFloat2("##position", &position.x, 0.01f)) {
				sprite_->SetPosition(position);
			}
		});

		// currentSize
		DebugUi::PropatyRow("CurrentSize", [&] {
			ImGui::Text("%f : %f", sprite_->GetSize().x, sprite_->GetSize().y);
		});

		// minSize
		DebugUi::PropatyRow("MinSize", [&] {
			ImGui::DragFloat2("##minSize", &minSize_.x, 0.01f);
		});

		// maxSize
		DebugUi::PropatyRow("MaxSize", [&] {
			ImGui::DragFloat2("##maxSize", &maxSize_.x, 0.01f);
		});

		// defaultColor
		DebugUi::PropatyRow("DefaultColor", [&] {
			ImGui::ColorEdit4("##defaultColor", &defaultColor_.x, ImGuiColorEditFlags_NoInputs);
		});

		// targetColor
		DebugUi::PropatyRow("TargetColor", [&] {
			ImGui::ColorEdit4("##targetColor", &targetColor_.x, ImGuiColorEditFlags_NoInputs);
		});

		DebugUi::EndTable();
	}

	// chargeShooter
	if (chargeShooter_) {
		ImGui::Text("ChargeShooter is assigned!");
	} else {
		ImGui::Text("ChargeShooter is not assigned.");
	}
#endif
}