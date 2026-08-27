#pragma once

#include "KamataEngine.h"
#include <memory>
#include <string>
#include <unordered_map>


class ModelManager {
public: // メンバ関数

	static ModelManager *GetInstance();

	KamataEngine::Model *Load(const std::string &modelName);

	void Finalize();


private: // メンバ変数

	std::unordered_map<std::string, std::unique_ptr<KamataEngine::Model>> models_;


private: // メンバ関数

	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(const ModelManager &) = delete;
	ModelManager operator=(const ModelManager &) = delete;
};