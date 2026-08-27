#include "TextRenderer.h"
#include "FontLoader.h"
#include <algorithm>
#include <d3dcompiler.h>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void TextRenderer::Initialize() {
	CreatePSO();

	auto *device = DirectXCommon::GetInstance()->GetDevice();
	CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);

	// 定数バッファ
	auto cdDesc = CD3DX12_RESOURCE_DESC::Buffer(256);
	HRESULT hr = device->CreateCommittedResource(
		&uploadHeap, D3D12_HEAP_FLAG_NONE, &cdDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuffer_)
	);
	assert(SUCCEEDED(hr));
	constBuffer_->Map(0, nullptr, reinterpret_cast<void **>(&cbMap_));

	// インスタンスバッファ
	auto instanceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(TextBase::GlyphInstance) * kMaxGlyphs_);
	hr = device->CreateCommittedResource(
		&uploadHeap, D3D12_HEAP_FLAG_NONE, &instanceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&instanceBuffer_)
	);
	assert(SUCCEEDED(hr));
	instanceBuffer_->Map(0, nullptr, reinterpret_cast<void **>(&instanceMap_));
}


void TextRenderer::Draw(const Matrix4x4 &wvp, const TextBase &text) const {
	FontLoader::GetInstance()->FlushUploads();

	auto &instances = text.GetInstances();
	if (instances.empty()) {
		return;
	}
	auto count = (std::min)(instances.size(), kMaxGlyphs_);

	// インスタンスバッファ
	std::memcpy(instanceMap_, instances.data(), sizeof(TextBase::GlyphInstance) * count);

	// 定数バッファ
	auto &col = text.GetCornerColors();
	auto &outline = text.GetOutline();
	auto &shadow = text.GetShadow();
	auto baselineY = text.GetBaselineY();
	std::memcpy(&cbMap_->wvp, &wvp, sizeof(float) * 16);
	std::memcpy(&cbMap_->colorLT, &col[0], sizeof(float) * 4);
	std::memcpy(&cbMap_->colorRT, &col[1], sizeof(float) * 4);
	std::memcpy(&cbMap_->colorLB, &col[2], sizeof(float) * 4);
	std::memcpy(&cbMap_->colorRB, &col[3], sizeof(float) * 4);
	std::memcpy(&cbMap_->colorLT, &col[0], sizeof(float) * 4);
	std::memcpy(&cbMap_->outlineColor, &outline.color, sizeof(float) * 4);
	std::memcpy(&cbMap_->shadowColor, &shadow.color, sizeof(float) * 4);
	std::memcpy(&cbMap_->outlineWidth, &outline.width, sizeof(float));
	std::memcpy(&cbMap_->shadowBaseY, &baselineY, sizeof(float));
	std::memcpy(&cbMap_->shadowSoftness, &shadow.softness, sizeof(float));
	std::memcpy(&cbMap_->shadowOffset, &shadow.offset, sizeof(float) * 2);
	std::memcpy(&cbMap_->shadowScale, &shadow.scale, sizeof(float) * 2);
	std::memcpy(&cbMap_->shadowShear, &shadow.shear, sizeof(float) * 2);

	auto *commandList = DirectXCommon::GetInstance()->GetCommandList();
	commandList->SetGraphicsRootSignature(sRootSignature_.Get());
	commandList->SetPipelineState(sPipelineState_.Get());

	ID3D12DescriptorHeap *heaps[] = {FontLoader::GetInstance()->GetSRVHeap()};
	commandList->SetDescriptorHeaps(1, heaps);

	commandList->SetGraphicsRootConstantBufferView(1, constBuffer_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootShaderResourceView(2, instanceBuffer_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(
		3, FontLoader::GetInstance()->GetGPUHandle(text.GetFontPath())
	);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	struct PassCB {
		uint32_t useOutline;
		uint32_t useShadow;
	};

	// 影 -> 本体の描画
	auto drawPass = [&](bool isShadow) {
		PassCB pass{};
		pass.useOutline = text.UseOutline() ? 1 : 0;
		pass.useShadow = isShadow ? 1 : 0;

		commandList->SetGraphicsRoot32BitConstants(0, 2, &pass, 0);
		commandList->DrawInstanced(4, static_cast<UINT>(count), 0, 0);
	};

	if (text.UseShadow()) {
		drawPass(true);
	}
	drawPass(false);
}


void TextRenderer::DrawGizmo(
	const Matrix4x4 &wvp, const Vector2 &rectSize, const Vector2 &pivot, bool showRect, bool showPivot
) {
#ifdef USE_IMGUI
	if (!showRect && !showPivot) {
		return;
	}

	const auto &origin = ImGui::GetMainViewport()->Pos;
	const auto &disp = ImGui::GetIO().DisplaySize;

	// ローカル座標 -> スクリーン座標
	auto toScreen = [&](float lx, float ly) {
		float nx = lx * wvp.m[0][0] + ly * wvp.m[1][0] + wvp.m[3][0];
		float ny = lx * wvp.m[0][1] + ly * wvp.m[1][1] + wvp.m[3][1];
		float nw = lx * wvp.m[0][3] + ly * wvp.m[1][3] + wvp.m[3][3];
		nx /= nw;
		ny /= nw;
		float sx = origin.x + (nx + 1.0f) * 0.5f * disp.x;
		float sy = origin.y + (1.0f - ny) * 0.5f * disp.y;
		return ImVec2(sx, sy);
	};

	auto *drawList = ImGui::GetForegroundDrawList();

	// rect
	if (showRect) {
		ImVec2 lt = toScreen(0.0f, 0.0f);
		ImVec2 rt = toScreen(rectSize.x, 0.0f);
		ImVec2 lb = toScreen(0.0f, -rectSize.y);
		ImVec2 rb = toScreen(rectSize.x, -rectSize.y);

		ImU32 rectColor = IM_COL32(0, 255, 0, 200);
		drawList->AddLine(lt, rt, rectColor);
		drawList->AddLine(rt, rb, rectColor);
		drawList->AddLine(rb, lb, rectColor);
		drawList->AddLine(lb, lt, rectColor);
	}

	// pivot
	if (showPivot) {
		auto p = toScreen(pivot.x * rectSize.x, -(1.0f - pivot.y) * rectSize.y);
		drawList->AddCircleFilled(p, 32.0f, IM_COL32(255, 0, 0, 200));
	}

#else
	(void)wvp;
	(void)rectSize;
	(void)pivot;
	(void)showRect;
	(void)showPivot;
#endif
}


void TextRenderer::CreatePSO() {
	if (sPipelineState_) {
		return;
	}

	auto *device = DirectXCommon::GetInstance()->GetDevice();

	// シェーダーコンパイル
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, errBlob;
	HRESULT hr = D3DCompileFromFile(
		kVsPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vsBlob, &errBlob
	);
	assert(SUCCEEDED(hr));
	hr = D3DCompileFromFile(
		kPsPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &psBlob, &errBlob
	);
	assert(SUCCEEDED(hr));

	// ルートシグネチャ
	D3D12_DESCRIPTOR_RANGE srvRange{};
	srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	srvRange.NumDescriptors = 1;
	srvRange.BaseShaderRegister = 1; // t1
	srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[4]{};
	// b0
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].Constants.ShaderRegister = 0;
	rootParameters[0].Constants.RegisterSpace = 0;
	rootParameters[0].Constants.Num32BitValues = 2;
	// b1
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].Descriptor.ShaderRegister = 1;
	rootParameters[1].Descriptor.RegisterSpace = 0;
	// t0
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[2].Descriptor.ShaderRegister = 0;
	rootParameters[2].Descriptor.RegisterSpace = 0;
	// t1
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[3].DescriptorTable.pDescriptorRanges = &srvRange;

	// s0
	D3D12_STATIC_SAMPLER_DESC sampler{};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = 4;
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pStaticSamplers = &sampler;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob, signatureErrBlob;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &signatureErrBlob);
	assert(SUCCEEDED(hr));
	hr = device->CreateRootSignature(
		0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&sRootSignature_)
	);
	assert(SUCCEEDED(hr));

	// pso
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = sRootSignature_.Get();
	psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
	psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};

	// アルファブレンド
	auto &renderTarget0 = psoDesc.BlendState.RenderTarget[0];
	renderTarget0.BlendEnable = TRUE;
	renderTarget0.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	renderTarget0.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	renderTarget0.BlendOp = D3D12_BLEND_OP_ADD;
	renderTarget0.SrcBlendAlpha = D3D12_BLEND_ONE;
	renderTarget0.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	renderTarget0.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	renderTarget0.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// ラスタライザ
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.DepthClipEnable = true;

	// 深度
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.InputLayout = {nullptr, 0};

	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&sPipelineState_));
	assert(SUCCEEDED(hr));
}