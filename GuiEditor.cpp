#include "GuiEditor.h"

#include "CollisionManager.h"
#include "DebugUi.h"
#include "GameObject.h"
#include "GizmoEditor.h"
#include "SceneManager.h"
#include "TimeT.h"
#include "TrackManager.h"
#include <algorithm>
#include <cassert>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void GuiEditor::Clear() {
	targets_.clear();
}


void GuiEditor::Register(GameObject *object) {
	if (!object) {
		assert(false && "オブジェクトがnullptrです");
		return;
	}

	targets_.push_back(object);
}


void GuiEditor::Update(const KamataEngine::Camera &camera) {
	if (selected_ && std::ranges::find(targets_, selected_) == targets_.end()) {
		selected_ = nullptr;
	}

	DrawHierarchy();
	DrawInspector();
	DrawSceneInfo();

	if (selected_) {
		selected_->EditTransform(camera);
	}
}


void GuiEditor::SetCollisionManager(CollisionManager *collisionManager) {
	collisionManager_ = collisionManager;
}


void GuiEditor::SetTrackManager(TrackManager *trackManager) {
	trackManager_ = trackManager;
}


void GuiEditor::DrawHierarchy() {
#ifdef USE_IMGUI
	ImGui::Begin("Hierarchy");

	for (auto &obj : targets_) {
		ImGui::PushID(obj);

		const bool isSelected = (obj == selected_);

		if (ImGui::Selectable(obj->GetName().c_str(), isSelected)) {
			selected_ = obj;
		}

		ImGui::PopID();
	}

	ImGui::End();
#endif
}


void GuiEditor::DrawInspector() {
#ifdef USE_IMGUI
	ImGui::Begin("Inspector");

	if (selected_) {
		ImGui::PushID(selected_);
		selected_->UpdateImGui();
		ImGui::PopID();

	} else {
		ImGui::Text("Nothing selected.");
	}

	ImGui::End();
#endif
}


void GuiEditor::DrawSceneInfo() {
#ifdef USE_IMGUI
	ImGui::Begin("Debug");

	// fps ------------------------------------------------------------------------------------

	ImGui::Text("FPS: %.1f (%.2f ms)", TimeT::GetFramerate(), TimeT::GetDeltaTimeMS());

	// ----------------------------------------------------------------------------------------

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// gizmo ----------------------------------------------------------------------------------

	if (ImGui::CollapsingHeader("ImGuizmo Info", ImGuiTreeNodeFlags_DefaultOpen)) {
		GizmoEditor::DrawModeUI();
	}

	// ----------------------------------------------------------------------------------------

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// collider -------------------------------------------------------------------------------

	if (ImGui::CollapsingHeader("Collision Info", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (collisionManager_) {
			collisionManager_->UpdateImGui();

		} else {
			ImGui::Text("CollisionManager is not assigned.");
		}
	}

	// ----------------------------------------------------------------------------------------

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// scene ----------------------------------------------------------------------------------

	if (ImGui::CollapsingHeader("Scene Info", ImGuiTreeNodeFlags_DefaultOpen)) {
		SceneManager::GetInstance()->UpdateImGui();
	}

	// ----------------------------------------------------------------------------------------

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// track ----------------------------------------------------------------------------------

	if (ImGui::CollapsingHeader("Track Info", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (trackManager_) {
			trackManager_->UpdateImGui();

		} else {
			ImGui::Text("TrackManager is not assigned.");
		}
	}

	// ----------------------------------------------------------------------------------------

	ImGui::End();
#endif
}