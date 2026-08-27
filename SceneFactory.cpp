#include "SceneFactory.h"

#include "GameScene.h"
#include "TitleScene.h"


std::unique_ptr<SceneBase> CreateScene(SceneType scene) {
	switch (scene) {
		case SceneType::Title:
			return std::make_unique<TitleScene>();

		case SceneType::Game:
			return std::make_unique<GameScene>();
	}

	assert(false && "シーン作成に失敗しました");
	return std::make_unique<TitleScene>();
}