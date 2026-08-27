#include "Health.h"

#include "GameObject.h"
#include "DebugUi.h"



void Health::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// hp
		DebugUi::PropatyRow("Hp", [&] {
			ImGui::Text("%d / %d", hp_, maxHp_);
		});

		// maxHp
		DebugUi::PropatyRow("MaxHp", [&] {
			auto maxHp = maxHp_;
			if (ImGui::DragInt("##maxHp", &maxHp, 1, 1, INT_MAX)) {
				SetMaxHp(maxHp, false);
			}
		});

		// autoDestroy
		DebugUi::PropatyRow("AutoDestroy", [&] {
			ImGui::Checkbox("##autoDestroy", &autoDestroy_);
		});

		// kill
		DebugUi::PropatyRow("Simulate", [&] {
			if (ImGui::Button("kill")) {
				TakeDamage(hp_);
			}
			ImGui::SameLine();
			if (ImGui::Button("restore")) {
				SetMaxHp(maxHp_, true);
			} }, false);

		DebugUi::EndTable();
	}

	// onDeath
	if (onDeath_) {
		ImGui::Text("OnDeath is assigned!");
	} else {
		ImGui::Text("OnDeath is not assigned.");
	}
#endif
}


void Health::TakeDamage(int damage) {
	// 無効ならリターン
	if (!enabled_) {
		return;
	}

	if (hp_ <= 0) {
		return;
	}

	hp_ -= damage;
	if (hp_ <= 0) {
		hp_ = 0;
		if (onDeath_) {
			onDeath_();
		}
		if (autoDestroy_) {
			owner_->SetIsDead(true);
		}
	}
}


void Health::SetOnDeath(std::function<void()> callback) {
	onDeath_ = std::move(callback);
}


bool Health::IsAlive() const {
	return hp_ > 0;
}


void Health::SetMaxHp(int maxHp, bool restoreHp) {
	maxHp_ = maxHp;

	if (restoreHp) {
		hp_ = maxHp_;
	} else {
		hp_ = (std::min)(hp_, maxHp_);
	}
}