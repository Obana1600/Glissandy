#pragma once

#include "KamataEngine.h"
#include "Component.h"
#include <string>
#include <vector>
#include <memory>


class GameObject {
public: // メンバ関数

	~GameObject();

	// 初期化処理
	void Initialize();

	// 更新処理
	void Update();

	// 描画処理
	void Draw(const KamataEngine::Camera &camera, DrawPass pass) const;

	// imguiの更新
	void UpdateImGui();

	// ワールド空間の座標を取得
	KamataEngine::Vector3 GetWorldPosition() const;

	// コンポーネントの追加
	template<typename T, typename... Args>
	T *AddComponent(Args &&...args);

	// コンポーネントの取得
	template<typename T>
	T *GetComponent() const;

	// すべてのコンポーネントの取得
	template<typename T>
	std::vector<T *> GetComponents() const;

	// 自分か祖先からコンポーネントを取得
	template<typename T>
	T *GetComponentInParent() const;

	// 子の追加
	GameObject *AddChild(std::unique_ptr<GameObject> child);

	// 名前で子を探す
	GameObject *FindChild(const std::string &name) const;


	// getter -------------------------------------------------------------------------------------

	GameObject *GetParent() const {
		return parent_;
	}

	const std::vector<std::unique_ptr<GameObject>> &GetChildren() const {
		return children_;
	}

	
	const KamataEngine::WorldTransform &GetWorldTransform() const {
		return worldTransform_;
	}

	const KamataEngine::Vector3 &GetTranslation() const {
		return worldTransform_.translation_;
	}

	const KamataEngine::Vector3 &GetRotation() const {
		return worldTransform_.rotation_;
	}

	const KamataEngine::Vector3 &GetScale() const {
		return worldTransform_.scale_;
	}

	bool IsDead() const {
		return isDead_;
	}

	const std::string &GetName() const {
		return name_;
	}

	// --------------------------------------------------------------------------------------------


	// setter -------------------------------------------------------------------------------------

	void SetTranslation(const KamataEngine::Vector3 &translation) {
		worldTransform_.translation_ = translation;
	}

	void SetRotation(const KamataEngine::Vector3 &rotation) {
		worldTransform_.rotation_ = rotation;
	}

	void SetScale(const KamataEngine::Vector3 &scale) {
		worldTransform_.scale_ = scale;
	}

	void SetIsDead(bool isDead) {
		isDead_ = isDead;
	}

	void SetName(const std::string &name) {
		name_ = name;
	}

	// --------------------------------------------------------------------------------------------


	void EditTransform(const KamataEngine::Camera &camera);


protected: // メンバ変数

	// transform
	KamataEngine::WorldTransform worldTransform_;

	// state
	bool isDead_ = false;

	// name
	std::string name_ = "GameObject";

	// component
	std::vector<std::unique_ptr<Component>> components_;

	// hierarchy
	GameObject *parent_ = nullptr;
	std::vector<std::unique_ptr<GameObject>> children_;


protected: // メンバ関数

	// ワールド行列更新
	void UpdateWorldTransform();
};




template <typename T, typename... Args>
inline T *GameObject::AddComponent(Args &&...args) {
	static_assert(std::is_base_of_v<Component, T>, "TはComponentの派生クラスである必要があります");

	// 生成
	auto component = std::make_unique<T>(std::forward<Args>(args)...);

	// 所有者を教える
	component->owner_ = this;

	// 所有権を渡す前にアドレスを控える
	T *result = component.get();
	components_.push_back(std::move(component));

	return result;
}


template <typename T>
inline T *GameObject::GetComponent() const {
	static_assert(std::is_base_of_v<Component, T>, "TはComponentの派生クラスである必要があります");

	for (auto &c : components_) {
		if (auto *result = dynamic_cast<T *>(c.get())) {
			return result;
		}
	}

	return nullptr;
}


template <typename T>
inline std::vector<T *> GameObject::GetComponents() const {
	static_assert(std::is_base_of_v<Component, T>, "TはComponentの派生クラスである必要があります");

	std::vector<T *> result;

	for (auto &c : components_) {
		if (auto *component = dynamic_cast<T *>(c.get())) {
			result.push_back(component);
		}
	}

	return result;
}


template <typename T>
inline T *GameObject::GetComponentInParent() const {
	static_assert(std::is_base_of_v<Component, T>, "TはComponentの派生クラスである必要があります");

	// 根に向かって遡る
	for (const auto *node = this; node; node = node->parent_) {
		if (auto *result = node->GetComponent<T>()) {
			return result;
		}
	}

	return nullptr;
}