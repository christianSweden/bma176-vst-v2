#include "GreyBoxChain.h"

#include <cstdio>

#ifdef _WIN32
#include <windows.h>   // OutputDebugStringA
#endif

#include "generated/UA176Weights.h"

namespace ua176 {

namespace w = ua176::weights;

GreyBoxChain::GreyBoxChain()
    : tubeColor_(w::tube_color_num, w::tube_color_num_degree, w::tube_color_den,
                 w::tube_color_den_degree),
      outputColor_(w::output_color_num, w::output_color_num_degree, w::output_color_den,
                   w::output_color_den_degree),
      gainInCtrl_(w::gain_in_l0_w, w::gain_in_l0_b, w::gain_in_l2_w, w::gain_in_l2_b,
                  w::gain_in_l4_w, w::gain_in_l4_b, w::gain_in_param_lo, w::gain_in_param_hi,
                  w::gain_in_in_dim, w::gain_in_out_dim),
      eqCtrl_(w::eq_l0_w, w::eq_l0_b, w::eq_l2_w, w::eq_l2_b, w::eq_l4_w, w::eq_l4_b,
              w::eq_param_lo, w::eq_param_hi, w::eq_in_dim, w::eq_out_dim),
       // VariMuDRC controller split (docs/controller-split-plan.md) — V3 per-ratio (2026-08-02):
       // four independent curve controllers, one per ratio (r2/r4/r8/r12), each 3-in
       // (input,threshold,interstage) → 3-out (T,R,W). Ballistics remains shared.
       drcCurveCtrls_{{
           MlpController(w::drc_curve_2_l0_w, w::drc_curve_2_l0_b, w::drc_curve_2_l2_w,
                         w::drc_curve_2_l2_b, w::drc_curve_2_l4_w, w::drc_curve_2_l4_b,
                         w::drc_curve_2_param_lo, w::drc_curve_2_param_hi,
                         w::drc_curve_2_in_dim, w::drc_curve_2_out_dim),
           MlpController(w::drc_curve_4_l0_w, w::drc_curve_4_l0_b, w::drc_curve_4_l2_w,
                         w::drc_curve_4_l2_b, w::drc_curve_4_l4_w, w::drc_curve_4_l4_b,
                         w::drc_curve_4_param_lo, w::drc_curve_4_param_hi,
                         w::drc_curve_4_in_dim, w::drc_curve_4_out_dim),
           MlpController(w::drc_curve_8_l0_w, w::drc_curve_8_l0_b, w::drc_curve_8_l2_w,
                         w::drc_curve_8_l2_b, w::drc_curve_8_l4_w, w::drc_curve_8_l4_b,
                         w::drc_curve_8_param_lo, w::drc_curve_8_param_hi,
                         w::drc_curve_8_in_dim, w::drc_curve_8_out_dim),
           MlpController(w::drc_curve_12_l0_w, w::drc_curve_12_l0_b, w::drc_curve_12_l2_w,
                         w::drc_curve_12_l2_b, w::drc_curve_12_l4_w, w::drc_curve_12_l4_b,
                         w::drc_curve_12_param_lo, w::drc_curve_12_param_hi,
                         w::drc_curve_12_in_dim, w::drc_curve_12_out_dim),
       }},
       drcBallisticsCtrl_(w::drc_ballistics_l0_w, w::drc_ballistics_l0_b, w::drc_ballistics_l2_w,
                          w::drc_ballistics_l2_b, w::drc_ballistics_l4_w, w::drc_ballistics_l4_b,
                          w::drc_ballistics_param_lo, w::drc_ballistics_param_hi,
                          w::drc_ballistics_in_dim, w::drc_ballistics_out_dim),
      gainOutCtrl_(w::gain_out_l0_w, w::gain_out_l0_b, w::gain_out_l2_w, w::gain_out_l2_b,
                   w::gain_out_l4_w, w::gain_out_l4_b, w::gain_out_param_lo,
                   w::gain_out_param_hi, w::gain_out_in_dim, w::gain_out_out_dim)
{
}

void GreyBoxChain::prepare(double sampleRate) noexcept
{
    sampleRate_ = sampleRate;
    drc_.prepare(sampleRate);
    reset();
}

void GreyBoxChain::reset() noexcept
{
    // Only the EQ's biquad state and the compressor's detector state are stateful across calls
    // (Gain and the Rational nonlinearities are memoryless -- see their own header comments).
    eq_.reset();
    drc_.reset();
}

void GreyBoxChain::setBypassed(bool bypassed) noexcept
{
    drc_.setBypassed(bypassed);
}

void GreyBoxChain::process(float* buffer, int numSamples, const std::array<float, 7>& cond) noexcept
{
    // Each conditioned block's controller runs once per call -- matches the trained model's
    // per-buffer (not per-sample) conditioning contract exactly.
    float gainInParam[1];
    float eqParam[kMlpMaxOutDim];
    float drcParam[7];
    float gainOutParam[1];

    // DEBUG LOCKED TO R4 (2026-08-02): all ratio positions use the r4 per-ratio controller
    // until r4 passes A/B testing. Remove this override after validation.
    const int clampedIdx = 1;  // force r4
    // PRODUCTION CODE (restore after r4 validated):
    // const int ratioIdx = static_cast<int>(cond[0] * 3.0f + 0.5f);
    // const int clampedIdx = ratioIdx < 0 ? 0 : (ratioIdx > 3 ? 3 : ratioIdx);
    const float curveCond[3] = { cond[1], cond[2], cond[6] };      // input, threshold, interstage
    const float ballisticsCond[3] = { cond[0], cond[3], cond[5] }; // ratio, attack, release

    // Gain-controller conditioning isolation (docs/greybox-structural-discipline-plan.md,
    // 2026-07-26): gain_in/gain_out were measured responding to attack/release/ratio by tens of
    // dB despite the hardware being inert to those axes with limiting off -- narrowed to the
    // same subsets training/nablafx_patches.py::_GAIN_IN_IDX=[1,6] / _GAIN_OUT_IDX=[4] use, kept
    // in sync the same way as curveCond/ballisticsCond above. EQ is deliberately NOT narrowed
    // (measured a literal pass-through) and stays on cond.data().
    const float gainInCond[2] = { cond[1], cond[6] };   // input,interstage
    const float gainOutCond[1] = { cond[4] };           // output

    gainInCtrl_.evaluate(gainInCond, gainInParam);
    eqCtrl_.evaluate(cond.data(), eqParam);
    drcCurveCtrls_[clampedIdx].evaluate(curveCond, drcParam);            // per-ratio: fills drcParam[0..2] = T,R,W
    drcBallisticsCtrl_.evaluate(ballisticsCond, drcParam + 3); // fills drcParam[3..6] = tau_a,tau_r,c_a,s_r
    gainOutCtrl_.evaluate(gainOutCond, gainOutParam);

    // DEBUG (2026-08-02): print conditioning + controller predictions every ~2 seconds.
    // Uses OutputDebugStringA on Windows so it reaches the VS debugger regardless
    // of whether the DAW host captures stdout. Disabled for parity harness builds.
#ifndef PARITY_HARNESS
    static int debugCounter = 0;
    if (++debugCounter % (48000 * 2 / numSamples) == 0)  // ~every 2s at 48kHz
    {
#ifdef _WIN32
        char buf[256];
        snprintf (buf, sizeof(buf),
                  "r4 LOCKED | cond: r=%.3f in=%.1f thr=%.1f atk=%.1f out=%.1f rel=%.1f int=%.1f",
                  cond[0], cond[1], cond[2], cond[3], cond[4], cond[5], cond[6]);
        OutputDebugStringA (buf);
        snprintf (buf, sizeof(buf),
                  "  T=%.2f R=%.2f W=%.2f g_in=%.2f g_out=%.2f",
                  drcParam[0], drcParam[1], drcParam[2], gainInParam[0], gainOutParam[0]);
        OutputDebugStringA (buf);
#else
        std::printf ("r4 LOCKED | cond: r=%.3f in=%.1f thr=%.1f atk=%.1f out=%.1f rel=%.1f int=%.1f\n",
                     cond[0], cond[1], cond[2], cond[3], cond[4], cond[5], cond[6]);
        std::printf ("  T=%.2f R=%.2f W=%.2f g_in=%.2f g_out=%.2f\n",
                     drcParam[0], drcParam[1], drcParam[2], gainInParam[0], gainOutParam[0]);
#endif
    }
#endif

    gainIn_.setGainDb(gainInParam[0]);
    eq_.setParams(eqParam, sampleRate_);
    // drc param order matches VariMuDRC.param_ranges dict order: T, R, W, tau_a, tau_r, c_a, s_r.
    drc_.setParams(drcParam[0], drcParam[1], drcParam[2], drcParam[3], drcParam[4], drcParam[5],
                   drcParam[6]);
    gainOut_.setGainDb(gainOutParam[0]);

    gainIn_.process(buffer, numSamples);
    tubeColor_.process(buffer, numSamples);
    eq_.process(buffer, numSamples);
    drc_.process(buffer, numSamples);
    gainOut_.process(buffer, numSamples);
    outputColor_.process(buffer, numSamples);
}

}  // namespace ua176
