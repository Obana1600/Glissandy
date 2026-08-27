#pragma once

#include "KamataEngine.h"
#include <vector>

class GameObject;
class CollisionManager;
class TrackManager;


class GuiEditor {
public: // メンバ関数

	void Clear();

	void Register(GameObject *object);

	void Update(const KamataEngine::Camera &camera);

	void SetCollisionManager(CollisionManager *collisionManager);

	void SetTrackManager(TrackManager *trackManager);


private: // メンバ変数

	std::vector<GameObject *> targets_;
	GameObject *selected_ = nullptr;

	CollisionManager *collisionManager_ = nullptr;
	TrackManager *trackManager_ = nullptr;


private: // メンバ関数

	void DrawHierarchy();

	void DrawInspector();

	void DrawSceneInfo();
};