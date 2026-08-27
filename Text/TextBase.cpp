#include "TextBase.h"
#include "ConvertString.h"
#include "DebugUi.h"
#include "FontLoader.h"
#include <algorithm>
#include <filesystem>
#ifdef USE_IMGUI
	#include <imgui/imgui_stdlib.h>
#endif

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void TextBase::Initialize() {
	fontLoader_ = FontLoader::GetInstance();

	fontLoader_->Load(fontPath_);
	lineHeight_ = fontLoader_->GetLineHeight(fontPath_);
	Rebuild();
}


void TextBase::UpdateImGui() {
#ifdef USE_IMGUI
	if (!ImGui::CollapsingHeader("TextBase", ImGuiTreeNodeFlags_DefaultOpen)) {
		return;
	}


	// text ---------------------------------------------------------------------------------------

	ImGui::SeparatorText("Text Input");

	auto inputText = text_;
	if (ImGui::InputTextMultiline("##text", &inputText)) {
		SetText(inputText);
	}

	// --------------------------------------------------------------------------------------------


	// font ---------------------------------------------------------------------------------------

	ImGui::SeparatorText("Font");
	if (DebugUi::BeginTable("Font")) {

		// fontPath
		DebugUi::PropatyRow("FontPath", [&] {
			ImGui::Text("%s", fontPath_.c_str());
		});

		// loadFont
		DebugUi::PropatyRow("LoadFont", [&] {
			ImGui::InputText("##inputFontFile", &inputFontFile_);
			ImGui::SameLine();
			if (ImGui::Button("Load") && !inputFontFile_.empty()) {
				SetFont(inputFontFile_);
			} }, false);

		// fontSize
		DebugUi::PropatyRow("FontSize", [&] {
			if (ImGui::DragFloat("##fontSize", &fontSize_, 0.5f, sizeMin_, sizeMax_)) {
				Rebuild();
			}
		});

		// autoSize
		DebugUi::PropatyRow("AutoSize", [&] {
			if (ImGui::Checkbox("##autoSize", &autoSize_)) {
				Rebuild();
			}
		});

		// sizeMin / sizeMax
		if (autoSize_) {
			ImGui::Indent();

			DebugUi::PropatyRow("SizeRange", [&] {
				float range[2] = {sizeMin_, sizeMax_};
				if (ImGui::DragFloat2("##sizeRange", range, 0.5f, 1.0f, 512.0f)) {
					sizeMin_ = range[0];
					sizeMax_ = range[1];
					fontSize_ = std::clamp(fontSize_, sizeMin_, sizeMax_);
					Rebuild();
				}
			});

			ImGui::Unindent();
		}

		//// style
		// DebugUi::PropatyRow("Bold", [&] {
		//	if (ImGui::Checkbox("##bold", &fontStyle_.bold)) {
		//		Rebuild();
		//	}
		// });

		// DebugUi::PropatyRow("Italic", [&] {
		//	if (ImGui::Checkbox("##italic", &fontStyle_.italic)) {
		//		Rebuild();
		//	}
		// });

		// DebugUi::PropatyRow("UnderLine", [&] {
		//	if (ImGui::Checkbox("##underLine", &fontStyle_.underLine)) {
		//		Rebuild();
		//	}
		// });

		// DebugUi::PropatyRow("StrikeThrough", [&] {
		//	if (ImGui::Checkbox("##strikeThrough", &fontStyle_.strileThrough)) {
		//		Rebuild();
		//	}
		// });

		DebugUi::EndTable();
	}

	// --------------------------------------------------------------------------------------------


	// color --------------------------------------------------------------------------------------

	// color
	ImGui::SeparatorText("Color");
	if (DebugUi::BeginTable("Color")) {

		// baseColor
		DebugUi::PropatyRow("BaseColor", [&] {
			ImGui::ColorEdit4("##baseColor", &baseColor_.x, ImGuiColorEditFlags_NoInputs);
		});

		// useGradient
		DebugUi::PropatyRow("UseGradient", [&] {
			ImGui::Checkbox("##useGradient", &useGradient_);
		});

		if (useGradient_) {
			ImGui::Indent();

			// colorMode
			DebugUi::PropatyRow("ColorMode", [&] {
				int mode = static_cast<int>(colorGradient_.colorMode);
				if (ImGui::Combo("##colorMode", &mode, "Single\0Horizontal\0Vertical\0FourCorner\0")) {
					colorGradient_.colorMode = static_cast<ColorMode>(mode);
				}
			});

			// cornerColor
			DebugUi::PropatyRow("LeftTop", [&] {
				ImGui::ColorEdit4("##leftTop", &colorGradient_.leftTop.x, ImGuiColorEditFlags_NoInputs);
			});

			DebugUi::PropatyRow("RightTop", [&] {
				ImGui::ColorEdit4("##rightTop", &colorGradient_.rightTop.x, ImGuiColorEditFlags_NoInputs);
			});

			DebugUi::PropatyRow("LeftBottom", [&] {
				ImGui::ColorEdit4("##leftBottom", &colorGradient_.leftBottom.x, ImGuiColorEditFlags_NoInputs);
			});

			DebugUi::PropatyRow("RightBottom", [&] {
				ImGui::ColorEdit4("##rightBottom", &colorGradient_.rightBottom.x, ImGuiColorEditFlags_NoInputs);
			});

			ImGui::Unindent();
		}

		// tint
		DebugUi::PropatyRow("Tint", [&] {
			ImGui::ColorEdit4("##tint", &tint_.x, ImGuiColorEditFlags_NoInputs);
		});

		DebugUi::EndTable();
	}

	// --------------------------------------------------------------------------------------------


	// outline ------------------------------------------------------------------------------------

	ImGui::SeparatorText("Outline");
	if (DebugUi::BeginTable("Outline")) {

		// useOutline
		DebugUi::PropatyRow("UseOutline", [&] {
			ImGui::Checkbox("##useOutline", &useOutline_);
		});

		if (useOutline_) {
			// color
			DebugUi::PropatyRow("Color", [&] {
				ImGui::ColorEdit4("##outlineColor", &outline_.color.x, ImGuiColorEditFlags_NoInputs);
			});

			// width
			DebugUi::PropatyRow("Width", [&] {
				ImGui::DragFloat("##outlineWidth", &outline_.width, 0.01f, 0.0f, 1.0f);
			});
		}

		DebugUi::EndTable();
	}

	// --------------------------------------------------------------------------------------------


	// shadow -------------------------------------------------------------------------------------

	ImGui::SeparatorText("Shadow");
	if (DebugUi::BeginTable("Shadow")) {

		// useShadow
		DebugUi::PropatyRow("UseShadow", [&] {
			ImGui::Checkbox("##useShadow", &useShadow_);
		});

		if (useShadow_) {
			// color
			DebugUi::PropatyRow("Color", [&] {
				ImGui::ColorEdit4("##shadowColor", &shadow_.color.x, ImGuiColorEditFlags_NoInputs);
			});

			// offset
			DebugUi::PropatyRow("Offset", [&] {
				ImGui::DragFloat2("##shadowOffset", &shadow_.offset.x, 0.01f);
			});

			// scale
			DebugUi::PropatyRow("Scale", [&] {
				ImGui::DragFloat2("##shadowScale", &shadow_.scale.x, 0.01f);
			});

			// shear
			DebugUi::PropatyRow("Shear", [&] {
				ImGui::DragFloat2("##shadowShear", &shadow_.shear.x, 0.01f);
			});

			// softness
			DebugUi::PropatyRow("Softness", [&] {
				ImGui::DragFloat("##shadowSoftness", &shadow_.softness, 0.01f, 0.0f, 1.0f);
			});
		}

		DebugUi::EndTable();
	}

	// --------------------------------------------------------------------------------------------


	// layout -------------------------------------------------------------------------------------

	ImGui::SeparatorText("Layout");
	if (DebugUi::BeginTable("Layout")) {

		// spacing
		DebugUi::PropatyRow("Character", [&] {
			if (ImGui::DragFloat("##spacingCharacter", &spacing_.character, 0.01f)) {
				Rebuild();
			}
		});

		DebugUi::PropatyRow("Word", [&] {
			if (ImGui::DragFloat("##spacingWord", &spacing_.word, 0.01f)) {
				Rebuild();
			}
		});

		DebugUi::PropatyRow("Line", [&] {
			if (ImGui::DragFloat("##spacingLine", &spacing_.line, 0.01f)) {
				Rebuild();
			}
		});

		DebugUi::PropatyRow("Paragraph", [&] {
			if (ImGui::DragFloat("##spacingParagraph", &spacing_.paragraph, 0.01f)) {
				Rebuild();
			}
		});

		// horizontalAlignment
		DebugUi::PropatyRow("Horizontal", [&] {
			int align = static_cast<int>(horizontalAlignment_);
			if (ImGui::Combo("##horizontalAlignment", &align, "Left\0Center\0Right\0")) {
				horizontalAlignment_ = static_cast<HorizontalAlignment>(align);
				Rebuild();
			}
		});

		// verticalAlignment
		DebugUi::PropatyRow("Vertical", [&] {
			int align = static_cast<int>(verticalAlignment_);
			if (ImGui::Combo("##verticalAlignment", &align, "Top\0Middle\0Bottom\0")) {
				verticalAlignment_ = static_cast<VerticalAlignment>(align);
				Rebuild();
			}
		});

		DebugUi::EndTable();
	}

	// --------------------------------------------------------------------------------------------


	// info -------------------------------------------------------------------------------------

	// info
	if (DebugUi::BeginTable("info")) {
		DebugUi::PropatyRow("RectSize", [&] {
			ImGui::Text("%.2f x %.2f", rectSize_.x, rectSize_.y);
		});

		DebugUi::PropatyRow("FontSizePerUnit", [&] {
			ImGui::Text("%.2f", fontSizePerUnit_);
		});

		DebugUi::PropatyRow("BaselineY", [&] {
			ImGui::Text("%.2f", baselineY_);
		});

		DebugUi::PropatyRow("LineHeight", [&] {
			ImGui::Text("%.2f", lineHeight_);
		});

		DebugUi::PropatyRow("GlyphCount", [&] {
			ImGui::Text("%zu", instances_.size());
		});

		DebugUi::EndTable();
	}

	// --------------------------------------------------------------------------------------------


	// fontLoader
	if (fontLoader_) {
		ImGui::Text("FontLoader is assigned!");
	} else {
		ImGui::Text("FontLoader is not assigned.");
	}
#endif
}


const std::array<KamataEngine::Vector4, 4> TextBase::GetCornerColors() const {
	std::array<Vector4, 4> result = {baseColor_, baseColor_, baseColor_, baseColor_};

	if (useGradient_) {
		switch (colorGradient_.colorMode) {
			case ColorMode::single:
				result[0] = colorGradient_.leftTop;
				result[1] = colorGradient_.leftTop;
				result[2] = colorGradient_.leftTop;
				result[3] = colorGradient_.leftTop;
				break;

			case ColorMode::horizontalGradient:
				result[0] = colorGradient_.leftTop;
				result[1] = colorGradient_.rightTop;
				result[2] = colorGradient_.leftTop;
				result[3] = colorGradient_.rightTop;
				break;

			case ColorMode::verticalGradient:
				result[0] = colorGradient_.leftTop;
				result[1] = colorGradient_.leftTop;
				result[2] = colorGradient_.leftBottom;
				result[3] = colorGradient_.leftBottom;
				break;

			case ColorMode::fourCornerGradient:
				result[0] = colorGradient_.leftTop;
				result[1] = colorGradient_.rightTop;
				result[2] = colorGradient_.leftBottom;
				result[3] = colorGradient_.rightBottom;
				break;
		}
	}

	// 係数を乗算
	for (auto &corner : result) {
		corner = {corner.x * tint_.x, corner.y * tint_.y, corner.z * tint_.z, corner.w * tint_.w};
	}

	return result;
}


void TextBase::Rebuild() {
	instances_.clear();
	baselineY_ = 0.0f;

	auto wide = ConvertString(text_);
	const float atlasWidth = static_cast<float>(fontLoader_->GetAtlasWidth());
	const float atlasHeight = static_cast<float>(fontLoader_->GetAtlasHeight());
	const float scale = static_cast<float>(fontSize_) / static_cast<float>(fontLoader_->GetAtlasFontSIze()) / fontSizePerUnit_;

	struct LineInfo {
		size_t startIdx;
		float width;
	};
	std::vector<LineInfo> lineInfos;
	size_t lineStartIdx = 0;
	float lineRight = 0.0f;

	float penX = 0.0f;
	bool prevNewline = false;

	for (auto &c : wide) {
		// 改行
		if (c == '\n') {
			lineInfos.push_back({lineStartIdx, lineRight});
			lineStartIdx = instances_.size();
			lineRight = 0.0f;
			penX = 0.0f;
			baselineY_ -= (lineHeight_ + spacing_.line) * scale;
			if (prevNewline) {
				baselineY_ -= spacing_.paragraph * scale;
			}
			prevNewline = true;
			continue;
		}

		auto &glyph = fontLoader_->GetGlyph(fontPath_, c);
		if (glyph.atlasSize.x > 0.0f && glyph.atlasSize.y > 0.0f) {
			GlyphInstance instance{};
			float left = penX + glyph.bearing.x * scale;
			float top = baselineY_ + glyph.bearing.y * scale;
			instance.rect[0] = left;
			instance.rect[1] = top;
			instance.rect[2] = glyph.atlasSize.x * scale;
			instance.rect[3] = glyph.atlasSize.y * scale;
			instance.uv[0] = glyph.atlasPos.x / atlasWidth;
			instance.uv[1] = glyph.atlasPos.y / atlasHeight;
			instance.uv[2] = (glyph.atlasPos.x + glyph.atlasSize.x) / atlasWidth;
			instance.uv[3] = (glyph.atlasPos.y + glyph.atlasSize.y) / atlasHeight;

			instances_.push_back(instance);
			lineRight = (std::max)(lineRight, left + instance.rect[2]);
		}

		penX += glyph.advanceX * scale;

		// character
		penX += spacing_.character * scale;

		// word
		if (c == L' ' || c == L'　') {
			penX += spacing_.word * scale;
		}

		prevNewline = false;
	}
	lineInfos.push_back({lineStartIdx, lineRight});

	// alignment (rectSizeが設定されている場合のみ)
	if (instances_.empty() || (rectSize_.x <= 0.0f && rectSize_.y <= 0.0f)) {
		return;
	}

	// 水平alignment
	if (rectSize_.x > 0.0f) {
		for (size_t li = 0; li < lineInfos.size(); li++) {
			float hOffset = 0.0f;
			switch (horizontalAlignment_) {
				case HorizontalAlignment::left:
				default:
					break;
				case HorizontalAlignment::center:
					hOffset = (rectSize_.x - lineInfos[li].width) / 2.0f;
					break;
				case HorizontalAlignment::right:
					hOffset = rectSize_.x - lineInfos[li].width;
					break;
			}
			size_t end = (li + 1 < lineInfos.size()) ? lineInfos[li + 1].startIdx : instances_.size();
			for (size_t i = lineInfos[li].startIdx; i < end; i++) {
				instances_[i].rect[0] += hOffset;
			}
		}
	}

	// 垂直alignment
	if (rectSize_.y > 0.0f) {
		float textTop = instances_[0].rect[1];
		float textBottom = instances_[0].rect[1] - instances_[0].rect[3];
		for (size_t i = 1; i < instances_.size(); i++) {
			textTop = (std::max)(textTop, instances_[i].rect[1]);
			textBottom = (std::min)(textBottom, instances_[i].rect[1] - instances_[i].rect[3]);
		}
		float textHeight = textTop - textBottom;

		float vOffset = -textTop;
		switch (verticalAlignment_) {
			case VerticalAlignment::top:
			default:
				break;
			case VerticalAlignment::middle:
				vOffset -= (rectSize_.y - textHeight) / 2.0f;
				break;
			case VerticalAlignment::bottom:
				vOffset -= (rectSize_.y - textHeight);
				break;
		}
		for (auto &inst : instances_) {
			inst.rect[1] += vOffset;
		}
		baselineY_ += vOffset;
	}
}


void TextBase::SetText(const std::string &text) {
	if (text == text_) {
		return;
	}
	text_ = text;
	Rebuild();
}


void TextBase::SetCornerColors(const std::array<KamataEngine::Vector4, 4> &cornerColor) {
	if (useGradient_) {
		switch (colorGradient_.colorMode) {
			case ColorMode::single:
				colorGradient_.leftTop = cornerColor[0];
				colorGradient_.rightTop = cornerColor[0];
				colorGradient_.leftBottom = cornerColor[0];
				colorGradient_.rightBottom = cornerColor[0];
				break;

			case ColorMode::horizontalGradient:
				colorGradient_.leftTop = cornerColor[0];
				colorGradient_.rightTop = cornerColor[1];
				colorGradient_.leftBottom = cornerColor[0];
				colorGradient_.rightBottom = cornerColor[1];
				break;

			case ColorMode::verticalGradient:
				colorGradient_.leftTop = cornerColor[0];
				colorGradient_.rightTop = cornerColor[0];
				colorGradient_.leftBottom = cornerColor[2];
				colorGradient_.rightBottom = cornerColor[2];
				break;

			case ColorMode::fourCornerGradient:
				colorGradient_.leftTop = cornerColor[0];
				colorGradient_.rightTop = cornerColor[1];
				colorGradient_.leftBottom = cornerColor[2];
				colorGradient_.rightBottom = cornerColor[3];
				break;
		}
	}

	baseColor_ = cornerColor[0];
}


void TextBase::SetTint(const KamataEngine::Vector4 &tint) {
	tint_ = tint;
}


void TextBase::SetFont(const std::string &fontPath) {
	if (fontPath_ == fontPath) {
		return;
	}
	fontPath_ = fontPath;
	fontLoader_->Load(fontPath);
	lineHeight_ = fontLoader_->GetLineHeight(fontPath);
	Rebuild();
}


void TextBase::SetRectSize(const KamataEngine::Vector2 &size) {
	if (rectSize_.x == size.x && rectSize_.y == size.y) {
		return;
	}
	rectSize_ = size;
	Rebuild();
}


void TextBase::SetFontSize(float size) {
	fontSize_ = size;
	Rebuild();
}


void TextBase::SetFontSizePerUnit(float v) {
	if (fontSizePerUnit_ == v) {
		return;
	}
	fontSizePerUnit_ = v;
	Rebuild();
}