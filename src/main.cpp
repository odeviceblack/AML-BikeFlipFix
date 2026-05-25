#include "main.hpp"
#include "mod.hpp"

MYMODCFGNAME(DeviceBlack.BikeFlipFix, BikeFlipFix, 2.0, DeviceBlack, BikeFlipFix)

namespace Main {

uintptr_t g_libGTASA = 0;
ModConfig g_modConfig;

static void LoadConfig() {
	g_modConfig.maxRotationSpeed = cfg->Bind("MaxRotationSpeed", 0.002f)->GetFloat();
	g_modConfig.phase1Multiplier = cfg->Bind("Phase1Multiplier", 1.0f)->GetFloat();
	g_modConfig.phase2Multiplier = cfg->Bind("Phase2Multiplier", 0.3f)->GetFloat();
	g_modConfig.phase3Multiplier = cfg->Bind("Phase3Multiplier", 0.5f)->GetFloat();
	g_modConfig.rotationTransitionTime = cfg->Bind("RotationTransitionTime", 0.3f)->GetFloat();

	// Clamp para evitar valores inválidos
	if(g_modConfig.maxRotationSpeed <= 0.f) g_modConfig.maxRotationSpeed = 0.0f;
	if(g_modConfig.phase1Multiplier <= 0.f) g_modConfig.phase1Multiplier = 0.0f;
	if(g_modConfig.phase2Multiplier <= 0.f) g_modConfig.phase2Multiplier = 0.0f;
	if(g_modConfig.phase3Multiplier <= 0.f) g_modConfig.phase3Multiplier = 0.0f;
	if(g_modConfig.rotationTransitionTime < 0.f) g_modConfig.rotationTransitionTime = 0.0f;

	logger->Info(
		"Config loaded — MaxRotationSpeed: %.5f | Phases: %.2f / %.2f / %.2f | TransitionTime: %.2f",
		g_modConfig.maxRotationSpeed,
		g_modConfig.phase1Multiplier,
		g_modConfig.phase2Multiplier,
		g_modConfig.phase3Multiplier,
		g_modConfig.rotationTransitionTime
	);
}

ON_MOD_LOAD() {
	g_libGTASA = aml->GetLib("libGTASA.so");

	if(!g_libGTASA) {
		logger->Error("Failed to load the game library!");
		return;
	}

	logger->Info("Game library loaded successfully! [0x%" PRIXPTR "]", g_libGTASA);

	LoadConfig();
	Mod::Setup();
}

} // namespace Main