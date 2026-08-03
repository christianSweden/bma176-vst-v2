#include "MlpController.h"

#include <algorithm>
#include <cmath>

namespace ua176 {

MlpController::MlpController(const float* l0w, const float* l0b, const float* l2w,
                              const float* l2b, const float* l4w, const float* l4b,
                              const float* paramLo, const float* paramHi, int inDim,
                              int outDim) noexcept
    : l0w_(l0w), l0b_(l0b), l2w_(l2w), l2b_(l2b), l4w_(l4w), l4b_(l4b),
      paramLo_(paramLo), paramHi_(paramHi), inDim_(inDim), outDim_(outDim)
{
}

void MlpController::evaluate(const float* cond, float* out) const noexcept
{
    float h1[kMlpHiddenDim];
    for (int o = 0; o < kMlpHiddenDim; ++o)
    {
        float acc = l0b_[o];
        for (int i = 0; i < inDim_; ++i)
            acc += l0w_[o * inDim_ + i] * cond[i];
        h1[o] = std::max(0.0f, acc);
    }

    float h2[kMlpHiddenDim];
    for (int o = 0; o < kMlpHiddenDim; ++o)
    {
        float acc = l2b_[o];
        for (int i = 0; i < kMlpHiddenDim; ++i)
            acc += l2w_[o * kMlpHiddenDim + i] * h1[i];
        h2[o] = std::max(0.0f, acc);
    }

    for (int o = 0; o < outDim_; ++o)
    {
        float acc = l4b_[o];
        for (int i = 0; i < kMlpHiddenDim; ++i)
            acc += l4w_[o * kMlpHiddenDim + i] * h2[i];
        const float sigmoid = 1.0f / (1.0f + std::exp(-acc));
        out[o] = paramLo_[o] + sigmoid * (paramHi_[o] - paramLo_[o]);
    }
}

}  // namespace ua176
