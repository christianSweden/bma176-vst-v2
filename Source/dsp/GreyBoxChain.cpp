#include "GreyBoxChain.h"

#include <cstdio>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
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
    eq_.reset();
    drc_.reset();
}

void GreyBoxChain::setBypassed(bool bypassed) noexcept
{
    drc_.setBypassed(bypassed);
}

void GreyBoxChain::process(float* buffer, int numSamples, const std::array<float, 7>& cond) noexcept
{
    float gainInParam[1];
    float eqParam[kMlpMaxOutDim];
    float drcParam[7];
    float gainOutParam[1];

    const int clampedIdx = 1;  // force r4
    const float curveCond[3] = { cond[1], cond[2], cond[6] };
    const float ballisticsCond[3] = { cond[0], cond[3], cond[5] };

    const float gainInCond[2] = { cond[1], cond[6] };
    const float gainOutCond[1] = { cond[4] };

    gainInCtrl_.evaluate(gainInCond, gainInParam);
    eqCtrl_.evaluate(cond.data(), eqParam);
    drcCurveCtrls_[clampedIdx].evaluate(curveCond, drcParam);
    drcBallisticsCtrl_.evaluate(ballisticsCond, drcParam + 3);
    gainOutCtrl_.evaluate(gainOutCond, gainOutParam);

    gainIn_.setGainDb(gainInParam[0]);
    eq_.setParams(eqParam, sampleRate_);
    drc_.setParams(drcParam[0], drcParam[1], drcParam[2], drcParam[3], drcParam[4], drcParam[5],
                   drcParam[6]);
    gainOut_.setGainDb(gainOutParam[0]);

    gainIn_.process(buffer, numSamples);
    tubeColor_.process(buffer, numSamples);
    eq_.process(buffer, numSamples);
    drc_.process(buffer, numSamples);
    gainOut_.process(buffer, numSamples);
    outputColor_.process(buffer, numSamples);

#ifndef PARITY_HARNESS
    static int debugCounter = 0;
    if (++debugCounter % (48000 * 2 / numSamples) == 0)
    {
        auto rmsDb = [&](const float* buf, int n) -> float {
            double sum = 0.0;
            for (int i = 0; i < n; ++i) sum += (double)buf[i] * buf[i];
            return 20.0f * std::log10(std::sqrt(sum / n) + 1e-12f);
        };
        const float outRms = rmsDb(buffer, numSamples);

        // Denormalize dial values for human-readable output
        const float dialIn    = cond[1] * 10.0f;
        const float dialThr   = cond[2] * 10.0f;
        const float dialAtk   = cond[3] * 10.0f;
        const float dialOut   = cond[4] * 10.0f;
        const float dialRel   = cond[5] * 10.0f;
        const float dialInt   = cond[6];
        const char* ratioName = (cond[0] <= 0.15f) ? "r2" : (cond[0] <= 0.45f) ? "r4" :
                                (cond[0] <= 0.75f) ? "r8" : "r12";

        // EQ band gains (every 3rd value starting at 0: 0,3,6,9,12)
        const float eqGain[5] = { eqParam[0], eqParam[3], eqParam[6], eqParam[9], eqParam[12] };

        // DRC: curve (T,R,W) + ballistics (tau_a, tau_r, s_a, s_r)
        const float T     = drcParam[0];
        const float R     = drcParam[1];
        const float W     = drcParam[2];
        const float tau_a = drcParam[3];
        const float tau_r = drcParam[4];
        const float s_a   = drcParam[5];
        const float s_r   = drcParam[6];
        const float grDb  = drc_.getGainReductionDb();

#ifdef _WIN32
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "CHAIN | %s in=%.1f thr=%.1f atk=%.1f out=%.1f rel=%.1f int=%.0f | cmp=%s",
                 ratioName, dialIn, dialThr, dialAtk, dialOut, dialRel, dialInt,
                 drc_.isBypassed() ? "OFF" : "ON");
        OutputDebugStringA(buf);
        snprintf(buf, sizeof(buf),
                 "CHAIN | g_in=%.2f g_out=%.2f | EQ: %.2f %.2f %.2f %.2f %.2f",
                 gainInParam[0], gainOutParam[0], eqGain[0], eqGain[1], eqGain[2], eqGain[3], eqGain[4]);
        OutputDebugStringA(buf);
        snprintf(buf, sizeof(buf),
                 "CHAIN | DRC: T=%.2f R=%.2f W=%.2f | bal: ta=%.4f tr=%.4f sa=%.2f sr=%.4f",
                 T, R, W, tau_a, tau_r, s_a, s_r);
        OutputDebugStringA(buf);
        snprintf(buf, sizeof(buf),
                 "CHAIN | chain_out=%.2f dBFS | GR=%.2f dB",
                 outRms, grDb);
        OutputDebugStringA(buf);
#else
        std::printf("CHAIN | %s in=%.1f thr=%.1f atk=%.1f out=%.1f rel=%.1f int=%.0f | cmp=%s\n",
                    ratioName, dialIn, dialThr, dialAtk, dialOut, dialRel, dialInt,
                    drc_.isBypassed() ? "OFF" : "ON");
        std::printf("CHAIN | g_in=%.2f g_out=%.2f | EQ: %.2f %.2f %.2f %.2f %.2f\n",
                    gainInParam[0], gainOutParam[0], eqGain[0], eqGain[1], eqGain[2], eqGain[3], eqGain[4]);
        std::printf("CHAIN | DRC: T=%.2f R=%.2f W=%.2f | bal: ta=%.4f tr=%.4f sa=%.2f sr=%.4f\n",
                    T, R, W, tau_a, tau_r, s_a, s_r);
        std::printf("CHAIN | chain_out=%.2f dBFS | GR=%.2f dB\n", outRms, grDb);
#endif
    }
#endif
}

}  // namespace ua176
