# GTA2 Radar From GTA3
This is a backport of GTA3's radar mini map to GTA2, made thanks to re3 project.
Some changes have been made in order to make it work correctly in GTA2 but the base code is still the same.

## References:

- [gta2-radar by Sektor](https://gtamp.com/forum/viewtopic.php?f=4&t=818) was used as a reference for this project.

Experiment results:
<p align="center">
<img src="https://i.imgur.com/TjvKrYh.jpeg" width="300" height="300">
<img src="https://i.imgur.com/ts2VPXP.jpeg" height="300"> <br/>
</p>


## Features:
 - Full Map for each districts and bonus levels.
 - Shows mission blips from gangs, churches, garages.
 - Shows current objectives and markers height levels.
 - Rectangular radar.
 - Larger radar on the paused current-level stats screen.
 - Low quality assets.
 - Ability to add new radar tiles for custom levels.

## Configuration:
Create or edit `scripts\GTA2Radar.ini` to override defaults:

```ini
; Main feature switches
EnableBuiltinArrows=0
EnablePickupBlips=1
EnablePauseStatsRadar=1

; Gameplay radar settings
RadarBlipsSize=7.0

; Built-in arrow settings
DynamicArrowsDistance=1.0

; Pause radar settings
PauseRadarLeft=-1
PauseRadarBottom=26.0
PauseRadarWidth=160.0
PauseRadarHeight=160.0
PauseRadarBlipsSize=4.0
PauseRadarRange=100.0
EnablePauseStatsLayout=1
PauseStatsTextY=52
PauseStatsSpriteY=64

; Pickup blip settings
EnablePickupIcons=1
EnableVehiclePickupBlips=1
EnableWeaponPickupBlips=1
EnableBonusPickupBlips=1
EnableTokenPickupBlips=1
EnableFrenzyPickupBlips=1
EnableOtherPickupBlips=1
PickupBlipMaxDistance=1.1
PickupIconReferenceSize=30.0

; Diagnostics
EnablePickupBlipLog=1
```

`RadarBlipsSize` controls all gameplay radar icons, including the original radar blips and pickup icons. `PauseRadarBlipsSize` controls those same icons while the pause radar is active.
`EnablePauseStatsRadar=1` shows the larger radar on the paused current-level stats screen. Set it to `0` to keep the game's default pause/stats screen behavior.
The gameplay radar uses `data\hud\radar_rect.dds`; the large pause radar uses `data\hud\radar_rect_256x256.dds` and falls back to the gameplay border if that file is missing.
Set `PauseRadarLeft=-1` or `PauseRadarBottom=-1` to auto-center that axis using the current WidescreenFix-expanded screen resolution.
`EnablePauseStatsLayout=1` moves the game's original current-level stats text and stats image/sprite while the large pause radar is enabled; adjust `PauseStatsTextY` and `PauseStatsSpriteY` if they overlap the pause radar. These layout values are ignored when `EnablePauseStatsRadar=0`.

Pickup blip groups are: vehicle (`149`), weapons (`200`-`223`), bonuses (`228`-`240`), token (`266`), frenzy (`286`), and other legacy object IDs.
When `EnablePickupIcons=1`, pickup blips use numeric HUD icons from `data\hud\<model>.dds` such as `data\hud\201.dds`. Missing icons automatically fall back to the colored marker.
`PickupIconReferenceSize` controls texture-size scaling. At the default `30.0`, a 30x30 icon uses the normal blip size, while wider or taller textures can extend beyond that square without being shrunk to fit it. Lower values make textured pickup icons larger; higher values make them smaller.

When pickup logging is enabled, scan diagnostics are written to `scripts\GTA2Radar.log`.
 
## Screenshots:
<p align="center">
<img src="https://i.imgur.com/TSIEHVP.png" width="320" height="180">
<img src="https://i.imgur.com/I1kLIPR.png" width="320" height="180"> <br/>
<img src="https://i.imgur.com/EremRPB.png" width="320" height="180"> 
</p>

## Compiling:
Requirements:
 - Visual Studio 2022
 - [Plugin SDK](https://github.com/DK22Pac/plugin-sdk)

## Download:
Download the latest archive from the [releases](https://github.com/gennariarmando/gta2-frontend-fix/releases) page.

# Installation:
#### Installing an ASI Loader:
An ASI Loader is required in order to inject the plugin into the game, if you already have one skip to the next step.\
Recommended: [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader)

Place the DLL file (renamed into "dinput.dll") into your GTA2 directory.

Requires [Widescreen Fix](https://thirteenag.github.io/wfp#gta2)

#### Installing GTA2 Radar:
Archive content: 
- GTA2Radar.asi
- data\hud folder
- data\radar folder

Paste the content of the "data" folder into your GTA2 "data" directory.\
Create a folder called "scripts" inside your GTA2 directory and paste GTA2Radar.asi in it.

## Links:
- [plugin-sdk](https://github.com/DK22Pac/plugin-sdk)
- [re3](https://github.com/GTAModding/re3)
