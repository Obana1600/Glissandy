#include "TextButton.h"

#include "DebugUi.h"
#include "GameObject.h"
#include "MusicManager.h"
#include "Text/Text2D.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void TextButton::Initialize() {
	if (auto *text2D = owner_->GetComponent<Text2D>()) {
		text2D_ = text2D;
	}
}


void TextButton::Update() {
#ifdef USE_IMGUI
	if (DebugUi::IsCapturingMouse()) {
		isHovered_ = false;
		isPressed_ = false;
		return;
	}
#endif

	if (!text2D_) {
		return;
	}

	const auto *input = Input::GetInstance();
	const auto rect = text2D_->GetScreenRect();
	const auto &mouse = input->GetMousePosition();

	// aabb判定
	isHovered_ = mouse.x >= rect[0].x &&
				 mouse.x <= rect[1].x &&
				 mouse.y >= rect[0].y &&
				 mouse.y <= rect[1].y;

	// 押下開始
	if (isHovered_ && input->IsTriggerMouse(0)) {
		isPressed_ = true;
	}

	// クリック
	if (isPressed_ && !input->IsPressMouse(0)) {
		isPressed_ = false;

		if (isHovered_ && onClick_) {
			MusicManager::GetInstance()->PlaySe("ui_click");
			onClick_();
		}
	}

	// 色変更
	text2D_->SetTint(isPressed_ ? clickTint_ : isHovered_ ? hoverTint_
														  : normalTint_);
}


void TextButton::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// isHovered
		DebugUi::PropatyRow("IsHovered", [&] {
			ImGui::Text("%s", isHovered_ ? "true" : "false");
		});

		// isPressed
		DebugUi::PropatyRow("IsPressed", [&] {
			ImGui::Text("%s", isPressed_ ? "true" : "false");
		});

		// normalTint
		DebugUi::PropatyRow("NormalTint", [&] {
			ImGui::ColorEdit4("##normalTint", &normalTint_.x, ImGuiColorEditFlags_NoInputs);
		});

		// hoverTint
		DebugUi::PropatyRow("HoverTint", [&] {
			ImGui::ColorEdit4("##hoverTint", &hoverTint_.x, ImGuiColorEditFlags_NoInputs);
		});

		// clickTint
		DebugUi::PropatyRow("ClickTint", [&] {
			ImGui::ColorEdit4("##clickTint", &clickTint_.x, ImGuiColorEditFlags_NoInputs);
		});

		DebugUi::EndTable();
	}

	// text2d
	if (text2D_) {
		ImGui::Text("Text2D is assigned!");
	} else {
		ImGui::Text("Text2D is not assigned.");
	}
#endif
}


void TextButton::SetOnClick(std::function<void()> callback) {
	onClick_ = std::move(callback);
}


void TextButton::SetNormalTint(const KamataEngine::Vector4 &tint) {
	normalTint_ = tint;
}


void TextButton::SetHoverTint(const KamataEngine::Vector4 &tint) {
	hoverTint_ = tint;
}