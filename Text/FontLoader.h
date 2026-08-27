#pragma once

#include <freetype/ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H

#include "KamataEngine.h"
#include <cstdint>
#include <d3d12.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <wrl.h>


struct GlyphInfo {
	KamataEngine::Vector2 atlasPos{};
	KamataEngine::Vector2 atlasSize{};
	KamataEngine::Vector2 bearing{};
	float advanceX{};
};


class FontLoader {
public: // メンバ関数

	static FontLoader *GetInstance();

	void Initialize();

	void Finalize();

	bool Load(const std::string &filePath);

	void FlushUploads();

	void FlushToDisk();

	const GlyphInfo &GetGlyph(const std::string &filePath, wchar_t character);

	float GetLineHeight(const std::string &filePath) const;

	const D3D12_GPU_DESCRIPTOR_HANDLE &GetGPUHandle(const std::string &fontPath) const;


	ID3D12DescriptorHeap *GetSRVHeap() const {
		return srvHeap_.Get();
	}

	const std::vector<std::string> &GetLoadedFonts() const {
		return loadedFonts_;
	}

	float GetAtlasFontSIze() const {
		return static_cast<float>(kAtlasFontSize_);
	}

	int GetAtlasWidth() const {
		return kAtlasWidth_;
	}

	int GetAtlasHeight() const {
		return kAtlasHeight_;
	}


private: // インナークラス

	struct FontData {
		std::unordered_map<char32_t, GlyphInfo> glyphs;
		std::vector<uint32_t> atlasPixels;
		std::string atlasFile;
		std::string fontFile;
		int destX = 0;
		int destY = 0;
		int maxTop = 0;
		int maxBottom = 0;
		int cellHeight = 0;
		float lineHeight = 0.0f;
		char32_t fallbackChar = U'□';
		bool dirty = false;
		bool diskDirty = false;

		Microsoft::WRL::ComPtr<ID3D12Resource> atlasResource;
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE atlasSrv{};
	};


private: // メンバ変数

	std::vector<wchar_t> text_;
	FT_Library ft_ = nullptr;
	std::unordered_map<std::string, FT_Face> fonts_;
	std::unordered_map<std::string, FontData> fontData_;
	std::vector<std::string> loadedFonts_;

	ID3D12Device *device_ = nullptr;
	ID3D12GraphicsCommandList *commandList_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;
	uint32_t srvUsed_ = 0;
	uint32_t srvInc_ = 0;


private: // 入出力

	static inline const std::string kBasePath_ = "Resources/fonts/";
	static inline const std::string kResponseTextFile_ = "Resources/fonts/response.txt";
	static inline const int kJsonVersion_ = 2;


private: // アトラス

	static const uint32_t kMaxAtlases_ = 64;
	static const int kAtlasFontSize_ = 90;
	static const int kAtlasWidth_ = 4096;
	static const int kAtlasHeight_ = 4096;


private: // メンバ関数

	void LoadResponseText();
	FT_Face OpenFace(const std::string &factPath) const;
	std::string CheckFontPath(const std::string &filePath) const;
	FontData CreateFontAtlas(const std::string &factPath, const std::string &atlasFileName);
	void CreateBitMapTexture(const std::string &atlasPath, FontData &data);
	void SaveAtlasImage(const std::string &atlasPath, const FontData &data) const;

	// glyph
	bool BakeGlyph(FT_Face face, FontData &data, char32_t codepoint);
	bool AddGlyph(const std::string &atlasFileName, char32_t codepoint);
	void ReuploadAtlas(FontData &data);

	// json
	bool IsCacheValid(const std::string &atlasPath, const std::string &jsonPath, const std::string &fontPath) const;
	void WriteJson(const std::string &jsonPath, const FontData &data) const;
	bool ReadJson(const std::string &jsonPath, FontData &data) const;


private: // メンバ関数

	FontLoader() = default;
	~FontLoader() = default;
	FontLoader(const FontLoader &) = delete;
	FontLoader &operator=(const FontLoader &) = delete;
};