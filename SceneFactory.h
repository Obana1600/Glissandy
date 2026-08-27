#pragma once

#include "SceneBase.h"
#include <memory>


enum class SceneType {
	Title,
	Game,
};

std::unique_ptr<SceneBase> CreateScene(SceneType scene);