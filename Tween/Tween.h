#pragma once

#include "TweenBase.h"
#include <functional>
#include <memory>


template <typename Derived>
class Tween: public TweenBase {
public: // メンバ関数

	~Tween() override = default;


	// control -----------------------------------------------------------

	// 再生を開始する
	Derived &Play();

	// 再生を一時停止する
	Derived &Pause();

	// 即座に完了させる
	Derived &Complete();

	// 強制終了する
	Derived &Kill(bool callComplete = false);

	// 最初から再生し直す
	Derived &Restart(bool includeDelay = true);

	// 再生位置を先頭に戻す
	Derived &Rewind(bool includeDelay = true);

	// -------------------------------------------------------------------


	// setting -----------------------------------------------------------

	// 再生開始前のディレイを設定する（秒）
	Derived &SetDelay(float delay);

	// ループ回数を設定する（-1で無限ループ）
	Derived &SetLoops(int loops);

	// 完了時に自動でKillするかを設定する
	Derived &SetAutoKill(bool autoKill);

	// 対象を設定する
	Derived &SetTarget(void *target);

	// -------------------------------------------------------------------


	// callback ----------------------------------------------------------

	// 再生開始時のコールバックを設定する
	Derived &OnPlay(std::function<void()> action);

	// 一時停止時のコールバックを設定する
	Derived &OnPause(std::function<void()> action);

	// 巻き戻し時のコールバックを設定する
	Derived &OnRewind(std::function<void()> action);

	// アニメーション開始時のコールバックを設定する
	Derived &OnStart(std::function<void()> action);

	// 毎フレーム更新時のコールバックを設定する
	Derived &OnUpdate(std::function<void()> action);

	// 各ループ完了ごとのコールバックを設定する
	Derived &OnStepComplete(std::function<void()> action);

	// 全ループ完了時のコールバックを設定する
	Derived &OnComplete(std::function<void()> action);

	// Kill時のコールバックを設定する
	Derived &OnKill(std::function<void()> action);

	// -------------------------------------------------------------------


protected: // メンバ関数

	void KillTween(bool complete) override {
		Kill(complete);
	}

	std::unique_ptr<TweenBase> Clone() const override = 0;

	void Update(float deltaTime) override = 0;

	void ApplyStartValue() override = 0;

	void ApplyEndValue() override = 0;


private: // メンバ関数

	Derived &Self() {
		return static_cast<Derived &>(*this);
	}
};



// inline func ====================================================================================

// control ---------------------------------------------------------------

template <typename Derived>
inline Derived &Tween<Derived>::Play() {
	if (!isActive_) {
		return Self();
	}

	isPlaying_ = true;

	// 再生時コールバック
	if (onPlay_) {
		onPlay_();
	}

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::Pause() {
	if (!isActive_) {
		return Self();
	}

	isPlaying_ = false;

	// 一時停止時コールバック
	if (onPause_) {
		onPause_();
	}

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::Complete() {
	if (!isActive_) {
		return Self();
	}
	if (loops_ == -1) {
		return Self();
	}

	ApplyEndValue();
	isComplete_ = true;
	isPlaying_ = false;

	// 各ループ完了時コールバック
	if (onStepComplete_) {
		onStepComplete_();
	}
	// 全ループ完了時コールバック
	if (onComplete_) {
		onComplete_();
	}

	if (autoKill_) {
		Kill(false);
	}

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::Kill(bool callComplete) {
	if (!isActive_) {
		return Self();
	}

	if (callComplete) {
		Complete();
		return Self();
	}

	isActive_ = false;

	OnKillImpl();

	// Kill時コールバック
	if (onKill_) {
		onKill_();
	}

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::Restart(bool includeDelay) {
	Rewind(includeDelay);
	Play();

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::Rewind(bool includeDelay) {
	if (!isActive_) {
		return Self();
	}

	bool rewinded = false;

	// ディレイの処理
	if (delay_ > 0.0f) {
		if (includeDelay) {
			rewinded = elapsedDelay_ > 0.0f;
			elapsedDelay_ = 0.0f;
			delayComplete_ = false;
		} else {
			rewinded = !delayComplete_;
			elapsedDelay_ = delay_;
			delayComplete_ = true;
		}
	}

	// すでに巻き戻されたか判定
	if (position_ > 0.0f || completedLoops_ > 0) {
		rewinded = true;
	}

	position_ = 0.0f;
	completedLoops_ = 0;
	isPlaying_ = false;
	isComplete_ = false;
	isBackwards_ = false;

	// 初期値を適用
	if (isInitialize_) {
		ApplyStartValue();
	}

	// 巻き戻し時コールバック
	if (rewinded && onRewind_) {
		onRewind_();
	}

	return Self();
}

// --------------------------------------------------------------- control


// setting ---------------------------------------------------------------

template <typename Derived>
inline Derived &Tween<Derived>::SetDelay(float delay) {
	delay_ = delay;
	delayComplete_ = delay_ <= 0.0f;

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::SetLoops(int loops) {
	if (loops < -1) {
		loops_ = -1;
	} else if (loops == 0) {
		loops_ = 1;
	} else {
		loops_ = loops;
	}

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::SetAutoKill(bool autoKill) {
	autoKill_ = autoKill;

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::SetTarget(void *target) {
	target_ = target;

	return Self();
}

// --------------------------------------------------------------- setting


// callBack --------------------------------------------------------------

template <typename Derived>
inline Derived &Tween<Derived>::OnPlay(std::function<void()> action) {
	onPlay_ = std::move(action);

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::OnPause(std::function<void()> action) {
	onPause_ = std::move(action);

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::OnRewind(std::function<void()> action) {
	onRewind_ = std::move(action);

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::OnStart(std::function<void()> action) {
	onStart_ = std::move(action);

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::OnUpdate(std::function<void()> action) {
	onUpdate_ = std::move(action);

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::OnStepComplete(std::function<void()> action) {
	onStepComplete_ = std::move(action);

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::OnComplete(std::function<void()> action) {
	onComplete_ = std::move(action);

	return Self();
}


template <typename Derived>
inline Derived &Tween<Derived>::OnKill(std::function<void()> action) {
	onKill_ = std::move(action);

	return Self();
}

// -------------------------------------------------------------- callBack

// ==================================================================================== inline func