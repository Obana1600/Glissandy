#include "Text3D.h"

#include "DebugUi.h"
#include "GameObject.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;


void Text3D::Initialize() {
	renderer_.Initialize();

	text_.Initialize();
	text_.SetFontSizePerUnit(kFontSizePerUnit);
	text_.SetRectSize(rectSize_);
}


void Text3D::Draw(const KamataEngine::Camera &camera) const {
	auto wvp = MakeWvp(camera);
	renderer_.Draw(wvp, text_);
}


void Text3D::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// rectSize
		DebugUi::PropatyRow("RectSize", [&] {
			auto size = rectSize_;
			if (ImGui::DragFloat2("##rectSize", &size.x, 0.1f, 0.0f, FLT_MAX)) {
				SetRectSize(size);
			}
		});

		// pivot
		DebugUi::PropatyRow("Pivot", [&] {
			ImGui::DragFloat2("##pivot", &pivot_.x, 0.01f, 0.0f, 1.0f);
		});

		// showRect
		DebugUi::PropatyRow("ShowRect", [&] {
			ImGui::Checkbox("##showRect", &showRect_);
		});

		// showPivot
		DebugUi::PropatyRow("ShowPivot", [&] {
			ImGui::Checkbox("##showPivot", &showPivot_);
		});

		DebugUi::EndTable();
	}

	text_.UpdateImGui();
#endif
}


void Text3D::SetText(const std::string &text) {
	text_.SetText(text);
}


void Text3D::SetRectSize(const KamataEngine::Vector2 &size) {
	rectSize_ = size;
	text_.SetRectSize(size);
}


KamataEngine::Matrix4x4 Text3D::MakeWvp(const KamataEngine::Camera &camera) const {
	auto pivotOffset = MakeTranslateMatrix({-pivot_.x * rectSize_.x, (1.0f - pivot_.y) * rectSize_.y, 0.0f});
	auto wvp = pivotOffset * owner_->GetWorldTransform().matWorld_ * camera.matView * camera.matProjection;

	return wvp;
}