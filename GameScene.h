#pragma once

#include "CollisionManager.h"
#include "Fade.h"
#include "FollowCamera.h"
#include "GameObject.h"
#include "ParticleManager.h"
#include "SceneBase.h"
#include "TrackManager.h"
#include <memory>
#include <vector>


class GameScene: public SceneBase {
public: // メンバ関数

	void Initialize() override;

	void Update() override;

	void Draw() const override;

	const std::string &GetName() const override {
		return name_;
	}


private: // インナークラス

	enum class GameState {
		Play,
		GameOver,
		Clear,
	};


private: // メンバ変数

	// state
	GameState state_ = GameState::Play;

	// collision
	std::unique_ptr<CollisionManager> collisionMgr_;

	// particle
	std::unique_ptr<ParticleManager> particleMgr_;

	// track
	std::unique_ptr<TrackManager> trackMgr_;

	// followCamera
	std::unique_ptr<FollowCamera> followCamera_;

	// fade
	std::unique_ptr<Fade> fade_;

	// skydome
	std::unique_ptr<GameObject> skydome_;

	// objects
	std::vector<std::unique_ptr<GameObject>> dynamicObjects_;

	// player
	std::unique_ptr<GameObject> player_;

	// enemy
	std::unique_ptr<GameObject> enemy_;

	// result
	std::unique_ptr<GameObject> resultLabel_;
	std::unique_ptr<GameObject> retryButton_;
	std::unique_ptr<GameObject> titleButton_;
	const KamataEngine::Vector4 kResultFadeColor_ = {0.0f, 0.0f, 0.0f, 0.2f};
	const float kResultFadeDuration_ = 1.0f;

	// hud
	std::vector<std::unique_ptr<GameObject>> hudObjects_;
	GameObject *hpFillPlayer_ = nullptr;
	GameObject *hpFillEnemy_ = nullptr;
	const std::string kHpBarTexturePlayer_ = "hp_player.png";
	const std::string kHpBarTextureEnemy_ = "hp_enemy.png";
	const std::string kHpBarTextureFill_ = "white1x1.png";
	// アンカー
	const KamataEngine::Vector2 kHpBarAnchorFrame_ = {0.5f, 0.5f};
	const KamataEngine::Vector2 kHpBarAnchorFill_ = {0.0f, 0.5f};
	// サイズ / 座標
	const KamataEngine::Vector2 kHpBarFrameSize_ = {428.0f, 110.0f};
	const KamataEngine::Vector4 kHpBarColorFrame_ = {1.0f, 1.0f, 1.0f, 1.0f};
	const KamataEngine::Vector3 kHpFramePlayerPosition_ = {0.0f, -300.0f, 0.0f};
	const KamataEngine::Vector3 kHpFramePlayerScale_ = {0.7f, 0.7f, 1.0f};
	const KamataEngine::Vector3 kHpFillPlayerPosition_ = {-121.0f, -292.0f, 0.0f};
	const KamataEngine::Vector2 kHpFillPlayerSize_ = {265.0f, 36.0f};
	const KamataEngine::Vector4 kHpBarColorPlayer_ = {1.0f, 0.392f, 0.588f, 1.0f};
	const KamataEngine::Vector3 kHpFrameEnemyPosition_ = {0.0f, 300.0f, 0.0f};
	const KamataEngine::Vector3 kHpFillEnemyPosition_ = {-204.0f, 280.0f, 0.0f};
	const KamataEngine::Vector2 kHpFillEnemySize_ = {380.0f, 50.0f};
	const KamataEngine::Vector4 kHpBarColorEnemy_ = {0.588f, 0.0f, 0.0f, 1.0f};

	// tutorial
	std::vector<std::unique_ptr<GameObject>> tutorialTexts_;
	std::unique_ptr<GameObject> skipLabel_;
	const Track *circleTrack_ = nullptr;
	bool isTutorial_ = true;
	static constexpr BYTE kSkipKey_ = DIK_TAB;
	static constexpr int kCircleLaneIndex_ = 2; // 円周5本の中央
	const float kTutorialTextHeight_ = 6.0f;
	const KamataEngine::Vector3 kSkipLabelPosition_ = {250.0f, -300.0f, 0.0f};
	const float kSkipLabelFontSize_ = 48.0f;
	const KamataEngine::Vector4 kSkipFadeColor_ = {0.0f, 0.0f, 0.0f, 1.0f};
	const float kSkipFadeDuration_ = 0.6f;

	std::string name_ = "Game";


private: // メンバ関数

	void ChangeState(GameState state);

	void InitializeResult();
	void UpdateResult();
	void DrawResult() const;

	void InitializeHud();
	void UpdateHud();
	void DrawHud() const;
	GameObject *AddHealthBar(const std::string &name, const std::string &textureName, const KamataEngine::Vector3 &position, const KamataEngine::Vector2 &size, const KamataEngine::Vector4 &color, const KamataEngine::Vector2 &anchorPoint);

	void InitializeTutorial();
	void UpdateTutorial();
	void DrawTutorial() const;
	void AddTutorialText(const Track *track, float t01, const std::string &text);
	void SkipTutorial();
	void EndTutorial();

	void UpdateGameCamera() override;

	void RegisterObject(std::unique_ptr<GameObject> object);
};