#include "Collider.h"

#include "DebugUi.h"
#include "Extension.h"
#include "GameObject.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;


#ifdef USE_IMGUI
namespace {

	// コリジョン属性のチェックボックス
	void CheckBoxCollisionAttribute(const char *label, CollisionAttribute &value, CollisionAttribute bit) {
		bool on = Any(value & bit);

		if (ImGui::Checkbox(label, &on)) {
			on ? (value |= bit)
			   : (value &= ~bit);
		}
	}

	// コリジョン属性のラジオボタン
	void RadioCollisionAttribute(const char *label, CollisionAttribute &value, CollisionAttribute bit) {
		if (ImGui::RadioButton(label, value == bit)) {
			value = bit;
		}
	}

} // namespace
#endif





void Collider::UpdateImGui() {
#ifdef USE_IMGUI
	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// colliderSize
		DebugUi::PropatyRow("ColliderSize", [&] {
			ImGui::DragFloat3("##colliderSize", &colliderSize_.x, 0.1f, 0.0f, FLT_MAX);
		});

		// offset
		DebugUi::PropatyRow("Offset", [&] {
			ImGui::DragFloat3("##offset", &offset_.x, 0.1f);
		});

		// debugColor
		DebugUi::PropatyRow("DebugColor", [&] {
			ImGui::ColorEdit4("##debugColor", &debugColor_.x, ImGuiColorEditFlags_NoInputs);
		});

		// isActive
		DebugUi::PropatyRow("IsActive", [&] {
			ImGui::Text("%s", IsActive() ? "true" : "false");
		});

		DebugUi::EndTable();
	}

	// 自分が何か
	ImGui::SeparatorText("Attribute");
	ImGui::PushID("attribute");
	RadioCollisionAttribute("Player", collisionAttribute_, CollisionAttribute::Player);
	RadioCollisionAttribute("Enemy", collisionAttribute_, CollisionAttribute::Enemy);
	RadioCollisionAttribute("PlayerBullet", collisionAttribute_, CollisionAttribute::PlayerBullet);
	RadioCollisionAttribute("EnemyBullet", collisionAttribute_, CollisionAttribute::EnemyBullet);
	RadioCollisionAttribute("Obstacle", collisionAttribute_, CollisionAttribute::Obstacle);
	ImGui::PopID();

	// 何と当たるか
	ImGui::SeparatorText("Mask");
	ImGui::PushID("mask");
	CheckBoxCollisionAttribute("Player", collisionMask_, CollisionAttribute::Player);
	CheckBoxCollisionAttribute("Enemy", collisionMask_, CollisionAttribute::Enemy);
	CheckBoxCollisionAttribute("PlayerBullet", collisionMask_, CollisionAttribute::PlayerBullet);
	CheckBoxCollisionAttribute("EnemyBullet", collisionMask_, CollisionAttribute::EnemyBullet);
	CheckBoxCollisionAttribute("Obstacle", collisionMask_, CollisionAttribute::Obstacle);
	ImGui::PopID();

	// 衝突コールバック
	ImGui::SeparatorText("Callback");
	ImGui::Text("Enter : %s", onCollisionEnter_ ? "assigned" : "not assigned");
	ImGui::Text("Stay  : %s", onCollisionStay_ ? "assigned" : "not assigned");
	ImGui::Text("Exit  : %s", onCollisionExit_ ? "assigned" : "not assigned");
#endif
}


void Collider::DrawDebug() const {
	// 底面と上面の頂点を求める
	auto obb = GetOBB();
	Vector3 bottom[4]{}, top[4]{};
	bottom[0] = obb.center + obb.orientation[0] * obb.size.x * -1 + obb.orientation[1] * obb.size.y * -1 + obb.orientation[2] * obb.size.z * -1; // 左下
	bottom[1] = obb.center + obb.orientation[0] * obb.size.x * -1 + obb.orientation[1] * obb.size.y * -1 + obb.orientation[2] * obb.size.z * 1;	 // 左上
	bottom[2] = obb.center + obb.orientation[0] * obb.size.x * 1 + obb.orientation[1] * obb.size.y * -1 + obb.orientation[2] * obb.size.z * 1;	 // 右上
	bottom[3] = obb.center + obb.orientation[0] * obb.size.x * 1 + obb.orientation[1] * obb.size.y * -1 + obb.orientation[2] * obb.size.z * -1;	 // 右下
	top[0] = obb.center + obb.orientation[0] * obb.size.x * -1 + obb.orientation[1] * obb.size.y * 1 + obb.orientation[2] * obb.size.z * -1;	 // 左下
	top[1] = obb.center + obb.orientation[0] * obb.size.x * -1 + obb.orientation[1] * obb.size.y * 1 + obb.orientation[2] * obb.size.z * 1;		 // 左上
	top[2] = obb.center + obb.orientation[0] * obb.size.x * 1 + obb.orientation[1] * obb.size.y * 1 + obb.orientation[2] * obb.size.z * 1;		 // 右上
	top[3] = obb.center + obb.orientation[0] * obb.size.x * 1 + obb.orientation[1] * obb.size.y * 1 + obb.orientation[2] * obb.size.z * -1;		 // 右下

	// スクリーン座標に変換して描画
	auto *drawer = PrimitiveDrawer::GetInstance();
	for (uint32_t i = 0; i < 4; i++) {
		drawer->DrawLine3d(bottom[i], bottom[(i + 1) % 4], debugColor_);
		drawer->DrawLine3d(bottom[i], top[i], debugColor_);
		drawer->DrawLine3d(top[i], top[(i + 1) % 4], debugColor_);
	}
}


void Collider::OnCollisionEnter(Collider *other) {
	if (onCollisionEnter_) {
		onCollisionEnter_(other);
	}
}


void Collider::OnCollisionStay(Collider *other) {
	if (onCollisionStay_) {
		onCollisionStay_(other);
	}
}


void Collider::OnCollisionExit(Collider *other) {
	if (onCollisionExit_) {
		onCollisionExit_(other);
	}
}


void Collider::SetOnCollisionEnter(std::function<void(Collider *)> callback) {
	onCollisionEnter_ = std::move(callback);
}


void Collider::SetOnCollisionStay(std::function<void(Collider *)> callback) {
	onCollisionStay_ = std::move(callback);
}


void Collider::SetOnCollisionExit(std::function<void(Collider *)> callback) {
	onCollisionExit_ = std::move(callback);
}


bool Collider::IsDead() const {
	return owner_->IsDead();
}


bool Collider::IsActive() const {
	return !IsDead() && IsEnabled();
}


OBB Collider::GetOBB() const {
	OBB result{};
	const auto &matWorld = owner_->GetWorldTransform().matWorld_;

	result.center = owner_->GetWorldPosition() + TransformNormal(offset_, matWorld);

	result.size.x = colliderSize_.x * 0.5f * Length({matWorld.m[0][0], matWorld.m[0][1], matWorld.m[0][2]});
	result.size.y = colliderSize_.y * 0.5f * Length({matWorld.m[1][0], matWorld.m[1][1], matWorld.m[1][2]});
	result.size.z = colliderSize_.z * 0.5f * Length({matWorld.m[2][0], matWorld.m[2][1], matWorld.m[2][2]});

	result.orientation[0] = Normalize({matWorld.m[0][0], matWorld.m[0][1], matWorld.m[0][2]});
	result.orientation[1] = Normalize({matWorld.m[1][0], matWorld.m[1][1], matWorld.m[1][2]});
	result.orientation[2] = Normalize({matWorld.m[2][0], matWorld.m[2][1], matWorld.m[2][2]});

	return result;
}