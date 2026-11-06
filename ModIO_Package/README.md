# AbioticTrackIR: 6-DOF Head Tracking & Parallax 3D Mod for Abiotic Factor

**AbioticTrackIR** brings authentic Six Degrees of Freedom (6-DOF) optical head tracking to **Abiotic Factor** using **OpenTrack** (as an accessible substitute for specialized hardware) as well as official **TrackIR** devices.

In addition to full rotational and translational tracking, this mod features an off-axis **Parallax 3D Perspective Frustum Engine**: it dynamically computes asymmetrical projection frustums (`OffCenterProjectionOffset`) and depth-coupled FOV based on your physical head position, turning any flat monitor into a holographic glass window into the subterranean research laboratories of GATE Cascade!

---

## 🌟 Key Features

### 1. Six Degrees of Freedom (6-DOF) Tracking
* **Rotations:**
  * **Pitch:** Look up into ventilation shafts and down into toxic hazard pits.
  * **Yaw:** Glance left and right down facility corridors.
  * **Roll:** Tilt your head to inspect angles or peek through security barricades.
* **Translations:**
  * **Lateral ($X$):** Physically lean around door frames, file cabinets, and defensive shields.
  * **Surge / Depth ($Y$):** Lean in towards security terminals, microscopes, and crafting benches.
  * **Heave / Height ($Z$):** Stand tall to view over desk barriers or duck slightly beneath catwalks.

### 2. Authentic 6-DOF Head-Look (Freelook)
* **Independent Camera Control:** Optical head tracking controls your scientist's viewpoint and perspective while your mouse independently aims your weapon and reticle with steady, pinpoint precision (matching the authentic TrackIR experience in *Arma*, *DayZ*, *DCS*, and *Star Citizen*).
* **Enhanced Situational Awareness:** Freely scan facility corridors, look up into ceiling vents, and check corners without pulling your gun barrel or weapon crosshair off your target.
* **Free-Look Hotkey:** Hold **Left Alt** (configurable) to freely glance around independently of your scientist's body motion.

### 3. Holographic Parallax 3D Window (Johnny Lee / Desktop VR Mode)
* **Real Depth on Normal 2D Monitors:** Inspired by Johnny Chung Lee's classic desktop VR head tracking and modern holographic displays, your monitor acts like a physical glass window looking into the GATE Cascade Research Facility.
* **Convergence Depth Plane ($D_{\text{focal}}$):** As your head moves laterally or vertically, physical camera translation and off-axis frustum shearing cancel out at the configured focal depth ($120\text{ cm}$ default). Objects at this depth stay anchored to your physical monitor glass.
* **Natural Motion Parallax:** Moving your head peeks around doorways and reveals occluded background hallways, while viewmodels and held items pop forward into the room.
* **Tailored for OpenTrack Face Tracking (Optional for TrackIR):** Specifically designed for OpenTrack users using webcam face trackers (*neuralnet*). Press **F10** to toggle in-game, and dynamically adjust your focal convergence depth using **F9** (deeper into screen) and **F8** (closer to viewer). TrackIR / IR clip users can leave it disabled for pure, nausea-free 6-DOF head-look.

### 4. Dynamic ADS & Sniper Zoom Dampening
When aiming down sights (ADS) with magnified weaponry, tracking sensitivity automatically dampens proportionally to the active optical FOV, ensuring jitter-free precision for long-range engagements.

### 5. Biomechanical Cervical Neck Model
Human eyes do not rotate around the center of the ocular sphere—they pivot around the cervical vertebrae (~$8\text{ cm}$ behind and ~$12\text{ cm}$ below the eyes). An integrated kinematic model replicates natural ocular displacement, preventing disorientation and near-plane clipping.

### 6. Smart Interface & Dialogue Protection
* **Automatic Menu Pause:** Suspends head tracking and resets projection shear when accessing inventory, crafting stations, or terminal interfaces, preventing mouse cursor disruption.
* **Cutscene / Dialogue Lock:** Respects engine look-input locks during NPC dialogues and scripted sequences.

### 7. Universal Tracker Support (Hardware Agnostic)
Connects directly via high-speed **FreeTrack 2.0 Shared Memory (`FT_SharedMem`)** with mutex synchronization and non-blocking **UDP (Port 4242)**:
* **Standard Webcams:** Using OpenTrack's AI *neuralnet* face tracker.
* **Smartphone Cameras:** Via DroidCam or FreePIE UDP streaming.
* **Infrared Tracking Clips:** DelanClip, TrackHat, and DIY IR LEDs via *PointTracker*.
* **Official TrackIR Hardware:** Seamless operation with TrackIR 4 / 5 and TrackClip Pro via `NPClient64`.
* **Alternative Devices:** Tobii Eye Tracker, Aruco paper markers, and PS3 Eye.

### 8. Standalone Diagnostic Bridge Tool
Includes `AbioticTrackIR-Bridge.exe`, a standalone diagnostic utility that visualizes real-time rotation, translation, packet rates, and frustum calculations without needing to run the game.

---

## 🎮 Default Hotkeys

| Hotkey | Action | Description |
| :--- | :--- | :--- |
| **Home / F12** | **Recenter View** | Calibrates center rest pose (auto-calibrates on face lock; Home key bypasses Steam screenshot hook). |
| **F11** | **Toggle Tracking** | Enables or disables head tracking on the fly. |
| **F10** | **Toggle Parallax 3D** | Toggles holographic off-axis perspective window mode. |
| **F9** | **Focal Depth +** | Increases parallax convergence depth (+15 cm deeper into screen). |
| **F8** | **Focal Depth -** | Decreases parallax convergence depth (-15 cm closer to viewer). |
| **Left Alt** | **Free-Look** | Hold to freely look around decoupled from character body heading. |

*(All hotkeys are fully customizable in `AbioticTrackIR.ini`)*

---

## 🚀 Quick Setup with OpenTrack

1. **Launch OpenTrack** on your computer.
2. Select your preferred **Input** device:
   * *neuralnet* (recommended for standard webcams without markers/clips)
   * *PointTracker* (for 3-point IR clips or caps)
3. Set **Output** to:
   * **`freetrack 2.0 Enhanced`** (Recommended — lowest latency direct memory mapping)
   * OR **`UDP over network`** (Port: `4242`).
4. Click **Start** in OpenTrack.
5. Launch **Abiotic Factor**!
6. Sit comfortably in your natural playing posture — view auto-centers on active face lock, or press **Home** / **F12** at any time.
7. *(Parallax 3D Window)*: Enabled by default at authentic 1:1 scale; fine-tune depth with **F9** (deeper) / **F8** (closer), or press **F10** to toggle on/off.

---

## 📥 Installation

### Method A: Mod.io (Recommended)
1. Subscribe to **AbioticTrackIR** on the official [Abiotic Factor Mod.io](https://mod.io/g/abiotic-factor) page.
2. The game will automatically download and unpack the mod into your game files.
3. Run the game with OpenTrack running in the background.

### Method B: Manual Drop-In
1. Download the latest release package.
2. Copy the following files into your game's 64-bit binary directory:
   ```text
   <Game Installation Directory>\AbioticFactor\Binaries\Win64\
   ├── version.dll                (Zero-dependency proxy hook)
   ├── AbioticTrackIR.ini         (Configuration settings)
   └── AbioticTrackIR-Bridge.exe  (Standalone diagnostic tool)
   ```
3. Launch **Abiotic Factor** directly via Steam or shortcut.

---

## 🛠️ Configuration Guide (`AbioticTrackIR.ini`)

`AbioticTrackIR.ini` resides in the same directory as `version.dll` and can be adjusted at any time:

```ini
; =====================================================================
; Abiotic Factor - TrackIR & OpenTrack 6-DOF + Parallax 3D Mod Config
; =====================================================================

[General]
Enabled = true             ; Master switch for head tracking
AimCoupled = false         ; false = Independent head-look (mouse aims, head looks); true = Experimental aim steering
PauseInMenus = true        ; Suspends tracking while inventory/crafting cursor is visible
UdpPort = 4242             ; OpenTrack UDP port (when UDP mode is used)

[Sensitivity]
Yaw = 1.0                  ; Horizontal look sensitivity
Pitch = 1.0                ; Vertical look sensitivity
Roll = 0.8                 ; Head tilt sensitivity
X = 1.0                    ; Lateral lean sensitivity
Y = 1.0                    ; Forward / backward depth sensitivity
Z = 1.0                    ; Vertical elevation sensitivity
ScaleSensWithFOV = true    ; Dynamically damps sensitivity when aiming down sights (ADS)

[Limits]
; Keep camera 100% anchored inside the player's eye socket without mesh clipping
EnableTranslation = false
; Maximum physical translation in centimeters when enabled
MaxTranslationX_Cm = 8.0
MaxTranslationY_Cm = 8.0
MaxTranslationZ_Cm = 6.0

[Inversion]
InvertYaw = false
InvertPitch = false
InvertRoll = false
InvertX = false
InvertY = false
InvertZ = false

[Smoothing]
; Factor: 0.0 = raw / instantaneous, 0.7 = heavy smoothing
Factor = 0.35
DeadzoneDegrees = 0.1      ; Eliminates micro-tremors when resting head
DeadzoneTransCm = 0.05

[NeckModel]
; Simulates cervical neck pivot (disabled by default in FPS games to prevent camera clipping inside player mesh)
Enabled = false
PivotForwardCm = -1.5
PivotUpCm = -2.0

[Parallax3D]
; Holographic off-axis perspective projection (Johnny Lee desktop VR window effect)
; Anchors convergence to the physical monitor glass. Designed for OpenTrack face tracking.
; Enabled by default. Toggle in-game with F10. Tune depth plane with F9 (deeper) and F8 (closer).
Enabled = true
FocalDistanceCm = 100.0    ; Convergence depth (cm): objects at this depth remain anchored to the monitor
; Monitor display dimensions in centimeters (measure physical panel width & height):
; 24": 53.1 x 29.9 | 27": 59.7 x 33.6 | 32": 69.8 x 39.3 | 34" UW: 79.8 x 33.4
MonitorWidthCm = 59.7
MonitorHeightCm = 33.6
ViewingDistanceCm = 65.0   ; Normal distance from eyes to screen
Intensity = 1.0            ; 1.0 = authentic 1:1 holographic window depth illusion
DynamicFOV = false         ; Disabled by default to prevent disorienting dolly-zoom vertigo
MaxOffsetFrustum = 0.25    ; Edge distortion safety clamp

[Hotkeys]
; Simplified F-keys: Home/F12=Recenter, F11=Toggle Tracking, F10=Toggle Parallax, F9=Depth+, F8=Depth-
; Available keys: VK_F1..VK_F12, LeftAlt, LeftCtrl, LeftShift, MiddleMouse, Mouse4, Mouse5
Center = VK_F12
ToggleTracking = VK_F11
ToggleParallax = VK_F10
ParallaxDepthInc = VK_F9
ParallaxDepthDec = VK_F8
Freelook = VK_LMENU
```

---

## 🔒 Privacy & Safety Guarantee

* **100% Offline & Client-Side:** Operates strictly on local shared memory and local loopback UDP (`127.0.0.1`).
* **Zero Telemetry:** No external network requests, analytics, or remote data collection of any kind.
* **Non-Destructive Hooking:** Implemented via standard Windows `version.dll` export forwarding without modifying the game executable or patching game PAK files on disk.
* **Clean Uninstallation:** Simply remove `version.dll`, `AbioticTrackIR.ini`, and `AbioticTrackIR-Bridge.exe` from your binary folder to restore the game to its unmodified state.

---

## 📜 License & Credits

Distributed under the MIT License. Built for the community with compatibility for [OpenTrack](https://github.com/opentrack/opentrack) and official TrackIR protocols.
