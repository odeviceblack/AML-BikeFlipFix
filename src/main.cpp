// -------------------------------------------
// ------------- [ MOD STARTUP ] -------------
// -------------------------------------------
#include <mod/amlmod.h>
#include <mod/config.h>
#include <mod/logger.h>
#include <aml-psdk/game_sa/plugin.h>
#include <aml-psdk/game_sa/Events.h>
#include <inttypes.h>

MYMODCFGNAME(DeviceBlack.BikeFlipFix, BikeFlipFix, 1.1, DeviceBlack, BikeFlipFix)

uintptr_t g_libGTASA = 0;

static void* (*CPad_GetPad)(int) = nullptr;
static int (*CPad_GetSteeringUpDown)(void*) = nullptr;

void OnGameProcess(); // forward declaration (goto ln. 65)
float fRotationVelocity = 0.0f; // value coming from the configuration file

ON_MOD_LOAD() {
    g_libGTASA = aml->GetLib("libGTASA.so");

    if(g_libGTASA) {
        logger->Info("Game library loaded successfully! [0x%" PRIXPTR "]", g_libGTASA);
    } else {
        logger->Error("Failed to load the game library!");
        return;
    }

	fRotationVelocity = cfg->Bind("RotationVelocity", 0.00125f)->GetFloat();

	SET_TO(CPad_GetPad, aml->GetSym(g_libGTASA, "_ZN4CPad6GetPadEi"));
	SET_TO(CPad_GetSteeringUpDown, aml->GetSym(g_libGTASA, "_ZN4CPad17GetSteeringUpDownEv"));

    Events::gameProcessEvent += OnGameProcess;
}

// -------------------------------------------
// ---------- [ MODIFICATION CODE ] ----------
// -------------------------------------------
#include <aml-psdk/game_sa/base/Matrix.h>
#include <aml-psdk/game_sa/entity/PlayerPed.h>
#include <aml-psdk/game_sa/entity/Vehicle.h>

// Input
static inline int GetSteeringUpDown(int pad) {
	void* v_pad = CPad_GetPad(pad);
	if(!v_pad) return 0;

	return CPad_GetSteeringUpDown(v_pad);
}

// Math
static inline CVector LocalToWorld(CMatrix* m, const CVector& v) {
	return CVector(
		m->GetRight().x * v.x + m->GetForward().x * v.y + m->GetUp().x * v.z,
		m->GetRight().y * v.x + m->GetForward().y * v.y + m->GetUp().y * v.z,
		m->GetRight().z * v.x + m->GetForward().z * v.y + m->GetUp().z * v.z
	);
}

// Helpers
static inline bool IsValidVehicleForMod(CVehicle* v) {
	if(!v) return false;
	if(!v->IsSubBMX() && !v->IsSubBike()) return false;
	return true;
}

static inline bool HasGroundContact(CVehicle* v) {
	return v->GetNumContactWheels() != 0;
}

// Main mod logic
void OnGameProcess() {
	CPlayerPed* player = FindPlayerPed(0);
	if(!player) return;

	CVehicle* veh = FindPlayerVehicle(0, false);
	if(!IsValidVehicleForMod(veh)) return;

	// If it's on the ground → default game behavior
	if(HasGroundContact(veh)) {
		veh->m_nStatus = STATUS_PLAYER;
		return;
	}

	// Active physics to allow manipulation of turn speed.
	veh->m_nStatus = STATUS_PHYSICS;

	veh->m_vecTurnSpeed = LocalToWorld(
		veh->GetMatrix(),
		CVector(
			GetSteeringUpDown(0) * fRotationVelocity,
			0.0f, 0.0f
		)
	);
}
