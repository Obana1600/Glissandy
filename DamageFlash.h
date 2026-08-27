#pragma once

#include "Component.h"

class ModelRenderer;


class DamageFlash: public Component {
public: // メンバ関数

	void Initialize() override;

	void Update() override;

	void UpdateImGui() override;

	const std::string &GetName() const override {
		return name_;
	}


	void Flash();


private: // メンバ変数

	KamataEngine::Vector4 flashColor_ = {1.0f, 0.0f, 0.0f, 1.0f};
	float duration_ = 0.2f;
	float timer_ = 0.0f;

	ModelRenderer *modelRenderer_ = nullptr;
	KamataEngine::Vector4 baseColor_{};

	std::string name_ = "DamageFlash";
};