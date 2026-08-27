#pragma once

#include "Component.h"
#include "GameObject.h"


class ModelRenderer: public Component {
public: // メンバ関数

	void Initialize() override;

	void Draw(const KamataEngine::Camera &camera) const override;

	void UpdateImGui() override;

	const std::string &GetName() const override {
		return name_;
	}


	const KamataEngine::Vector4 &GetColor() const;

	void SetColor(const KamataEngine::Vector4 &color);

	void SetModel(KamataEngine::Model *model) {
		model_ = model;
	}


private: // メンバ変数

	KamataEngine::Model *model_ = nullptr;
	KamataEngine::ObjectColor objColor_;

	std::string name_ = "ModelRenderer";
};