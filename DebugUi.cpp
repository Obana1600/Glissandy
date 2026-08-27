#include "DebugUi.h"

#ifdef USE_IMGUI
#include <imguizmo/ImGuizmo.h>
#endif


bool DebugUi::IsCapturingMouse() {
#ifdef USE_IMGUI
	auto &io = ImGui::GetIO();
	return io.WantCaptureMouse || ImGuizmo::IsOver() || ImGuizmo::IsUsingAny();
#else
	return false;
#endif
}