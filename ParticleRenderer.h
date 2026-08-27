#pragma once

#include "KamataEngine.h"


struct ParticleInstance {
	KamataEngine::Vector3 position;
	KamataEngine::Vector2 size;
	float roll;
	KamataEngine::Vector4 color;
	KamataEngine::Vector4 uv;
};

static constexpr size_t kMaxParticles = 4096;



class ParticleRenderer {
public: // メンバ関数

	void Initialize();

	void Draw(
		const KamataEngine::Camera &camera,
		const ParticleInstance *instance,
		size_t instanceCount,
		uint32_t textureHandle
	) const;


private: // インナークラス

	struct ConstBufferData {
		float viewProj[16];
		float cameraRight[3];
		float pad0;
		float cameraUp[3];
		float pad1;
	};


private: // メンバ変数

	Microsoft::WRL::ComPtr<ID3D12Resource> constBuffer_{};
	ConstBufferData *cbMap_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> instanceBuffer_{};
	ParticleInstance *instanceMap_ = nullptr;

	static inline Microsoft::WRL::ComPtr<ID3D12RootSignature> sRootSignature_{};
	static inline Microsoft::WRL::ComPtr<ID3D12PipelineState> sPipelineState_{};

	static inline const std::wstring kVsPath_ = L"Resources/EngineShaders/Particle.VS.hlsl";
	static inline const std::wstring kPsPath_ = L"Resources/EngineShaders/Particle.PS.hlsl";


private: // メンバ関数

	void CreatePSO();
};