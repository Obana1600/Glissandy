#pragma once

#include "TextBase.h"


class TextRenderer final {
public: // メンバ関数

	void Initialize();

	void Draw(const KamataEngine::Matrix4x4 &wvp, const TextBase &text) const;

	static void DrawGizmo(
		const KamataEngine::Matrix4x4 &wvp,
		const KamataEngine::Vector2 &rectSize,
		const KamataEngine::Vector2 &pivot,
		bool showRect,
		bool showPivot
	);


private: // インナークラス

	struct TextCB {
		float wvp[16];

		float colorLT[4];
		float colorRT[4];
		float colorLB[4];
		float colorRB[4];

		float outlineColor[4];
		float shadowColor[4];
		float outlineWidth;
		float shadowBaseY;
		float shadowSoftness;
		float padding;
		float shadowOffset[2];
		float shadowScale[2];
		float shadowShear[2];
	};


private: // メンバ変数

	Microsoft::WRL::ComPtr<ID3D12Resource> instanceBuffer_;
	TextBase::GlyphInstance *instanceMap_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> constBuffer_;
	TextCB *cbMap_ = nullptr;

	static inline Microsoft::WRL::ComPtr<ID3D12RootSignature> sRootSignature_{};
	static inline Microsoft::WRL::ComPtr<ID3D12PipelineState> sPipelineState_{};

	static inline const std::wstring kVsPath = L"Resources/EngineShaders/Text.VS.hlsl";
	static inline const std::wstring kPsPath = L"Resources/EngineShaders/Text.PS.hlsl";

	static inline const size_t kMaxGlyphs_ = 512;


private: // メンバ関数

	static void CreatePSO();
};