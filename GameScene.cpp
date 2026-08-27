#include "GameScene.h"

#include "BossController.h"
#include "ChargeShooter.h"
#include "Collider.h"
#include "ConeTrack.h"
#include "CursorLock.h"
#include "DebugUi.h"
#include "Health.h"
#include "MusicManager.h"
#include "ParticleEmitter.h"
#include "PlayerController.h"
#include "Prefab.h"
#include "RailMover.h"
#include "Reticle.h"
#include "SceneManager.h"
#include "StraightTrack.h"
#include "Text/Text2D.h"
#include "Text/Text3D.h"
#include "TextButton.h"
#include <algorithm>
#include <cassert>
#include <cmath>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;


namespace {

	// コンポーネントのトグル
	template <typename T>
	void SetComponentEnabled(GameObject *object, bool enabled) {
		if (auto *component = object->GetComponent<T>()) {
			component->SetEnabled(enabled);
		}
	}

	// 岩の名前
	constexpr const char *kRockName = "rock";

	// リザルトのボタンの大きさ
	constexpr Vector2 kResultButtonRectSize = {450.0f, 90.0f};


	// 先に読んでおくSE
	constexpr const char *kPreloadSounds[] = {
		"charge",
		"shot",
		"hit_enemy",
		"damage_player",
		"lane_change",
		"ui_click",
		"boss_place_rock",
		"rock_break",
		"boss_death",
		"gameover",
		"clear",
	};


	// 階層をたどってすべてのColliderを登録する
	void RegisterColliders(CollisionManager *manager, GameObject *object) {
		for (auto *collider : object->GetComponents<Collider>()) {
			manager->Register(collider);
		}

		for (const auto &child : object->GetChildren()) {
			RegisterColliders(manager, child.get());
		}
	}


	// 階層をたどってInspectorに登録する
	void RegisterToEditor(GuiEditor *editor, GameObject *object) {
		editor->Register(object);

		for (const auto &child : object->GetChildren()) {
			RegisterToEditor(editor, child.get());
		}
	}

} // namespace



void GameScene::Initialize() {
	InitializeInternal();
	PrimitiveDrawer::GetInstance()->SetCamera(&camera_);
	AxisIndicator::GetInstance()->Initialize();
	AxisIndicator::GetInstance()->SetTargetCamera(&camera_);


	// collider -----------------------------------------------------------------------------------

	collisionMgr_ = std::make_unique<CollisionManager>();

	// --------------------------------------------------------------------------------------------


	// particle -----------------------------------------------------------------------------------

	particleMgr_ = std::make_unique<ParticleManager>();
	particleMgr_->Initialize();

	// --------------------------------------------------------------------------------------------


	// skydome ------------------------------------------------------------------------------------

	skydome_ = Prefab::CreateSkydome();

	// --------------------------------------------------------------------------------------------


	// track --------------------------------------------------------------------------------------

	trackMgr_ = std::make_unique<TrackManager>();

	Vector3 bossCenter{};
	const Track *bossTrack = nullptr;

	{
		constexpr float kLaneWidth = 10.0f;
		constexpr float kConeLaneWidth = 10.0f;

		// 本数と長さを決めて追加
		auto addStraight = [this](int laneCount, float length, float endWidth) {
			auto track = std::make_unique<StraightTrack>();
			track->SetStart({0.0f, 0.0f, 0.0f});
			track->SetEnd({0.0f, 0.0f, length});
			track->SetLaneCount(laneCount);
			track->SetEndWidth(endWidth);

			return static_cast<StraightTrack *>(trackMgr_->Add(std::move(track)));
		};

		// 直線レール
		auto *start = addStraight(1, 400.0f, 0.0f);		  // 1本
		auto *open2 = addStraight(2, 500.0f, kLaneWidth); // 2本に分岐
		auto *gather2 = addStraight(2, 80.0f, 0.0f);	  // 1本へ収束
		auto *open3 = addStraight(3, 600.0f, kLaneWidth); // 3本に分岐
		auto *gather3 = addStraight(3, 80.0f, 0.0f);	  // 1本へ収束
		auto *bridge = addStraight(1, 120.0f, 0.0f);	  // 短い直線

		// 円周レール
		auto coneTrack = std::make_unique<ConeTrack>();
		coneTrack->SetLaneCount(5);
		coneTrack->SetStartWidth(kConeLaneWidth);
		coneTrack->SetEndWidth(kConeLaneWidth);
		auto *circle = static_cast<ConeTrack *>(trackMgr_->Add(std::move(coneTrack)));

		// 先頭の位置
		start->SetStart({0.0f, 0.0f, 0.0f});
		start->SetEnd({0.0f, 0.0f, 350.0f});

		start->SetNext(open2);
		open2->SetNext(gather2);
		gather2->SetNext(open3);
		open3->SetNext(gather3);
		gather3->SetNext(bridge);
		bridge->SetNext(circle);

		trackMgr_->AlignAll();

		// レールの見た目
		trackMgr_->BuildAllRails();

		// 説明文
		AddTutorialText(start, 0.45f, "W / S で加速・減速");
		AddTutorialText(open2, 0.2f, "A / D でレーンを移動");
		AddTutorialText(open3, 0.2f, "左クリックを長押しして溜め\n離すと発射");

		circleTrack_ = circle;
		bossCenter = circle->GetCenter();
		bossTrack = circle;
	}

	// --------------------------------------------------------------------------------------------


	// gui ----------------------------------------------------------------------------------------

	guiEditor_->SetCollisionManager(collisionMgr_.get());
	guiEditor_->SetTrackManager(trackMgr_.get());

	// --------------------------------------------------------------------------------------------


	// player -------------------------------------------------------------------------------------

	player_ = Prefab::CreatePlayer();

	if (auto *railMover = player_->GetComponent<RailMover>()) {
		railMover->SetTrack(trackMgr_->GetTracks()[0].get());
		railMover->SetTrackManager(trackMgr_.get());
	}

	for (auto *emitter : player_->GetComponents<ParticleEmitter>()) {
		emitter->SetParticleManager(particleMgr_.get());
	}

	if (auto *chargeShooter = player_->GetComponent<ChargeShooter>()) {
		chargeShooter->SetCamera(&camera_);
		chargeShooter->SetCollisionManager(collisionMgr_.get());
		chargeShooter->SetSpawnCallback([this](std::unique_ptr<GameObject> obj) {
			RegisterObject(std::move(obj));
		});
	}
	if (auto *health = player_->GetComponent<Health>()) {
		health->SetOnDeath([this] {
			ChangeState(GameState::GameOver);
		});
	}

	// --------------------------------------------------------------------------------------------


	// enemy --------------------------------------------------------------------------------------

	enemy_ = Prefab::CreateEnemy();
	enemy_->SetTranslation(bossCenter);

	if (auto *boss = enemy_->GetComponent<BossController>()) {
		boss->SetPlayer(player_.get());
		boss->SetTrack(bossTrack);
		boss->SetSpawnCallback([this](std::unique_ptr<GameObject> obj) {
			RegisterObject(std::move(obj));
		});
		boss->SetRockCountCallback([this] {
			return static_cast<int>(std::count_if(dynamicObjects_.begin(), dynamicObjects_.end(), [](const auto &object) {
				return object->GetName() == kRockName && !object->IsDead();
			}));
		});
	}

	// 登場演出を始める
	Prefab::PlayEnter(enemy_.get());

	// --------------------------------------------------------------------------------------------


	// followCamera -------------------------------------------------------------------------------

	followCamera_ = std::make_unique<FollowCamera>();
	followCamera_->SetTarget(player_.get());
	followCamera_->SetCamera(&camera_);

	// --------------------------------------------------------------------------------------------


	// fade ---------------------------------------------------------------------------------------

	fade_ = std::make_unique<Fade>();
	fade_->Initialize();

	// --------------------------------------------------------------------------------------------


	// result -------------------------------------------------------------------------------------

	InitializeResult();

	// --------------------------------------------------------------------------------------------


	// hud ----------------------------------------------------------------------------------------

	InitializeHud();

	// --------------------------------------------------------------------------------------------


	// tutorial -----------------------------------------------------------------------------------

	InitializeTutorial();

	// --------------------------------------------------------------------------------------------


	// music --------------------------------------------------------------------------------------

	for (const auto *name : kPreloadSounds) {
		MusicManager::GetInstance()->Load(name);
	}

	MusicManager::GetInstance()->PlayBgm("bgm_game");

	// --------------------------------------------------------------------------------------------
}


void GameScene::Update() {

	// object -----------------------------------

	std::erase_if(dynamicObjects_, [](const auto &obj) {
		return obj->IsDead();
	});

	// ------------------------------------------


	// collider ---------------------------------

	collisionMgr_->Clear();

	RegisterColliders(collisionMgr_.get(), player_.get());
	RegisterColliders(collisionMgr_.get(), enemy_.get());
	for (auto &obj : dynamicObjects_) {
		RegisterColliders(collisionMgr_.get(), obj.get());
	}

	// ------------------------------------------


	// tutorial ---------------------------------

	UpdateTutorial();

	// ------------------------------------------


	// player -----------------------------------

	player_->Update();

	// ------------------------------------------


	// enemy ------------------------------------

	enemy_->Update();

	// ------------------------------------------


	// objects ----------------------------------

	for (auto &obj : dynamicObjects_) {
		obj->Update();
	}

	// ------------------------------------------


	// collider ---------------------------------

	collisionMgr_->CheckAllCollisions();

	// ------------------------------------------


	// particle ---------------------------------

	particleMgr_->Update();

	// ------------------------------------------


	// fade -------------------------------------

	fade_->Update();
	UpdateTransition();

	// ------------------------------------------


	// result -----------------------------------

	if (enemy_->IsDead()) {
		ChangeState(GameState::Clear);
	}

	UpdateResult();

	// ------------------------------------------


	// camera -----------------------------------

	UpdateCamera();

	// ------------------------------------------


	// skydome ----------------------------------

	skydome_->SetTranslation(camera_.translation_);
	skydome_->Update();

	// ------------------------------------------


	// hud --------------------------------------

	UpdateHud();

	// ------------------------------------------


	// gui --------------------------------------

	guiEditor_->Clear();

	RegisterToEditor(guiEditor_.get(), player_.get());
	RegisterToEditor(guiEditor_.get(), enemy_.get());
	for (auto &obj : dynamicObjects_) {
		RegisterToEditor(guiEditor_.get(), obj.get());
	}

	for (auto &object : hudObjects_) {
		guiEditor_->Register(object.get());
	}

	guiEditor_->Register(resultLabel_.get());
	guiEditor_->Register(retryButton_.get());
	guiEditor_->Register(titleButton_.get());

	if (isTutorial_) {
		for (auto &object : tutorialTexts_) {
			guiEditor_->Register(object.get());
		}
		guiEditor_->Register(skipLabel_.get());
	}

	fade_->UpdateImGui();

	UpdateEditor();

	// ------------------------------------------
}


void GameScene::Draw() const {
	Model::PreDraw(); // 3D描画ここから =============================================================

	// skydome ----------------------------------

	skydome_->Draw(camera_, DrawPass::Model);

	// ------------------------------------------


	// track ------------------------------------

	trackMgr_->DrawAllRails(camera_);

	// ------------------------------------------


	// player -----------------------------------

	player_->Draw(camera_, DrawPass::Model);

	// ------------------------------------------


	// enemy ------------------------------------

	enemy_->Draw(camera_, DrawPass::Model);

	// ------------------------------------------


	// object -----------------------------------

	for (const auto &obj : dynamicObjects_) {
		obj->Draw(camera_, DrawPass::Model);
	}

	// ------------------------------------------

	Model::PostDraw(); // 3D描画ここまで ============================================================



	Sprite::PreDraw(); // 2d描画ここから ============================================================

	// player -----------------------------------

	player_->Draw(camera_, DrawPass::Sprite);

	// ------------------------------------------


	// hud --------------------------------------

	DrawHud();

	// ------------------------------------------


	// fade -------------------------------------

	fade_->Draw();

	// ------------------------------------------

	Sprite::PostDraw(); // 2d描画ここまで ===========================================================


	// particle -----------------------------------------------------------------------------------

	particleMgr_->Draw(camera_);

	// --------------------------------------------------------------------------------------------


	// transition ---------------------------------------------------------------------------------

	DrawTransition();

	// --------------------------------------------------------------------------------------------


	// tutorial -----------------------------------------------------------------------------------

	DrawTutorial();

	// --------------------------------------------------------------------------------------------


	// result -------------------------------------------------------------------------------------

	DrawResult();

	// --------------------------------------------------------------------------------------------

#ifdef USE_IMGUI
	collisionMgr_->DrawAllColliders();

	trackMgr_->DrawAllTracks();

	AxisIndicator::GetInstance()->Draw();
#endif
}


void GameScene::ChangeState(GameState state) {
	if (state_ == state) {
		return;
	}

	// リザルトから戻らない
	if (state_ == GameState::GameOver || state_ == GameState::Clear) {
		return;
	}

	state_ = state;

	// プレイ中ならリターン
	if (state_ == GameState::Play) {
		return;
	}

	// リザルト共通
	fade_->Start(kResultFadeColor_, kResultFadeDuration_);

	// チャージを止める
	if (auto *shooter = player_->GetComponent<ChargeShooter>()) {
		shooter->StopCharge();
	}

	// 操作/射撃/照準は共通で止める
	SetComponentEnabled<PlayerController>(player_.get(), false);
	SetComponentEnabled<ChargeShooter>(player_.get(), false);
	SetComponentEnabled<Reticle>(player_.get(), false);

	if (state_ == GameState::GameOver) {
		// プレイヤーを止める
		SetComponentEnabled<RailMover>(player_.get(), false);

		// 敵のhealthを止める
		SetComponentEnabled<Health>(enemy_.get(), false);

		// ジングルを鳴らす
		MusicManager::GetInstance()->PlayJingle("gameover");

		resultLabel_->GetComponent<Text2D>()->SetText("GAME OVER");

	} else {
		// プレイヤーのhealthを止める
		SetComponentEnabled<Health>(player_.get(), false);

		// ボスの動きを止める
		SetComponentEnabled<BossController>(enemy_.get(), false);

		MusicManager::GetInstance()->PlayJingle("clear");

		resultLabel_->GetComponent<Text2D>()->SetText("CLEAR");
	}
}


void GameScene::InitializeResult() {
	// ボタンの作成
	auto makeButton = [](const std::string &label, const Vector3 &position, std::function<void()> onClick) {
		auto result = Prefab::CreateTextButton(label, kResultButtonRectSize, std::move(onClick));
		result->SetTranslation(position);

		return result;
	};

	resultLabel_ = std::make_unique<GameObject>();
	resultLabel_->SetName("resultLabel");
	resultLabel_->SetTranslation({0.0f, 120.0f, 0.0f});
	auto *labelText = resultLabel_->AddComponent<Text2D>();
	resultLabel_->Initialize();
	labelText->SetFontSize(128);
	labelText->SetRectSize({520.0f, 120.0f});

	retryButton_ = makeButton("リトライ", {0.0f, -120.0f, 0.0f}, [] {
		SceneManager::GetInstance()->ChangeScene(SceneType::Game);
	});

	titleButton_ = makeButton("タイトルに戻る", {0.0f, -220.0f, 0.0f}, [] {
		SceneManager::GetInstance()->ChangeScene(SceneType::Title);
	});
}


void GameScene::UpdateResult() {
	if (state_ == GameState::Play) {
		return;
	}

	CursorLock::SetLocked(false);

	resultLabel_->Update();
	titleButton_->Update();
	if (state_ == GameState::GameOver) {
		retryButton_->Update();
	}
}


void GameScene::DrawResult() const {
	if (state_ == GameState::Play) {
		return;
	}

	resultLabel_->Draw(camera_, DrawPass::Text2D);
	titleButton_->Draw(camera_, DrawPass::Text2D);

	if (state_ == GameState::GameOver) {
		retryButton_->Draw(camera_, DrawPass::Text2D);
	}
}


void GameScene::InitializeHud() {
	// player
	auto *framePlayer = AddHealthBar("hpFramePlayer", kHpBarTexturePlayer_, kHpFramePlayerPosition_, kHpBarFrameSize_, kHpBarColorFrame_, kHpBarAnchorFrame_);
	framePlayer->SetScale(kHpFramePlayerScale_);
	hpFillPlayer_ = AddHealthBar("hpFillPlayer", kHpBarTextureFill_, kHpFillPlayerPosition_, kHpFillPlayerSize_, kHpBarColorPlayer_, kHpBarAnchorFill_);

	// enemy
	AddHealthBar("hpFrameEnemy", kHpBarTextureEnemy_, kHpFrameEnemyPosition_, kHpBarFrameSize_, kHpBarColorFrame_, kHpBarAnchorFrame_);
	hpFillEnemy_ = AddHealthBar("hpFillEnemy", kHpBarTextureFill_, kHpFillEnemyPosition_, kHpFillEnemySize_, kHpBarColorEnemy_, kHpBarAnchorFill_);
}


void GameScene::UpdateHud() {
	auto applyRate = [](GameObject *bar, const GameObject *target) {
		if (!bar || !target) {
			return;
		}

		const auto *health = target->GetComponent<Health>();
		if (!health || health->GetMaxHp() <= 0) {
			return;
		}

		const auto rate = std::clamp(static_cast<float>(health->GetHp()) / static_cast<float>(health->GetMaxHp()), 0.0f, 1.0f);
		bar->SetScale({rate, 1.0f, 1.0f});
	};

	applyRate(hpFillPlayer_, player_.get());
	applyRate(hpFillEnemy_, enemy_.get());

	for (const auto &object : hudObjects_) {
		object->Update();
	}
}


void GameScene::DrawHud() const {
	for (const auto &object : hudObjects_) {
		object->Draw(camera_, DrawPass::Sprite);
	}
}


GameObject *GameScene::AddHealthBar(const std::string &name, const std::string &textureName, const KamataEngine::Vector3 &position, const KamataEngine::Vector2 &size, const KamataEngine::Vector4 &color, const KamataEngine::Vector2 &anchorPoint) {
	auto bar = Prefab::CreateHealthBar(name, textureName, size, color, anchorPoint);
	bar->SetTranslation(position);

	auto *result = bar.get();
	hudObjects_.push_back(std::move(bar));

	return result;
}


void GameScene::InitializeTutorial() {
	skipLabel_ = std::make_unique<GameObject>();
	skipLabel_->SetName("skipLabel");
	skipLabel_->SetTranslation(kSkipLabelPosition_);

	auto *text = skipLabel_->AddComponent<Text2D>();

	skipLabel_->Initialize();

	text->SetFontSize(kSkipLabelFontSize_);
	text->SetText("TAB : チュートリアルをスキップ");
}


void GameScene::UpdateTutorial() {
	if (!isTutorial_) {
		return;
	}

	// 円周まで来たらチュートリアルを終える
	if (const auto *mover = player_->GetComponent<RailMover>()) {
		if (mover->GetTrack() == circleTrack_) {
			EndTutorial();
			return;
		}
	}

	// スキップ
	if (state_ == GameState::Play && !DebugUi::IsCapturingKeyboard() && Input::GetInstance()->TriggerKey(kSkipKey_)) {
		MusicManager::GetInstance()->PlaySe("ui_click");
		SkipTutorial();
		return;
	}

	for (auto &object : tutorialTexts_) {
		object->Update();
	}

	skipLabel_->Update();
}


void GameScene::DrawTutorial() const {
	if (!isTutorial_) {
		return;
	}

	for (const auto &object : tutorialTexts_) {
		object->Draw(camera_, DrawPass::Text3D);
	}

	if (state_ == GameState::Play) {
		skipLabel_->Draw(camera_, DrawPass::Text2D);
	}
}


void GameScene::AddTutorialText(const Track *track, float t01, const std::string &text) {
	if (!track) {
		assert(false && "軌道がnullptrです");
		return;
	}

	const auto s = track->GetLength() * std::clamp(t01, 0.0f, 1.0f);
	const auto tangent = track->GetTangent(s);

	auto object = Prefab::CreateTutorialText(text);
	object->SetTranslation(track->GetPoint(s) + track->GetLaneOffset(s, 0.0f) + Vector3{0.0f, kTutorialTextHeight_, 0.0f});

	tutorialTexts_.push_back(std::move(object));
}


void GameScene::SkipTutorial() {
	auto *mover = player_->GetComponent<RailMover>();
	if (!mover || !circleTrack_) {
		return;
	}

	// 円周の中央レーンの先頭へ飛ばす
	mover->SetTrack(circleTrack_, 0.0f, kCircleLaneIndex_);

	// 明転
	fade_->SetColor(kSkipFadeColor_);
	fade_->Start({kSkipFadeColor_.x, kSkipFadeColor_.y, kSkipFadeColor_.z, 0.0f}, kSkipFadeDuration_);

	EndTutorial();
}


void GameScene::EndTutorial() {
	isTutorial_ = false;
	tutorialTexts_.clear();
}


void GameScene::UpdateGameCamera() {
	// ゲームオーバー中はカメラごと止める
	if (state_ == GameState::GameOver) {
		return;
	}

	followCamera_->Update();
}


void GameScene::RegisterObject(std::unique_ptr<GameObject> object) {
	for (auto *emitter : object->GetComponents<ParticleEmitter>()) {
		emitter->SetParticleManager(particleMgr_.get());
	}

	Prefab::PlayEnter(object.get());

	dynamicObjects_.push_back(std::move(object));
}