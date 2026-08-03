#include "VariMuDrcProcessor.h"

#include <algorithm>
#include <cmath>

namespace ua176 {

namespace
{
constexpr float kEps = 1e-5f;
constexpr float kMinLevelDb = -80.0f;
constexpr float kGrRangeDb = -kMinLevelDb;  // 80.0f -- gain-reduction normalization range
constexpr float kKneeFloor = 1e-12f;
constexpr float kTauEffFloor = 1e-6f;
}  // namespace

void VariMuDrcProcessor::prepare(double sampleRate) noexcept
{
    fs_ = sampleRate;
    reset();
}

void VariMuDrcProcessor::setParams(float T, float R, float W, float tauA, float tauR, float cA,
                                    float sR) noexcept
{
    T_ = T;
    R_ = R;
    W_ = W;
    tauA_ = tauA;
    tauR_ = tauR;
    cA_ = cA;
    sR_ = sR;
}

void VariMuDrcProcessor::setBypassed(bool bypassed) noexcept
{
    bypassed_ = bypassed;
}

float VariMuDrcProcessor::softKnee(float xDb) const noexcept
{
    // Paper Eq. 2 (docs/increment2-varimudrc-design.md): below/knee/above regions, checked in
    // this exact precedence (matches training/processors/varimu_drc.py::_soft_knee()'s
    // where(above, ..., where(in_knee, ..., below)) composition -- knee wins the boundary at
    // 2*over == +-W since it's the last mask applied there).
    const float over = xDb - T_;
    const float wSafe = std::max(W_, kKneeFloor);

    if (2.0f * over < -W_)
        return xDb;  // below knee: y = x
    if (2.0f * over > W_)
        return T_ + over / R_;  // above knee: y = T + (x-T)/R

    // in knee: y = x + (1/R - 1)*(x - T + W/2)^2 / (2W)
    const float t = over + W_ / 2.0f;
    return xDb + (1.0f / R_ - 1.0f) * (t * t) / (2.0f * wSafe);
}

void VariMuDrcProcessor::process(float* buffer, int numSamples) noexcept
{
    if (bypassed_)
    {
        currentGrDb_.store(0.0f, std::memory_order_relaxed);
        return;
    }

    float peakGrDb = 0.0f;  // 0 = no reduction; tracks the most negative gsDb this buffer

    for (int n = 0; n < numSamples; ++n)
    {
        const float x = buffer[n];

        float xDb = 20.0f * std::log10(std::abs(x) + kEps);
        xDb = std::max(xDb, kMinLevelDb);

        const float yDb = softKnee(xDb);
        const float gcDb = yDb - xDb;  // <= 0, demanded gain reduction

        const float demand = std::clamp(-gcDb / kGrRangeDb, 0.0f, 1.0f);

        // Switching one-pole (paper Eq. 5/6): rising demand -> attack (fast), falling/equal ->
        // release. NOTE the switch direction (a larger normalized value means MORE gain
        // reduction, so a RISING demand is the attack/fast phase) -- getting this backwards
        // silently swaps attack and release.
        const bool attack = demand > gsPrev_;

        // V3 (docs/v3-scope-of-record.md SS2.1): the two branches use DIFFERENT modulation
        // variables, not just different parameters. ATTACK modulates on demand-gsPrev_, the
        // instantaneous error/"distance still to close" -- known from the first sample of the
        // transient, unlike gsPrev_ (the detector's own lagging state), which had nothing correct
        // to act on while the old s_a*gs form's timing mattered. RELEASE is UNCHANGED (verified
        // separately not to share attack's representational ceiling -- changelog/20260729.md, V3
        // Task 4). Matches analysis/tau_eff_form_bakeoff.py::demand_err_form exactly.
        float tauEff;
        if (attack)
            tauEff = std::max(tauA_ * std::exp(-cA_ * (demand - gsPrev_)), kTauEffFloor);
        else
            tauEff = std::max(tauR_ * std::exp(sR_ * gsPrev_), kTauEffFloor);

        const float alpha = std::exp(-1.0f / (tauEff * (float) fs_));
        const float gs = alpha * gsPrev_ + (1.0f - alpha) * demand;
        gsPrev_ = gs;

        const float gsDb = -gs * kGrRangeDb;
        peakGrDb = std::min(peakGrDb, gsDb);
        const float gLin = std::pow(10.0f, gsDb / 20.0f);
        buffer[n] = x * gLin;
    }

    currentGrDb_.store(peakGrDb, std::memory_order_relaxed);
}

void VariMuDrcProcessor::reset() noexcept
{
    gsPrev_ = 0.0f;
}

}  // namespace ua176
