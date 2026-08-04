# BM176 GUI Cleanup — Implementation Spec

**Target branch:** `V4`
**Status of repo at time of writing:** clean, at `a4aff12` (`chore: untrack build_linux artifacts`)

---

## Context

The V4 GUI renders a procedural 1920×354 recreation of the BM176 front panel. Visually it
is close, but it feels clunky to use. A read-through of the GUI layer found that most of
the "clunk" is not cosmetic — it is a set of real interaction and plumbing defects:

- Discrete knobs become permanently undraggable after the first click.
- Knob positions are never driven by their parameters, so automation, preset recall and
  DAW undo do not move the GUI, and reopening the editor shows hardcoded defaults.
- No automation gestures are sent for any knob.
- The APVTS listener mutates components from whatever thread wrote the parameter, which
  can be the audio thread.
- Three knobs' pointers do not line up with the legends printed underneath them.
- The entire 1920×354 vector panel is re-rendered on every repaint.

This spec covers fixing those. **DSP is out of scope** — do not touch `Source/dsp/` or
`processBlock`. Parameter IDs, ranges and defaults in `createParameterLayout()` must not
change; hosts already have sessions saved against them.

### Ground rules

- All child components are laid out in **design space** (1920×354). Never introduce
  screen-pixel coordinates into `BM176Editor::resized()` or component paint code.
- Keep the procedural look. No changes to gradients, colours or artwork geometry except
  where a work item explicitly says so.
- `Source/GUI/BM176Colours.h` and `BM176Geometry.h` are the only homes for constants.
  Do not inline new hex values or magic coordinates into paint code.
- Work items are ordered. **Item 0 must be verified before anything else**, because it
  may change what the other items look like on screen.

### Build note

`CMakeLists.txt` expects JUCE at `../JUCE` (`/home/christian/PluginDev/JUCE`). That
directory does not currently exist on this machine — the previous build used **JUCE
8.0.14**. Restore it before starting; nothing here can be compile-verified without it.

```bash
cmake -B build_linux -DCMAKE_BUILD_TYPE=Release
cmake --build build_linux --target BM176_Standalone -j
./build_linux/BM176_artefacts/Release/Standalone/BM176
```

`build_linux/` and `build_vs/` are now gitignored — do not re-add them.

---

## Item 0 — Editor scaling double-applies (verify first)

**File:** `Source/PluginEditor.cpp:35-48`

`resized()` sets *both* a scale transform and already-scaled bounds:

```cpp
editor.setTransform(juce::AffineTransform::scale(scale));
editor.setBounds(roundToInt(ox), roundToInt(oy), roundToInt(w), roundToInt(h));
```

In JUCE, `setBounds` sets a component's *logical* bounds and the transform then maps them
into the parent. Passing already-scaled dimensions to `setBounds` therefore applies the
scale twice, and — more importantly — shrinks the coordinate space that children and the
panel artwork are clipped to. `BM176Editor`'s constructor sets `setSize(1920, 354)`, but
`resized()` immediately overwrites that with the scaled size, and
`panel.setBounds(getLocalBounds())` inherits it.

### Confirmed by screenshot (2026-08-04)

Two standalone screenshots at different window sizes confirm the defect and bound it:

**Small window — panel artwork is clipped.** Missing everything past roughly design
x≈1200 and design y≈200:

- both right-hand screws (design 1886, 22 / 1886, 332)
- the `ATTACK`, `VERNIER`, `OUTPUT`, `RELEASE` legends (1364 / 1555 / 1734)
- `INTERSTAGE` / `IN` / `ATTACK` / `OFF` (1253) and `IN` / `BYPASS` / `ON` (1848)
- every scale ring, numeral and dot for the Attack, Output and Release knobs
- the `bma` wordmark (977, 296) and `LIMITING AMPLIFIER BM 176` (977, 336)
- `SIDECHAIN` (196, 194) and the left `VERNIER` (567, 196) — i.e. it clips vertically too

**Large window — renders correctly.** At a window wider than 1920 the entire panel is
present, including everything listed above. So the clip boundary scales with window size
and only bites when the window is smaller than design size, which is the normal case
(default is 1440×266).

**What the screenshots do *not* explain.** In the small window the knobs, switches, VU,
jacks and LED all still render, at correct design-relative positions, across the full
panel width — including the Release knob at design x=1734 and the ON lamp at 1807, both
well past where the artwork stops. Under a plain double-scale those children should be
clipped too. So the exact mechanism is not fully pinned down from screenshots alone.

**Do this before writing the fix.** Confirm the mechanism empirically rather than trusting
the reasoning above — log the real numbers once in `BM176Editor::resized()`:

```cpp
DBG("editor logical: " << getWidth() << "x" << getHeight()
    << "  panel: "     << panel.getWidth() << "x" << panel.getHeight());
```

and in `BM176AudioProcessorEditor::resized()` log `getWidth()`, `getHeight()` and `scale`.
Resize the standalone small and large and compare. You are looking for whether the
editor's logical width is ~1920 (correct) or ~`window width × scale` (the bug). Remove the
logging before committing.

**Fix** — logical bounds stay at design size; the transform does both scale and centring:

```cpp
void BM176AudioProcessorEditor::resized()
{
    const float scale = juce::jmin(getWidth()  / (float) bm176::DESIGN_WIDTH,
                                   getHeight() / (float) bm176::DESIGN_HEIGHT);
    const float ox = (getWidth()  - bm176::DESIGN_WIDTH  * scale) * 0.5f;
    const float oy = (getHeight() - bm176::DESIGN_HEIGHT * scale) * 0.5f;

    editor.setBounds(0, 0, bm176::DESIGN_WIDTH, bm176::DESIGN_HEIGHT);
    editor.setTransform(juce::AffineTransform::scale(scale).translated(ox, oy));
}
```

`BM176Geometry.h` already has `getScaleTransform()` and `getDesignBounds()` helpers that
compute the same scale/offset — neither is called anywhere. Either use `getDesignBounds()`
here for the offsets or delete both helpers as dead code; don't leave them unused.

**Verify — test small first.** The bug only shows below design size, so a large window
proves nothing. Check at the 720 px minimum, at the 1440 default, at 1920, and at the 2560
maximum. At every size: all four screws present, all legends and scale rings present, the
bottom `bma` wordmark and `LIMITING AMPLIFIER BM 176` present, panel centred with equal
letterbox on both sides, nothing clipped.

If Item 0 turns out to also have been suppressing part of the panel that other items
touch, re-screenshot before starting Item 4 — the "identical to before" comparison there
must be against the *fixed* rendering, not the clipped one.

---

## Item 1 — `BMKnob` interaction

**Files:** `Source/Components/BMKnob.h`, `Source/Components/BMKnob.cpp`

### 1a. Discrete knobs are permanently undraggable (primary bug)

`mouseDown` sets `snappedToDetent = true` (`BMKnob.cpp:78`) and **nothing anywhere ever
sets it back to false**. `mouseDrag` returns early on it (`:84`). So after one click on
Ratio, Sidechain, Meter, Input or Output, that knob can never be dragged again for the
life of the editor.

`mouseDown` also changes the value on *any* click — top half increments, bottom half
decrements (`:72-79`) — so there is no way to click a discrete knob without altering it.

**Fix:** delete `snappedToDetent` entirely (member at `BMKnob.h:64`). Discrete knobs drag
like continuous ones; `snapValue()` already quantises them inside `setValue()`, so
stepping falls out for free. Clicking without dragging changes nothing.

### 1b. New callback surface

`BMKnob` needs to report gesture boundaries so the binder (Item 2) can wrap host
automation gestures, and needs a way to be set programmatically without echoing back.

Add to `BMKnob.h`:

```cpp
std::function<void()>      onDragStart;
std::function<void(float)> onValueChange;   // already exists
std::function<void()>      onDragEnd;

void setValue(float newValue,
              juce::NotificationType n = juce::sendNotification);
void setDefaultValue(float v);              // in knob value space
```

`setValue` fires `onValueChange` only when `n != juce::dontSendNotification`. The existing
`approximatelyEqual` guard stays — it is what stops host↔GUI feedback loops from running
away.

Add a `float defaultValue = 5.0f;` member.

### 1c. Mouse handling

Replace `mouseDown`/`mouseDrag`, add `mouseUp`:

```cpp
void BMKnob::mouseDown(const juce::MouseEvent& e)
{
    dragStartValue = value;
    e.source.enableUnboundedMouseMovement(true);
    setMouseCursor(juce::MouseCursor::NoCursor);
    if (onDragStart) onDragStart();
}

void BMKnob::mouseDrag(const juce::MouseEvent& e)
{
    float sensitivity = dragValueRange() / DRAG_PIXELS_FULL_RANGE;
    if (e.mods.isShiftDown() || e.mods.isCommandDown() || e.mods.isCtrlDown())
        sensitivity *= 0.2f;

    setValue(dragStartValue - e.getDistanceFromDragStartY() * sensitivity);
}

void BMKnob::mouseUp(const juce::MouseEvent&)
{
    setMouseCursor(juce::MouseCursor::NormalCursor);
    juce::Desktop::getInstance().getMainMouseSource()
        .setScreenPosition(localPointToGlobal(getLocalBounds().toFloat().getCentre()));
    if (onDragEnd) onDragEnd();
}
```

- `enableUnboundedMouseMovement(true)` + hiding the cursor is what stops the pointer
  running off the screen edge mid-drag. `mouseUp` warps it back to the knob centre, which
  is the standard plugin behaviour.
- `getDistanceFromDragStartY()` is relative to the mouse-down point, so `dragStartY`
  becomes unused — delete the member.
- Add `dragValueRange()`, returning `2.0f` for vernier knobs and `maxVal - minVal`
  otherwise, and a `DRAG_PIXELS_FULL_RANGE = 300.0f` constant.

Note the drag distance is in *design* pixels — because the editor is transformed, a
300 design-pixel drag is 300 × scale screen pixels. At the default 0.75 scale that is
225 screen px for a full sweep, which feels about right. Tune the constant against the
built app, not on paper.

### 1d. Wheel step

`mouseWheelMove` (`:97-112`) scales the raw `deltaY`, whose magnitude per notch varies
by platform and device, so the step size is unpredictable. Use the *sign* only and a
fixed step:

```cpp
void BMKnob::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& d)
{
    const float dy = (d.deltaY != 0.0f) ? d.deltaY : d.deltaX;
    if (juce::approximatelyEqual(dy, 0.0f)) return;
    const float dir = dy > 0.0f ? 1.0f : -1.0f;

    float step = isDiscrete ? 10.0f / (float) (numPositions - 1)
                            : dragValueRange() / 50.0f;
    if (e.mods.isShiftDown()) step *= 0.2f;

    if (onDragStart) onDragStart();
    setValue(value + dir * step);
    if (onDragEnd) onDragEnd();
}
```

Wrapping in drag-start/end makes each notch one complete automation gesture.

### 1e. Double-click reset

`mouseDoubleClick` (`:92-95`) hardcodes `setValue(5.0f)`. That is wrong for the vernier
knobs (range −1…1, so it clamps to 1.0 — the opposite end from their −1.0 default) and
ignores each parameter's real default. Use `defaultValue`, wrapped as a gesture the same
way as the wheel.

### 1f. Dead code

Delete `centreDetent` (`BMKnob.h:41`) and `setCentreDetent()` (`BMKnob.cpp:22`) — the flag
is settable but never read anywhere.

**Verify:** every knob drags smoothly and repeatedly; discrete knobs step through their
detents and stop at both ends; a click without drag changes nothing; the cursor never
leaves the window during a drag and returns to the knob centre on release; shift-drag is
noticeably finer; double-click returns each knob to its panel default (verniers to
centre, not to an end stop).

---

## Item 2 — Two-way parameter binding

**New files:** `Source/GUI/BMParameterBinder.h`, `Source/GUI/BMParameterBinder.cpp`
**Modified:** `Source/GUI/BM176Editor.h/.cpp`, `Source/Components/BM176HardwareSwitch.h/.cpp`

### The problems being fixed

1. `BM176Editor.cpp:151-157` registers APVTS listeners for only seven parameters, none of
   them knob-driven. `input`, `output`, `threshold`, `attack`, `release`, `ratio` and
   `sidechain` are write-only from the GUI — automation, preset recall and undo never move
   those knobs.
2. Every knob is initialised with a hardcoded `setValue(5.0f)` rather than its parameter's
   actual value, so reopening the editor shows stale positions.
3. No knob calls `beginChangeGesture`/`endChangeGesture`. Only the four switches do.
   Automation writing is broken in every host.
4. `parameterChanged` (`:192-211`) calls `setValue()` → `repaint()` and `setState()`
   directly. APVTS notifies listeners on whichever thread wrote the parameter, which can
   be the audio thread. This is a GUI mutation off the message thread.
5. `parameterChanged` → `switch.setState()` → `onChange` → `setValueNotifyingHost()`
   bounces a spurious write straight back at the host.

### Approach

Use `juce::ParameterAttachment` (`juce_audio_processors/utilities/juce_ParameterAttachments.h`,
present since JUCE 6; confirm the header in the restored 8.0.14 tree). It already solves
3, 4 and 5: it marshals host→GUI updates onto the message thread via an internal
`AsyncUpdater`, and provides `beginGesture()` / `setValueAsPartOfGesture()` /
`endGesture()` / `setValueAsCompleteGesture()` / `sendInitialUpdate()`.

`BMKnob` stays a plain `juce::Component` — none of its paint code is touched.

### `BMKnobBinder`

Knobs work in their own value space (0–10, or −1…1 for verniers); parameters have their
own ranges. Two optional conversion lambdas bridge them; when omitted the mapping is
identity.

```cpp
namespace bm176
{
    class BMKnobBinder
    {
    public:
        using Convert = std::function<float(float)>;

        BMKnobBinder(juce::AudioProcessorValueTreeState& apvts,
                     const juce::String& paramID,
                     BMKnob& knobToBind,
                     Convert knobToParamFn = nullptr,
                     Convert paramToKnobFn = nullptr);

    private:
        BMKnob&  knob;
        Convert  knobToParam;
        Convert  paramToKnob;
        juce::ParameterAttachment attachment;   // MUST be declared last

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMKnobBinder)
    };
}
```

```cpp
BMKnobBinder::BMKnobBinder(juce::AudioProcessorValueTreeState& apvts,
                           const juce::String& paramID,
                           BMKnob& knobToBind,
                           Convert knobToParamFn,
                           Convert paramToKnobFn)
    : knob(knobToBind)
    , knobToParam(knobToParamFn ? std::move(knobToParamFn) : [](float v) { return v; })
    , paramToKnob(paramToKnobFn ? std::move(paramToKnobFn) : [](float v) { return v; })
    , attachment(*apvts.getParameter(paramID),
                 [this](float newParamValue)
                 {
                     knob.setValue(paramToKnob(newParamValue),
                                   juce::dontSendNotification);
                 })
{
    auto* param = apvts.getParameter(paramID);
    jassert(param != nullptr);

    knob.setDefaultValue(paramToKnob(param->convertFrom0to1(param->getDefaultValue())));

    knob.onDragStart   = [this] { attachment.beginGesture(); };
    knob.onValueChange = [this](float v) { attachment.setValueAsPartOfGesture(knobToParam(v)); };
    knob.onDragEnd     = [this] { attachment.endGesture(); };

    attachment.sendInitialUpdate();
}
```

Declaration order matters: `attachment`'s callback captures `this` and calls
`paramToKnob`, so the converters must be constructed first. Keep `attachment` last in the
header.

`ParameterAttachment` takes and reports **denormalised** values, so the converters map
between knob space and the parameter's real range — not 0–1.

### Conversions per control

| Knob | Parameter | Param range | Converters |
|---|---|---|---|
| `inputKnob` | `input` | float 0–10 | identity (omit) |
| `outputKnob` | `output` | float 0–10 | identity |
| `thresholdKnob` | `threshold` | float 0–10 | identity |
| `attackKnob` | `attack` | float 0–10 | identity |
| `releaseKnob` | `release` | float 0–10 | identity |
| `vernierInKnob` | `inputVernier` | float −1…1 | identity |
| `vernierOutKnob` | `outputVernier` | float −1…1 | identity |
| `ratioKnob` | `ratio` | choice 0–4 | knob→param `std::round(v / 2.5f)`; param→knob `v * 2.5f` |
| `sidechainKnob` | `sidechain` | choice 0–5 | knob→param `std::round(v / 2.0f)`; param→knob `v * 2.0f` |
| `meterKnob` | `meterMode` | choice 0–2 | knob→param `std::round(v / 5.0f)`; param→knob `v * 5.0f` |

Only the three choice knobs need converters — everything else already shares its
parameter's range, which is why the existing hand-rolled divisions in `BM176Editor.cpp`
happened to produce correct values. Delete all of that arithmetic; the binder replaces it.

The `attackKnob` keeps `setValueRange(2.0f, 10.0f)` — the knob restricts its own travel
while the parameter stays 0–10.

### `BMSwitchBinder`

The `power` switch is inverted: switch-on means power-off (`BM176Editor.cpp:139`).

```cpp
class BMSwitchBinder
{
public:
    BMSwitchBinder(juce::AudioProcessorValueTreeState& apvts,
                   const juce::String& paramID,
                   BM176HardwareSwitch& switchToBind,
                   bool inverted = false);
private:
    BM176HardwareSwitch& sw;
    bool invert;
    juce::ParameterAttachment attachment;   // declared last
};
```

```cpp
    : sw(switchToBind), invert(inverted)
    , attachment(*apvts.getParameter(paramID),
                 [this](float v)
                 {
                     sw.setState(invert ? (v < 0.5f) : (v >= 0.5f),
                                 juce::dontSendNotification);
                 })
{
    sw.setCallback([this](bool on)
    {
        attachment.setValueAsCompleteGesture(invert ? (on ? 0.0f : 1.0f)
                                                    : (on ? 1.0f : 0.0f));
    });
    attachment.sendInitialUpdate();
}
```

Bindings: `interstage`→`interstageSwitch`, `compressorOn`→`attackOffSwitch`,
`bypass`→`bypassSwitch`, `power`→`powerSwitch` (inverted).

### `BM176HardwareSwitch` changes

`setState` currently fires `onChange` unconditionally, which is what creates the feedback
loop. Give it a notification argument, and split the visual side-effect out so the ON lamp
can follow the power switch in both directions:

```cpp
void setState(bool on, juce::NotificationType n = juce::sendNotification);
std::function<void(bool)> onStateVisual;   // fires on every change, user or host
```

```cpp
void BM176HardwareSwitch::setState(bool on, juce::NotificationType n)
{
    if (state == on) return;
    state = on;
    targetY = state ? slotBottomY() : slotTopY();
    if (!dragging) velocity += state ? 80.0f : -80.0f;
    startTimerHz(60);                                   // see Item 4
    if (onStateVisual) onStateVisual(state);
    if (n != juce::dontSendNotification && onChange) onChange(state);
}
```

In the editor, replace the lamp line inside the old power callback with:
`powerSwitch.onStateVisual = [this](bool on) { onLamp.setState(! on); };`

### Editor cleanup

Delete from `BM176Editor`:

- the `AudioProcessorValueTreeState::Listener` base and the `parameterChanged` override
- all seven `addParameterListener` / `removeParameterListener` pairs (`:151-157`, `:162-168`)
- every `onValueChange` lambda and every `setValueNotifyingHost` call
- every hardcoded `setValue(5.0f)` / `setValue(0.0f)` / `setValue(-1.0f)` initialiser
  (`sendInitialUpdate()` supplies the real value)
- the `meterModeIdx` member

Add member binder objects, constructed after the components they bind so destruction
order is safe.

The knobs' *configuration* calls (`setIsBig`, `setDiscrete`, `setVernierMode`,
`setAngleRange`, `setValueRange`) all stay — they describe the control, not its value.

### Meter mode and the missing GR inversion

`meterModeIdx` was driven by a listener. Read it from the parameter atomically in the
editor's timer instead. **`BMVU::setMode()` is declared and defined but never called from
anywhere** — `grMode` is permanently false, so the GR needle reads as a level rather than
inverted gain reduction. Fix both together:

```cpp
void BM176Editor::timerCallback()
{
    const int mode = juce::jlimit(0, 2, (int) pMeterMode->load());

    float dbVal = -96.0f;
    switch (mode)
    {
        case 0: if (getInputLevelFn)    dbVal = getInputLevelFn();    break;
        case 1: if (getGainReductionFn) dbVal = getGainReductionFn(); break;
        case 2: if (getOutputLevelFn)   dbVal = getOutputLevelFn();   break;
    }

    vuMeter.setMode(mode == 1);
    vuMeter.setTargetDB(dbVal);
}
```

Cache `pMeterMode = apvts.getRawParameterValue("meterMode")` in the constructor.

**Verify:** automate `input` in a host and watch the knob track it. Save a preset with
non-default values, reload, confirm the knobs match. Undo a knob move in the host and
confirm the GUI follows. Close and reopen the editor and confirm positions persist. Check
the host's automation lane records a clean ramp, not a stair-step or a single jump.
Switch the meter to GR and confirm the needle now deflects the opposite way.

---

## Item 3 — Pointer / legend alignment

**File:** `Source/GUI/BM176Editor.cpp`

Three discrete knobs sweep through a wider arc than the legend printed under them, so the
pointer never lands on its label. `angleFromValue()` maps the knob's 0–10 span linearly
across `[minAngleDeg, maxAngleDeg]`, so with N detents the pointer sits at
`minAngle + i/(N-1) × range`.

| Knob | Detents | Current sweep | Pointer lands at | Legend drawn at (`BMPanel.cpp`) | Change to |
|---|---|---|---|---|---|
| Ratio | 5 | ±75° (`:13`) | −75, −37.5, 0, 37.5, 75 | −60, −30, 0, 30, 60 (`:154`) | `setAngleRange(-60.0f, 60.0f)` |
| Sidechain | 6 | ±90° (`:23`) | −90, −54, −18, 18, 54, 90 | −75, −45, −15, 15, 45, 75 (`:158`) | `setAngleRange(-75.0f, 75.0f)` |
| Meter | 3 | ±60° (`:42`) | −60, 0, 60 | −45, 0, 45 (`:162`) | `setAngleRange(-45.0f, 45.0f)` |

Each corrected range makes the detent angles equal the legend angles exactly. Change only
the knob sweeps — the panel legends are correct and must not move.

Visible in the 2026-08-04 large-window screenshot: the Sidechain knob sits at its `OFF`
detent, but the pointer lies flat left (−90°) while the `OFF` legend is printed up-left at
−75°. The pointer overshoots its own label by a full 15°.

**Verify:** step each of the three knobs through every position and confirm the pointer
points squarely at each printed label at both extremes and in the middle.

---

## Item 4 — Rendering and timers

### 4a. Cache the panel artwork

**File:** `Source/Components/BMPanel.h/.cpp`

`BMPanel::paint` (`:40-167`) re-executes the entire panel every time it is called — four
screws, ~20 tracked-text legends each building a `GlyphArrangement` and a `Path`, four
knob scale rings, three discrete label sets, the wordmark and the bottom branding. Only
the brushed-noise texture is cached (`:12-38`).

Nothing is `setOpaque(true)`, so every knob turn and every VU needle tick dirties the
panel beneath it and re-runs all of that. `045f74c` added image caches to `BMKnob` and
`BMVU` but not to the panel, which is by far the most expensive of the three.

Move the whole body of `paint()` into a private `drawPanelArtwork(juce::Graphics&)` and
cache it:

```cpp
void BMPanel::paint(juce::Graphics& g)
{
    if (! panelCacheValid
        || cachedPanel.getWidth()  != getWidth()
        || cachedPanel.getHeight() != getHeight())
    {
        cachedPanel = juce::Image(juce::Image::RGB, getWidth(), getHeight(), false);
        juce::Graphics cg(cachedPanel);
        drawPanelArtwork(cg);
        panelCacheValid = true;
    }
    g.drawImageAt(cachedPanel, 0, 0);
}

void BMPanel::resized() { panelCacheValid = false; textureBuilt = false; }
```

Use `Image::RGB` (not ARGB) and call `setOpaque(true)` in the constructor — the panel is
the background and covers its whole bounds, so this also stops JUCE painting anything
underneath it. At design size the cache is 1920 × 354 × 3 ≈ 2 MB, which is fine.

Because the panel component lives in design space its size never changes at runtime after
Item 0, so this renders exactly once.

### 4b. Stop idle switch timers

**File:** `Source/Components/BM176HardwareSwitch.cpp`

The constructor calls `startTimerHz(60)` (`:9`) and the timer never stops — four switches
tick 60 times a second forever, integrating a spring that settled at startup. The
`repaint()` is guarded (`:106`) but the callback still runs.

Remove `startTimerHz(60)` from the constructor. Start it in `setState` (shown in Item 2)
and in `mouseDrag`, and stop it when the spring settles:

```cpp
if (std::abs(targetY - actuatorY) < 0.15f && std::abs(velocity) < 0.5f)
{
    actuatorY = targetY;
    velocity  = 0.0f;
    stopTimer();
    repaint();
    return;
}
```

`resized()` already snaps `actuatorY` straight to `targetY`, so no timer is needed to
reach the initial position.

### 4c. VU timer

`BMVU` runs its own 30 Hz timer (`BMVU.cpp:9`) alongside the editor's 24 Hz timer
(`PluginEditor.cpp:22`), which is what feeds it. The callback early-outs cheaply when
settled, so this is minor — but the two unsynced rates cause uneven needle motion.

Apply the same treatment: drop `startTimerHz(30)` from the constructor, and start the
timer from `setTargetDB` when the target differs from the displayed value; stop it in the
settled branch (`:39-43`). Raise the editor timer to 30 Hz to match the needle integrator's
assumed `dt` of `1.0f/30.0f` (`:30`) — at 24 Hz the integration constant is already wrong.

**Verify:** the panel renders identically to before (compare screenshots). Knob drags and
needle movement are smooth. With the plugin idle and no GUI interaction, CPU attributable
to the editor is near zero — check with `top` on the standalone.

---

## Item 5 — Dead code

None of this changes behaviour; it is all provably unreferenced.

| What | Where | Action |
|---|---|---|
| `BMToggleSwitch` | `Source/Components/BMToggleSwitch.h/.cpp` (122 lines) | Only ever `#include`d at `BM176Editor.h:8`, never instantiated. Delete both files, the include, and both `CMakeLists.txt` entries (the `juce_add_plugin` list and `target_sources`). |
| `BM176LookAndFeel` | `Source/GUI/BM176LookAndFeel.h/.cpp` | Never instantiated; `setLookAndFeel` is never called. It only sets a window background colour and a `Label` font, and there are no `Label`s. Delete, plus both CMake entries. |
| `placeToggle` lambda | `BM176Editor.cpp:222-225` | Unused. Delete. |
| `bypassSwitch` bounds | `BM176Editor.cpp:245` | Hardcoded `1848 - 18`, `88 - 50`, `36`, `100` — identical to what `placeSwitch(bypassSwitch, 1848.0f, 88.0f)` produces. Use the helper. |
| `topLabel` / `bottomLabel` | `BM176HardwareSwitch.h:41-42`, `.cpp:35-36` | Stored by `setLabels` and never drawn — only `title` reaches `paint()` (`:182-187`). Reduce the API to `setTitle(const juce::String&)` and update the four call sites in `BM176Editor.cpp` to pass only the title. Zero visual change. *(Alternative, if you want the switch to label its own throws instead of relying on the panel legends: actually draw them above and below the bezel and remove the duplicate panel text at `BMPanel.cpp:96-99, 104-106`. That is a visual change — get sign-off before doing it.)* |
| `getScaleTransform` / `getDesignBounds` | `BM176Geometry.h:40-58` | Unused. Either use in Item 0 or delete. |

**Verify:** clean rebuild with no warnings (`-Wall -Wextra -Wpedantic` is already on), and
the GUI is pixel-identical to before this item.

---

## Out of scope (raised, deliberately deferred)

- **No embedded font.** `resolveCondensedFont` (`BM176Fonts.h:22-35`) walks a candidate
  list and falls back to whatever is installed, then `BMPanel` fakes condensing with
  `scale(0.88f, 1.0f)` plus a `cx * 0.136f` recentring hack (`:178`, `:358`). The panel
  therefore looks different on every machine. Fixing it properly means adding a
  licence-clear condensed TTF (e.g. Roboto Condensed, Apache-2.0) as a `BinaryData`
  resource via `juce_add_binary_data` and deleting the fallback. Deferred pending a font
  choice.
- **No tooltips or value readout.** There is currently no way to see a parameter's actual
  value. Deferred as a feature, not a fix.
- **No keyboard or accessibility support.** `setWantsKeyboardFocus(false)` on every knob;
  no `AccessibilityHandler` anywhere.

---

## Suggested commit sequence

Keep these separate so any one can be reverted independently:

1. `fix: correct editor scale transform` (Item 0)
2. `fix: knob drag, wheel and reset behaviour` (Item 1)
3. `fix: two-way parameter binding with automation gestures` (Item 2)
4. `fix: align discrete knob pointers to panel legends` (Item 3)
5. `perf: cache panel artwork, stop idle timers` (Item 4)
6. `chore: remove unused GUI components` (Item 5)

## Final acceptance check

Build the standalone and, in a host, confirm:

- Full panel visible and centred at minimum, default and maximum window size.
- Every knob drags repeatedly and smoothly; discrete knobs step cleanly and stop at both
  ends; pointers land on their printed legends.
- Cursor stays put during drags and returns to the knob centre.
- Automating any parameter moves its control; presets recall correctly; host undo works;
  reopening the editor preserves positions.
- Automation lanes record smooth ramps.
- Meter switches between IN / GR / OUT, and GR deflects inversely.
- Idle CPU attributable to the GUI is negligible.
