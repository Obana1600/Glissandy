#include "Bullet.h"

#include "GameObject.h"
#include "DebugUi.h"
#include "TimeT.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void Bullet::Update() {
	auto step = velocity_ * TimeT::GetDeltaTime();
	owner_->SetTranslation(owner_->GetWorldTransform().translation_ + step);

	traveled_ += Length(step);
	if (traveled_ >= range_) {
		if (onExpire_) {
			onExpire_();
		} else {
			owner_->SetIsDead(true);
		}
	}
}


void Bullet::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// velocity
		DebugUi::PropatyRow("Velocity", [&] {
			ImGui::DragFloat3("##velocity", &velocity_.x, 0.1f);
		});

		// traveled
		DebugUi::PropatyRow("Traveled", [&] {
			ImGui::Text("%.2f / %.2f", traveled_, range_);
		});

		// range
		DebugUi::PropatyRow("Range", [&] {
			ImGui::DragFloat("##range", &range_, 1.0f, 0.0f, FLT_MAX);
		});

		// damage
		DebugUi::PropatyRow("Damage", [&] {
			ImGui::DragInt("##damage", &damage_, 1, 0, INT_MAX);
		});

		DebugUi::EndTable();
	}

	// onExpire
	if (onExpire_) {
		ImGui::Text("OnExpire is assigned!");
	} else {
		ImGui::Text("OnExpire is not assigned.");
	}
#endif
}