#include "SpriteRenderer.h"

#include "DebugUi.h"
#include "GameObject.h"

using namespace KamataEngine;



void SpriteRenderer::Initialize() {
	auto textureHandle = TextureManager::Load(textureName_);

	sprite_ = std::unique_ptr<Sprite>(Sprite::Create(textureHandle, {0.0f, 0.0f}));
	sprite_->SetAnchorPoint(anchorPoint_);
	sprite_->SetSize(size_);
	sprite_->SetColor(color_);
}


void SpriteRenderer::Update() {
	if (!sprite_) {
		return;
	}

	const auto &translation = owner_->GetTranslation();
	sprite_->SetPosition({
		WinApp::kWindowWidth * 0.5f + translation.x,
		WinApp::kWindowHeight * 0.5f - translation.y
	});

	const auto &scale = owner_->GetScale();
	sprite_->SetSize({size_.x * scale.x, size_.y * scale.y});

	sprite_->SetRotation(owner_->GetRotation().z);
}


void SpriteRenderer::Draw(const KamataEngine::Camera &) const {
	if (!sprite_) {
		return;
	}

	sprite_->Draw();
}


void SpriteRenderer::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// textureName
		DebugUi::PropatyRow("TextureName", [&] {
			ImGui::Text("%s", textureName_.c_str());
		});

		// size
		DebugUi::PropatyRow("Size", [&] {
			ImGui::DragFloat2("##size", &size_.x, 1.0f, 0.0f, FLT_MAX);
		});

		// anchorPoint
		DebugUi::PropatyRow("AnchorPoint", [&] {
			auto anchorPoint = anchorPoint_;
			if (ImGui::DragFloat2("##anchorPoint", &anchorPoint.x, 0.01f, 0.0f, 1.0f)) {
				SetAnchorPoint(anchorPoint);
			}
		});

		// color
		DebugUi::PropatyRow("Color", [&] {
			auto color = color_;
			if (ImGui::ColorEdit4("##color", &color.x, ImGuiColorEditFlags_NoInputs)) {
				SetColor(color);
			}
		});

		DebugUi::EndTable();
	}

	// sprite
	if (sprite_) {
		ImGui::Text("Sprite is assigned!");
	} else {
		ImGui::Text("Sprite is not assigned.");
	}
#endif
}


void SpriteRenderer::SetColor(const KamataEngine::Vector4 &color) {
	color_ = color;

	if (sprite_) {
		sprite_->SetColor(color_);
	}
}


void SpriteRenderer::SetAnchorPoint(const KamataEngine::Vector2 &anchorPoint) {
	anchorPoint_ = anchorPoint;

	if (sprite_) {
		sprite_->SetAnchorPoint(anchorPoint_);
	}
}