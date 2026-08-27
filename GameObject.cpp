#include "GameObject.h"

#include "DebugUi.h"
#include "GizmoEditor.h"
#include "Tween/TweenManager.h"
#include <cassert>
#include <numbers>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



GameObject::~GameObject() {
	// 自分を対象にしたtweenを止める
	TweenManager::Kill(this);
}


void GameObject::Initialize() {
	for (auto &c : components_) {
		c->Initialize();
	}

	// トランスフォーム初期化
	worldTransform_.Initialize();

	// 子の初期化
	for (auto &child : children_) {
		child->worldTransform_.parent_ = &worldTransform_;
		child->Initialize();
	}
}


void GameObject::Update() {
	for (auto &c : components_) {
		if (!c->IsEnabled()) {
			continue;
		}

		c->Update();
	}

	// ワールド行列更新
	UpdateWorldTransform();

	// 子の更新
	for (auto &child : children_) {
		child->Update();
	}
}


void GameObject::Draw(const KamataEngine::Camera &camera, DrawPass pass) const {
	for (auto &c : components_) {
		if (!c->IsEnabled()) {
			continue;
		}

		if (c->GetDrawPass() == pass) {
			c->Draw(camera);
		}
	}

	// 子
	for (auto &child : children_) {
		child->Draw(camera, pass);
	}
}




void GameObject::UpdateImGui() {
#ifdef USE_IMGUI
	constexpr float toDegrees = 180.0f / std::numbers::pi_v<float>;
	constexpr float toRadians = std::numbers::pi_v<float> / 180.0f;

	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// translation
		DebugUi::PropatyRow("Translation", [&] {
			ImGui::DragFloat3("##translation", &worldTransform_.translation_.x, 0.1f);
		});

		// rotation
		DebugUi::PropatyRow("Rotation", [&] {
			float degrees[3] = {
				worldTransform_.rotation_.x * toDegrees,
				worldTransform_.rotation_.y * toDegrees,
				worldTransform_.rotation_.z * toDegrees,
			};

			if (ImGui::DragFloat3("##rotation", degrees, 0.5f)) {
				worldTransform_.rotation_.x = degrees[0] * toRadians;
				worldTransform_.rotation_.y = degrees[1] * toRadians;
				worldTransform_.rotation_.z = degrees[2] * toRadians;
			}
		});

		// scale
		DebugUi::PropatyRow("Scale", [&] {
			ImGui::DragFloat3("##scale", &worldTransform_.scale_.x, 0.01f);
		});

		// isDead
		DebugUi::PropatyRow("IsDead", [&] {
			ImGui::Checkbox("##isDead", &isDead_);
		});

		DebugUi::EndTable();
	}

	ImGui::Spacing();

	// components
	for (auto &component : components_) {
		ImGui::PushID(component.get());

		bool enabled = component->IsEnabled();
		if (ImGui::Checkbox("##enabled", &enabled)) {
			component->SetEnabled(enabled);
		}
		ImGui::SameLine();

		if (ImGui::CollapsingHeader(component->GetName().c_str())) {
			component->UpdateImGui();
		}

		ImGui::PopID();
	}
#endif
}


GameObject *GameObject::AddChild(std::unique_ptr<GameObject> child) {
	assert(child && "子がnullptrです");

	child->parent_ = this;
	child->worldTransform_.parent_ = &worldTransform_;

	// 所有権を渡す前にアドレスを控える
	GameObject *result = child.get();
	children_.push_back(std::move(child));

	return result;
}


GameObject *GameObject::FindChild(const std::string &name) const {
	for (auto &child : children_) {
		if (child->GetName() == name) {
			return child.get();
		}
	}

	return nullptr;
}


KamataEngine::Vector3 GameObject::GetWorldPosition() const {
	Vector3 result{};

	result.x = worldTransform_.matWorld_.m[3][0];
	result.y = worldTransform_.matWorld_.m[3][1];
	result.z = worldTransform_.matWorld_.m[3][2];

	return result;
}


void GameObject::EditTransform(const KamataEngine::Camera &camera) {
#ifdef USE_IMGUI
	if (GizmoEditor::Manipulate(worldTransform_, camera)) {
		UpdateWorldTransform();
	}
#else
	(void)camera;
#endif
}


void GameObject::UpdateWorldTransform() {
	worldTransform_.matWorld_ =
		MakeScaleMatrix(worldTransform_.scale_) *
		MakeRotateXMatrix(worldTransform_.rotation_.x) *
		MakeRotateYMatrix(worldTransform_.rotation_.y) *
		MakeRotateZMatrix(worldTransform_.rotation_.z) *
		MakeTranslateMatrix(worldTransform_.translation_);

	// 親があるなら親のワールド行列をかける
	if (worldTransform_.parent_) {
		worldTransform_.matWorld_ *= worldTransform_.parent_->matWorld_;
	}

	worldTransform_.TransferMatrix();
}