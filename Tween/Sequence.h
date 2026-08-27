#pragma once

#include "Tween.h"
#include "Tweener.h"
#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <vector>


class Sequence: public Tween<Sequence> {
public: // メンバ関数

	Sequence() = default;
	~Sequence() override = default;
	Sequence(Sequence &&) = default;


	// Prepend: 先頭に追加 -------------------------------------------------

	// Tweener を先頭に追加する（既存要素の開始時刻をずらす）
	template <typename T>
	Sequence &Prepend(Tweener<T> tween);

	// Sequence をムーブして先頭に追加する
	Sequence &Prepend(Sequence &&seq);

	// Sequence をコピーして先頭に追加する（元の Sequence は保持される）
	Sequence &Prepend(const Sequence &seq);

	// -------------------------------------------------------------------


	// Append: 末尾に追加（順番に実行）----------------------------------------

	// Tweener を末尾に追加する（直前の Append の後ろに続けて実行）
	template <typename T>
	Sequence &Append(Tweener<T> tween);

	// Sequence をムーブして末尾に追加する
	Sequence &Append(Sequence &&seq);

	// Sequence をコピーして末尾に追加する（元の Sequence は保持される）
	Sequence &Append(const Sequence &seq);

	// -------------------------------------------------------------------


	// Join: 直前のAppendと同じ開始時刻に追加（並列実行）-------------------------

	// Tweener を直前の Append と同じ開始時刻に追加する（並列実行）
	template <typename T>
	Sequence &Join(Tweener<T> tween);

	// Sequence をムーブして直前の Append と並列実行する
	Sequence &Join(Sequence &&seq);

	// Sequence をコピーして直前の Append と並列実行する（元の Sequence は保持される）
	Sequence &Join(const Sequence &seq);

	// -------------------------------------------------------------------


	// Insert: 指定時刻に追加 -----------------------------------------------

	// 指定した時刻に Tweener を追加する（秒）
	template <typename T>
	Sequence &Insert(float atPosition, Tweener<T> tween);

	// 指定した時刻に Sequence をムーブして追加する
	Sequence &Insert(float atPosition, Sequence &&seq);

	// 指定した時刻に Sequence をコピーして追加する（元の Sequence は保持される）
	Sequence &Insert(float atPosition, const Sequence &seq);

	// -------------------------------------------------------------------


	// インターバルを先頭に追加
	Sequence &PrependInterval(float duration);

	// 空白時間を末尾に追加
	Sequence &AppendInterval(float duration);


private: // インナークラス

	struct SequenceElement {
		std::unique_ptr<TweenBase> tween{};
		float startTime = 0.0f;
		float elapsedTime = 0.0f;
	};


private: // メンバ変数

	std::vector<SequenceElement> elements_{};

	float appendPosition_ = 0.0f;	   // 次の Append の挿入位置（秒）
	float lastAppendStartTime_ = 0.0f; // 直前の Append の開始時刻（Join で参照する）


private: // メンバ関数

	std::unique_ptr<TweenBase> Clone() const override;

	Sequence &PrependImpl(std::unique_ptr<TweenBase> ptr, float delay, float totalDuration);

	Sequence &AppendImpl(std::unique_ptr<TweenBase> ptr, float delay, float totalDuration);

	Sequence &JoinImpl(std::unique_ptr<TweenBase> ptr, float delay, float totalDuration);

	Sequence &InsertImpl(float atPosition, std::unique_ptr<TweenBase> ptr, float delay, float totalDuration);

	void Update(float deltaTime) override;

	void ApplyStartValue() override;

	void ApplyEndValue() override;

	void OnKillImpl() override;

	void RewindChildren();

	void ShiftElements(float offset);
};



// inline func ====================================================================================

// prepend ---------------------------------------------------------------

template <typename T>
inline Sequence &Sequence::Prepend(Tweener<T> tween) {
	// 無限ループの禁止
	assert(tween.loops_ != -1 && "Cannot add infinite-loop Tween to Sequence");

	// 子のdelayを無効化
	float delay = tween.delay_;
	float totalDur = tween.duration_ * tween.loops_;
	tween.delay_ = 0.0f;
	tween.delayComplete_ = true;
	tween.autoKill_ = false;

	float offset = delay + totalDur;
	float startTime = delay;

	ShiftElements(offset);
	elements_.insert(elements_.begin(), SequenceElement{std::make_unique<Tweener<T>>(std::move(tween)), startTime, 0.0f});

	return *this;
}


inline Sequence &Sequence::Prepend(Sequence &&seq) {
	float delay = seq.delay_;
	float totalDur = seq.duration_ * seq.loops_;

	return PrependImpl(std::make_unique<Sequence>(std::move(seq)), delay, totalDur);
}


inline Sequence &Sequence::Prepend(const Sequence &seq) {
	return PrependImpl(seq.Clone(), seq.delay_, seq.duration_ * seq.loops_);
}


inline Sequence &Sequence::PrependImpl(std::unique_ptr<TweenBase> ptr, float delay, float totalDuration) {
	// 無限ループの禁止
	assert(ptr->loops_ != -1 && "Cannot add infinite-loop Sequence to Sequence");

	// 子のdelayを無効化
	ptr->delay_ = 0.0f;
	ptr->delayComplete_ = true;
	ptr->autoKill_ = false;

	float offset = delay + totalDuration;
	float startTime = delay;

	ShiftElements(offset);
	elements_.insert(elements_.begin(), SequenceElement{std::move(ptr), startTime, 0.0f});

	return *this;
}

// --------------------------------------------------------------- prepend



// append ----------------------------------------------------------------

template <typename T>
inline Sequence &Sequence::Append(Tweener<T> tween) {
	// 無限ループの禁止
	assert(tween.loops_ != -1 && "Cannot add infinite-loop Tween to Sequence");

	// 子のdelayを無効化
	float delay = tween.delay_;
	float totalDur = tween.duration_ * tween.loops_;
	tween.delay_ = 0.0f;
	tween.delayComplete_ = true;
	tween.autoKill_ = false;

	float startTime = appendPosition_ + delay;
	lastAppendStartTime_ = startTime;
	appendPosition_ = startTime + totalDur;
	this->duration_ = (std::max)(this->duration_, appendPosition_);

	elements_.push_back(SequenceElement{std::make_unique<Tweener<T>>(std::move(tween)), startTime, 0.0f});

	return *this;
}


inline Sequence &Sequence::Append(Sequence &&seq) {
	float delay = seq.delay_;
	float totalDur = seq.duration_ * seq.loops_;

	return AppendImpl(std::make_unique<Sequence>(std::move(seq)), delay, totalDur);
}


inline Sequence &Sequence::Append(const Sequence &seq) {
	return AppendImpl(seq.Clone(), seq.delay_, seq.duration_ * seq.loops_);
}


inline Sequence &Sequence::AppendImpl(std::unique_ptr<TweenBase> ptr, float delay, float totalDur) {
	// 無限ループの禁止
	assert(ptr->loops_ != -1 && "Cannot add infinite-loop Tween to Sequence");

	// 子のdelayを無効化
	ptr->delay_ = 0.0f;
	ptr->delayComplete_ = true;
	ptr->autoKill_ = false;

	float startTime = appendPosition_ + delay;
	lastAppendStartTime_ = startTime;
	appendPosition_ = startTime + totalDur;
	this->duration_ = (std::max)(this->duration_, appendPosition_);

	elements_.push_back(SequenceElement{std::move(ptr), startTime, 0.0f});

	return *this;
}

// ---------------------------------------------------------------- append



// join ------------------------------------------------------------------

template <typename T>
inline Sequence &Sequence::Join(Tweener<T> tween) {
	// 無限ループの禁止
	assert(tween.loops_ != -1 && "Cannot add infinite-loop Tween to Sequence");

	// 子のdelayを無効化
	float delay = tween.delay_;
	float totalDur = tween.duration_ * tween.loops_;
	tween.delay_ = 0.0f;
	tween.delayComplete_ = true;
	tween.autoKill_ = false;

	float startTime = lastAppendStartTime_ + delay;
	this->duration_ = (std::max)(this->duration_, startTime + totalDur);

	elements_.push_back(SequenceElement{std::make_unique<Tweener<T>>(std::move(tween)), startTime, 0.0f});

	return *this;
}


inline Sequence &Sequence::Join(Sequence &&seq) {
	float delay = seq.delay_;
	float totalDur = seq.duration_ * seq.loops_;

	return JoinImpl(std::make_unique<Sequence>(std::move(seq)), delay, totalDur);
}


inline Sequence &Sequence::Join(const Sequence &seq) {
	return JoinImpl(seq.Clone(), seq.delay_, seq.duration_ * seq.loops_);
}


inline Sequence &Sequence::JoinImpl(std::unique_ptr<TweenBase> ptr, float delay, float totalDuration) {
	// 無限ループの禁止
	assert(ptr->loops_ != -1 && "Cannot add infinite-loop Tween to Sequence");

	// 子のdelayを無効化
	ptr->delay_ = 0.0f;
	ptr->delayComplete_ = true;
	ptr->autoKill_ = false;

	float startTime = lastAppendStartTime_ + delay;
	this->duration_ = (std::max)(this->duration_, startTime + totalDuration);

	elements_.push_back(SequenceElement{std::move(ptr), startTime, 0.0f});

	return *this;
}

// ------------------------------------------------------------------ join



// insert ----------------------------------------------------------------

template <typename T>
inline Sequence &Sequence::Insert(float atPosition, Tweener<T> tween) {
	// 無限ループの禁止
	assert(tween.loops_ != -1 && "Cannot add infinite-loop Tween to Sequence");

	// 子のdelayを無効化
	float delay = tween.delay_;
	float totalDur = tween.duration_ * tween.loops_;
	tween.delay_ = 0.0f;
	tween.delayComplete_ = true;
	tween.autoKill_ = false;

	float startTime = atPosition + delay;
	this->duration_ = (std::max)(this->duration_, startTime + totalDur);

	elements_.push_back(SequenceElement{std::make_unique<Tweener<T>>(std::move(tween)), startTime, 0.0f});

	return *this;
}


inline Sequence &Sequence::Insert(float atPosition, Sequence &&seq) {
	float delay = seq.delay_;
	float totalDur = seq.duration_ * seq.loops_;

	return InsertImpl(atPosition, std::make_unique<Sequence>(std::move(seq)), delay, totalDur);
}


inline Sequence &Sequence::Insert(float atPosition, const Sequence &seq) {
	return InsertImpl(atPosition, seq.Clone(), seq.delay_, seq.duration_ * seq.loops_);
}


inline Sequence &Sequence::InsertImpl(float atPosition, std::unique_ptr<TweenBase> ptr, float delay, float totalDuration) {
	// 無限ループの禁止
	assert(ptr->loops_ != -1 && "Cannot add infinite-loop Tween to Sequence");

	// 子のdelayを無効化
	ptr->delay_ = 0.0f;
	ptr->delayComplete_ = true;
	ptr->autoKill_ = false;

	float startTime = atPosition + delay;
	this->duration_ = (std::max)(this->duration_, startTime + totalDuration);

	elements_.push_back(SequenceElement{std::move(ptr), startTime, 0.0f});

	return *this;
}

// ---------------------------------------------------------------- insert



inline Sequence &Sequence::PrependInterval(float duration) {
	ShiftElements(duration);

	return *this;
}


inline Sequence &Sequence::AppendInterval(float duration) {
	appendPosition_ += duration;
	this->duration_ = (std::max)(this->duration_, appendPosition_);

	return *this;
}


inline std::unique_ptr<TweenBase> Sequence::Clone() const {
	Sequence copy{};

	// 親のコピー
	static_cast<TweenBase &>(copy) = static_cast<const TweenBase &>(*this);
	copy.appendPosition_ = (std::max)(this->appendPosition_, this->duration_);
	copy.lastAppendStartTime_ = this->lastAppendStartTime_;

	// 子のコピー
	for (auto &elem : elements_) {
		copy.elements_.push_back(SequenceElement{elem.tween->Clone(), elem.startTime, elem.elapsedTime});
	}

	return std::make_unique<Sequence>(std::move(copy));
}


inline void Sequence::Update(float deltaTime) {
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

		// アニメーション開始時コールバック
		if (this->onStart_) {
			this->onStart_();
		}
	}

	// 親の時間更新
	this->position_ += deltaTime;

	// 子の更新
	for (auto &elem : elements_) {
		if (!elem.tween->isPlaying_ || elem.tween->isComplete_) {
			continue;
		}
		if (this->position_ < elem.startTime) {
			continue;
		}

		float childElapsed = this->position_ - elem.startTime; // 子が開始してからの経過時間
		float childDelta = childElapsed - elem.elapsedTime;	   // 前フレームとの差分

		if (childDelta > 0.0f) {
			elem.elapsedTime = childElapsed;
			elem.tween->Update(childDelta);
		}
	}

	// 更新時コールバック
	if (this->onUpdate_) {
		this->onUpdate_();
	}

	// 親のループ処理
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

		// 子を初期値に戻す
		RewindChildren();
	}
}


inline void Sequence::ApplyStartValue() {
	RewindChildren();
}


inline void Sequence::ApplyEndValue() {
	for (auto &elem : elements_) {
		if (!elem.tween->isActive_ || elem.tween->isComplete_) {
			continue;
		}

		elem.tween->ApplyEndValue();
		elem.tween->isComplete_ = true;
		elem.tween->isPlaying_ = false;

		// 各ループ完了時コールバック
		if (elem.tween->onStepComplete_) {
			elem.tween->onStepComplete_();
		}
		// 全ループ完了時コールバック
		if (elem.tween->onComplete_) {
			elem.tween->onComplete_();
		}
	}
}


inline void Sequence::OnKillImpl() {
	for (auto &elem : elements_) {
		elem.tween->isActive_ = false;

		// Kill時コールバック
		if (elem.tween->onKill_) {
			elem.tween->onKill_();
		}
	}
}


inline void Sequence::RewindChildren() {
	for (auto &elem : elements_) {
		elem.tween->position_ = 0.0f;
		elem.tween->completedLoops_ = 0;
		elem.tween->isPlaying_ = true;
		elem.tween->isComplete_ = false;
		elem.tween->isBackwards_ = false;

		elem.elapsedTime = 0.0f;
	}

	// 逆順で初期値を適用
	for (int i = static_cast<int>(elements_.size()) - 1; i >= 0; i--) {
		if (elements_[i].tween->isInitialize_) {
			elements_[i].tween->ApplyStartValue();
		}
	}
}


inline void Sequence::ShiftElements(float offset) {
	for (auto &elem : elements_) {
		elem.startTime += offset;
	}

	this->duration_ += offset;
	appendPosition_ += offset;
	lastAppendStartTime_ += offset;
}

// ==================================================================================== inline func