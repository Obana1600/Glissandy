#pragma once

#include "GameObject.h"
#include <functional>
#include <memory>


namespace Prefab {

	std::unique_ptr<GameObject> CreatePlayer();

	std::unique_ptr<GameObject> CreateEnemy();

	std::unique_ptr<GameObject> CreatePlayerBullet();

	std::unique_ptr<GameObject> CreateRock();

	std::unique_ptr<GameObject> CreateTutorialText(const std::string &text);

	std::unique_ptr<GameObject> CreateSkydome();

	std::unique_ptr<GameObject> CreateTitleLogo();

	std::unique_ptr<GameObject> CreateTextButton(const std::string &label, const KamataEngine::Vector2 &rectSize, std::function<void()> onClick);

	std::unique_ptr<GameObject> CreateHealthBar(const std::string &name, const std::string &textureName, const KamataEngine::Vector2 &size, const KamataEngine::Vector4 &color, const KamataEngine::Vector2 &anchorPoint);


	void PlayEnter(GameObject *object);

} // namespace Prefab