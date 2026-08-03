// Single Direct-Form-I biquad section. Coefficient formulas are the RBJ cookbook, ported EXACTLY
// from nablafx/nablafx/processors/dsp.py::biquad() (verified against source, not from general
// knowledge) -- including its own b/a0, a/a0 normalization, so a0 is implicitly 1 here and never
// appears in the recursion. Only the 3 filter types this project's ParametricEQ actually uses
// (low_shelf, peaking, high_shelf) are implemented -- dsp.py also has low_pass/high_pass, unused
// here.
#pragma once

namespace ua176 {

enum class BiquadType
{
    LowShelf,
    Peaking,
    HighShelf,
    HighPass
};

class Biquad
{
public:
    void computeCoeffs(BiquadType type, float gainDb, float cutoffHz, float q,
                        double sampleRate) noexcept;
    float processSample(float x) noexcept;
    void process(float* buffer, int numSamples) noexcept;
    void reset() noexcept;

private:
    // b0/a0, b1/a0, b2/a0, a1/a0, a2/a0 (a0 already divided out, per dsp.py::biquad()).
    float b0_ = 1.0f, b1_ = 0.0f, b2_ = 0.0f, a1_ = 0.0f, a2_ = 0.0f;
    float x1_ = 0.0f, x2_ = 0.0f, y1_ = 0.0f, y2_ = 0.0f;
};

}  // namespace ua176
