#pragma once

#include "EaseManager.h"
#include "Extension.h"
#include "KamataEngine.h"
#include "Tween.h"
#include <functional>
#include <memory>
#include <type_traits>


template <typename T>
using ParamType = std::conditional_t<std::is_arithmetic_v<T>, T, const T &>;

template <typename T>
using Getter = std::function<T()>;

template <typename T>
using Setter = std::function<void(ParamType<T>)>;



template <typename T>
class Tweener: public Tween<Tweener<T>> {
public: // メンバ関数

	Tweener(Getter<T> getter, Setter<T> setter, ParamType<T> endValue, float duration);

	~Tweener() override = default;


	// setting -----------------------------------------------------------

	// イージング関数を設定する
	Tweener<T> &SetEase(EaseType easeType);

	// ループ回数とループ時の挙動を設定する（-1で無限ループ）
	Tweener<T> &SetLoops(int loops, LoopType loopType);

	// 終端値を相対値として扱うかを設定設定する
	Tweener<T> &SetRelative(bool isRelative);

	// -------------------------------------------------------------------


private: // メンバ変数

	Getter<T> getter_ = nullptr;
	Setter<T> setter_ = nullptr;

	T startValue_{};
	T endValue_{};


private: // メンバ関数

	std::unique_ptr<TweenBase> Clone() const override;

	void Update(float deltaTime) override;

	void ApplyStartValue() override;

	void ApplyEndValue() override;

	T Lerp(ParamType<T> start, ParamType<T> end, float t);
};



// inline func ====================================================================================

template <typename T>
inline Tweener<T>::Tweener(Getter<T> getter, Setter<T> setter, ParamType<T> endValue, float duration) {
	getter_ = getter;
	setter_ = setter;
	endValue_ = endValue;
	this->duration_ = duration;
}



// setting ---------------------------------------------------------------

template <typename T>
inline Tweener<T> &Tweener<T>::SetEase(EaseType easeType) {
	this->easeType_ = easeType;

	return *this;
}


template <typename T>
inline Tweener<T> &Tweener<T>::SetLoops(int loops, LoopType loopType) {
	if (loops < -1) {
		this->loops_ = -1;
	} else if (loops == 0) {
		this->loops_ = 1;
	} else {
		this->loops_ = loops;
	}

	this->loopType_ = loopType;

	return *this;
}


template <typename T>
inline Tweener<T> &Tweener<T>::SetRelative(bool isRelative) {
	this->isRelative_ = isRelative;

	return *this;
}

// --------------------------------------------------------------- setting



template <typename T>
inline std::unique_ptr<TweenBase> Tweener<T>::Clone() const {
	return std::make_unique<Tweener<T>>(*this);
}


template <typename T>
inline void Tweener<T>::Update(float deltaTime) {
	using namespace KamataEngine::MathUtility;

	// ディレイ
	if (!this->delayComplete_) {
		this->elapsedDelay_ += deltaTime;

		if (this->elapsedDelay_ >= this->delay_) {
			this->delayComplete_ = true;
		} else {
			return;
		}
	}

	// 初期化
	if (!this->isInitialize_) {
		this->isInitialize_ = true;
		startValue_ = getter_();

		if (this->isRelative_) {
			endValue_ = startValue_ + endValue_;
		}

		// アニメーション開始時コールバック
		if (this->onStart_) {
			this->onStart_();
		}
	}

	// 時間更新
	this->position_ += deltaTime;

	// イージング適用
	float evalPos = (std::min)(this->position_, this->duration_);
	float t = EaseManager::EvaluateUnclamped(*this, evalPos, this->duration_);
	if (this->isBackwards_) {
		t = 1.0f - t;
	}
	setter_(Lerp(startValue_, endValue_, t));

	// 更新時コールバック
	if (this->onUpdate_) {
		this->onUpdate_();
	}

	// ループ処理
	if (this->position_ >= this->duration_) {
		this->position_ = 0.0f;
		this->completedLoops_++;

		// 無限ループでなく、ループ回数に達した
		if (this->loops_ != -1 && this->completedLoops_ >= this->loops_) {
			this->Complete();

			return;
		}

		// 各ループ完了時コールバック
		if (this->onStepComplete_) {
			this->onStepComplete_();
		}

		// yoyoは方向を反転
		if (this->loopType_ == LoopType::Yoyo) {
			this->isBackwards_ = !this->isBackwards_;
		}
	}
}


template <typename T>
inline void Tweener<T>::ApplyStartValue() {
	setter_(startValue_);
}


template <typename T>
inline void Tweener<T>::ApplyEndValue() {
	if (this->loopType_ == LoopType::Incremental && this->loops_ != -1) {
		setter_(Lerp(startValue_, endValue_, static_cast<float>(this->loops_)));
	} else {
		setter_(this->isBackwards_ ? startValue_ : endValue_);
	}
}


template <typename T>
inline T Tweener<T>::Lerp(ParamType<T> start, ParamType<T> end, float t) {
	using namespace KamataEngine::MathUtility;
	return start + (end - start) * t;
}

// ==================================================================================== inline func