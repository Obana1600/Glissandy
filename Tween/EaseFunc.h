#pragma once

#include <chrono>
#include <cmath>
#include <numbers>
#include <random>


namespace EaseFunc {

	// linear
	constexpr float Linear(float t) {
		return t;
	}


	// sine ---------------------------------------------------------------------------------------

	inline float InSine(float t) {
		return 1.0f - std::cos((t * std::numbers::pi_v<float>) / 2.0f);
	}

	inline float OutSine(float t) {
		return std::sin((t * std::numbers::pi_v<float>) / 2.0f);
	}

	inline float InOutSine(float t) {
		return -(std::cos(std::numbers::pi_v<float> * t) - 1.0f) / 2.0f;
	}

	// --------------------------------------------------------------------------------------------


	// quad ---------------------------------------------------------------------------------------

	constexpr float InQuad(float t) {
		return t * t;
	}

	constexpr float OutQuad(float t) {
		return 1.0f - (1.0f - t) * (1.0f - t);
	}

	constexpr float InOutQuad(float t) {
		const float n = -2.0f * t + 2.0f;
		return (t < 0.5f) ? 2.0f * t * t : 1.0f - (n * n) / 2.0f;
	}

	// --------------------------------------------------------------------------------------------


	// cubic --------------------------------------------------------------------------------------

	constexpr float InCubic(float t) {
		return t * t * t;
	}

	constexpr float OutCubic(float t) {
		const float n = 1.0f - t;
		return 1.0f - n * n * n;
	}

	constexpr float InOutCubic(float t) {
		const float n = -2.0f * t + 2.0f;
		return (t < 0.5f) ? 4.0f * t * t * t : 1.0f - (n * n * n) / 2.0f;
	}

	// --------------------------------------------------------------------------------------------


	// quart --------------------------------------------------------------------------------------

	constexpr float InQuart(float t) {
		return t * t * t * t;
	}

	constexpr float OutQuart(float t) {
		const float n = 1.0f - t;
		return 1.0f - n * n * n * n;
	}

	constexpr float InOutQuart(float t) {
		if (t < 0.5f) {
			return 8.0f * t * t * t * t;
		} else {
			const float n = -2.0f * t + 2.0f;
			return 1.0f - (n * n * n * n) / 2.0f;
		}
	}

	// --------------------------------------------------------------------------------------------


	// quint --------------------------------------------------------------------------------------

	constexpr float InQuint(float t) {
		return t * t * t * t * t;
	}

	constexpr float OutQuint(float t) {
		const float n = 1.0f - t;
		return 1.0f - n * n * n * n * n;
	}

	constexpr float InOutQuint(float t) {
		const float n = -2.0f * t + 2.0f;
		return (t < 0.5f) ? 16.0f * t * t * t * t * t : 1.0f - (n * n * n * n * n) / 2.0f;
	}

	// --------------------------------------------------------------------------------------------


	// expo ---------------------------------------------------------------------------------------

	inline float InExpo(float t) {
		return (t <= 0.0f) ? 0.0f : std::pow(2.0f, 10.0f * t - 10.0f);
	}

	inline float OutExpo(float t) {
		return (t >= 1.0f) ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
	}

	inline float InOutExpo(float t) {
		return t <= 0.0f   ? 0.0f
			   : t >= 1.0f ? 1.0f
			   : t < 0.5f  ? std::pow(2.0f, 20.0f * t - 10.0f) / 2.0f
						   : (2.0f - std::pow(2.0f, -20.0f * t + 10.0f)) / 2.0f;
	}

	// --------------------------------------------------------------------------------------------


	// circ ---------------------------------------------------------------------------------------

	inline float InCirc(float t) {
		return 1.0f - std::sqrt(1.0f - t * t);
	}

	inline float OutCirc(float t) {
		const float n = t - 1.0f;
		return std::sqrt(1.0f - n * n);
	}

	inline float InOutCirc(float t) {
		const float a = 2.0f * t;
		const float b = -2.0f * t + 2.0f;
		return t < 0.5f
				   ? (1.0f - std::sqrt(1.0f - a * a)) / 2.0f
				   : (std::sqrt(1.0f - b * b) + 1.0f) / 2.0f;
	}

	// --------------------------------------------------------------------------------------------


	// elastic ------------------------------------------------------------------------------------

	inline float InElastic(float t) {
		constexpr float c4 = (2.0f * std::numbers::pi_v<float>) / 3.0f;

		return t <= 0.0f
				   ? 0.0f
			   : t >= 1.0f
				   ? 1.0f
				   : -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * c4);
	}

	inline float OutElastic(float t) {
		constexpr float c4 = (2.0f * std::numbers::pi_v<float>) / 3.0f;

		return t <= 0.0f
				   ? 0.0f
			   : t >= 1.0f
				   ? 1.0f
				   : std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f;
	}

	inline float InOutElastic(float t) {
		constexpr float c5 = (2.0f * std::numbers::pi_v<float>) / 4.5f;

		return t <= 0.0f
				   ? 0.0f
			   : t >= 1.0f
				   ? 1.0f
			   : t < 0.5f
				   ? -(std::pow(2.0f, 20.0f * t - 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) / 2.0f
				   : (std::pow(2.0f, -20.0f * t + 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
	}

	// --------------------------------------------------------------------------------------------


	// back ---------------------------------------------------------------------------------------

	constexpr float InBack(float t) {
		constexpr float c1 = 1.70158f;
		constexpr float c3 = c1 + 1.0f;

		return c3 * t * t * t - c1 * t * t;
	}

	constexpr float OutBack(float t) {
		constexpr float c1 = 1.70158f;
		constexpr float c3 = c1 + 1.0f;

		const float n = t - 1.0f;
		return 1.0f + c3 * n * n * n + c1 * n * n;
	}

	constexpr float InOutBack(float t) {
		constexpr float c1 = 1.70158f;
		constexpr float c2 = c1 * 1.525f;

		const float a = 2.0f * t;
		const float b = 2.0f * t - 2.0f;
		return t < 0.5f
				   ? (a * a * ((c2 + 1.0f) * a - c2)) / 2.0f
				   : (b * b * ((c2 + 1.0f) * b + c2) + 2.0f) / 2.0f;
	}

	// --------------------------------------------------------------------------------------------


	// bounce -------------------------------------------------------------------------------------

	constexpr float OutBounce(float t) {
		constexpr float n1 = 7.5625f;
		constexpr float d1 = 2.75f;

		if (t < 1.0f / d1) {
			return n1 * t * t;
		} else if (t < 2.0f / d1) {
			const float u = t - 1.5f / d1;
			return n1 * u * u + 0.75f;
		} else if (t < 2.5f / d1) {
			const float u = t - 2.25f / d1;
			return n1 * u * u + 0.9375f;
		} else {
			const float u = t - 2.625f / d1;
			return n1 * u * u + 0.984375f;
		}
	}

	constexpr float InBounce(float t) {
		return 1.0f - OutBounce(1.0f - t);
	}

	constexpr float InOutBounce(float t) {
		return t < 0.5f
				   ? (1.0f - OutBounce(1.0f - 2.0f * t)) / 2.0f
				   : (1.0f + OutBounce(2.0f * t - 1.0f)) / 2.0f;
	}

	// --------------------------------------------------------------------------------------------


	// random
	inline float Random(float t) {
		static std::mt19937 gen(static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count()));
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		return t < 1.0 ? dist(gen) : 1.0f;
	}

} // namespace EaseFunc