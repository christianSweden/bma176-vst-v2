// Generic evaluator for the trained SIREN nets (dim_in=1, hidden=64, num_layers=3): 3 hidden
// Linear+sin(w0*x) layers (layer0's w0 differs from layers 1-2, read per-layer from the
// generated header rather than assumed) + 1 final Linear+Identity layer (no sine). Matches
// nablafx/nablafx/processors/siren.py::SirenNet/Siren/Sine exactly:
//   Siren.forward: out = sin(w0 * (Wx + b))   (last_layer has no sine, just Wx + b)
// Stateless, memoryless, per-sample -- uses OUR retrained weights
// (weights/static_mlp_nonlinearity_h64-l3_tanh_dense.pt, training/retrain_siren_nl.py), not
// nablafx's originally-shipped file, which was found to memorize its training grid (W12.2).
#pragma once

namespace ua176 {

constexpr int kSirenHiddenDim = 64;

class SirenNonlinearity
{
public:
    SirenNonlinearity(const float* l0w, const float* l0b, float l0w0, const float* l1w,
                       const float* l1b, float l1w0, const float* l2w, const float* l2b,
                       float l2w0, const float* lastW, const float* lastB) noexcept;

    void process(float* buffer, int numSamples) const noexcept;

private:
    float evalSample(float x) const noexcept;

    const float* l0w_;
    const float* l0b_;
    float l0w0_;
    const float* l1w_;
    const float* l1b_;
    float l1w0_;
    const float* l2w_;
    const float* l2b_;
    float l2w0_;
    const float* lastW_;
    const float* lastB_;
};

}  // namespace ua176
