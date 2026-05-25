#pragma once

// C++ Includes
#include <cinttypes>
#include <cmath>

// AML Includes
#include <mod/amlmod.h>
#include <mod/config.h>
#include <mod/logger.h>

// SDK Include
#include <aml-psdk/game_sa/plugin.h>

namespace Main {

struct ModConfig {
	float maxRotationSpeed; // Base rotation speed applied to the bike
	float phase1Multiplier; // Multiplier during the initial jump phase  (0s – 1s)
	float phase2Multiplier; // Multiplier during the mid-air phase	   (1s – 2s)
	float phase3Multiplier; // Multiplier during the falling phase	   (2s+)
	float rotationTransitionTime; // Time to reach target rotation speed
};

extern uintptr_t g_libGTASA;
extern ModConfig g_modConfig;

} // namespace BikeFlipFix
