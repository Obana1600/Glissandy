#include "ModelRenderer.h"

#include "DebugUi.h"

using namespace KamataEngine;


void ModelRenderer::Initialize() {
	objColor_.Initialize();
}


void ModelRenderer::Draw(const KamataEngine::Camera &camera) const {
	if (model_) {
		model_->Draw(owner_->GetWorldTransform(), camera, &objColor_);
	}
}


void ModelRenderer::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// color
		DebugUi::PropatyRow("Color", [&] {
			auto color = objColor_.GetColor();
			if (ImGui::ColorEdit4("##color", &color.x, ImGuiColorEditFlags_NoInputs)) {
				objColor_.SetColor(color);
			}
		});

		DebugUi::EndTable();
	}

	// model
	if (model_) {
		ImGui::Text("Model is assigned!");
	} else {
		ImGui::Text("Model is not assigned.");
	}
#endif
}


const KamataEngine::Vector4 &ModelRenderer::GetColor() const {
	return objColor_.GetColor();
}


void ModelRenderer::SetColor(const KamataEngine::Vector4 &color) {
	objColor_.SetColor(color);
}