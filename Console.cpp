#include "Console.hpp"

namespace UTILS {
	float IntensityHigh = 0.8f;
	float IntensityLow = 0.4f;

	bool* DevFlag = (bool*)((uintptr_t)GetModuleHandle(NULL) + 0x128D8D8);

	ImVec4 colorFromEnum(UTILS::ConColor color) {
		ImVec4 result = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

		float intensity = (color & FOREGROUND_INTENSITY) ? UTILS::IntensityHigh : UTILS::IntensityLow;

		if (color & FOREGROUND_RED) result.x += 1.0 * intensity;
		if (color & FOREGROUND_GREEN) result.y += 1.0 * intensity;
		if (color & FOREGROUND_BLUE) result.z += 1.0 * intensity;

		return result;
	}
}
