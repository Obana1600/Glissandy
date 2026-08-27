#include "ParticleManager.h"

#include "Extension.h"
#include "Geometry.h"
#include "TimeT.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void ParticleManager::Initialize() {
	renderer_.Initialize();
	textureHandle_ = TextureManager::Load("white1x1.png");
}


void ParticleManager::Update() {
	auto dt = TimeT::GetDeltaTime();

	for (size_t i = 0; i < aliveCount_;) {
		auto &p = particles_[i];
		p.age += dt;

		// 死んだパーティクルを末尾を入れ替える
		if (p.age >= p.lifetime) {
			aliveCount_--;
			p = particles_[aliveCount_];
			continue;
		}

		// パーティクルの更新
		p.velocity += p.acceleration * dt;
		p.position += p.velocity * dt;
		p.roll += p.rollSpeed * dt;

		float t = p.age / p.lifetime;

		// インスタンスに流す
		auto &pi = instances_[i];
		pi.position = p.position;
		pi.size.x = Lerp(p.startSize.x, p.endSize.x, t);
		pi.size.y = Lerp(p.startSize.y, p.endSize.y, t);
		pi.color.x = Lerp(p.startColor.x, p.endColor.x, t);
		pi.color.y = Lerp(p.startColor.y, p.endColor.y, t);
		pi.color.z = Lerp(p.startColor.z, p.endColor.z, t);
		pi.color.w = Lerp(p.startColor.w, p.endColor.w, t);
		pi.roll = p.roll;
		pi.uv = p.uv;

		i++;
	}
}


void ParticleManager::Draw(const KamataEngine::Camera &camera) const {
	renderer_.Draw(camera, instances_.data(), static_cast<UINT>(aliveCount_), textureHandle_);
}


void ParticleManager::Emit(const EmitParams &params, size_t count) {
	const auto space = kMaxParticles - aliveCount_;
	count = (std::min)(count, space);

	const auto basis = MakeBasis(params.direction);

	for (size_t i = 0; i < count; i++) {
		auto &p = particles_[aliveCount_];
		const auto sample = SampleShape(params, basis);

		p.position = sample.position;
		p.velocity = sample.direction * Random(params.speedMin, params.speedMax);
		p.acceleration = params.acceleration;

		p.startSize = params.startSize;
		p.endSize = params.endSize;

		p.startColor = params.startColor;
		p.endColor = params.endColor;

		p.roll = Random(0.0f, 2.0f * std::numbers::pi_v<float>);
		p.rollSpeed = Random(params.rollSpeedMin, params.rollSpeedMax);

		p.age = 0.0f;
		p.lifetime = (std::max)(Random(params.lifetimeMin, params.lifetimeMax), 1e-4f);

		p.uv = params.uv;

		aliveCount_++;
	}
}


ParticleManager::ShapeSample ParticleManager::SampleShape(const EmitParams &params, const ShapeBasis &basis) {
	ShapeSample result{};

	switch (params.shape) {
		case EmitShape::Point:
			result.position = params.position;
			result.direction = SampleConeDirection(basis, params.angle);

			break;

		case EmitShape::Circle:
			result.direction = SampleRadialDirection(basis);
			result.position = params.position + result.direction * SampleCircleRadius(params.radius, params.radiusThickness);

			break;

		case EmitShape::Sphere:
			result.direction = SampleConeDirection(basis, std::numbers::pi_v<float>);
			result.position = params.position + result.direction * SampleSphereRadius(params.radius, params.radiusThickness);

			break;

		case EmitShape::Cone:
			{
				const auto radial = SampleRadialDirection(basis);
				result.position = params.position + radial * SampleCircleRadius(params.radius, params.radiusThickness);
				result.direction = radial * std::sin(params.angle) + basis.axis * std::cos(params.angle);

				break;
			}

		case EmitShape::Box:
			result.position = params.position + SampleBoxPosition(basis, params.boxSize);
			result.direction = basis.axis;

			break;
	}

	return result;
}


ParticleManager::ShapeBasis ParticleManager::MakeBasis(const KamataEngine::Vector3 &direction) const {
	ShapeBasis result{};

	result.axis = Normalize(direction);
	result.up = Normalize(Perpendicular(result.axis));
	result.right = Cross(result.axis, result.up);

	return result;
}


KamataEngine::Vector3 ParticleManager::SampleBoxPosition(const ShapeBasis &basis, const KamataEngine::Vector3 &size) {
	auto xOffset = basis.right * (Random(-0.5f, 0.5f) * size.x);
	auto yOffset = basis.up * (Random(-0.5f, 0.5f) * size.y);
	auto zOffset = basis.axis * (Random(-0.5f, 0.5f) * size.z);

	return xOffset + yOffset + zOffset;
}


KamataEngine::Vector3 ParticleManager::SampleConeDirection(const ShapeBasis &basis, float angle) {
	auto axisWeight = Lerp(std::cos(angle), 1.0f, Random(0.0f, 1.0f));
	auto radialWeight = std::sqrt((std::max)(0.0f, 1.0f - axisWeight * axisWeight));
	auto phi = Random(0.0f, 2.0f * std::numbers::pi_v<float>);

	return (basis.right * radialWeight * std::cos(phi)) + (basis.up * radialWeight * std::sin(phi)) + (basis.axis * axisWeight);
}


KamataEngine::Vector3 ParticleManager::SampleRadialDirection(const ShapeBasis &basis) {
	float phi = Random(0.0f, 2.0f * std::numbers::pi_v<float>);
	return basis.right * std::cos(phi) + basis.up * std::sin(phi);
}


float ParticleManager::SampleCircleRadius(float radius, float thickness) {
	auto outer = radius;
	auto inner = radius * (1.0f - thickness);
	auto t = Random(0.0f, 1.0f);

	return std::sqrt(Lerp(inner * inner, outer * outer, t));
}


float ParticleManager::SampleSphereRadius(float radius, float thickness) {
	auto outer = radius;
	auto inner = radius * (1.0f - thickness);
	auto t = Random(0.0f, 1.0f);

	return std::cbrt(Lerp(inner * inner * inner, outer * outer * outer, t));
}


float ParticleManager::Random(float min, float max) {
	assert(min <= max);

	std::uniform_real_distribution<float> dist(min, max);
	return dist(rng_);
}