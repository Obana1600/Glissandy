#include "MusicManager.h"

#include "DebugUi.h"
#include "KamataEngine.h"

using namespace KamataEngine;


namespace {

	constexpr const char *kDirectory = "music/";
	constexpr const char *kExtension = ".wav";

} // namespace



MusicManager *MusicManager::GetInstance() {
	static MusicManager instance;
	return &instance;
}


uint32_t MusicManager::Load(const std::string &soundName) {
	auto it = soundHandles_.find(soundName);

	// すでに読み込んでいるならそれを返す
	if (it != soundHandles_.end()) {
		return it->second;
	}

	// 新しく読み込む
	auto handle = Audio::GetInstance()->LoadWave(kDirectory + soundName + kExtension);
	soundHandles_[soundName] = handle;

	return handle;
}


void MusicManager::Finalize() {
	StopBgm();

	soundHandles_.clear();
}


void MusicManager::UpdateImGui() {
#ifdef USE_IMGUI
	ImGui::Begin(name_.c_str());

	// テーブル
	if (DebugUi::BeginTable(name_.c_str())) {

		// masterVolume
		DebugUi::PropatyRow("MasterVolume", [&] {
			float volume = masterVolume_;
			if (ImGui::SliderFloat("##masterVolume", &volume, 0.0f, 1.0f)) {
				SetMasterVolume(volume);
			}
		});

		// bgmVolume
		DebugUi::PropatyRow("BgmVolume", [&] {
			float volume = bgmVolume_;
			if (ImGui::SliderFloat("##bgmVolume", &volume, 0.0f, 1.0f)) {
				SetBgmVolume(volume);
			}
		});

		// seVolume
		DebugUi::PropatyRow("SeVolume", [&] {
			ImGui::SliderFloat("##seVolume", &seVolume_, 0.0f, 1.0f);
		});

		// bgm
		DebugUi::PropatyRow("Bgm", [&] {
			if (hasBgm_) {
				ImGui::Text("%s (%s)", bgmName_.c_str(), isBgmLoop_ ? "loop" : "once");
			} else {
				ImGui::Text("-");
			}
		});

		// isBgmPlaying
		DebugUi::PropatyRow("IsBgmPlaying", [&] {
			ImGui::Text("%s", IsBgmPlaying() ? "true" : "false");
		});

		// loadedCount
		DebugUi::PropatyRow("LoadedCount", [&] {
			ImGui::Text("%zu", soundHandles_.size());
		});

		// stop
		DebugUi::PropatyRow("Simulate", [&] {
			if (ImGui::Button("stop bgm")) {
				StopBgm();
			}
		});

		DebugUi::EndTable();
	}

	ImGui::End();
#endif
}


void MusicManager::PlayBgm(const std::string &soundName) {
	if (hasBgm_ && isBgmLoop_ && bgmName_ == soundName && IsBgmPlaying()) {
		return;
	}

	PlayOnBgmSlot(soundName, true);
}


void MusicManager::PlayJingle(const std::string &soundName) {
	PlayOnBgmSlot(soundName, false);
}


void MusicManager::StopBgm() {
	if (!hasBgm_) {
		return;
	}

	Audio::GetInstance()->StopWave(bgmVoiceHandle_);

	hasBgm_ = false;
	isBgmLoop_ = false;
	bgmName_.clear();
}


bool MusicManager::IsBgmPlaying() const {
	if (!hasBgm_) {
		return false;
	}

	return Audio::GetInstance()->IsPlaying(bgmVoiceHandle_);
}


uint32_t MusicManager::PlaySe(const std::string &soundName) {
	return Audio::GetInstance()->PlayWave(Load(soundName), false, masterVolume_ * seVolume_);
}


void MusicManager::StopSe(uint32_t voiceHandle) {
	Audio::GetInstance()->StopWave(voiceHandle);
}


void MusicManager::SetMasterVolume(float volume) {
	masterVolume_ = volume;

	if (hasBgm_) {
		Audio::GetInstance()->SetVolume(bgmVoiceHandle_, masterVolume_ * bgmVolume_);
	}
}


void MusicManager::SetBgmVolume(float volume) {
	bgmVolume_ = volume;

	if (hasBgm_) {
		Audio::GetInstance()->SetVolume(bgmVoiceHandle_, masterVolume_ * bgmVolume_);
	}
}


void MusicManager::PlayOnBgmSlot(const std::string &soundName, bool isLoop) {
	StopBgm();

	bgmVoiceHandle_ = Audio::GetInstance()->PlayWave(Load(soundName), isLoop, masterVolume_ * bgmVolume_);

	hasBgm_ = true;
	isBgmLoop_ = isLoop;
	bgmName_ = soundName;
}