#include "mod.hpp"
#include "main.hpp"

#include <aml-psdk/game_sa/Events.h>
#include <aml-psdk/game_sa/base/Matrix.h>
#include <aml-psdk/game_sa/base/Timer.h>
#include <aml-psdk/game_sa/entity/PlayerPed.h>
#include <aml-psdk/game_sa/entity/Vehicle.h>

namespace Mod {

// ─── CPad bindings ───────────────────────────────────────────────────────────

class CPad;
static CPad* (*CPad_GetPad)(int) = nullptr;
static int (*CPad_GetSteeringUpDown)(void*) = nullptr;

static int GetSteeringInput() {
	if(!CPad_GetPad || !CPad_GetSteeringUpDown)
		return 0;

	CPad* pad = CPad_GetPad(0);
	return pad ? CPad_GetSteeringUpDown(pad) : 0;
}

// ─── Math helpers ────────────────────────────────────────────────────────────

static CVector LocalToWorld(const CMatrix* mat, const CVector& local) {
	return {
		mat->GetRight().x * local.x + mat->GetForward().x * local.y + mat->GetUp().x * local.z,
		mat->GetRight().y * local.x + mat->GetForward().y * local.y + mat->GetUp().y * local.z,
		mat->GetRight().z * local.x + mat->GetForward().z * local.y + mat->GetUp().z * local.z,
	};
}

static CVector WorldToLocal(const CMatrix* mat, const CVector& world) {
	return {
		mat->GetRight().x * world.x + mat->GetRight().y * world.y + mat->GetRight().z * world.z,
		mat->GetForward().x * world.x + mat->GetForward().y * world.y + mat->GetForward().z * world.z,
		mat->GetUp().x * world.x + mat->GetUp().y * world.y + mat->GetUp().z * world.z,
	};
}

static float Smoothstep(float current, float target, float t) {
	t = t < 0.f ? 0.f : (t > 1.f ? 1.f : t); // clamp
	t = t * t * (3.f - 2.f * t);  // hermite
	return current + (target - current) * t;
}

// ─── Vehicle helpers ─────────────────────────────────────────────────────────

static CVehicle* GetValidPlayerBike() {
	CVehicle* veh = FindPlayerVehicle(0, false);
	if(!veh) return nullptr;
	return (veh->IsSubBike() || veh->IsSubBMX()) ? veh : nullptr;
}

static bool IsBikeTipping(CVehicle* veh) {
	const CVector& up = veh->GetMatrix()->GetUp();
	return fabsf(up.x) > 0.1f || fabsf(up.y) > 0.1f;
}

static float GetPhaseMultiplier(float airTime) {
	const auto& cfg = Main::g_modConfig;
	if(airTime < 1.f) return cfg.phase1Multiplier;
	if(airTime < 2.f) return cfg.phase2Multiplier;
	return cfg.phase3Multiplier;
}

// ─── State ───────────────────────────────────────────────────────────────────

struct AirState {
	float airTime = 0.f;
	float currentSpeedX = 0.f;
	uint32_t lastTimeMs = 0;
	bool active	= false;

	void Reset() { *this = {}; }
};

static AirState s_air;

// ─── Main loop ───────────────────────────────────────────────────────────────

void OnGameProcess() {
	CVehicle* veh = GetValidPlayerBike();

	if(!veh) {
		s_air.Reset();
		return;
	}

	if(veh->GetNumContactWheels() != 0) {
		if(s_air.active)
			veh->m_nStatus = STATUS_PLAYER;

		s_air.Reset();
		return;
	}

	const int steering = GetSteeringInput();
	const bool isTipping = IsBikeTipping(veh);

	if(steering == 0 && !isTipping) {
		veh->m_nStatus = STATUS_PLAYER;
		s_air.Reset();
		return;
	}

	const uint32_t nowMS = CTimer::GetTimeInMS();
	const float	deltaMS = s_air.lastTimeMs ? float(nowMS - s_air.lastTimeMs) : 0.f;
	s_air.lastTimeMs = nowMS;
	s_air.active = true;

	s_air.airTime += deltaMS / 1000.f;

	const float multiplier = GetPhaseMultiplier(s_air.airTime);
	const float maxSpeed = Main::g_modConfig.maxRotationSpeed * multiplier;
	const float targetSpeedX = float(steering) * maxSpeed; // steering já tem sinal

	if(s_air.currentSpeedX == 0.f && deltaMS == 0.f) {
		s_air.currentSpeedX = WorldToLocal(veh->GetMatrix(), veh->m_vecTurnSpeed).x;
	}

	const float transitionMS = Main::g_modConfig.rotationTransitionTime * 1000.f; // config em segundos
	const float t = (transitionMS > 0.f) ? (deltaMS / transitionMS) : 1.f;

	s_air.currentSpeedX = Smoothstep(s_air.currentSpeedX, targetSpeedX, t);

	veh->m_nStatus = STATUS_PHYSICS;
	veh->m_vecTurnSpeed = LocalToWorld(veh->GetMatrix(), CVector(s_air.currentSpeedX, 0.f, 0.f));
}

// ─── Setup ───────────────────────────────────────────────────────────────────

void Setup() {
	SET_TO(CPad_GetPad, aml->GetSym(Main::g_libGTASA, "_ZN4CPad6GetPadEi"));
	SET_TO(CPad_GetSteeringUpDown, aml->GetSym(Main::g_libGTASA, "_ZN4CPad17GetSteeringUpDownEv"));

	if(!CPad_GetPad) {
		logger->Error("Symbol not found: CPad::GetPad");
		return;
	}

	if(!CPad_GetSteeringUpDown) {
		logger->Error("Symbol not found: CPad::GetSteeringUpDown");
		return;
	}

	Events::gameProcessEvent += OnGameProcess;
}

} // namespace Mod