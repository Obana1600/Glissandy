#include "Prefab.h"

#include "BossController.h"
#include "Bullet.h"
#include "ChargeShooter.h"
#include "Collider.h"
#include "DamageFlash.h"
#include "Health.h"
#include "ModelManager.h"
#include "ModelRenderer.h"
#include "MusicManager.h"
#include "ParticleEmitter.h"
#include "PlayerController.h"
#include "RailMover.h"
#include "Reticle.h"
#include "SpriteRenderer.h"
#include "Text/Text2D.h"
#include "Text/Text3D.h"
#include "TextButton.h"
#include "Tween/TweenManager.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;


namespace {

	// lifecycle ----------------------------------------------------------------------------------

	// 登場/滞在/退場の演出パラメータ
	struct LifecycleParams {
		float dropHeight = 12.0f;
		float enterTime = 0.35f;
		float pulseScale = 1.12f;
		float pulseCycle = 0.7f;
		float exitTime = 0.3f;

		// 退場時に鳴らすSE
		const char *exitSe = nullptr;
	};

	// --------------------------------------------------------------------------------------------


	// player -------------------------------------------------------------------------------------

	constexpr auto maskPlayer = CollisionAttribute::Enemy | CollisionAttribute::EnemyBullet | CollisionAttribute::Obstacle;

	constexpr int maxHpPlayer = 100;

	constexpr Vector3 colliderSizePlayer = {2.0f, 2.0f, 2.0f};

	constexpr EmitParams particlePlayer{
		.speedMin = 0.5f,
		.speedMax = 2.0f,
		.startColor = {1.0f, 1.0f, 1.0f, 1.0f},
		.endColor = {0.4f, 0.9f, 1.0f, 0.0f},
		.lifetimeMin = 0.4f,
		.lifetimeMax = 0.8f,
		.shape = EmitShape::Sphere,
		.radius = 1.5f,
		.radiusThickness = 1.0f,
	};

	constexpr float particleRatePlayer = 30.0f;

	// --------------------------------------------------------------------------------------------


	// enemy --------------------------------------------------------------------------------------

	constexpr auto maskEnemy = CollisionAttribute::Player | CollisionAttribute::PlayerBullet | CollisionAttribute::Obstacle;

	constexpr int maxHpEnemy = 1000;

	constexpr int dealDamageEnemy = 1;

	constexpr Vector3 scaleEnemy = {2.0f, 2.0f, 2.0f};

	// 部位の名前
	constexpr const char *nameEnemyBody = "enemy_body";
	constexpr const char *nameEnemyLeft = "enemy_left";
	constexpr const char *nameEnemyRight = "enemy_right";

	// 部位の当たり判定
	constexpr Vector3 colliderSizeEnemyBody = {10.0f, 15.0f, 10.0f};
	constexpr Vector3 colliderSizeEnemyArm = {8.0f, 10.0f, 3.0f};

	constexpr Vector3 colliderOffsetEnemyBody = {0.0f, 0.0f, 0.0f};
	constexpr Vector3 colliderOffsetEnemyLeft = {10.0f, 0.0f, 0.0f};
	constexpr Vector3 colliderOffsetEnemyRight = {-10.0f, 0.0f, 0.0f};

	constexpr LifecycleParams lifecycleEnemy{
		.dropHeight = 30.0f,
		.enterTime = 1.2f,
		.pulseScale = 1.0f,
		.exitTime = 1.5f,
		.exitSe = "boss_death",
	};

	// --------------------------------------------------------------------------------------------


	// playerBullet -------------------------------------------------------------------------------

	constexpr auto maskPlayerBullet = CollisionAttribute::Enemy | CollisionAttribute::Obstacle;

	constexpr Vector3 colliderSizePlayerBullet = {2.0f, 5.5f, 2.0f};
	constexpr Vector3 colliderOffsetPlayerBullet = {0.0f, 1.5f, 0.0f};

	constexpr LifecycleParams lifecyclePlayerBullet{
		.enterTime = 0.0f,
		.pulseScale = 1.0f,
		.exitTime = 0.12f,
	};

	constexpr EmitParams particlePlayerBullet{
		.speedMin = 0.5f,
		.speedMax = 30.0f,
		.startColor = {1.0f, 0.4f, 0.75f, 1.0f},
		.endColor = {0.6f, 1.0f, 0.3f, 0.0f},
		.lifetimeMin = 0.25f,
		.lifetimeMax = 1.0f,
		.rollSpeedMin = 0.0f,
		.rollSpeedMax = 50.0f,
		.shape = EmitShape::Sphere,
		.radius = 1.5f,
		.radiusThickness = 1.0f,
	};

	constexpr float particleRatePlayerBullet = 60.0f;

	constexpr int particleBurstPlayerBullet = 100;

	// --------------------------------------------------------------------------------------------


	// rock ---------------------------------------------------------------------------------------

	constexpr auto maskRock = CollisionAttribute::Player | CollisionAttribute::Enemy | CollisionAttribute::PlayerBullet;

	constexpr int maxHpRock = 1;

	constexpr Vector3 colliderSizeRock = {2.0f, 6.0f, 2.0f};

	constexpr Vector3 scaleRock = {2.0f, 2.0f, 2.0f};

	constexpr int dealDamageRockToPlayer = 10;
	constexpr int dealDamageRockToEnemy = 100;

	constexpr LifecycleParams lifecycleRock{
		.dropHeight = 12.0f,
		.enterTime = 0.35f,
		.pulseScale = 1.12f,
		.pulseCycle = 0.7f,
		.exitTime = 0.25f,
		.exitSe = "rock_break",
	};

	constexpr EmitParams particleRockAmbient{
		.direction = {0.0f, 1.0f, 0.0f},
		.angle = 0.52f, // 約30度
		.speedMin = 1.0f,
		.speedMax = 3.0f,
		.acceleration = {0.0f, 15.0f, 0.0f},
		.startColor = {1.0f, 0.25f, 0.15f, 1.0f},
		.endColor = {0.0f, 0.0f, 0.0f, 0.0f},
		.lifetimeMin = 0.6f,
		.lifetimeMax = 1.2f,
		.shape = EmitShape::Cone,
		.radius = 2.0f,
		.radiusThickness = 1.0f,
	};

	constexpr Vector3 particleOffsetRockAmbient = {0.0f, -1.0f, 0.0f};

	constexpr float particleRateRockAmbient = 10.0f;

	constexpr EmitParams particleRockBurst{
		.speedMin = 5.0f,
		.speedMax = 12.0f,
		.acceleration = {0.0f, -12.0f, 0.0f},
		.startColor = {1.0f, 0.25f, 0.15f, 1.0f},
		.endColor = {0.0f, 0.0f, 0.0f, 0.0f},
		.lifetimeMin = 0.5f,
		.lifetimeMax = 1.0f,
		.shape = EmitShape::Sphere,
		.radius = 1.5f,
		.radiusThickness = 1.0f,
	};

	constexpr int particleBurstRock = 40;

	// --------------------------------------------------------------------------------------------


	// tutorialText -------------------------------------------------------------------------------

	constexpr Vector2 rectSizeTutorialText = {24.0f, 6.0f};

	// --------------------------------------------------------------------------------------------


	// titleLogo ----------------------------------------------------------------------------------

	// 元画像は2438x872
	constexpr Vector2 sizeTitleLogo = {900.0f, 322.0f};

	constexpr const char *textureTitleLogo = "Glissandy.png";

	// --------------------------------------------------------------------------------------------


	// skydome ------------------------------------------------------------------------------------

	constexpr float scaleSkydome = 2.0f;

	// --------------------------------------------------------------------------------------------


	// lifecycle ----------------------------------------------------------------------------------

	template <typename F>
	void ForEachInHierarchy(GameObject *obj, const F &func) {
		func(obj);

		for (const auto &child : obj->GetChildren()) {
			ForEachInHierarchy(child.get(), func);
		}
	}


	// 階層全体の当たり判定を切り替える
	void SetHierarchyColliderEnabled(GameObject *obj, bool enabled) {
		ForEachInHierarchy(obj, [enabled](GameObject *node) {
			for (auto *collider : node->GetComponents<Collider>()) {
				collider->SetEnabled(enabled);
			}
		});
	}


	void PlayEnter(GameObject *obj, const LifecycleParams &params) {
		auto *mover = obj->GetComponent<RailMover>();

		// 滞在中の鼓動
		auto startPulse = [obj, params] {
			if (params.pulseScale == 1.0f) {
				return;
			}

			TweenManager::To(obj, &GameObject::GetScale, &GameObject::SetScale, obj->GetScale() * params.pulseScale, params.pulseCycle)
				.SetLoops(-1, LoopType::Yoyo)
				.SetEase(EaseType::InOutSine);
		};

		if (params.enterTime <= 0.0f) {
			startPulse();
			return;
		}

		// 着地点
		const auto ground = mover ? mover->GetPointOnRail()
								  : obj->GetTranslation();

		// 当たり判定とレール追従を止める
		SetHierarchyColliderEnabled(obj, false);
		if (mover) {
			mover->SetEnabled(false);
		}

		obj->SetTranslation(ground + Vector3{0.0f, params.dropHeight, 0.0f});

		TweenManager::To(obj, &GameObject::GetTranslation, &GameObject::SetTranslation, ground, params.enterTime)
			.SetEase(EaseType::OutCubic)
			.OnComplete([obj, mover, startPulse] {
				SetHierarchyColliderEnabled(obj, true);
				if (mover) {
					mover->SetEnabled(true);
				}
				startPulse();
			});
	}


	void PlayExit(GameObject *obj, const LifecycleParams &params) {
		if (params.exitSe) {
			MusicManager::GetInstance()->PlaySe(params.exitSe);
		}

		std::vector<ModelRenderer *> renderers;

		ForEachInHierarchy(obj, [&renderers](GameObject *node) {
			// tweenを止める
			TweenManager::Kill(node);

			// colliderを止める
			for (auto *collider : node->GetComponents<Collider>()) {
				collider->SetEnabled(false);
			}

			// 死亡パーティクル
			for (auto *emitter : node->GetComponents<ParticleEmitter>()) {
				emitter->Burst();
				emitter->Stop();
			}

			// damageFlashを止める
			if (auto *flash = node->GetComponent<DamageFlash>()) {
				flash->SetEnabled(false);
			}

			for (auto *renderer : node->GetComponents<ModelRenderer>()) {
				renderers.push_back(renderer);
			}
		});

		if (params.exitTime <= 0.0f || renderers.empty()) {
			obj->SetIsDead(true);
			return;
		}

		// 部位ごとに黒にする
		for (size_t i = 0; i < renderers.size(); i++) {
			auto &tween = TweenManager::To(renderers[i], &ModelRenderer::GetColor, &ModelRenderer::SetColor, Vector4{0.0f, 0.0f, 0.0f, 1.0f}, params.exitTime)
							  .SetEase(EaseType::InCubic);

			if (i == 0) {
				tween.OnComplete([obj] {
					obj->SetIsDead(true);
				});
			}
		}
	}


	// 退場
	void SetupLifecycle(GameObject *obj, const LifecycleParams &params) {
		if (auto *health = obj->GetComponent<Health>()) {
			health->SetAutoDestroy(false);
			health->SetOnDeath([obj, params] {
				PlayExit(obj, params);
			});
		}
	}

	// --------------------------------------------------------------------------------------------


	// enemy --------------------------------------------------------------------------------------

	// ボスの部位
	std::unique_ptr<GameObject> CreateEnemyPart(const std::string &name, const Vector3 &colliderSize, const Vector3 &colliderOffset) {
		auto part = std::make_unique<GameObject>();
		part->SetName(name);

		// modelRenderer
		auto *modelRenderer = part->AddComponent<ModelRenderer>();
		modelRenderer->SetModel(ModelManager::GetInstance()->Load(name));

		// damageFlash
		auto *flash = part->AddComponent<DamageFlash>();

		// collider
		auto *collider = part->AddComponent<Collider>();
		collider->SetColliderSize(colliderSize);
		collider->SetOffset(colliderOffset);
		collider->SetAttribute(CollisionAttribute::Enemy);
		collider->SetMask(maskEnemy);
		collider->SetOnCollisionEnter([flash](Collider *other) {
			// 当たった部位だけ光る
			flash->Flash();

			// 相手にダメージ
			if (auto *otherHealth = other->GetOwner()->GetComponentInParent<Health>()) {
				otherHealth->TakeDamage(dealDamageEnemy);
			}
		});

		return part;
	}

	// --------------------------------------------------------------------------------------------

} // namespace



std::unique_ptr<GameObject> Prefab::CreatePlayer() {
	auto result = std::make_unique<GameObject>();
	result->SetName("player");

	// playerController
	result->AddComponent<PlayerController>();

	// chargeShooter
	result->AddComponent<ChargeShooter>();

	// reticle
	result->AddComponent<Reticle>();

	// railMover
	result->AddComponent<RailMover>();

	// modelRenderer
	auto *modelRenderer = result->AddComponent<ModelRenderer>();
	modelRenderer->SetModel(ModelManager::GetInstance()->Load("player"));

	// particle
	auto *particle = result->AddComponent<ParticleEmitter>();
	particle->SetParams(particlePlayer);
	particle->SetRatePerSecond(particleRatePlayer);
	particle->SetBurstCount(0);
	particle->Play();

	// health
	auto *health = result->AddComponent<Health>();
	health->SetMaxHp(maxHpPlayer, true);

	// damageFlash
	auto *flash = result->AddComponent<DamageFlash>();

	// collider
	auto *collider = result->AddComponent<Collider>();
	collider->SetColliderSize(colliderSizePlayer);
	collider->SetAttribute(CollisionAttribute::Player);
	collider->SetMask(maskPlayer);
	collider->SetOnCollisionEnter([flash](Collider *other) {
		(void)other;
		flash->Flash();

		MusicManager::GetInstance()->PlaySe("damage_player");
	});

	result->Initialize();
	return result;
}


std::unique_ptr<GameObject> Prefab::CreateEnemy() {
	auto result = std::make_unique<GameObject>();
	result->SetName("enemy");

	// railMover
	result->AddComponent<RailMover>();

	// health
	auto *health = result->AddComponent<Health>();
	health->SetMaxHp(maxHpEnemy, true);

	// bossController
	result->AddComponent<BossController>();

	// 部位
	result->AddChild(CreateEnemyPart(nameEnemyBody, colliderSizeEnemyBody, colliderOffsetEnemyBody));
	result->AddChild(CreateEnemyPart(nameEnemyLeft, colliderSizeEnemyArm, colliderOffsetEnemyLeft));
	result->AddChild(CreateEnemyPart(nameEnemyRight, colliderSizeEnemyArm, colliderOffsetEnemyRight));

	result->SetScale(scaleEnemy);

	result->Initialize();
	SetupLifecycle(result.get(), lifecycleEnemy);

	return result;
}


std::unique_ptr<GameObject> Prefab::CreatePlayerBullet() {
	auto result = std::make_unique<GameObject>();
	result->SetName("playerBullet");

	// modelRenderer
	auto *modelRenderer = result->AddComponent<ModelRenderer>();
	modelRenderer->SetModel(ModelManager::GetInstance()->Load("bullet"));

	// bullet
	auto *bullet = result->AddComponent<Bullet>();

	// particle
	auto *particle = result->AddComponent<ParticleEmitter>();
	particle->SetParams(particlePlayerBullet);
	particle->SetRatePerSecond(particleRatePlayerBullet);
	particle->SetBurstCount(particleBurstPlayerBullet);
	particle->Play();

	// collider
	auto *collider = result->AddComponent<Collider>();
	collider->SetColliderSize(colliderSizePlayerBullet);
	collider->SetOffset(colliderOffsetPlayerBullet);
	collider->SetAttribute(CollisionAttribute::PlayerBullet);
	collider->SetMask(maskPlayerBullet);
	collider->SetOnCollisionEnter([bullet](Collider *other) {
		MusicManager::GetInstance()->PlaySe("hit_enemy");

		// 相手にダメージ
		if (auto *otherHealth = other->GetOwner()->GetComponentInParent<Health>()) {
			otherHealth->TakeDamage(bullet->GetDamage());
		}

		// 自滅する
		PlayExit(bullet->GetOwner(), lifecyclePlayerBullet);
	});

	result->Initialize();

	// 弾の退場時コールバック
	bullet->SetOnExpire([obj = result.get()] {
		PlayExit(obj, lifecyclePlayerBullet);
	});

	return result;
}


std::unique_ptr<GameObject> Prefab::CreateRock() {
	auto result = std::make_unique<GameObject>();
	result->SetName("rock");

	// railMover
	result->AddComponent<RailMover>();

	// modelRenderer
	auto *modelRenderer = result->AddComponent<ModelRenderer>();
	modelRenderer->SetModel(ModelManager::GetInstance()->Load("rest"));

	// health
	auto *health = result->AddComponent<Health>();
	health->SetMaxHp(maxHpRock, true);

	// particle
	auto *particleAmbient = result->AddComponent<ParticleEmitter>();
	particleAmbient->SetName("ParticleEmitter (Ambient)");
	particleAmbient->SetParams(particleRockAmbient);
	particleAmbient->SetOffset(particleOffsetRockAmbient);
	particleAmbient->SetRatePerSecond(particleRateRockAmbient);
	particleAmbient->SetBurstCount(0);
	particleAmbient->Play();

	auto *particleBurst = result->AddComponent<ParticleEmitter>();
	particleBurst->SetName("ParticleEmitter (Burst)");
	particleBurst->SetParams(particleRockBurst);
	particleBurst->SetBurstCount(particleBurstRock);

	// damageFlash
	auto *flash = result->AddComponent<DamageFlash>();

	// collider
	auto *collider = result->AddComponent<Collider>();
	collider->SetColliderSize(colliderSizeRock);
	collider->SetAttribute(CollisionAttribute::Obstacle);
	collider->SetMask(maskRock);
	collider->SetOnCollisionEnter([health, flash](Collider *other) {
		flash->Flash();

		// プレイヤーの弾と接触時
		if (Any(other->GetAttribute() & CollisionAttribute::PlayerBullet)) {
			health->TakeDamage(health->GetHp());
			return;
		}

		// 相手にダメージ
		if (auto *otherHealth = other->GetOwner()->GetComponentInParent<Health>()) {
			auto damage = Any(other->GetAttribute() & CollisionAttribute::Enemy)
							  ? dealDamageRockToEnemy
							  : dealDamageRockToPlayer;
			otherHealth->TakeDamage(damage);
		}

		// 自壊する
		health->TakeDamage(health->GetHp());
	});

	result->SetScale(scaleRock);

	result->Initialize();
	SetupLifecycle(result.get(), lifecycleRock);

	return result;
}




std::unique_ptr<GameObject> Prefab::CreateTutorialText(const std::string &text) {
	auto result = std::make_unique<GameObject>();
	result->SetName("tutorialText");

	// text3D
	auto *text3D = result->AddComponent<Text3D>();

	result->Initialize();

	// initの後に呼ぶもの
	text3D->SetRectSize(rectSizeTutorialText);
	text3D->SetText(text);

	return result;
}




std::unique_ptr<GameObject> Prefab::CreateSkydome() {
	auto result = std::make_unique<GameObject>();
	result->SetName("skydome");

	// modelRenderer
	auto *modelRenderer = result->AddComponent<ModelRenderer>();
	modelRenderer->SetModel(ModelManager::GetInstance()->Load("skydome"));

	result->SetScale({scaleSkydome, scaleSkydome, scaleSkydome});

	result->Initialize();
	return result;
}


std::unique_ptr<GameObject> Prefab::CreateTitleLogo() {
	auto result = std::make_unique<GameObject>();
	result->SetName("titleLogo");

	// spriteRenderer
	auto *spriteRenderer = result->AddComponent<SpriteRenderer>();
	spriteRenderer->SetTextureName(textureTitleLogo);
	spriteRenderer->SetSize(sizeTitleLogo);

	result->Initialize();
	return result;
}


std::unique_ptr<GameObject> Prefab::CreateTextButton(const std::string &label, const KamataEngine::Vector2 &rectSize, std::function<void()> onClick) {
	auto result = std::make_unique<GameObject>();
	result->SetName(label);

	// text2D
	auto *text = result->AddComponent<Text2D>();

	// textButton
	auto *button = result->AddComponent<TextButton>();

	result->Initialize();

	// initの後に呼ぶもの
	text->SetRectSize(rectSize);
	text->SetText(label);
	button->SetOnClick(std::move(onClick));

	return result;
}


std::unique_ptr<GameObject> Prefab::CreateHealthBar(const std::string &name, const std::string &textureName, const KamataEngine::Vector2 &size, const KamataEngine::Vector4 &color, const KamataEngine::Vector2 &anchorPoint) {
	auto result = std::make_unique<GameObject>();
	result->SetName(name);

	// spriteRenderer
	auto *spriteRenderer = result->AddComponent<SpriteRenderer>();
	spriteRenderer->SetTextureName(textureName);
	spriteRenderer->SetSize(size);
	spriteRenderer->SetColor(color);
	spriteRenderer->SetAnchorPoint(anchorPoint);

	result->Initialize();
	return result;
}


void Prefab::PlayEnter(GameObject *object) {
	if (!object) {
		assert(false && "オブジェクトがnullptrです");
		return;
	}

	const auto &name = object->GetName();

	if (name == "enemy") {
		PlayEnter(object, lifecycleEnemy);

	} else if (name == "rock") {
		PlayEnter(object, lifecycleRock);

	} else if (name == "playerBullet") {
		PlayEnter(object, lifecyclePlayerBullet);
	}
}