#pragma once

#include <functional>
#include <memory>


// ループ時の挙動
enum class LoopType {
	Restart,	// 最初から再生し直す
	Yoyo,		// ループするたびに反転
	Incremental // 終端値を加算しながらループする
};


// イージング関数の種類
enum class EaseType {
	Linear,

	InSine,
	OutSine,
	InOutSine,

	InQuad,
	OutQuad,
	InOutQuad,

	InCubic,
	OutCubic,
	InOutCubic,

	InQuart,
	OutQuart,
	InOutQuart,

	InQuint,
	OutQuint,
	InOutQuint,

	InExpo,
	OutExpo,
	InOutExpo,

	InCirc,
	OutCirc,
	InOutCirc,

	InElastic,
	OutElastic,
	InOutElastic,

	InBack,
	OutBack,
	InOutBack,

	InBounce,
	OutBounce,
	InOutBounce,

	Random,

	NumCount
};



class TweenBase {
public: // メンバ関数

	virtual ~TweenBase() = default;


	virtual void Update(float deltaTime) = 0;

	virtual std::unique_ptr<TweenBase> Clone() const = 0;

	virtual void KillTween(bool complete) = 0;


	// getter -----------------------------------

	bool IsActive() const {
		return isActive_;
	}

	bool IsPlaying() const {
		return isPlaying_;
	}

	bool IsComplete() const {
		return isComplete_;
	}

	// ------------------------------------------


protected: // メンバ変数

	// フレンドクラス
	friend class Sequence;
	friend class EaseManager;
	friend class TweenManager;

	// state
	bool isActive_ = true;		// 有効か（Killされるとfalse）
	bool isPlaying_ = true;		// 再生中か
	bool isComplete_ = false;	// 完了したか
	bool isBackwards_ = false;	// 逆再生中か（Yoyoループの折り返し時）
	bool isRelative_ = false;	// 終端値を相対値として扱うか
	bool isInitialize_ = false; // 初期化されたか（初回Updateでtrue）

	// param
	float position_ = 0.0f;	 // 現在の再生位置（秒）
	float duration_ = 0.0f;	 // 1ループの長さ（秒）
	int loops_ = 1;			 // ループ回数（-1で無限）
	int completedLoops_ = 0; // 完了したループ数

	// setting
	LoopType loopType_ = LoopType::Restart; // ループ時の挙動
	EaseType easeType_ = EaseType::OutQuad; // イージング関数の種類
	float delay_ = 0.0f;					// 再生開始前のディレイ（秒）
	float elapsedDelay_ = 0.0f;				// ディレイの経過時間（秒）
	bool delayComplete_ = false;			// ディレイが完了したか
	bool autoKill_ = true;					// 完了時に自動でKillするか

	// メンバ関数ポインタのキャプチャ対象
	void *target_ = nullptr;

	// callBack
	std::function<void()> onPlay_{};		 // 再生開始時に呼び出される
	std::function<void()> onPause_{};		 // 一時停止時
	std::function<void()> onRewind_{};		 // 巻き戻し時
	std::function<void()> onStart_{};		 // アニメーション開始時
	std::function<void()> onUpdate_{};		 // 毎フレーム更新時
	std::function<void()> onStepComplete_{}; // 各ループ終了時
	std::function<void()> onComplete_{};	 // 全ループ完了時
	std::function<void()> onKill_{};		 // Kill時


protected: // メンバ関数

	virtual void ApplyStartValue() = 0;

	virtual void ApplyEndValue() = 0;

	virtual void OnKillImpl() {
	}
};