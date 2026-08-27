#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>


class MusicManager {
public: // メンバ関数

	static MusicManager *GetInstance();

	uint32_t Load(const std::string &soundName);

	void Finalize();

	void UpdateImGui();


	// bgm ----------------------------------------------------------------------------------------

	// ループ再生
	void PlayBgm(const std::string &soundName);

	// 1回だけ再生
	void PlayJingle(const std::string &soundName);

	void StopBgm();

	bool IsBgmPlaying() const;

	// --------------------------------------------------------------------------------------------


	// se -----------------------------------------------------------------------------------------

	uint32_t PlaySe(const std::string &soundName);

	void StopSe(uint32_t voiceHandle);

	// --------------------------------------------------------------------------------------------


	// volume -------------------------------------------------------------------------------------

	void SetMasterVolume(float volume);

	void SetBgmVolume(float volume);

	void SetSeVolume(float volume) {
		seVolume_ = volume;
	}

	float GetMasterVolume() const {
		return masterVolume_;
	}

	float GetBgmVolume() const {
		return bgmVolume_;
	}

	float GetSeVolume() const {
		return seVolume_;
	}

	// --------------------------------------------------------------------------------------------


private: // メンバ変数

	std::unordered_map<std::string, uint32_t> soundHandles_;

	uint32_t bgmVoiceHandle_ = 0;
	bool hasBgm_ = false;
	bool isBgmLoop_ = false;
	std::string bgmName_;

	// volume
	float masterVolume_ = 1.0f;
	float bgmVolume_ = 1.0f;
	float seVolume_ = 1.0f;

	std::string name_ = "MusicManager";


private: // メンバ関数

	MusicManager() = default;
	~MusicManager() = default;
	MusicManager(const MusicManager &) = delete;
	MusicManager &operator=(const MusicManager &) = delete;

	void PlayOnBgmSlot(const std::string &soundName, bool isLoop);
};