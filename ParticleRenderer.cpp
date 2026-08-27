#include "ParticleRenderer.h"

#include <d3dcompiler.h>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void ParticleRenderer::Initialize() {
	CreatePSO();

	auto *device = DirectXCommon::GetInstance()->GetDevice();
	CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);

	// 定数バッファ
	auto cbDesc = CD3DX12_RESOURCE_DESC::Buffer(256);
	HRESULT hr = device->CreateCommittedResource(
		&uploadHeap, D3D12_HEAP_FLAG_NONE, &cbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuffer_)
	);
	assert(SUCCEEDED(hr));
	constBuffer_->Map(0, nullptr, reinterpret_cast<void **>(&cbMap_));

	// インスタンスバッファ
	auto instanceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(ParticleInstance) * kMaxParticles);
	hr = device->CreateCommittedResource(
		&uploadHeap, D3D12_HEAP_FLAG_NONE, &instanceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&instanceBuffer_)
	);
	assert(SUCCEEDED(hr));
	instanceBuffer_->Map(0, nullptr, reinterpret_cast<void **>(&instanceMap_));
}


void ParticleRenderer::Draw(
	const KamataEngine::Camera &camera,
	const ParticleInstance *instance,
	size_t instanceCount,
	uint32_t textureHandle
) const {
	if (instanceCount == 0) {
		return;
	}

	// 定数バッファ
	auto viewProj = camera.matView * camera.matProjection;
	auto right = Vector3(camera.matView.m[0][0], camera.matView.m[1][0], camera.matView.m[2][0]);
	auto up = Vector3(camera.matView.m[0][1], camera.matView.m[1][1], camera.matView.m[2][1]);
	std::memcpy(&cbMap_->viewProj, &viewProj, sizeof(cbMap_->viewProj));
	std::memcpy(&cbMap_->cameraRight, &right, sizeof(cbMap_->cameraRight));
	std::memcpy(&cbMap_->cameraUp, &up, sizeof(cbMap_->cameraUp));

	// インスタンスバッファ
	instanceCount = (std::min)(instanceCount, kMaxParticles);
	std::memcpy(instanceMap_, instance, sizeof(ParticleInstance) * instanceCount);

	auto *commandList = DirectXCommon::GetInstance()->GetCommandList();
	commandList->SetGraphicsRootSignature(sRootSignature_.Get());
	commandList->SetPipelineState(sPipelineState_.Get());

	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(commandList, 2, textureHandle);

	commandList->SetGraphicsRootConstantBufferView(0, constBuffer_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootShaderResourceView(1, instanceBuffer_->GetGPUVirtualAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	commandList->DrawInstanced(4, static_cast<UINT>(instanceCount), 0, 0);
}


void ParticleRenderer::CreatePSO() {
	if (sPipelineState_) {
		return;
	}

	auto *device = DirectXCommon::GetInstance()->GetDevice();

	// シェーダコンパイル
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, errBlob;
	HRESULT hr = D3DCompileFromFile(
		kVsPath_.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vsBlob, &errBlob
	);
	assert(SUCCEEDED(hr));
	hr = D3DCompileFromFile(
		kPsPath_.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &psBlob, &errBlob
	);
	assert(SUCCEEDED(hr));

	// ルートシグネチャ
	D3D12_DESCRIPTOR_RANGE srvRange{};
	srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	srvRange.NumDescriptors = 1;
	srvRange.BaseShaderRegister = 1; // t1
	srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[3]{};
	// b0
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	rootParameters[0].Descriptor.RegisterSpace = 0;
	// t0
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].Descriptor.ShaderRegister = 0;
	rootParameters[1].Descriptor.RegisterSpace = 0;
	// t1
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &srvRange;

	// s0
	D3D12_STATIC_SAMPLER_DESC sampler{};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// ルートシグネチャ
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = 3;
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

	// ブレンド
	auto &renderTarget0 = psoDesc.BlendState.RenderTarget[0];
	renderTarget0.BlendEnable = TRUE;
	renderTarget0.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	renderTarget0.DestBlend = D3D12_BLEND_ONE;
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
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
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