#pragma once

#include "KamataEngine.h"
#include <array>
#include <string>
#include <vector>

class FontLoader;


class TextBase {
public: // インナークラス

	struct Outline {
		KamataEngine::Vector4 color = {0.0f, 0.0f, 0.0f, 1.0f};
		float width = 0.0f;
	};


	struct Shadow {
		KamataEngine::Vector4 color = {0.0f, 1.0f, 0.0f, 1.0f};
		KamataEngine::Vector2 offset = {0.0f, 0.0f};
		KamataEngine::Vector2 scale = {1.0f, 1.0f};
		KamataEngine::Vector2 shear = {0.0f, 0.0f};
		float softness = 0.5f;
	};


	struct GlyphInstance {
		float rect[4];
		float uv[4];
	};


public: // メンバ関数

	void Initialize();

	void UpdateImGui();

	const std::array<KamataEngine::Vector4, 4> GetCornerColors() const;


	const std::string &GetFontPath() const {
		return fontPath_;
	}

	const Outline GetOutline() const {
		return outline_;
	}

	bool UseOutline() const {
		return useOutline_;
	}

	const Shadow GetShadow() const {
		return shadow_;
	}

	bool UseShadow() const {
		return useShadow_;
	}

	float GetBaselineY() const {
		return baselineY_;
	}

	const std::vector<GlyphInstance> &GetInstances() const {
		return instances_;
	}


	void SetText(const std::string &text);

	void SetCornerColors(const std::array<KamataEngine::Vector4, 4> &cornerColor);

	void SetTint(const KamataEngine::Vector4 &tint);

	void SetRectSize(const KamataEngine::Vector2 &size);

	void SetFontSize(float size);

	void SetFontSizePerUnit(float v);


private: // インナークラス

	struct FontStyle {
		bool bold = false;
		bool italic = false;
		bool underLine = false;
		bool strileThrough = false;
	};

	enum class ColorMode {
		single,
		horizontalGradient,
		verticalGradient,
		fourCornerGradient,
	};

	struct ColorGradient {
		ColorMode colorMode = ColorMode::single;
		KamataEngine::Vector4 leftTop = {1.0f, 1.0f, 1.0f, 1.0f};
		KamataEngine::Vector4 leftBottom = {1.0f, 1.0f, 1.0f, 1.0f};
		KamataEngine::Vector4 rightTop = {1.0f, 1.0f, 1.0f, 1.0f};
		KamataEngine::Vector4 rightBottom = {1.0f, 1.0f, 1.0f, 1.0f};
	};

	struct Spacing {
		float character;
		float word;
		float line;
		float paragraph;
	};

	enum class HorizontalAlignment {
		left,
		center,
		right,
	};

	enum class VerticalAlignment {
		top,
		middle,
		bottom,
	};


private: // メンバ変数

	// font
	std::string text_ = "Text";
	std::string fontPath_ = "NotoSansJP-Medium.ttf";
	FontStyle fontStyle_{};

	// size
	float fontSize_ = 64.0;
	bool autoSize_ = false;
	float sizeMin_ = 18.0f;
	float sizeMax_ = 72.0f;

	// color
	KamataEngine::Vector4 baseColor_ = {1.0f, 1.0f, 1.0f, 1.0f};
	bool useGradient_ = false;
	ColorGradient colorGradient_{};
	KamataEngine::Vector4 tint_ = {0.05f, 0.05f, 0.05f, 1.0f};

	// outline
	Outline outline_{};
	bool useOutline_ = false;

	// shadow
	Shadow shadow_{};
	bool useShadow_ = true;

	// その他
	Spacing spacing_{};
	HorizontalAlignment horizontalAlignment_ = HorizontalAlignment::center;
	VerticalAlignment verticalAlignment_ = VerticalAlignment::middle;

	// レイアウト
	KamataEngine::Vector2 rectSize_{};
	float fontSizePerUnit_ = 1.0f;
	float baselineY_ = 0.0f;
	float lineHeight_ = 0.0f;
	std::vector<GlyphInstance> instances_;

	// imgui
	std::string inputFontFile_;

	static inline FontLoader *fontLoader_ = nullptr;


private: // メンバ関数

	void Rebuild();

	void SetFont(const std::string &fontPath);
};