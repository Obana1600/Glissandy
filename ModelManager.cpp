#include "ModelManager.h"

using namespace KamataEngine;



ModelManager *ModelManager::GetInstance() {
	static ModelManager instance;
	return &instance;
}


KamataEngine::Model *ModelManager::Load(const std::string &modelName) {
	auto it = models_.find(modelName);

	// すでにモデルがあるならそれを返す
	if (it != models_.end()) {
		return it->second.get();
	}

	// 新しくモデルを生成
	auto newModel = std::unique_ptr<Model>(Model::CreateFromOBJ(modelName));
	auto ptr = newModel.get();
	models_[modelName] = std::move(newModel);

	return ptr;
}


void ModelManager::Finalize() {
	models_.clear();
}