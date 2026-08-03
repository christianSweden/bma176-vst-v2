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

    // Per-stage RMS gauge — prints every ~2s.
    // Remove after gain anchor / controller diagnosis is complete.
#ifndef PARITY_HARNESS
    static int debugCounter = 0;
    if (++debugCounter % (48000 * 2 / numSamples) == 0)
    {
        double sumSq = 0.0;
        for (int i = 0; i < numSamples; ++i)
            sumSq += (double)buffer[i] * buffer[i];
        const float outRms = 20.0f * std::log10(std::sqrt(sumSq / numSamples) + 1e-12f);

#ifdef _WIN32
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "BM176 | g_in=%.2f g_out=%.2f T=%.2f R=%.2f W=%.2f | chain_out=%.2f dBFS",
                 gainInParam[0], gainOutParam[0], drcParam[0], drcParam[1], drcParam[2], outRms);
        OutputDebugStringA(buf);
#else
        std::printf("BM176 | g_in=%.2f g_out=%.2f T=%.2f R=%.2f W=%.2f | chain_out=%.2f dBFS\n",
                    gainInParam[0], gainOutParam[0], drcParam[0], drcParam[1], drcParam[2], outRms);
#endif
    }
#endif
}

}  // namespace ua176
