#pragma once

#include <imgui.h>


namespace DebugUi {

	bool IsCapturingMouse();


	inline bool IsCapturingKeyboard() {
#ifdef USE_IMGUI
		auto &io = ImGui::GetIO();
		return io.WantCaptureKeyboard;
#else
		return false;
#endif
	}


	inline bool BeginTable(const char *label, int columns = 2, float labelWidth = 120.0f) {
#ifdef USE_IMGUI
		if (!ImGui::BeginTable(label, columns, ImGuiTableFlags_SizingFixedFit)) {
			return false;
		}

		ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, labelWidth);
		ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthStretch);

		return true;
#else
		(void)label;
		(void)columns;
		(void)labelWidth;
		return false;
#endif
	}


	inline void EndTable() {
#ifdef USE_IMGUI
		ImGui::EndTable();
#endif
	}


	template<class F>
	inline void PropatyRow(
		const char *label,
		F &&widget,
		bool fullWidth = true
	) {
#ifdef USE_IMGUI
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextUnformatted(label);
		ImGui::TableSetColumnIndex(1);

		if (fullWidth) {
			ImGui::SetNextItemWidth(-1);
		}

		ImGui::PushID(label);
		widget();
		ImGui::PopID();
#else
		(void)label;
		(void)widget;
		(void)fullWidth;
#endif
	}

} // namespace DebugUi