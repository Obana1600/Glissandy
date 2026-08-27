#pragma once

#include "Sequence.h"
#include "Tween.h"
#include "Tweener.h"
#include <functional>
#include <memory>
#include <vector>
#include <type_traits>


class TweenManager {
public: // 静的メンバ関数

	// インスタンス取得
	static TweenManager *GetInstance();

	// targetをキャプチャしているtweenerをkillする
	static void Kill(void *target, bool complete = false);


	// Sequence登録 -------------------------------------------------------

	// 空の Sequence を生成して登録する
	static Sequence &ToSequence();

	// Sequence をムーブして登録する
	static Sequence &ToSequence(Sequence &&seq);

	// Sequence をコピーして登録する（元の Sequence は保持される）
	static Sequence &ToSequence(const Sequence &seq);

	// -------------------------------------------------------------------


	// Tweener登録 --------------------------------------------------------

	// メンバ関数ポインタ版
	template <typename Obj, typename G, typename S>
	static Tweener<std::decay_t<G>> &To(Obj *obj, G (Obj::*getter)() const, void (Obj::*setter)(S), const std::decay_t<G> &endValue, float duration);

	/// std::function版
	template <typename T>
	static Tweener<T> &To(std::function<T()> getter, std::function<void(ParamType<T>)> setter, ParamType<T> endValue, float duration);

	// Tweenerコピー版
	template <typename T>
	static Tweener<T> &To(Tweener<T> tweener);

	// -------------------------------------------------------------------


	// Tweener生成 --------------------------------------------------------

	// メンバ関数ポインタ版
	template <typename Obj, typename G, typename S>
	static Tweener<std::decay_t<G>> Create(Obj *obj, G (Obj::*getter)() const, void (Obj::*setter)(S), const std::decay_t<G> &endValue, float duration);

	// std::function版
	template <typename T>
	static Tweener<T> Create(std::function<T()> getter, std::function<void(ParamType<T>)> setter, ParamType<T> endValue, float duration);

	// -------------------------------------------------------------------


public: // メンバ関数

	void UpdateAll(float deltaTime);

	void Clear();


private: // メンバ変数

	std::vector<std::unique_ptr<TweenBase>> tweens_{};


private: // メンバ関数

	TweenManager() = default;
	~TweenManager() = default;
	TweenManager(const TweenManager &) = delete;
	TweenManager &operator=(const TweenManager &) = delete;
};



// inline func ====================================================================================

inline TweenManager *TweenManager::GetInstance() {
	static TweenManager instance;
	return &instance;
}


// sequence register -----------------------------------------------------

inline Sequence &TweenManager::ToSequence() {
	auto seq = std::make_unique<Sequence>();
	auto &ref = *seq;

	GetInstance()->tweens_.push_back(std::move(seq));

	return ref;
}


inline Sequence &TweenManager::ToSequence(Sequence &&seq) {
	auto ptr = std::make_unique<Sequence>(std::move(seq));
	auto &ref = *ptr;

	GetInstance()->tweens_.push_back(std::move(ptr));

	return ref;
}


inline Sequence &TweenManager::ToSequence(const Sequence &seq) {
	const TweenBase &base = seq;
	auto ptr = base.Clone();
	auto &ref = static_cast<Sequence &>(*ptr);

	GetInstance()->tweens_.push_back(std::move(ptr));

	return ref;
}

// ----------------------------------------------------- sequence register



// tweener register ------------------------------------------------------

template <typename Obj, typename G, typename S>
inline Tweener<std::decay_t<G>> &TweenManager::To(Obj *obj, G (Obj::*getter)() const, void (Obj::*setter)(S), const std::decay_t<G> &endValue, float duration) {
	using T = std::decay_t<G>;

	return To<T>(Tweener<T>(
		[obj, getter]() -> T { return (obj->*getter)(); },
		[obj, setter](ParamType<T> v) { (obj->*setter)(v); },
		endValue,
		duration
	)).SetTarget(obj);
}


template <typename T>
inline Tweener<T> &TweenManager::To(std::function<T()> getter, std::function<void(ParamType<T>)> setter, ParamType<T> endValue, float duration) {
	return To(Tweener<T>(getter, setter, endValue, duration));
}


template <typename T>
inline Tweener<T> &TweenManager::To(Tweener<T> tweener) {
	auto ptr = std::make_unique<Tweener<T>>(std::move(tweener));
	auto &ref = *ptr;

	GetInstance()->tweens_.push_back(std::move(ptr));

	return ref;
}

// ------------------------------------------------------ tweener register



// tweener create --------------------------------------------------------

template <typename Obj, typename G, typename S>
inline Tweener<std::decay_t<G>> TweenManager::Create(Obj *obj, G (Obj::*getter)() const, void (Obj::*setter)(S), const std::decay_t<G> &endValue, float duration) {
	using T = std::decay_t<G>;

	Tweener<T> tween(
		[obj, getter]() -> T { return (obj->*getter)(); },
		[obj, setter](ParamType<T> v) { (obj->*setter)(v); },
		endValue,
		duration
	);
	tween.SetTarget(obj);

	return tween;
}


template <typename T>
inline Tweener<T> TweenManager::Create(std::function<T()> getter, std::function<void(ParamType<T>)> setter, ParamType<T> endValue, float duration) {
	return Tweener<T>(getter, setter, endValue, duration);
}

// -------------------------------------------------------- tweener create



inline void TweenManager::Kill(void *target, bool complete) {
	if (!target) {
		return;
	}

	for (auto &tween : GetInstance()->tweens_) {
		if (tween->target_ != target) {
			continue;
		}

		tween->KillTween(complete);
	}
}


inline void TweenManager::UpdateAll(float deltaTime) {
	const size_t count = tweens_.size();
	for (size_t i = 0; i < count; i++) {
		if (!tweens_[i]->IsActive() || !tweens_[i]->IsPlaying()) {
			continue;
		}

		tweens_[i]->Update(deltaTime);
	}

	std::erase_if(tweens_, [](const auto &tween) {
		return !tween->IsActive();
	});
}


inline void TweenManager::Clear() {
	tweens_.clear();
}

// ==================================================================================== inline func