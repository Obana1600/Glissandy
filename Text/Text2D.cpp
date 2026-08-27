#include "Text2D.h"

#include "DebugUi.h"
#include "GameObject.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void Text2D::Initialize() {
	renderer_.Initialize();

	text_.Initialize();
	text_.SetRectSize(rectSize_);
}


void Text2D::Draw(const KamataEngine::Camera &) const {
	auto wvp = MakeWvp();
	renderer_.Draw(wvp, text_);
}


void Text2D::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// rectSize
		DebugUi::PropatyRow("RectSize", [&] {
			auto size = rectSize_;
			if (ImGui::DragFloat2("##rectSize", &size.x, 1.0f, 0.0f, FLT_MAX)) {
				SetRectSize(size);
			}
		});

		// pivot
		DebugUi::PropatyRow("Pivot", [&] {
			ImGui::DragFloat2("##pivot", &pivot_.x, 0.01f, 0.0f, 1.0f);
		});

		// anchorMin
		DebugUi::PropatyRow("AnchorMin", [&] {
			ImGui::DragFloat2("##anchorMin", &anchorMin_.x, 0.01f, 0.0f, 1.0f);
		});

		// anchorMax
		DebugUi::PropatyRow("AnchorMax", [&] {
			ImGui::DragFloat2("##anchorMax", &anchorMax_.x, 0.01f, 0.0f, 1.0f);
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


void Text2D::SetText(const std::string &text) {
	text_.SetText(text);
}


void Text2D::SetCornerColor(const std::array<KamataEngine::Vector4, 4> &color) {
	text_.SetCornerColors(color);
}


void Text2D::SetTint(const KamataEngine::Vector4 &tint) {
	text_.SetTint(tint);
}


void Text2D::SetRectSize(const KamataEngine::Vector2 &size) {
	rectSize_ = size;
	text_.SetRectSize(rectSize_);
}


void Text2D::SetFontSize(float fontSize) {
	text_.SetFontSize(fontSize);
}


std::array<KamataEngine::Vector2, 2> Text2D::GetScreenRect() const {
	auto wvp = MakeWvp();

	// スクリーン座標へ変換するラムダ
	auto toScreen = [&](float lx, float ly) {
		float nx = lx * wvp.m[0][0] + ly * wvp.m[1][0] + wvp.m[3][0];
		float ny = lx * wvp.m[0][1] + ly * wvp.m[1][1] + wvp.m[3][1];
		float nw = lx * wvp.m[0][3] + ly * wvp.m[1][3] + wvp.m[3][3];
		nx /= nw;
		ny /= nw;
		return Vector2(
			(nx + 1.0f) * 0.5f * WinApp::kWindowWidth,
			(1.0f - ny) * 0.5f * WinApp::kWindowHeight
		);
	};

	// 四隅をとる
	const std::array<Vector2, 4> corners = {
		toScreen(0.0f, 0.0f),
		toScreen(rectSize_.x, 0.0f),
		toScreen(0.0f, -rectSize_.y),
		toScreen(rectSize_.x, -rectSize_.y)
	};

	// aabbをとる
	auto min = corners[0];
	auto max = corners[0];
	for (auto &corner : corners) {
		min.x = (std::min)(min.x, corner.x);
		min.y = (std::min)(min.y, corner.y);
		max.x = (std::max)(max.x, corner.x);
		max.y = (std::max)(max.y, corner.y);
	}

	return {min, max};
}


KamataEngine::Matrix4x4 Text2D::MakeWvp() const {
	auto anchorPos = Vector2(anchorMin_.x * WinApp::kWindowWidth, anchorMin_.y * WinApp::kWindowHeight);
	auto anchorTranslate = MakeTranslateMatrix(Vector3{anchorPos.x, anchorPos.y, 0.0f});
	auto pivotOffset = MakeTranslateMatrix(Vector3{-pivot_.x * rectSize_.x, (1.0f - pivot_.y) * rectSize_.y, 0.0f});
	auto wvp = pivotOffset * owner_->GetWorldTransform().matWorld_ * anchorTranslate * kOrthoMat_;

	return wvp;
}