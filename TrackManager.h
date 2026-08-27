#pragma once

#include "Track.h"
#include <memory>
#include <vector>


class TrackManager {
public: // メンバ関数

	Track *Add(std::unique_ptr<Track> track);

	void Clear();

	void DrawAllTracks() const;

	// レールの見た目を作り直す
	void BuildAllRails();

	// レールの描画
	void DrawAllRails(const KamataEngine::Camera &camera) const;

	void UpdateImGui();

	void AlignAll();


	const std::vector<std::unique_ptr<Track>> &GetTracks() const {
		return tracks_;
	}


private: // メンバ変数

	std::vector<std::unique_ptr<Track>> tracks_;

	bool isShowTrack_ = true;


private: // メンバ関数

	void AlignChain(Track *head);
};