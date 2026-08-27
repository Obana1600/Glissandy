#include "CursorLock.h"
#include "GizmoEditor.h"
#include "KamataEngine.h"
#include "ModelManager.h"
#include "MusicManager.h"
#include "SceneManager.h"
#include "Text/FontLoader.h"
#include "TimeT.h"
#include "Tween/TweenManager.h"
#include <Windows.h>


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	using namespace KamataEngine;

	// エンジンの初期化
	KamataEngine::Initialize(L"LE2B_05_オバナ_サトル_Glissandy");

	auto *dxCommon = DirectXCommon::GetInstance();
	auto *imguiMgr = ImGuiManager::GetInstance();

#ifdef USE_IMGUI
	// ドッキングフラグの有効化
	auto &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif

	// フォント
	FontLoader::GetInstance()->Initialize();

	// シーン
	auto *sceneMgr = SceneManager::GetInstance();
	sceneMgr->Initialize();

	// ゲームループ
	while (true) {
		// エンジンの更新処理
		if (KamataEngine::Update() || sceneMgr->IsExitRequested()) {
			break;
		}

		TimeT::Update();
		CursorLock::Update();
		TweenManager::GetInstance()->UpdateAll(TimeT::GetDeltaTime());


		imguiMgr->Begin(); // 更新処理ここから
#ifdef USE_IMGUI
		ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
#endif
		GizmoEditor::BeginFrame();

		sceneMgr->Update();

		imguiMgr->End(); // 更新処理ここまで


		dxCommon->PreDraw(); // 描画処理ここから

		sceneMgr->Draw();

		imguiMgr->Draw();
		dxCommon->PostDraw(); // 描画処理ここまで
	}

	CursorLock::SetLocked(false);
	sceneMgr->Finalize();
	FontLoader::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();
	MusicManager::GetInstance()->Finalize();

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}