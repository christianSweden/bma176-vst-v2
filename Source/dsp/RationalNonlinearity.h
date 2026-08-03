// Generic evaluator for the trained Rational (Pade P(x)/Q(x)) waveshapers, version "A":
//   P(x) = a_0 + a_1 x + a_2 x^2 + ... + a_n x^n
//   Q(x) = 1 + |b_1 x| + |b_2 x^2| + ... + |b_m x^m|
// The abs is PER-TERM (each |b_i x^i| individually, then summed) -- NOT version "B"'s
// abs-of-the-sum (1 + |b_1 x + b_2 x^2 + ...|), a different function that would silently
// diverge from the trained model. See rational/torch/rational_pytorch_functions.py::
// Rational_PYTORCH_A_F and docs/siren-to-rational-eval-plan.md (Phase 3 implementation
// contract) for the derivation and the parity landmine this comment is protecting against.
//
// Degree-generic BY DESIGN (docs/siren-to-rational-eval-plan.md Phase 3 guiding rule): numDegree/
// denDegree are per-instance constructor params read from the generated header, never assumed to
// be (6,5). A future capacity bump (measured-curve validation, Phase 4) is a config/header change,
// not a rewrite of this class.
//
// Requires numDegree >= denDegree -- inherited from rational.torch's own construction
// (Rational_PYTORCH_A_F pads the denominator via torch.zeros(len_num-len_deno-1), which raises
// for a negative length, so the upstream library itself assumes this). export_plugin_weights.py's
// extract_rational() asserts the same constraint at export time.
//
// Stateless, memoryless, per-sample -- matches nablafx.processors.ddsp.StaticRationalNonlinearity
// exactly (1-in, 1-out, no conditioning).
#pragma once

namespace ua176 {

// Must be >= the largest deployed Rational's (numDegree + 1) -- the shared power table covers
// x^0..x^numDegree. P6/Q5 (the current shipping config) needs 7; sized with headroom for a
// future capacity bump without touching this constant.
constexpr int kRationalMaxNumCoeffs = 16;

class RationalNonlinearity
{
public:
    RationalNonlinearity(const float* num, int numDegree, const float* den, int denDegree) noexcept;

    void process(float* buffer, int numSamples) const noexcept;

private:
    float evalSample(float x) const noexcept;

    const float* num_;
    int numDegree_;
    const float* den_;
    int denDegree_;
};

}  // namespace ua176
