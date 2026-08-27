#include "GizmoEditor.h"

#include "DebugUi.h"
#include <numbers>

#ifdef USE_IMGUI
#include <imguizmo/ImGuizmo.h>
#endif

using namespace GizmoEditor;
using namespace KamataEngine;
using namespace KamataEngine::MathUtility;


#ifdef USE_IMGUI
static ImGuizmo::OPERATION sOperation = ImGuizmo::OPERATION::TRANSLATE;
static ImGuizmo::MODE sMode = ImGuizmo::MODE::WORLD;
#endif


void GizmoEditor::BeginFrame() {
#ifdef USE_IMGUI
	ImGuizmo::BeginFrame();

	ImGuizmo::SetOrthographic(false);

	ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());

	ImGuizmo::SetRect(0, 0, WinApp::kWindowWidth, WinApp::kWindowHeight);
#endif
}


void GizmoEditor::DrawModeUI() {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Operation")) {
		ImGui::Indent();

		// テーブル
		if (DebugUi::BeginTable("Operation")) {

			// translate
			DebugUi::PropatyRow("Translate", [&] {
				if (ImGui::RadioButton("##translate", sOperation == ImGuizmo::OPERATION::TRANSLATE)) {
					sOperation = ImGuizmo::OPERATION::TRANSLATE;
				}
			});

			// rotate
			DebugUi::PropatyRow("Rotate", [&] {
				if (ImGui::RadioButton("##rotate", sOperation == ImGuizmo::OPERATION::ROTATE)) {
					sOperation = ImGuizmo::OPERATION::ROTATE;
				}
			});

			// scale
			DebugUi::PropatyRow("Scale", [&] {
				if (ImGui::RadioButton("##scale", sOperation == ImGuizmo::OPERATION::SCALE)) {
					sOperation = ImGuizmo::OPERATION::SCALE;
				}
			});

			DebugUi::EndTable();
		}

		ImGui::Unindent();
		ImGui::TreePop();
	}

	ImGui::Spacing();

	if (ImGui::TreeNode("Mode")) {
		ImGui::Indent();

		// テーブル
		if (DebugUi::BeginTable("Mode")) {

			// local
			DebugUi::PropatyRow("Local", [&] {
				if (ImGui::RadioButton("##local", sMode == ImGuizmo::MODE::LOCAL)) {
					sMode = ImGuizmo::MODE::LOCAL;
				}
			});

			// world
			DebugUi::PropatyRow("World", [&] {
				if (ImGui::RadioButton("##world", sMode == ImGuizmo::MODE::WORLD)) {
					sMode = ImGuizmo::MODE::WORLD;
				}
			});

			DebugUi::EndTable();
		}

		ImGui::Unindent();
		ImGui::TreePop();
	}
#endif
}


bool GizmoEditor::Manipulate(KamataEngine::WorldTransform &worldTransform, const KamataEngine::Camera &camera) {
#ifdef USE_IMGUI
	ImGuizmo::PushID(&worldTransform);

	auto mat =
		MakeScaleMatrix(worldTransform.scale_) *
		MakeRotateXMatrix(worldTransform.rotation_.x) *
		MakeRotateYMatrix(worldTransform.rotation_.y) *
		MakeRotateZMatrix(worldTransform.rotation_.z) *
		MakeTranslateMatrix(worldTransform.translation_);
	if (worldTransform.parent_) {
		mat *= worldTransform.parent_->matWorld_;
	}

	const auto isEdited =
		ImGuizmo::Manipulate(&camera.matView.m[0][0], &camera.matProjection.m[0][0], sOperation, sMode, &mat.m[0][0]);

	if (isEdited) {
		// 親がいるならローカルに戻す
		if (worldTransform.parent_) {
			mat *= Inverse(worldTransform.parent_->matWorld_);
		}

		Vector3 rotation{};
		ImGuizmo::DecomposeMatrixToComponents(&mat.m[0][0], &worldTransform.translation_.x, &rotation.x, &worldTransform.scale_.x);
		worldTransform.rotation_ = rotation * std::numbers::pi_v<float> / 180.0f;
	}

	ImGuizmo::PopID();
	return isEdited;
#else
	(void)worldTransform;
	(void)camera;
	return false;
#endif
}