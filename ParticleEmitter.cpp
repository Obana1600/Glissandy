#include "ParticleEmitter.h"

#include "DebugUi.h"
#include "GameObject.h"
#include "TimeT.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



void ParticleEmitter::Initialize() {
	baseDirection_ = params_.direction;
}


void ParticleEmitter::Update() {
	if (!isPlaying_ || ratePerSecond_ <= 0.0f || !particleMgr_) {
		return;
	}

	const auto dt = TimeT::GetDeltaTime();

	elapsed_ += dt;
	if (duration_ > 0.0f && elapsed_ >= duration_) {
		if (loop_) {
			elapsed_ -= duration_;
		} else {
			isPlaying_ = false;
			return;
		}
	}

	UpdateTransform();

	// 今フレームの放出数を数える
	emitAccumlator_ += ratePerSecond_ * dt;
	const auto n = static_cast<size_t>(emitAccumlator_);
	if (n > 0) {
		emitAccumlator_ -= static_cast<float>(n);
		particleMgr_->Emit(params_, n);
	}
}


void ParticleEmitter::UpdateTransform() {
	auto &matWorld = owner_->GetWorldTransform().matWorld_;

	params_.position = owner_->GetWorldPosition() + TransformNormal(offset_, matWorld);
	if (useOwnerRotation_) {
		params_.direction = TransformNormal(baseDirection_, matWorld);
	}
}


void ParticleEmitter::UpdateImGui() {
#ifdef USE_IMGUI
	if (DebugUi::BeginTable(name_.c_str())) {

		// burstCount
		DebugUi::PropatyRow("BurstCount", [&] {
			ImGui::DragInt("##burstCount", &burstCount_, 1, 0, 1024);
		});

		// duration
		DebugUi::PropatyRow("Duration", [&] {
			ImGui::DragFloat("##duration", &duration_, 0.01f, 0.0f, FLT_MAX);
		});

		// ratePerSecond
		DebugUi::PropatyRow("RatePerSecond", [&] {
			ImGui::DragFloat("##ratePerSecond", &ratePerSecond_, 0.1f, 0.0f, FLT_MAX);
		});

		// loop
		DebugUi::PropatyRow("Loop", [&] {
			ImGui::Checkbox("##loop", &loop_);
		});

		// isPlaying
		DebugUi::PropatyRow("IsPlaying", [&] {
			ImGui::Text("%s", isPlaying_ ? "true" : "false");
		});

		// elapsed
		DebugUi::PropatyRow("Elapsed", [&] {
			ImGui::Text("%.2f / %.2f", elapsed_, duration_);
		});

		// offset
		DebugUi::PropatyRow("Offset", [&] {
			ImGui::DragFloat3("##offset", &offset_.x, 0.1f);
		});

		// baseDirection
		DebugUi::PropatyRow("BaseDirection", [&] {
			ImGui::DragFloat3("##baseDirection", &baseDirection_.x, 0.01f);
		});

		// useOwnerRotation
		DebugUi::PropatyRow("UseOwnerRotation", [&] {
			ImGui::Checkbox("##useOwnerRotation", &useOwnerRotation_);
		});

		// simulate
		DebugUi::PropatyRow("Simulate", [&] {
			if (ImGui::Button("burst")) {
				Burst();
			}
			ImGui::SameLine();
			if (ImGui::Button("play")) {
				Play();
			}
			ImGui::SameLine();
			if (ImGui::Button("stop")) {
				Stop();
			} }, false);

		DebugUi::EndTable();
	}

	ImGui::SeparatorText("EmitParams");
	if (DebugUi::BeginTable("EmitParams")) {

		// shape
		DebugUi::PropatyRow("Shape", [&] {
			int shape = static_cast<int>(params_.shape);
			if (ImGui::Combo("##shape", &shape, "Point\0Circle\0Sphere\0Cone\0Box\0")) {
				params_.shape = static_cast<EmitShape>(shape);
			}
		});

		// radius
		DebugUi::PropatyRow("Radius", [&] {
			ImGui::DragFloat("##radius", &params_.radius, 0.1f, 0.0f, FLT_MAX);
		});

		// radiusThickness
		DebugUi::PropatyRow("RadiusThickness", [&] {
			ImGui::DragFloat("##radiusThickness", &params_.radiusThickness, 0.01f, 0.0f, 1.0f);
		});

		// boxSize
		DebugUi::PropatyRow("BoxSize", [&] {
			ImGui::DragFloat3("##boxSize", &params_.boxSize.x, 0.1f, 0.0f, FLT_MAX);
		});

		// direction
		DebugUi::PropatyRow("Direction", [&] {
			ImGui::DragFloat3("##direction", &params_.direction.x, 0.01f);
		});

		// angle
		DebugUi::PropatyRow("Angle", [&] {
			ImGui::DragFloat("##angle", &params_.angle, 0.01f, 0.0f, std::numbers::pi_v<float>);
		});

		// speed
		DebugUi::PropatyRow("Speed", [&] {
			float speed[2] = {params_.speedMin, params_.speedMax};
			if (ImGui::DragFloat2("##speed", speed, 0.1f)) {
				params_.speedMin = speed[0];
				params_.speedMax = speed[1];
			}
		});

		// acceleration
		DebugUi::PropatyRow("Acceleration", [&] {
			ImGui::DragFloat3("##acceleration", &params_.acceleration.x, 0.1f);
		});

		// lifetime
		DebugUi::PropatyRow("Lifetime", [&] {
			float lifetime[2] = {params_.lifetimeMin, params_.lifetimeMax};
			if (ImGui::DragFloat2("##lifetime", lifetime, 0.01f, 0.0f, FLT_MAX)) {
				params_.lifetimeMin = lifetime[0];
				params_.lifetimeMax = lifetime[1];
			}
		});

		// startSize
		DebugUi::PropatyRow("StartSize", [&] {
			ImGui::DragFloat2("##startSize", &params_.startSize.x, 0.05f, 0.0f, FLT_MAX);
		});

		// endSize
		DebugUi::PropatyRow("EndSize", [&] {
			ImGui::DragFloat2("##endSize", &params_.endSize.x, 0.05f, 0.0f, FLT_MAX);
		});

		// startColor
		DebugUi::PropatyRow("StartColor", [&] {
			ImGui::ColorEdit4("##startColor", &params_.startColor.x, ImGuiColorEditFlags_NoInputs);
		});

		// endColor
		DebugUi::PropatyRow("EndColor", [&] {
			ImGui::ColorEdit4("##endColor", &params_.endColor.x, ImGuiColorEditFlags_NoInputs);
		});

		// rollSpeed
		DebugUi::PropatyRow("RollSpeed", [&] {
			float rollSpeed[2] = {params_.rollSpeedMin, params_.rollSpeedMax};
			if (ImGui::DragFloat2("##rollSpeed", rollSpeed, 0.1f)) {
				params_.rollSpeedMin = rollSpeed[0];
				params_.rollSpeedMax = rollSpeed[1];
			}
		});

		// uv
		DebugUi::PropatyRow("Uv", [&] {
			ImGui::DragFloat4("##uv", &params_.uv.x, 0.01f, 0.0f, 1.0f);
		});

		DebugUi::EndTable();
	}

	// particleMgr
	if (particleMgr_) {
		ImGui::Text("ParticleManager is assigned!");
	} else {
		ImGui::Text("ParticleManager is not assigned.");
	}
#endif
}


void ParticleEmitter::Play() {
	isPlaying_ = true;
	elapsed_ = 0.0f;
}


void ParticleEmitter::Stop() {
	isPlaying_ = false;
}


void ParticleEmitter::Burst() {
	if (!particleMgr_ || burstCount_ <= 0) {
		return;
	}

	UpdateTransform();

	particleMgr_->Emit(params_, static_cast<size_t>(burstCount_));
}