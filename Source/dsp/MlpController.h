// Generic evaluator for the trained HC-MLP controllers: Linear(in,16)->ReLU->Linear(16,16)->ReLU
// ->Linear(16,N)->Sigmoid, output denormalized per-param via lo + sigmoid_out*(hi-lo). Matches
// training/nablafx_patches.py::ReLUStaticCondController (eq, in=7 -- the only block still on the
// full vector), ::SubsetCondController (gain_in in=2, gain_out in=1 -- docs/greybox-structural-
// discipline-plan.md, 2026-07-26) and ::SplitVariMuController's two sub-MLPs (curve, in=4;
// ballistics, in=3 -- docs/controller-split-plan.md) exactly, plus nablafx's own
// denormalize_parameters() formula. Weight layout: row-major "out-then-in" (W[o*in+i]), matching
// PyTorch nn.Linear storage directly -- see Source/dsp/generated/UA176Weights.h's header comment.
//
// inDim is a per-instance constructor param (not a fixed global), since curve/ballistics/gain_in/
// gain_out all see conditioning subsets narrower than the full 7-vector (eq is the only one that
// still doesn't) -- see GreyBoxChain, which builds each controller's own subset array before
// calling evaluate(). `inDim` itself comes from the exported UA176Weights.h's <block>_in_dim
// constant, so `evaluate()`'s `cond` buffer and the header's declared inDim must agree -- a
// stale header (e.g. one exported before the gain-isolation change) paired with the patched
// GreyBoxChain.cpp below would read past a 1-2-element stack array, not fail to compile. Always
// re-export after a conditioning-subset change, before building.
#pragma once

namespace ua176 {

// kMlpMaxOutDim must be >= the largest controller's param count (ParametricEQ = 15).
constexpr int kMlpMaxOutDim = 15;
constexpr int kMlpHiddenDim = 16;

class MlpController
{
public:
    MlpController(const float* l0w, const float* l0b, const float* l2w, const float* l2b,
                  const float* l4w, const float* l4b, const float* paramLo, const float* paramHi,
                  int inDim, int outDim) noexcept;

    // `cond` must point to at least inDim() valid floats. Writes outDim() denormalized physical
    // parameters into `out` (caller-owned, size >= outDim()).
    void evaluate(const float* cond, float* out) const noexcept;

    int inDim() const noexcept { return inDim_; }
    int outDim() const noexcept { return outDim_; }

private:
    const float* l0w_;
    const float* l0b_;
    const float* l2w_;
    const float* l2b_;
    const float* l4w_;
    const float* l4b_;
    const float* paramLo_;
    const float* paramHi_;
    int inDim_;
    int outDim_;
};

}  // namespace ua176
