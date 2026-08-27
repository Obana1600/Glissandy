#pragma once

#include "KamataEngine.h"
#include "ParticleRenderer.h"
#include <array>
#include <cstdint>
#include <numbers>
#include <random>
#include <vector>


struct Particle {
	KamataEngine::Vector3 position;
	KamataEngine::Vector3 velocity;
	KamataEngine::Vector3 acceleration;

	KamataEngine::Vector2 startSize;
	KamataEngine::Vector2 endSize;

	KamataEngine::Vector4 startColor;
	KamataEngine::Vector4 endColor;

	float roll;
	float rollSpeed;

	float age;
	float lifetime;

	KamataEngine::Vector4 uv = {0.0f, 0.0f, 1.0f, 1.0f};
};


enum class EmitShape {
	Point,
	Circle,
	Sphere,
	Cone,
	Box,
};


struct EmitParams {
	KamataEngine::Vector3 position{};

	KamataEngine::Vector3 direction = {0.0f, 1.0f, 0.0f};
	float angle = std::numbers::pi_v<float>;
	float speedMin = 1.0f;
	float speedMax = 5.0f;

	KamataEngine::Vector3 acceleration{};

	KamataEngine::Vector2 startSize = {1.0f, 1.0f};
	KamataEngine::Vector2 endSize = {0.0f, 0.0f};

	KamataEngine::Vector4 startColor = {1.0f, 1.0f, 1.0f, 1.0f};
	KamataEngine::Vector4 endColor = {1.0f, 1.0f, 1.0f, 0.0f};

	float lifetimeMin = 0.5f;
	float lifetimeMax = 1.0f;

	float rollSpeedMin = 0.0f;
	float rollSpeedMax = 5.0f;

	KamataEngine::Vector4 uv = {0.0f, 0.0f, 1.0f, 1.0f};

	EmitShape shape = EmitShape::Point;
	float radius = 1.0f;
	float radiusThickness = 1.0f;
	KamataEngine::Vector3 boxSize = {1.0f, 1.0f, 1.0f};
};



class ParticleManager {
public: // メンバ関数

	void Initialize();

	void Update();

	void Draw(const KamataEngine::Camera &camera) const;

	void Emit(const EmitParams &params, size_t count);


private: // メンバ変数

	std::array<ParticleInstance, kMaxParticles> instances_{};
	std::array<Particle, kMaxParticles> particles_{};
	size_t aliveCount_ = 0;

	ParticleRenderer renderer_;

	std::mt19937 rng_{std::random_device{}()};

	uint32_t textureHandle_ = 0;


private: // インナークラス

	struct ShapeBasis {
		KamataEngine::Vector3 axis;
		KamataEngine::Vector3 right;
		KamataEngine::Vector3 up;
	};


	struct ShapeSample {
		KamataEngine::Vector3 position;
		KamataEngine::Vector3 direction;
	};


private: // メンバ関数

	ShapeSample SampleShape(const EmitParams &params, const ShapeBasis &basis);

	ShapeBasis MakeBasis(const KamataEngine::Vector3 &direction) const;
	KamataEngine::Vector3 SampleBoxPosition(const ShapeBasis &basis, const KamataEngine::Vector3 &size);
	KamataEngine::Vector3 SampleConeDirection(const ShapeBasis &basis, float angle);
	KamataEngine::Vector3 SampleRadialDirection(const ShapeBasis &basis);
	float SampleCircleRadius(float radius, float thickness);
	float SampleSphereRadius(float radius, float thickness);

	float Random(float min, float max);
};