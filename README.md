<p align="center"><img src="https://i.imgur.com/aZlfo4C.png" width="400"> <br/></p>

# TLDR: 
This is fork of [GTA2 Radar mod made by _AG](https://github.com/gennariarmando/gta2-radar), but compiled so it's compatible with v11.44. 
Plus various additional options were added from other GTA2 projects.

_GTA2 Radar from GTA3. This is a backport of GTA3's radar mini map to GTA2, made thanks to re3 project.
Some changes have been made in order to make it work correctly in GTA2 but the base code is still the same._

## Features:
 - Full Map for each districts and bonus levels.
 - Shows mission blips from gangs, churches, garages.
 - Shows current objectives and markers height levels.
 - Rectangular radar.
 - Larger radar on the paused current-level stats screen.
 - Low quality assets.
 - Ability to add new radar tiles for custom levels.
 - Pickup blips or icons on radar, based on [gta2-radar by Sektor](https://gtamp.com/forum/viewtopic.php?f=4&t=818).
 - Bigger pause screen radar

Experiment results:
<p align="center">
<img src="https://i.imgur.com/TjvKrYh.jpeg" width="300" height="300">
<img src="https://i.imgur.com/ts2VPXP.jpeg" height="300"> <br/>
</p>

## Configuration:
Create or edit `scripts\GTA2Radar.ini` to override defaults:

```ini
[GTA2Radar]
; Main feature switches
EnableBuiltinArrows=1           # Built-in navigation arrows: 0 = disable, 1 = enable, 2 = dynamic
EnablePickupBlips=1             # 0 = default radar mod behavior (paint/phones/...), 1 = show pickups
EnablePauseStatsRadar=0         # 0 = radar remain same during pause, 1 = radar uses pause radar settings from below 

; Gameplay radar settings
RadarBlipsSize=7.0

; Built-in arrow settings
DynamicArrowsDistance = 1.0     # 
ToggleDefaultArrows = 0x52      # Default 0x52 = R key (https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)

; Pause radar settings          # (looks fine on 1440p screen, but may need adjustment for other resolutions)
EnablePauseStatsLayout=1
PauseStatsTextY=52              # Shifts the pause stats text.
PauseStatsSpriteY=64            # Shifts the pause stats text sprite.
PauseRadarLeft=-1
PauseRadarBottom=100.0          # Pauses the radar offset from the bottom of the screen.
PauseRadarWidth=300.0
PauseRadarHeight=300.0
PauseRadarBlipsSize=5.5
PauseRadarRange=100.0           # How zoomed out the radar is.
; PauseRadarFramePadding=0.25
; PauseRadarAutoOffsetX=-256.0

; Pickup blip settings
EnablePickupIcons=1             # 1 = uses icons for pickups from data\hud\xxx.dds, 0 = colored markers
EnableVehiclePickupBlips=1      # VAN car with antenna pointing towards level start location (model ID 149)
EnableWeaponPickupBlips=1       # Shows weapon pickups (primarily models IDs 200–223)
EnableBonusPickupBlips=1        # Shows bonuses and power-ups (model IDs 228–240)
EnableTokenPickupBlips=1        # Shows collectible tokens (model ID 266)
EnableFrenzyPickupBlips=1       # Shows the kill-frenzy pickup (model ID 286)
EnableOtherPickupBlips=1        # Shows placed mines (ID 10) and PEW PEW text for bulled projectiles (ID 13, 56, 128, 138, 182, 183, 254, and 265)
PickupBlipMaxDistance=1.1
PickupIconReferenceSize=30.0

; Diagnostics
EnablePickupBlipLog=0
```

# Rest of original README.md 
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
