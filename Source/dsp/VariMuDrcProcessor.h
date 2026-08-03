// Faithful port of training/processors/varimu_drc.py::VariMuDRC.process()/_soft_knee()/
// _switching_one_pole(). Takes ALREADY-DENORMALIZED physical params (T,R,W,tau_a,tau_r,c_a,s_r --
// the MlpController's job, not this class's). The ONE stateful, per-sample-recursive block in the
// whole chain: gsPrev_ persists across process() calls, cleared only by reset() (called at stream
// start, matching training/validate_checkpoint.py's model.reset_states() discipline before every
// independent test tone).
//
// V3 (2026-07-30, docs/v3-scope-of-record.md SS2.1): the ATTACK branch's modulation term was
// replaced -- s_a (tau_eff = tau_a*exp(s_a*gs)) is gone, replaced by c_a
// (tau_eff = tau_a*exp(-c_a*(demand-gs))). gs is the detector's OWN lagging state -- by
// construction it has not reached its target during the transient being timed, so the old term
// had nothing correct to act on while it mattered. demand-gs is the instantaneous error (how far
// the detector still has to move), known from the first sample. RELEASE is unchanged
// (tau_eff = tau_r*exp(s_r*gs)) -- this was verified separately to NOT share attack's
// representational ceiling (changelog/20260729.md, V3 Task 4: tau_r-only already fits real
// per-burst release targets to within 0.2-2.6%).
//
// Numerical floors ported EXACTLY -- do not drop any of these, each one prevents a specific
// inf/NaN failure mode documented in the Python source's own comments:
//   eps=1e-5 before log10 (silent samples), min_level_db=-80 clamp, knee-width floor 1e-12,
//   tau_eff clamp min=1e-6.
#pragma once

#include <atomic>

namespace ua176 {

class VariMuDrcProcessor
{
public:
    void prepare(double sampleRate) noexcept;
    void setParams(float T, float R, float W, float tauA, float tauR, float cA, float sR) noexcept;
    void setBypassed(bool bypassed) noexcept;
    void process(float* buffer, int numSamples) noexcept;
    void reset() noexcept;

    // Peak gain reduction (dB, <= 0) over the most recently processed buffer -- 0 = no reduction.
    // Safe to call from any thread (relaxed atomic, standard JUCE metering idiom -- see
    // PluginProcessor's own getGainReductionDb()); written once per process() call, not per-sample.
    float getGainReductionDb() const noexcept { return currentGrDb_.load(std::memory_order_relaxed); }

private:
    float softKnee(float xDb) const noexcept;

    double fs_ = 48000.0;
    float T_ = 0.0f, R_ = 1.0f, W_ = 0.0f, tauA_ = 0.05f, tauR_ = 1.0f, cA_ = 0.0f, sR_ = 0.0f;
    float gsPrev_ = 0.0f;
    bool bypassed_ = false;
    std::atomic<float> currentGrDb_ { 0.0f };
};

}  // namespace ua176
