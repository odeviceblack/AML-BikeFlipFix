# BikeFlipFix

A GTA San Andreas Android mod that restores proper mid-air pitch control for bikes and BMXs.

## Overview

In the Android version of GTA San Andreas, bikes and BMXs lose the ability to pitch forward or backward while airborne — stick input for flipping is simply ignored mid-air. **BikeFlipFix** corrects this by hooking into the game's physics loop and directly applying the player's vertical steering input as rotational force whenever the vehicle leaves the ground.

## How It Works

Every game frame, the mod evaluates two conditions:

1. **Is the player riding a bike or BMX?**
2. **Is the vehicle airborne?** (no wheels in contact with the ground)

If both are true, the vehicle's status is switched to `STATUS_PHYSICS`, enabling direct manipulation of its turn speed. The vertical axis of the left stick is then transformed from local vehicle space into world space and applied as a pitch rotation vector.

As soon as the vehicle touches the ground again, control returns to `STATUS_PLAYER` and vanilla behavior resumes — no side effects on normal riding.

```
Ground contact  →  STATUS_PLAYER   (vanilla behavior)
Airborne        →  STATUS_PHYSICS  →  pitch from stick input
```

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

Pitch sensitivity is controlled via `BikeFlipFix.ini`, found in the AML config folder: `Android/data/com.rockstargames.gtasa/config/`

```ini
[Preferences]
RotationVelocity=0.00125
```

| Key | Default | Description |
|---|---|---|
| `RotationVelocity` | `0.00125` | Pitch force applied per frame while airborne. Raise for snappier flips; lower for more gradual rotation. |

Changes take effect on the next game launch — no recompilation needed.

## Technical Reference

| Symbol | Purpose |
|---|---|
| `GetSteeringUpDown(pad)` | Reads raw vertical axis input from the given controller pad |
| `LocalToWorld(matrix, vector)` | Transforms a local-space vector into world space using the vehicle's matrix |
| `IsValidVehicleForMod(veh)` | Returns `true` only for bikes and BMXs |
| `HasGroundContact(veh)` | Returns `true` if at least one wheel is touching the ground |
| `OnGameProcess()` | Hooked into `Events::gameProcessEvent`; runs every game tick |

## Credits

**DeviceBlack** — Android mod author.  
**Nadalao** — Original PC CLEO version.

This mod was independently developed for Android after coming across Nadalao's PC CLEO script. His implementation served as the original inspiration, though the Android version was written from scratch with a different approach. Full credit to him for the original idea.

## License

This project is released under the [MIT License](LICENSE) — free to use, modify, and redistribute, provided credit is given to the original author.
