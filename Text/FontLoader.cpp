#include "FontLoader.h"

#include "ConvertString.h"
#include <DirectXTex.h>
#include <Windows.h>
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <json.hpp>

using namespace KamataEngine;



FontLoader *FontLoader::GetInstance() {
	static FontLoader instance;
	return &instance;
}


void FontLoader::Initialize() {
	// デバイス取得
	device_ = DirectXCommon::GetInstance()->GetDevice();
	commandList_ = DirectXCommon::GetInstance()->GetCommandList();

	// SRVヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = kMaxAtlases_;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HRESULT hr = device_->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&srvHeap_));
	assert(SUCCEEDED(hr));
	(void)hr;
	srvInc_ = device_->GetDescriptorHandleIncrementSize(heapDesc.Type);
	srvUsed_ = 0;

	// freetype初期化
	FT_Error err = FT_Init_FreeType(&ft_);
	if (err) {
		assert(false);
		return;
	}

	// 固定文字読み込み
	LoadResponseText();

	// すでにLoadしたフォントをキャッシュ
	for (auto &it : std::filesystem::directory_iterator(kBasePath_)) {
		auto name = it.path().filename().string();
		std::string suffix = "_" + std::to_string(kAtlasFontSize_) + ".png.json";
		if (!name.ends_with(suffix)) {
			continue;
		}

		std::string fontFile = name.substr(0, name.size() - suffix.size());
		std::string fontPath = kBasePath_ + fontFile;

		Load(fontPath);
	}
}


void FontLoader::Finalize() {
	FlushToDisk();

	if (ft_) {
		// faceの解放
		for (auto &[key, face] : fonts_) {
			if (face) {
				FT_Done_Face(face);
			}
		}
		fonts_.clear();

		// libraryの解放
		FT_Done_FreeType(ft_);
		ft_ = nullptr;
	}

	// リソースの解放
	fontData_.clear();
	srvHeap_.Reset();
}


bool FontLoader::Load(const std::string &filePath) {
	std::filesystem::path factPath = CheckFontPath(filePath);
	const auto hasFontFile = std::filesystem::is_regular_file(factPath);

	// すでにアトラスがあるならtrue
	std::string atlasName = factPath.filename().string() + "_" + std::to_string(kAtlasFontSize_);
	if (fontData_.find(atlasName) != fontData_.end()) {
		return true;
	}

	// 各ファイルパス
	std::string atlasPath = kBasePath_ + atlasName + ".png";
	std::string jsonPath = atlasPath + ".json";

	FontData data{};
	bool needWrite = false;

	// responseより新しいキャッシュがあれば再生成をスキップ
	if (IsCacheValid(atlasPath, jsonPath, factPath.string()) && ReadJson(jsonPath, data)) {
		if (hasFontFile) {
			fonts_[atlasName] = OpenFace(factPath.string());
		}

	} else if (hasFontFile) {
		data = CreateFontAtlas(factPath.string(), atlasName);
		needWrite = true;
	} else {
		false;
	}

	data.atlasFile = atlasPath;
	data.fontFile = factPath.filename().string();
	if (needWrite) {
		WriteJson(jsonPath, data);
	}

	CreateBitMapTexture(atlasPath, data);
	fontData_[atlasName] = std::move(data);
	loadedFonts_.push_back(factPath.string());

	return true;
}


void FontLoader::FlushUploads() {
	for (auto &[key, data] : fontData_) {
		if (data.dirty) {
			ReuploadAtlas(data);
			data.dirty = false;
		}
	}
}


void FontLoader::FlushToDisk() {
	for (auto &[key, data] : fontData_) {
		if (!data.diskDirty) {
			continue;
		}

		SaveAtlasImage(data.atlasFile, data);
		WriteJson(data.atlasFile + ".json", data);

		data.diskDirty = false;
	}
}


const GlyphInfo &FontLoader::GetGlyph(const std::string &filePath, wchar_t character) {
	static const GlyphInfo empty{};

	// ファイルを開く
	std::filesystem::path factPath(CheckFontPath(filePath));
	std::string atlasName = factPath.filename().string() + "_" + std::to_string(kAtlasFontSize_);

	// フォント検索
	auto it = fontData_.find(atlasName);
	if (it == fontData_.end()) {
		return empty;
	}
	FontData &data = it->second;

	// 文字検索
	auto gi = data.glyphs.find(static_cast<char32_t>(character));
	if (gi != data.glyphs.end()) {
		return gi->second;
	}

	// 追加
	if (AddGlyph(atlasName, static_cast<char32_t>(character))) {
		return data.glyphs[static_cast<char32_t>(character)];
	}

	// フォールバック
	auto fallback = data.glyphs.find(data.fallbackChar);
	if (fallback != data.glyphs.end()) {
		return fallback->second;
	}

	return empty;
}


float FontLoader::GetLineHeight(const std::string &filePath) const {
	// ファイルを開く
	std::filesystem::path factPath(CheckFontPath(filePath));
	std::string atlasName = factPath.filename().string() + "_" + std::to_string(kAtlasFontSize_);

	auto it = fontData_.find(atlasName);
	if (it == fontData_.end() || it->second.lineHeight <= 0.0f) {
		return static_cast<float>(kAtlasFontSize_);
	}
	return it->second.lineHeight;
}


const D3D12_GPU_DESCRIPTOR_HANDLE &FontLoader::GetGPUHandle(const std::string &fontPath) const {
	// ファイルを開く
	std::filesystem::path factPath(CheckFontPath(fontPath));
	std::string atlasName = factPath.filename().string() + "_" + std::to_string(kAtlasFontSize_);

	auto it = fontData_.find(atlasName);
	assert(it != fontData_.end());
	return it->second.atlasSrv;
}


void FontLoader::LoadResponseText() {
	// ファイルを開く
	std::ifstream file(kResponseTextFile_);
	if (!file.is_open()) {
		assert(false && "Failed to open response.txt");
		return;
	}

	// ファイル全体を読み込む
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	std::erase(content, '\n');
	std::erase(content, '\r');
	std::erase(content, '\t');
	file.close();

	// ワイド文字に変換し格納する
	std::wstring wide = ConvertString(content);
	text_.assign(wide.begin(), wide.end());

	// 重複削除
	std::sort(text_.begin(), text_.end());
	text_.erase(std::unique(text_.begin(), text_.end()), text_.end());
}


FT_Face FontLoader::OpenFace(const std::string &factPath) const {
	FT_Face face = nullptr;
	FT_New_Face(ft_, factPath.c_str(), 0, &face);
	FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(kAtlasFontSize_));

	int spread = 8;
	FT_Property_Set(ft_, "sdf", "spread", &spread);
	FT_Property_Set(ft_, "bsdf", "spread", &spread);

	return face;
}


std::string FontLoader::CheckFontPath(const std::string &filePath) const {
	if (filePath.starts_with(kBasePath_) || filePath.starts_with("Resources/")) {
		return filePath;
	}

	return kBasePath_ + filePath;
}


FontLoader::FontData FontLoader::CreateFontAtlas(const std::string &factPath, const std::string &atlasFileName) {
	// フォントを開く
	auto face = OpenFace(factPath);

	// フォントデータ基本情報
	FontData data{};
	data.lineHeight = face->size->metrics.height / 64.0f;
	data.fallbackChar = U'□';

	// RGBAアトラスバッファ
	data.atlasPixels.resize(kAtlasWidth_ * kAtlasHeight_, 0);

	// グリフを読み込む
	auto loadGlyph = [&](uint32_t gid) {
		auto err = FT_Load_Glyph(face, gid, FT_LOAD_DEFAULT);
		assert(!err);
		err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		assert(!err);
		err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF);
		assert(!err);
	};

	// 文字の最大の高さを取得
	auto measure = [&](uint32_t gid) {
		loadGlyph(gid);
		auto &glyph = face->glyph;

		data.maxTop = (std::max)(data.maxTop, glyph->bitmap_top);
		data.maxBottom = (std::max)(data.maxBottom, static_cast<int>(glyph->bitmap.rows - glyph->bitmap_top));
	};

	for (auto &c : text_) {
		measure(FT_Get_Char_Index(face, c));
	}
	measure(FT_Get_Char_Index(face, data.fallbackChar));
	data.cellHeight = data.maxTop + data.maxBottom;

	// 文字を配置する位置
	for (auto &c : text_) {
		BakeGlyph(face, data, c);
	}
	BakeGlyph(face, data, data.fallbackChar);

	fonts_[atlasFileName] = face;

	SaveAtlasImage(kBasePath_ + atlasFileName + ".png", data);

	return data;
}


void FontLoader::CreateBitMapTexture(const std::string &atlasPath, FontData &data) {
	// ピクセルデータを読む
	auto wpath = ConvertString(atlasPath);
	DirectX::ScratchImage atlasImage;
	HRESULT hr = DirectX::LoadFromWICFile(wpath.c_str(), DirectX::WIC_FLAGS::WIC_FLAGS_FORCE_RGB, nullptr, atlasImage);
	assert(SUCCEEDED(hr));
	const auto &meta = atlasImage.GetMetadata();

	// GPUテクスチャを作る
	CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
	auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(meta.format, (UINT)meta.width, (UINT)meta.height, 1, (UINT16)meta.mipLevels);
	hr = device_->CreateCommittedResource(
		&defaultHeap, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&data.atlasResource)
	);

	// 中間リソースを作る
	CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
	auto uploadSize = GetRequiredIntermediateSize(data.atlasResource.Get(), 0, 1);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);
	device_->CreateCommittedResource(
		&uploadHeap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&data.uploadBuffer)
	);

	//
	auto cpuHandle = srvHeap_->GetCPUDescriptorHandleForHeapStart();
	cpuHandle.ptr += srvUsed_ * srvInc_;
	auto gpuHandle = srvHeap_->GetGPUDescriptorHandleForHeapStart();
	gpuHandle.ptr += srvUsed_ * srvInc_;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = meta.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = (UINT)meta.mipLevels;
	device_->CreateShaderResourceView(data.atlasResource.Get(), &srvDesc, cpuHandle);
	data.atlasSrv = gpuHandle;
	srvUsed_++;

	// 転送
	auto *image = atlasImage.GetImage(0, 0, 0);
	if (data.atlasPixels.empty()) {
		data.atlasPixels.resize(kAtlasWidth_ * kAtlasHeight_);
		size_t rowBytes = kAtlasWidth_ * sizeof(uint32_t);
		for (size_t y = 0; y < kAtlasHeight_; y++) {
			std::memcpy(
				reinterpret_cast<uint8_t *>(data.atlasPixels.data()) + y * rowBytes,
				image->pixels + y * image->rowPitch,
				rowBytes
			);
		}
	}
	D3D12_SUBRESOURCE_DATA sub{};
	sub.pData = image->pixels;
	sub.RowPitch = (LONG_PTR)image->rowPitch;
	sub.SlicePitch = (LONG_PTR)image->slicePitch;
	UpdateSubresources(commandList_, data.atlasResource.Get(), data.uploadBuffer.Get(), 0, 0, 1, &sub);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		data.atlasResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	commandList_->ResourceBarrier(1, &barrier);
}


void FontLoader::SaveAtlasImage(const std::string &atlasPath, const FontData &data) const {
	// atlasからscratchImageを作成する
	DirectX::ScratchImage scratchImage;
	HRESULT hr = scratchImage.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, kAtlasWidth_, kAtlasHeight_, 1, 1);
	assert(SUCCEEDED(hr));

	auto image = scratchImage.GetImage(0, 0, 0);

	// 行ごとにコピー
	auto srcData = reinterpret_cast<const uint8_t *>(data.atlasPixels.data());
	auto rowBytes = kAtlasWidth_ * sizeof(uint32_t);
	for (size_t y = 0; y < kAtlasHeight_; y++) {
		std::memcpy(image->pixels + y * image->rowPitch, srcData + y * rowBytes, rowBytes);
	}

	// 書き出し
	auto outPath = ConvertString(atlasPath);
	std::filesystem::create_directories(std::filesystem::path(outPath).parent_path());
	hr = DirectX::SaveToWICFile(*image, DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG), outPath.c_str());
	assert(SUCCEEDED(hr));
}


bool FontLoader::BakeGlyph(FT_Face face, FontData &data, char32_t codepoint) {
	if (data.glyphs.count(codepoint)) {
		return true;
	}

	// face読み込み
	auto gid = FT_Get_Char_Index(face, codepoint);
	if (gid == 0) {
		return false;
	}
	FT_Load_Glyph(face, gid, FT_LOAD_DEFAULT);
	FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF);
	auto &glyph = face->glyph;
	auto &bitmap = glyph->bitmap;

	// 改行
	if (data.destX + static_cast<int>(bitmap.width) >= kAtlasWidth_) {
		data.destX = 0;
		data.destY += data.cellHeight + 2;

		// atlasの高さを超えたら終了
		if (data.destY + data.cellHeight >= kAtlasHeight_) {
			return false;
		}
	}

	// cpuアトラスに書き込み
	int offsetY = data.maxTop - glyph->bitmap_top;
	for (int y = 0; y < static_cast<int>(bitmap.rows); y++) {
		int destRow = data.destY + offsetY + y;
		// 範囲外はスキップ
		if (destRow < data.destY || destRow >= data.destY + data.cellHeight) {
			continue;
		}
		for (int x = 0; x < static_cast<int>(bitmap.width); x++) {
			uint8_t alpha = bitmap.buffer[y * bitmap.pitch + x];

			// 下線を揃える
			int index = (data.destY + offsetY + y) * kAtlasWidth_ + (data.destX + x);
			data.atlasPixels[index] = 0xFFFFFF00 | alpha;
		}
	}

	// glyphInfo登録
	GlyphInfo info{};
	info.atlasPos = {static_cast<float>(data.destX), static_cast<float>(data.destY + offsetY)};
	info.atlasSize = {static_cast<float>(bitmap.width), static_cast<float>(bitmap.rows)};
	info.bearing = {static_cast<float>(glyph->bitmap_left), static_cast<float>(glyph->bitmap_top)};
	info.advanceX = static_cast<float>(glyph->advance.x) / 64.0f;
	data.glyphs[codepoint] = info;

	// 次の文字の位置を更新
	data.destX += bitmap.width + 2;

	return true;
}


bool FontLoader::AddGlyph(const std::string &atlasFileName, char32_t codepoint) {
	auto it = fonts_.find(atlasFileName);
	if (it == fonts_.end()) {
		return false;
	}

	auto &data = fontData_[atlasFileName];
	if (!BakeGlyph(it->second, data, codepoint)) {
		return false;
	}

	data.dirty = true;
	data.diskDirty = true;

	return true;
}

void FontLoader::ReuploadAtlas(FontData &data) {
	// コピーバリアを張る
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		data.atlasResource.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_COPY_DEST
	);
	commandList_->ResourceBarrier(1, &barrier);

	// atlasPixelsからアップロード
	D3D12_SUBRESOURCE_DATA sub{};
	sub.pData = data.atlasPixels.data();
	sub.RowPitch = kAtlasWidth_ * sizeof(uint32_t);
	sub.SlicePitch = sub.RowPitch * kAtlasHeight_;
	UpdateSubresources(commandList_, data.atlasResource.Get(), data.uploadBuffer.Get(), 0, 0, 1, &sub);

	// リソースバリアに変更し張る
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		data.atlasResource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	commandList_->ResourceBarrier(1, &barrier);
}


bool FontLoader::IsCacheValid(const std::string &atlasPath, const std::string &jsonPath, const std::string &fontPath) const {
	std::error_code ec;

	// atlasかjsonが存在するか
	if (!std::filesystem::exists(atlasPath, ec) || !std::filesystem::exists(jsonPath, ec)) {
		return false;
	}

	// atlasとjsonの更新日時を取得できるか
	auto atlasT = std::filesystem::last_write_time(atlasPath, ec);
	if (ec) {
		return false;
	}
	auto jsonT = std::filesystem::last_write_time(jsonPath, ec);
	if (ec) {
		return false;
	}

	// 元ファイルより新しいか
	for (auto &source : {kResponseTextFile_, fontPath}) {
		if (!std::filesystem::exists(source, ec)) {
			continue;
		}

		auto sourceT = std::filesystem::last_write_time(source, ec);
		if (ec) {
			continue;
		}

		if (atlasT < sourceT || jsonT < sourceT) {
			return false;
		}
	}

	return true;
}


void FontLoader::WriteJson(const std::string &jsonPath, const FontData &data) const {
	nlohmann::ordered_json json;

	json["glyphCount"] = data.glyphs.size();
	json["font"] = data.fontFile;

	// 検証用の情報
	json["version"] = kJsonVersion_;
	json["fontSize"] = kAtlasFontSize_;
	json["atlasWidth"] = kAtlasWidth_;
	json["atlasHeight"] = kAtlasHeight_;

	// 次に書き込む位置
	json["destX"] = data.destX;
	json["destY"] = data.destY;

	// メトリクス
	json["maxTop"] = data.maxTop;
	json["maxBottom"] = data.maxBottom;
	json["cellHeight"] = data.cellHeight;
	json["lineHeight"] = data.lineHeight;
	json["fallbackChar"] = static_cast<uint32_t>(data.fallbackChar);

	// コードポイント順に並べる
	std::vector<char32_t> codes;
	codes.reserve(data.glyphs.size());
	for (auto &[code, glyph] : data.glyphs) {
		codes.push_back(code);
	}
	std::ranges::sort(codes);

	// 各グリフ
	auto items = nlohmann::ordered_json::array();
	for (auto &code : codes) {
		auto &glyph = data.glyphs.at(code);

		nlohmann::ordered_json item;
		item["code"] = static_cast<uint32_t>(code);
		item["pos"] = {glyph.atlasPos.x, glyph.atlasPos.y};
		item["size"] = {glyph.atlasSize.x, glyph.atlasSize.y};
		item["bearing"] = {glyph.bearing.x, glyph.bearing.y};
		item["advanceX"] = glyph.advanceX;

		items.push_back(std::move(item));
	}
	json["glyphs"] = std::move(items);

	// 書き出し
	std::ofstream file(jsonPath);
	if (!file) {
		return;
	}
	file << json.dump(4) << "\n";
}


bool FontLoader::ReadJson(const std::string &jsonPath, FontData &data) const {
	std::ifstream file(jsonPath);
	if (!file) {
		return false;
	}

	// 失敗を検知する
	auto json = nlohmann::ordered_json::parse(file, nullptr, false);
	if (json.is_discarded()) {
		return false;
	}

	// 検証
	if (json.value("version", 0) != kJsonVersion_) {
		return false;
	}
	if (json.value("fontSize", 0) != kAtlasFontSize_) {
		return false;
	}
	if (json.value("atlasWidth", 0) != kAtlasWidth_) {
		return false;
	}
	if (json.value("atlasHeight", 0) != kAtlasHeight_) {
		return false;
	}

	// 基本情報の読み込み
	data.fontFile = json.value("font", std::string{});
	data.destX = json.value("destX", 0);
	data.destY = json.value("destY", 0);
	data.maxTop = json.value("maxTop", 0);
	data.maxBottom = json.value("maxBottom", 0);
	data.cellHeight = json.value("cellHeight", 0);
	data.lineHeight = json.value("lineHeight", 0.0f);
	data.fallbackChar = static_cast<char32_t>(json.value("fallbackChar", 0u));

	// 配列をvec2として読むラムダ
	auto readAsVector2 = [](const nlohmann::ordered_json &item, const char *key) {
		Vector2 result{};

		auto it = item.find(key);
		if (it != item.end() && it->is_array() && it->size() == 2) {
			result.x = (*it)[0].get<float>();
			result.y = (*it)[1].get<float>();
		}

		return result;
	};

	// 各グリフ
	auto glyphs = json.find("glyphs");
	if (glyphs == json.end() || !glyphs->is_array()) {
		return false;
	}

	for (auto &item : *glyphs) {
		auto code = static_cast<char32_t>(item.value("code", 0u));
		if (code == 0) {
			continue;
		}

		GlyphInfo glyph{};
		glyph.atlasPos = readAsVector2(item, "pos");
		glyph.atlasSize = readAsVector2(item, "size");
		glyph.bearing = readAsVector2(item, "bearing");
		glyph.advanceX = item.value("advanceX", 0.0f);

		data.glyphs[code] = std::move(glyph);
	}

	return true;
}