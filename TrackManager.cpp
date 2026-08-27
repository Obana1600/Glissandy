#include "TrackManager.h"
#include <unordered_set>


Track *TrackManager::Add(std::unique_ptr<Track> track) {
	auto *ptr = track.get();
	tracks_.push_back(std::move(track));

	return ptr;
}


void TrackManager::Clear() {
	tracks_.clear();
}


void TrackManager::DrawAllTracks() const {
	if (!isShowTrack_) {
		return;
	}

	for (auto &track : tracks_) {
		track->DrawDebug();
	}
}


void TrackManager::BuildAllRails() {
	for (auto &track : tracks_) {
		track->BuildRail();
	}
}


void TrackManager::DrawAllRails(const KamataEngine::Camera &camera) const {
	for (auto &track : tracks_) {
		track->DrawRail(camera);
	}
}


void TrackManager::UpdateImGui() {
#ifdef USE_IMGUI
	const auto count = tracks_.size();

	ImGui::Indent();
	for (size_t i = 0; i < count; i++) {
		auto &track = tracks_[i];
		ImGui::Spacing();

		ImGui::PushID(track.get());

		auto name = "Track" + std::to_string(i);
		if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			track->UpdateImGui();
		}

		ImGui::PopID();
	}
	ImGui::Unindent();

	if (ImGui::Button("AlignAll")) {
		AlignAll();
	}

	BuildAllRails();
#endif
}


void TrackManager::AlignAll() {
	std::unordered_set<const Track *> pointed;

	// nextを集める
	for (const auto &track : tracks_) {
		if (const auto *next = track->GetNext()) {
			pointed.insert(next);
		}
	}

	// nextがないものを先頭とする
	for (const auto &track : tracks_) {
		if (pointed.contains(track.get())) {
			continue;
		}
		AlignChain(track.get());
	}
}


void TrackManager::AlignChain(Track *head) {
	std::unordered_set<const Track *> visited;

	for (Track *current = head; current;) {
		if (!visited.insert(current).second) {
			break;
		}

		auto *next = current->GetNext();
		if (next) {
			next->AlignTo(*current);
		}
		current = next;
	}
}