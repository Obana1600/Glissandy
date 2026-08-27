#pragma once

#include "KamataEngine.h"


namespace GizmoEditor {

	void BeginFrame();

	void DrawModeUI();

	bool Manipulate(KamataEngine::WorldTransform &worldTransform, const KamataEngine::Camera &camera);

} // namespace GizmoEditor