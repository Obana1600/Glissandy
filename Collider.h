#pragma once

#include "Component.h"
#include "CollisionAttribute.h"
#include "Geometry.h"
#include <functional>


class Collider: public Component {
public: // メンバ関数

	void UpdateImGui() override;

	const std::string &GetName() const override {
		return name_;
	}


	// コライダーのワイヤー描画
	void DrawDebug() const;

	// 衝突時コールバックの呼び出し
	void OnCollisionEnter(Collider *other);
	void OnCollisionStay(Collider *other);
	void OnCollisionExit(Collider *other);

	// 衝突時コールバックの登録
	void SetOnCollisionEnter(std::function<void(Collider *)> callback);
	void SetOnCollisionStay(std::function<void(Collider *)> callback);
	void SetOnCollisionExit(std::function<void(Collider *)> callback);

	// OBBの取得
	OBB GetOBB() const;

	// 死んでいるか（ownerの委譲）
	bool IsDead() const;

	// 判定に参加するか
	bool IsActive() const;


	// getter -------------------------------------------------------------------------------------

	GameObject *GetOwner() const {
		return owner_;
	}

	CollisionAttribute GetAttribute() const {
		return collisionAttribute_;
	}

	CollisionAttribute GetMask() const {
		return collisionMask_;
	}

	// --------------------------------------------------------------------------------------------


	// setter -------------------------------------------------------------------------------------

	void SetColliderSize(const KamataEngine::Vector3 &size) {
		colliderSize_ = size;
	}

	void SetOffset(const KamataEngine::Vector3 &offset) {
		offset_ = offset;
	}

	void SetAttribute(CollisionAttribute attribute) {
		collisionAttribute_ = attribute;
	}

	void SetMask(CollisionAttribute mask) {
		collisionMask_ = mask;
	}

	void SetDebugColor(const KamataEngine::Vector4 &color) {
		debugColor_ = color;
	}

	// --------------------------------------------------------------------------------------------


protected: // メンバ変数

	KamataEngine::Vector3 colliderSize_ = {1.0f, 1.0f, 1.0f};
	KamataEngine::Vector3 offset_{};
	CollisionAttribute collisionAttribute_ = CollisionAttribute::None;
	CollisionAttribute collisionMask_ = CollisionAttribute::All;
	KamataEngine::Vector4 debugColor_ = {1.0f, 1.0f, 1.0f, 1.0f};

	std::function<void(Collider *)> onCollisionEnter_;
	std::function<void(Collider *)> onCollisionStay_;
	std::function<void(Collider *)> onCollisionExit_;

	std::string name_ = "Collider";
};