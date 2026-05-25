# BikeFlipFix

A GTA San Andreas Android mod that restores proper mid-air pitch control for bikes and BMXs.

## Overview

In the Android version of GTA San Andreas, bikes and BMXs lose the ability to pitch forward or backward while airborne — stick input for flipping is simply ignored mid-air. **BikeFlipFix** corrects this by hooking into the game's physics loop and applying the player's vertical steering input as rotational force whenever the vehicle leaves the ground, with smooth interpolation and a three-phase air control system.

https://github.com/user-attachments/assets/b1cb0043-64b4-4bda-975a-1b2c87fa1636

## How It Works

Every game frame, the mod evaluates the following conditions:

1. **Is the player riding a bike or BMX?**
2. **Is the vehicle airborne?** (no wheels in contact with the ground)
3. **Is the player steering, or is the bike tipping over?**

If the conditions are met, the vehicle's status is switched to `STATUS_PHYSICS`, enabling direct manipulation of its turn speed. The rotation is smoothly interpolated toward the target speed using hermite smoothstep, and scaled by a phase multiplier that changes based on how long the bike has been airborne.

As soon as the vehicle touches the ground again, control returns to `STATUS_PLAYER` and vanilla behavior resumes — no side effects on normal riding.

```
Ground contact  →  STATUS_PLAYER   (vanilla behavior)
Airborne        →  STATUS_PHYSICS  →  smoothed pitch from stick input
```

### Air Phases

The mod divides airborne time into three phases, each with its own rotation multiplier:

| Phase | Time range | Config key |
|---|---|---|
| Phase 1 | 0s – 1s | `Phase1Multiplier` |
| Phase 2 | 1s – 2s | `Phase2Multiplier` |
| Phase 3 | 2s+     | `Phase3Multiplier` |

## Requirements

| Dependency | Notes |
|---|---|
| [Android Mod Loader (AML)](https://github.com/AndroidModLoader/AndroidModLoader) | Required mod loader |
| [AML Plugin-SDK](https://github.com/AndroidModLoader/aml-psdk) | Plugin SDK providing game structs and events |
| GTA San Andreas Android | Targeting the standard `libGTASA.so` offsets |

## Installation

1. Install **Android Mod Loader** on your device.
2. Place the compiled `.so` file in your AML mods folder: `Android/data/com.rockstargames.gtasa/mods/`
3. Launch the game. The mod will confirm it loaded correctly via the AML logger.

## Configuration

All settings are controlled via `BikeFlipFix.ini`, found in the AML config folder: `Android/data/com.rockstargames.gtasa/config/`

```ini
[Preferences]
; Base speed of rotation of the motorcycle in the air.
; Higher values = faster and more aggressive rotation.
; Lower values = slower and smoother rotation.
MaxRotationSpeed=0.002000

; Spin multiplier at the start of the jump (0s – 1s).
; Higher values = stronger rotation right after leaving the ground.
; Lower values = weaker rotation at the beginning.
Phase1Multiplier=1.000000

; Rotation multiplier in mid-air (1s – 2s).
; Higher values = maintain rotational force in medium jumps.
; Lower values indicate that rotation decreases as the jump lengthens.
Phase2Multiplier=0.300000

; Spin multiplier on fall (2s+).
; Higher values = more control in long jumps/falls.
; Lower values mean less control in prolonged falls.
Phase3Multiplier=0.500000

; Time (in seconds) to reach the target rotation speed.
; Higher values = slower and smoother transition.
; Lower values = more immediate response to the input (0.001 = no smoothing).
RotationTransitionTime=0.300000
```

| Key | Default | Description |
|---|---|---|
| `MaxRotationSpeed` | `0.002` | Base rotation speed while airborne. |
| `Phase1Multiplier` | `1.0` | Rotation multiplier from 0s to 1s in the air. |
| `Phase2Multiplier` | `0.3` | Rotation multiplier from 1s to 2s in the air. |
| `Phase3Multiplier` | `0.5` | Rotation multiplier after 2s in the air. |
| `RotationTransitionTime` | `0.3` | Smoothing time in seconds to reach target rotation speed. |

Changes take effect on the next game launch — no recompilation needed.

## Technical Reference

| Symbol | Purpose |
|---|---|
| `GetSteeringInput()` | Reads raw vertical axis input from pad 0 |
| `LocalToWorld(matrix, vector)` | Transforms a local-space vector into world space using the vehicle's rotation matrix |
| `WorldToLocal(matrix, vector)` | Inverse transform — world space into local vehicle space |
| `IsBikeTipping(veh)` | Returns `true` if the bike's up vector is tilting beyond threshold |
| `GetValidPlayerBike()` | Returns the player's current vehicle only if it's a bike or BMX |
| `Smoothstep(current, target, t)` | Hermite interpolation between two values |
| `GetPhaseMultiplier(airTime)` | Returns the active multiplier based on time spent airborne |
| `OnGameProcess()` | Hooked into `Events::gameProcessEvent`; runs every game tick |

## Credits

**DeviceBlack** — Android mod author.  
**Nadalao** — Original PC CLEO version.

This mod was independently developed for Android after coming across Nadalao's PC CLEO script. His implementation served as the original inspiration, though the Android version was written from scratch with a different approach. Full credit to him for the original idea.

## License

This project is released under the [MIT License](LICENSE) — free to use, modify, and redistribute, provided credit is given to the original author.