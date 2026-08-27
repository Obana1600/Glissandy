#include "DamageFlash.h"

#include "DebugUi.h"
#include "GameObject.h"
#include "ModelRenderer.h"
#include "TimeT.h"


void DamageFlash::Initialize() {
	modelRenderer_ = owner_->GetComponent<ModelRenderer>();
	if (modelRenderer_) {
		baseColor_ = modelRenderer_->GetColor();
	}
}


void DamageFlash::Update() {
	if (timer_ <= 0.0f) {
		return;
	}

	timer_ -= TimeT::GetDeltaTime();

	if (timer_ <= 0.0f) {
		timer_ = 0.0f;
		if (modelRenderer_) {
			modelRenderer_->SetColor(baseColor_);
		}
	}
}


void DamageFlash::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// flashColor
		DebugUi::PropatyRow("FlashColor", [&] {
			ImGui::ColorEdit4("##flashColor", &flashColor_.x, ImGuiColorEditFlags_NoInputs);
		});

		// duration
		DebugUi::PropatyRow("Duration", [&] {
			ImGui::DragFloat("##duration", &duration_, 0.01f, 0.0f, FLT_MAX);
		});

		// timer
		DebugUi::PropatyRow("Timer", [&] {
			ImGui::Text("%.2f / %.2f", timer_, duration_);
		});

		// baseColor
		DebugUi::PropatyRow("BaseColor", [&] {
			ImGui::ColorEdit4("##baseColor", &baseColor_.x, ImGuiColorEditFlags_NoInputs);
		});

		// simulate
		DebugUi::PropatyRow("Simulate", [&] {
			if (ImGui::Button("flash")) {
				Flash();
			}
		});

		DebugUi::EndTable();
	}

	// modelRenderer
	if (modelRenderer_) {
		ImGui::Text("ModelRenderer is assigned!");
	} else {
		ImGui::Text("ModelRenderer is not assigned.");
	}
#endif
}


void DamageFlash::Flash() {
	timer_ = duration_;

	if (modelRenderer_) {
		modelRenderer_->SetColor(flashColor_);
	}
}