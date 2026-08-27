#pragma once

#include "KamataEngine.h"
#include <string>

class GameObject;


enum class DrawPass {
	Model,
	Text3D,
	Text2D,
	Sprite,
};



class Component {
public: // メンバ関数

	virtual ~Component();

	virtual void Initialize() {};

	virtual void Update() {};

	virtual void Draw(const KamataEngine::Camera &) const {};

	virtual void UpdateImGui() {};

	virtual void DrawGizmo(const KamataEngine::Camera &) const {};

	virtual const std::string &GetName() const = 0;

	virtual DrawPass GetDrawPass() const {
		return DrawPass::Model;
	}


	bool IsEnabled() const {
		return enabled_;
	}

	void SetEnabled(bool enabled) {
		enabled_ = enabled;
	}


protected: // メンバ変数

	bool enabled_ = true;

	GameObject *owner_ = nullptr;
	friend class GameObject;
};