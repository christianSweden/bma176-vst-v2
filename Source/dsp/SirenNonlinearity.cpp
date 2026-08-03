#include "SirenNonlinearity.h"

#include <cmath>

namespace ua176 {

SirenNonlinearity::SirenNonlinearity(const float* l0w, const float* l0b, float l0w0,
                                      const float* l1w, const float* l1b, float l1w0,
                                      const float* l2w, const float* l2b, float l2w0,
                                      const float* lastW, const float* lastB) noexcept
    : l0w_(l0w), l0b_(l0b), l0w0_(l0w0), l1w_(l1w), l1b_(l1b), l1w0_(l1w0),
      l2w_(l2w), l2b_(l2b), l2w0_(l2w0), lastW_(lastW), lastB_(lastB)
{
}

float SirenNonlinearity::evalSample(float x) const noexcept
{
    // Layer 0: dim_in=1, so l0w_ is (64,1) flattened -- l0w_[o] directly, no inner loop needed.
    float h0[kSirenHiddenDim];
    for (int o = 0; o < kSirenHiddenDim; ++o)
        h0[o] = std::sin(l0w0_ * (l0w_[o] * x + l0b_[o]));

    float h1[kSirenHiddenDim];
    for (int o = 0; o < kSirenHiddenDim; ++o)
    {
        float acc = l1b_[o];
        for (int i = 0; i < kSirenHiddenDim; ++i)
            acc += l1w_[o * kSirenHiddenDim + i] * h0[i];
        h1[o] = std::sin(l1w0_ * acc);
    }

    float h2[kSirenHiddenDim];
    for (int o = 0; o < kSirenHiddenDim; ++o)
    {
        float acc = l2b_[o];
        for (int i = 0; i < kSirenHiddenDim; ++i)
            acc += l2w_[o * kSirenHiddenDim + i] * h1[i];
        h2[o] = std::sin(l2w0_ * acc);
    }

    // last_layer: dim_out=1, Identity activation -- no sine.
    float acc = lastB_[0];
    for (int i = 0; i < kSirenHiddenDim; ++i)
        acc += lastW_[i] * h2[i];
    return acc;
}

void SirenNonlinearity::process(float* buffer, int numSamples) const noexcept
{
    for (int n = 0; n < numSamples; ++n)
        buffer[n] = evalSample(buffer[n]);
}

}  // namespace ua176
