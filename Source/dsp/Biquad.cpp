#include "Biquad.h"

#include <algorithm>
#include <cmath>

namespace ua176 {

namespace
{
// M_PI is a non-standard macro gated behind compiler-specific feature switches (glibc/GCC expose
// it by default in practice; MSVC requires _USE_MATH_DEFINES before the FIRST transitive include
// of <cmath>, which is easy to break silently) -- define our own instead of relying on either.
constexpr float kPi = 3.14159265358979323846f;
}  // namespace

void Biquad::computeCoeffs(BiquadType type, float gainDb, float cutoffHz, float q,
                            double sampleRate) noexcept
{
    const float A = std::pow(10.0f, gainDb / 40.0f);
    const float w0 = 2.0f * kPi * (cutoffHz / (float) sampleRate);
    const float alpha = std::sin(w0) / (2.0f * q);
    const float cosw0 = std::cos(w0);
    const float sqrtA = std::sqrt(A);

    float b0, b1, b2, a0, a1, a2;

    switch (type)
    {
        case BiquadType::LowShelf:
            b0 = A * ((A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * sqrtA * alpha);
            b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosw0);
            b2 = A * ((A + 1.0f) - (A - 1.0f) * cosw0 - 2.0f * sqrtA * alpha);
            a0 = (A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * sqrtA * alpha;
            a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cosw0);
            a2 = (A + 1.0f) + (A - 1.0f) * cosw0 - 2.0f * sqrtA * alpha;
            break;
        case BiquadType::HighShelf:
            b0 = A * ((A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * sqrtA * alpha);
            b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosw0);
            b2 = A * ((A + 1.0f) + (A - 1.0f) * cosw0 - 2.0f * sqrtA * alpha);
            a0 = (A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * sqrtA * alpha;
            a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosw0);
            a2 = (A + 1.0f) - (A - 1.0f) * cosw0 - 2.0f * sqrtA * alpha;
            break;
        case BiquadType::HighPass:
        {
            const float c = 1.0f + alpha;
            b0 = (1.0f + cosw0) / (2.0f * c);
            b1 = -(1.0f + cosw0) / c;
            b2 = (1.0f + cosw0) / (2.0f * c);
            a0 = 1.0f;
            a1 = -2.0f * cosw0 / c;
            a2 = (1.0f - alpha) / c;
            break;
        }
        case BiquadType::Peaking:
        default:
            b0 = 1.0f + alpha * A;
            b1 = -2.0f * cosw0;
            b2 = 1.0f - alpha * A;
            a0 = 1.0f + (alpha / A);
            a1 = -2.0f * cosw0;
            a2 = 1.0f - (alpha / A);
            break;
    }

    b0_ = b0 / a0;
    b1_ = b1 / a0;
    b2_ = b2 / a0;
    a1_ = a1 / a0;
    a2_ = a2 / a0;
}

float Biquad::processSample(float x) noexcept
{
    // Direct Form I: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
    // (a0 already normalized to 1 in computeCoeffs).
    const float y = b0_ * x + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_;
    x2_ = x1_;
    x1_ = x;
    y2_ = y1_;
    y1_ = y;  // internal recursion feedback stays UNCLAMPED -- matches torchaudio's _lfilter,
              // which computes each section's full-buffer IIR recursion with no clamping inside
              // it (DifferentiableIIR/DifferentiableFIR); clamping is a post-hoc step applied
              // once to that finished array before it becomes the NEXT section's input.
    // nablafx's sosfilt() (dsp.py) calls torchaudio.functional.lfilter(y, a, b) per cascaded
    // section with the library default clamp=True -- every section's output is hard-clipped to
    // [-1,1] before feeding the next section. This port never replicated that, which was invisible
    // while every checkpoint's EQ/gain conditioning stayed under +-1FS mid-cascade, and surfaced
    // as a real (non-precision) parity divergence once a checkpoint's EQ got aggressive enough to
    // exceed it (docs/stage-c-relu-controller-mismatch.md has the full investigation). Only the
    // value handed onward is clamped -- this section's own y1_/y2_ feedback stays raw, matching
    // the same asymmetry in torchaudio's implementation (confirmed by re-running threshold_0 and
    // ratio_12 with this fix: max|err| dropped from 0.088/0.076 to ~1e-5, both clean).
    return std::clamp(y, -1.0f, 1.0f);
}

void Biquad::process(float* buffer, int numSamples) noexcept
{
    for (int n = 0; n < numSamples; ++n)
        buffer[n] = processSample(buffer[n]);
}

void Biquad::reset() noexcept
{
    x1_ = x2_ = y1_ = y2_ = 0.0f;
}

}  // namespace ua176
