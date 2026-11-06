# AbioticTrackIR: 6-DOF Head Tracking & Parallax 3D Mod for Abiotic Factor

**AbioticTrackIR** brings full Six Degrees of Freedom (6-DOF) head tracking to **Abiotic Factor** using **OpenTrack** (as a substitute for official hardware) as well as official **TrackIR** devices.

In addition to standard pitch, yaw, roll, and lateral/depth translations, this mod features a complete **Parallax 3D Perspective Engine** (the Side Quest!): it dynamically calculates asymmetrical off-axis projection frustums (`OffCenterProjectionOffset`) and depth-coupled FOV based on your physical head position, turning your flat monitor into an authentic holographic glass window into the subterranean laboratories of GATE Cascade!

---

## 🌟 Key Features

### 1. Complete 6-DOF Head Tracking
* **Rotations:** Pitch (look up/down), Yaw (look left/right), Roll (tilt head side-to-side).
* **Translations:**
  * **X (Lateral):** Physically lean around door frames, file cabinets, and barricades.
  * **Y (Surge / Depth):** Lean in towards terminals, crafting benches, and microscopes.
  * **Z (Heave / Height):** Stretch up to see over obstacles or duck down slightly.

### 2. Built for OpenTrack & TrackIR
* **Native OpenTrack Support:** Connects directly via high-speed **FreeTrack 2.0 Shared Memory (`FT_SharedMem`)** and non-blocking **UDP (Port 4242)**.
* **OpenTrack Hardware Flexibility:** Use whatever input you have:
  * Standard Webcams (via OpenTrack *neuralnet* face tracker)
  * Phone Cameras (via DroidCam / FreePIE UDP)
  * IR Headsets & Clips (TrackHat, DelanClip, DIY IR LEDs via PointTracker)
  * Aruco paper markers
  * Tobii Eye Tracker / PS3 Eye
* **Official TrackIR Compatibility:** Works seamlessly with TrackIR 4 / 5 and TrackClip Pro via `NPClient64`.

### 3. Biomechanical Cervical Neck Model
Real human eyes do not rotate around the eyeball center—the head pivots around the cervical spine (~8 cm behind and ~12 cm below the eyes). AbioticTrackIR incorporates an anthropomorphic neck model so rotating your head naturally displaces your eyes, eliminating clipping and motion sickness.

### 4. Aim Decoupling & Independent Free-Look
* **Aim-Coupled Mode:** Direct, intuitive view tracking.
* **Decoupled Free-Look Mode:** Hold **Left Alt** (configurable) to look around freely in 360° while your scientist character continues walking, sprinting, or holding items forward.

### 5. Parallax 3D Frustum Engine (The Depth Illusion)
By coupling your real-world eye coordinates with Unreal Engine 5's projection matrix:
* **Off-Axis Projection Frustum:** The camera shears the viewing frustum opposite to your head offset. Near objects (weapon viewmodel, desk edges, railings) shift faster than distant walls, creating undeniable real stereoscopic motion parallax on a normal 2D monitor.
* **Dynamic Focal Distance / FOV:** Leaning closer to the screen naturally widens your field of view, exactly like peering closer into a real window.

---

## 🎮 Default Hotkeys

| Hotkey | Action | Description |
| :--- | :--- | :--- |
| **F12** | **Recenter / Calibrate** | Immediately zeroes current head posture as the center forward view. |
| **F11** | **Toggle Tracking** | Toggles head tracking on or off on the fly. |
| **F10** | **Toggle Parallax 3D** | Toggles off-axis asymmetrical perspective frustum. |
| **Left Alt** | **Free-Look** | Hold to look around freely decoupled from your character's body. |

*(All hotkeys are customizable in `AbioticTrackIR.ini`)*

---

## 🚀 Quick Setup with OpenTrack

1. **Launch OpenTrack** (installed on your PC).
2. Set **Input** to your preferred tracker (e.g., *neuralnet* for webcam, or *PointTracker* for IR clip).
3. Set **Output** to:
   * **`freetrack 2.0 Enhanced`** (Recommended - uses high-speed direct memory mapping)
   * OR **`UDP over network`** (Port: `4242`).
4. Click **Start** in OpenTrack.
5. Launch **Abiotic Factor**!
6. Press **F12** while sitting comfortably to calibrate your center view.

---

## 🛠️ Configuration (`AbioticTrackIR.ini`)

Located in the game directory or mod directory:

```ini
[General]
Enabled = true
UdpPort = 4242

[Sensitivity]
Yaw = 1.0
Pitch = 1.0
Roll = 0.8
X = 1.0
Y = 1.0
Z = 1.0

[Smoothing]
Factor = 0.35             ; 0.0 = raw, 0.7 = heavy smoothing
DeadzoneDegrees = 0.1     ; Eliminates micro-tremors when resting head
DeadzoneTransCm = 0.05

[NeckModel]
Enabled = true
PivotForwardCm = -8.0
PivotUpCm = -12.0

[Parallax3D]
Enabled = true
; Monitor width & height in cm (measure screen panel):
; 24" = 53.1 x 29.9 | 27" = 59.7 x 33.6 | 32" = 69.8 x 39.3 | 34" UW = 79.8 x 33.4
MonitorWidthCm = 59.7
MonitorHeightCm = 33.6
ViewingDistanceCm = 65.0
Intensity = 1.0
DynamicFOV = true
MaxOffsetFrustum = 0.85

[Hotkeys]
Center = VK_F12
ToggleTracking = VK_F11
ToggleParallax = VK_F10
Freelook = VK_LMENU
```

---

## 📦 Publishing to Mod.io

1. Zip the contents of the `ModIO_Package` directory into `AbioticTrackIR.zip`.
2. Ensure `AbioticTrackIR.uplugin` is at the root level of the ZIP file.
3. Go to [https://mod.io/g/abiotic-factor](https://mod.io/g/abiotic-factor).
4. Click **Add Mod**, enter title: `AbioticTrackIR - 6-DOF Head Tracking & Parallax 3D`.
5. Upload `AbioticTrackIR.zip` and publish!
