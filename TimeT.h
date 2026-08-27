#pragma once

#include <chrono>
#include <cstdint>


class TimeT {
public: // 静的メンバ関数

	// フレームの更新
	static void Update();

	// フレームレートの取得
	static float GetFramerate();

	// デルタタイム（ミリ秒）の取得
	static float GetDeltaTimeMS();


	static float GetDeltaTime() {
		return deltaTime_;
	}

	static float GetTime() {
		return time_;
	}

	static uint64_t GetFrameCount() {
		return frameCount_;
	}


private: // 静的メンバ変数

	// 目標デルタタイム
	static constexpr float kTarget_ = 1.0f / 60.0f;

	static inline float deltaTime_ = kTarget_;
	static inline float time_ = 0.0f;
	static inline uint64_t frameCount_ = 0;

	static inline std::chrono::steady_clock::time_point lastTime_;
	static inline bool isInitialized_ = false;


private: // メンバ関数

	TimeT() = delete;
};