#pragma once

#include "KamataEngine.h"


struct Line {
	KamataEngine::Vector3 origin; //!< 始点
	KamataEngine::Vector3 diff;	  //!< 終点への差分ベクトル
};


struct Ray {
	KamataEngine::Vector3 origin; //!< 始点
	KamataEngine::Vector3 diff;	  //!< 終点への差分ベクトル
};


struct Segment {
	KamataEngine::Vector3 origin; //!< 始点
	KamataEngine::Vector3 diff;	  //!< 終点への差分ベクトル
};


struct Plane {
	KamataEngine::Vector3 normal; //!< 法線
	float distance; //!< 距離
};


struct Triangle {
	KamataEngine::Vector3 vertices[3]; //!< 頂点
};


struct Sphere {
	KamataEngine::Vector3 center; //!< 中心点
	float radius;	//!< 半径
};


struct AABB {
	KamataEngine::Vector3 min; //!< 最小点
	KamataEngine::Vector3 max; //!< 最大点
};


struct OBB {
	KamataEngine::Vector3 center; //!< 中心点
	KamataEngine::Vector3 orientation[3]; //!< 座標軸。正規化・直交必須
	KamataEngine::Vector3 size;			  //!< 座標軸方向の長さの半分。中心から面までの距離
};


struct Capsule {
	Segment segment;
	float radius;
};



// linear ------------------------------------------------------------

// 正射影ベクトル
KamataEngine::Vector3 Project(const KamataEngine::Vector3 &v1, const KamataEngine::Vector3 &v2);
// 最近接点
KamataEngine::Vector3 ClosestPoint(const KamataEngine::Vector3 &point, const Segment &segment);

// -------------------------------------------------------------------


// plane -------------------------------------------------------------

// 垂直ベクトル
KamataEngine::Vector3 Perpendicular(const KamataEngine::Vector3 &v);

// -------------------------------------------------------------------

// 反射ベクトル
KamataEngine::Vector3 Reflect(const KamataEngine::Vector3 &input, const KamataEngine::Vector3 &normal);