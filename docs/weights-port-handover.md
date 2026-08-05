# Handover: port new 4:1-only weights from neural_dev_v4

**Branch:** `V.4-new-bma176-measuring-weights`
**Written:** 2026-08-05, by Opus, after reading both codebases (BM176's DSP chain and neural_dev_v4's training/export pipeline) — not guessed.

## What this is

`neural_dev_v4` (`\\wsl.localhost\Ubuntu\home\christian\PluginDev\neural_dev_v4` from Windows, `/home/christian/PluginDev/neural_dev_v4` from WSL/Linux) is producing a new, better-measured set of neural weights for the BM176 plugin's grey-box DSP chain. So far it only has real captured/identified hardware data for the **4:1** compression ratio — the other four ratios (1.5:1, 2:1, 8:1, 12:1) have no real training data yet (see `neural_dev_v4/docs/v4.3-r4-fidelity-report.md` §6, which lays out a *future* plan for r2/r8/r12 capture campaigns, not yet run).

Goal of this port: get the plugin building and sounding correct at **4:1**, with every other control (HP filter, verniers, interstage, attack-off, bypass, all switches) working exactly as it does today, and the other four ratio positions still selectable in the GUI without crashing, going silent, or producing garbage — even though they won't be acoustically correct yet.

**No GUI changes are needed or wanted for this port.** The Ratio knob keeps all 5 positions.

## The one real blocker: a compile-time dependency on weights that won't exist

This part isn't a judgment call — it's a straightforward fact about how the C++ is wired today, confirmed by reading `GreyBoxChain.cpp`.

The DRC "curve" block (the ratio-dependent compression curve shape) is **already architected as 4 independent MLPs**, one per ratio, exactly matching neural_dev_v4's `SplitVariMuController` design:

```cpp
// GreyBoxChain.h:50-56 (existing comment, already documents this)
// the DRC block's curve side is a per-ratio ModuleList of 4 independent MLPs...
// Indices: 0=r2, 1=r4, 2=r8, 3=r12.
```

`GreyBoxChain.cpp`'s constructor references the weight arrays **by name** — `ua176::weights::drc_curve_2_*`, `drc_curve_4_*`, `drc_curve_8_*`, `drc_curve_12_*` — to build `drcCurveCtrls_[4]`. At runtime, `GreyBoxChain.cpp:86-91` picks which of the 4 to use:

```cpp
const int ratioIdx = static_cast<int>(cond[0] * 3.0f + 0.5f);
const int clampedIdx = ratioIdx < 0 ? 0 : (ratioIdx > 3 ? 3 : ratioIdx);
...
drcCurveCtrls_[clampedIdx].evaluate(curveCond, drcParam);
```

If `neural_dev_v4`'s export only writes `drc_curve_4_*` (because that's the only ratio with real data), **the plugin will fail to compile** the moment `UA176Weights.h` is replaced wholesale, because `drc_curve_2_*`/`_8_*`/`_12_*` will be undefined.

### Recommended fix — alias all 4 curve slots to the one real model

In `GreyBoxChain.cpp`'s constructor, instead of constructing `drcCurveCtrls_[0..3]` from `drc_curve_2/8/12`'s (nonexistent) weight arrays, construct **all four** from `drc_curve_4_*`:

```cpp
// All four ratio slots currently share the single 4:1-trained curve model —
// r2/r8/r12 have no real hardware data yet (see docs/weights-port-handover.md).
// TODO: replace with per-ratio weights once r2/r8/r12 capture campaigns land.
for (auto& ctrl : drcCurveCtrls_)
    ctrl = MlpController(w::drc_curve_4_l0_w, w::drc_curve_4_l0_b, /* ...same array set for all 4... */);
```

(Adjust to however `MlpController`'s actual constructor/assignment is called in the current code — the point is: point all four `drcCurveCtrls_[i]` at the same `drc_curve_4_*` arrays rather than four different named arrays.)

This means: whichever ratio the user selects, the model runs the 4:1-trained curve. Not correct for 1.5:1/2:1/8:1/12:1, but never broken — no crash, no silence, no NaNs, and it'll sound like a plausible (if not ratio-accurate) compressor rather than random noise. This was explicitly requested — "just bypass that in code, or however you feel suitable" — and this is the minimal-footprint way to do it while keeping the whole signal chain alive.

**Alternative considered and rejected:** keep the *old* (currently-checked-in) `drc_curve_2/8/12` weights for the non-4:1 slots and only swap in the new `drc_curve_4`. This is more surgical but pointless here: the old `drc_curve_2/8/12` arrays are themselves random-init placeholders, not real trained models (confirmed — no r2/r8/r12 capture data has ever existed in this project), so they're not meaningfully "more correct" than aliasing to r4. Aliasing to the one real model is simpler and probably sounds *more* plausible than keeping noise.

### Note on `1.5:1` specifically

This isn't new or introduced by this port — worth knowing so it's not mistaken for a regression. `1.5:1` has never had its own model slot. `Conditioning.h:14-21` already buckets it into the same conditioning value as `2:1` (`ratio <= 3 → 0.0`), so today, before this port, `1.5:1` and `2:1` already sound identical. After this port, with the aliasing fix above, all five GUI positions will sound identical (all running the 4:1 model) until r1.5/r2/r8/r12 get real training data.

## What should keep working untouched — confirmed independent of the ratio/curve weights

Checked by tracing each control's actual code path, not assumed:

- **Sidechain HP filter** (`PluginProcessor.cpp:141-166`) — a plain `ua176::Biquad`, applied to the buffer before the neural chain runs. Zero dependency on any MLP weights. Will work exactly as today regardless of what gets ported.
- **Input/output verniers** (`PluginProcessor.cpp:182-203`) — closed-form piecewise-linear dB trim, applied after the neural chain. Zero MLP dependency. Unaffected.
- **Attack-off / compressor bypass** (`compressorOn` param → `GreyBoxChain::setBypassed` → `VariMuDrcProcessor.cpp:61-65`) — short-circuits just the DRC block; `gain_in`, `eq`, `gain_out`, and the two Padé waveshapers keep running. Confirmed independent of which curve weights are loaded, since it skips that block entirely.
- **Bypass switch / power** — whole-plugin bypass, obviously independent of any weights.

## Caveat: Interstage is *not* a separate DSP stage — flag this, don't assume

Unlike HP/verniers, **interstage is not implemented as independent DSP**. `pInterstage` only ever feeds `cond[6]`, a conditioning input into the shared `gain_in`, `eq`, and DRC-curve MLPs (`GreyBoxChain.cpp:88,91`). There's no separate "transformer emulation" block to fall back on.

This means interstage's correctness after the port depends entirely on **which weight blocks actually get re-exported from neural_dev_v4**. If the new export regenerates the *whole* `UA176Weights.h` (gain_in, eq, gain_out, drc_ballistics, tube_color, output_color, and drc_curve_4 — everything except drc_curve_2/8/12, which don't exist), interstage should work correctly for the 4:1 case, since it was presumably captured as part of the same measurement campaign. If instead only `drc_curve_4_*` gets swapped in while the rest of the header stays on old weights, interstage conditioning would be running through *stale* gain_in/eq weights next to a *fresh* curve model — a mismatch worth knowing about rather than discovering as a mystery later.

**Action:** before/during the port, confirm with whoever ran the neural_dev_v4 export (or check `neural_dev_v4/docs/v4.3-r4-handover.md`, which as of this writing says "Code-side complete. Plugin rebuild + DAW validation pending") exactly which blocks the new export touches. If it's everything except curve_2/8/12, no further action needed beyond the aliasing fix above.

## Discrepancy to verify, not silently resolve: Attack knob direction

You described the current/desired behavior as: **"Attack is as it is now — slowest attack setting at Dot 2."**

I evaluated the *currently shipped* weights directly (ran the same forward-pass math `MlpController::evaluate` does, in Python, against the checked-in `drc_ballistics` arrays) at ratio=4:1, release=0.5, sweeping the attack parameter:

```
attack=2.0  -> tau_a = 0.01980s   (fastest)
attack=4.0  -> tau_a = 0.02185s
attack=6.0  -> tau_a = 0.02275s
attack=10.0 -> tau_a = 0.02339s   (slowest)
```

`tau_a` increases monotonically with the attack parameter — so in the **currently shipped code**, Dot 2 (the knob's minimum, near the "OFF" label) is the **fastest** attack, and the "FAST"-labeled end is actually the **slowest**. That's the opposite of what you described.

I'm not resolving this either direction — it needs a human decision:
- If "Dot 2 = slowest" is what you actually want (matches real 1176-style hardware, where low numbers = slow attack), that's a property of the trained ballistics model's output scaling, not something to patch around in C++ — it'd need to be corrected at the training/export side in `neural_dev_v4` (or the `param_lo`/`param_hi` for `drc_ballistics`'s attack output could plausibly be inverted, if that's actually equivalent — worth checking with whoever owns that export code rather than guessing).
- If the shipped behavior (Dot 2 = fastest) is actually correct and the description above was just a slip, no action needed — but say so explicitly so it doesn't get "fixed" into a regression.

Either way: **the GUI's attack knob scale/labels/travel should not change** — this is purely about which direction the underlying model's output maps, independent of the knob itself.

## Suggested port checklist

1. Copy/regenerate `Source/dsp/generated/UA176Weights.h` from `neural_dev_v4`'s export (the export script, `neural_dev_v4/training/export_plugin_weights.py`, already writes directly to this exact path in this exact format — no conversion needed, confirmed by reading both the script and the current header).
2. Confirm which blocks the new header actually contains (see interstage caveat above) — specifically confirm `drc_curve_2_*`/`_8_*`/`_12_*` are indeed absent, so the aliasing fix is known to be necessary rather than assumed.
3. Apply the `GreyBoxChain.cpp` constructor fix so all 4 `drcCurveCtrls_` slots point at `drc_curve_4_*` (see above).
4. Build both `BM176_Standalone` and `BM176_VST3` clean.
5. Sanity-check in a real host: HP filter, verniers, attack-off, bypass all behave as before (should need zero DSP changes, per the independence confirmed above). Confirm the plugin doesn't crash/go silent on any of the 5 ratio positions.
6. Resolve the attack-direction discrepancy above with a human decision before considering this done — don't let it sit as an unflagged surprise.
7. `Source/dsp/SirenNonlinearity.h/.cpp` is confirmed dead code (no references anywhere outside itself, superseded by `RationalNonlinearity`) — not in scope for this port, just noting it exists in case it causes confusion when scanning the dsp folder.
