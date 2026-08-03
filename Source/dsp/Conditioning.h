// Mirrors training/config.py::CONTROL_ORDER and training/normalization.py::normalize_control
// EXACTLY. Do not change the output order or the per-control formulas without updating both.
#pragma once

#include <array>

namespace ua176 {

// CONTROL_ORDER = ("ratio", "input", "threshold", "attack", "output", "release", "interstage")
inline std::array<float, 7> normalizeConditioning(float ratio, float input, float threshold,
                                                   float attack, float output, float release,
                                                   float interstage) noexcept
{
    // ratio: ORDINAL {2:0, 4:1/3, 8:2/3, 12:1} -- a stepped switch, not value/12 (matches
    // training/normalization.py::RATIO_ORDINAL exactly). Falls back to the nearest detent for any
    // off-detent input rather than raising (this runs in a real-time audio callback).
    float ratioNorm;
    if (ratio <= 3.0f)       ratioNorm = 0.0f;         // 2:1
    else if (ratio <= 6.0f)  ratioNorm = 1.0f / 3.0f;  // 4:1
    else if (ratio <= 10.0f) ratioNorm = 2.0f / 3.0f;  // 8:1
    else                     ratioNorm = 1.0f;         // 12:1

    // dial controls: value / 10 (0-10 clock dials)
    const float inputNorm = input / 10.0f;
    const float thresholdNorm = threshold / 10.0f;
    const float attackNorm = attack / 10.0f;
    const float outputNorm = output / 10.0f;
    const float releaseNorm = release / 10.0f;

    // interstage: identity (0/1 in/out of circuit)
    const float interstageNorm = interstage;

    return { ratioNorm, inputNorm, thresholdNorm, attackNorm, outputNorm, releaseNorm, interstageNorm };
}

}  // namespace ua176
