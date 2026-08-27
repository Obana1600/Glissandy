#include "Component.h"

#include <Tween/TweenManager.h>


Component::~Component() {
	// 自分を対象にしたtweenを止める
	TweenManager::Kill(this);
}