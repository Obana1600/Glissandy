#include "StatusEffect.h"

#include "DebugUi.h"
#include "TimeT.h"
#include <algorithm>



void StatusEffect::Update() {
	for (auto &timer : timers_) {
		if (timer <= 0.0f) {
			continue;
		}

		timer -= TimeT::GetDeltaTime();

		if (timer <= 0.0f) {
			timer = 0.0f;
		}
	}
}


void StatusEffect::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// stun
		DebugUi::PropatyRow("Stun", [&] {
			ImGui::Text("%.2f", timers_[StatusIndex(StatusAttribute::Stun)]);
		});

		DebugUi::EndTable();
	}
#endif
}


void StatusEffect::Apply(StatusAttribute status, float duration) {
	timers_[StatusIndex(status)] = (std::max)(timers_[StatusIndex(status)], duration);
}


bool StatusEffect::Has(StatusAttribute status) const {
	return timers_[StatusIndex(status)] > 0.0f;
}