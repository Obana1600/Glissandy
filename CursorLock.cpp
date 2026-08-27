#include "CursorLock.h"

#include "KamataEngine.h"
#include <Windows.h>

using namespace KamataEngine;


static bool sLocked = false;


void CursorLock::Update() {
	const auto hwnd = WinApp::GetInstance()->GetHwnd();

	// ウィンドウが最前面でないなら解除
	if (!sLocked || GetForegroundWindow() != hwnd) {
		ClipCursor(nullptr);
		return;
	}

	RECT rect{};
	GetClientRect(hwnd, &rect);

	POINT lt = {rect.left, rect.top};
	POINT rb = {rect.right, rect.bottom};
	ClientToScreen(hwnd, &lt);
	ClientToScreen(hwnd, &rb);

	RECT clip = {lt.x, lt.y, rb.x, rb.y};
	ClipCursor(&clip);
}


void CursorLock::SetLocked(bool locked) {
	if (sLocked == locked) {
		return;
	}

	sLocked = locked;
	ShowCursor(!locked);

	if (!locked) {
		ClipCursor(nullptr);
	}
}


bool CursorLock::IsLocked() {
	return sLocked;
}