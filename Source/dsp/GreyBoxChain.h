// Top-level chain, mirroring nablafx.GreyBoxModel's exact block order (cfg/model/ua176_greybox.yaml):
//   Gain(in) -> StaticRationalNonlinearity(tube color) -> ParametricEQ -> VariMuDRC ->
//   Gain(makeup) -> StaticRationalNonlinearity(output color)
// NL blocks were SIREN through V2/V2.1's Phase 2; swapped to Rational (Pade P(x)/Q(x)) in Phase 3
// (docs/siren-to-rational-eval-plan.md, fable GO 2026-07-21) -- gradient imbalance root-cause fix.
// Each static-cond block's controller runs ONCE PER process() CALL (matches the trained model's
// per-buffer conditioning contract exactly -- nablafx's Controller.forward() calls each
// StaticCondController once per GreyBoxModel.forward(), never per-sample). Do not add
// per-sample/smoothed conditioning here; it would silently diverge from what was validated.
#pragma once

#include <array>

#include "Biquad.h"
#include "GainProcessor.h"
#include "MlpController.h"
#include "ParametricEqProcessor.h"
#include "RationalNonlinearity.h"
#include "VariMuDrcProcessor.h"

namespace ua176 {

class GreyBoxChain
{
public:
    GreyBoxChain();

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setBypassed(bool bypassed) noexcept;

    // cond is the 7-vector from Conditioning.h::normalizeConditioning, in CONTROL_ORDER.
    void process(float* buffer, int numSamples, const std::array<float, 7>& cond) noexcept;

    // Peak gain reduction (dB, <= 0) over the most recently processed buffer. Safe to call from
    // any thread -- see VariMuDrcProcessor::getGainReductionDb()'s comment.
    float getGainReductionDb() const noexcept { return drc_.getGainReductionDb(); }

private:
    GainProcessor gainIn_;
    RationalNonlinearity tubeColor_;
    ParametricEqProcessor eq_;
    VariMuDrcProcessor drc_;
    GainProcessor gainOut_;
    RationalNonlinearity outputColor_;
    Biquad postFilter_;  // V4: output transformer LF rolloff (HP 110 Hz)

    MlpController gainInCtrl_;
    MlpController eqCtrl_;
    // VariMuDRC controller split (docs/controller-split-plan.md): the DRC block's curve
    // side is a per-ratio ModuleList of 4 independent MLPs (V3, 2026-08-02), each
    // 3-in (input, threshold, interstage — ratio implicit) → 3-out (T, R, W).
    // Ballistics is still one shared controller (ratio, attack, release → tau_a, tau_r, c_a, s_r).
    // Indices: 0=r2, 1=r4, 2=r8, 3=r12. Ratio routing: int(cond[0] * 3.0) → index.
    std::array<MlpController, 4> drcCurveCtrls_;
    MlpController drcBallisticsCtrl_;
    MlpController gainOutCtrl_;

    double sampleRate_ = 48000.0;
};

}  // namespace ua176
