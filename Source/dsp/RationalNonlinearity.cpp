#include "RationalNonlinearity.h"

#include <cmath>

namespace ua176 {

RationalNonlinearity::RationalNonlinearity(const float* num, int numDegree, const float* den,
                                            int denDegree) noexcept
    : num_(num), numDegree_(numDegree), den_(den), denDegree_(denDegree)
{
}

float RationalNonlinearity::evalSample(float x) const noexcept
{
    // Powers of x: pw[k] = x^k for k = 0..numDegree_. One shared table serves both P and Q
    // because numDegree_ >= denDegree_ is guaranteed (see header comment) -- this matches
    // rational.torch's _get_xps, whose column count is max(len_num, len_deno), which for our
    // configs (num_degree+1 > den_degree) is always len_num = numDegree_ + 1 columns.
    float pw[kRationalMaxNumCoeffs];
    pw[0] = 1.0f;
    for (int k = 1; k <= numDegree_; ++k)
        pw[k] = pw[k - 1] * x;

    float numerator = 0.0f;
    for (int k = 0; k <= numDegree_; ++k)
        numerator += num_[k] * pw[k];

    // Q(x) = 1 + |b_1 x| + |b_2 x^2| + ... + |b_m x^m| -- version A, each term abs'd
    // INDIVIDUALLY then summed (NOT abs of the sum -- that's version B; see header comment).
    float denominator = 1.0f;
    for (int k = 1; k <= denDegree_; ++k)
        denominator += std::abs(den_[k - 1] * pw[k]);

    return numerator / denominator;
}

void RationalNonlinearity::process(float* buffer, int numSamples) const noexcept
{
    for (int n = 0; n < numSamples; ++n)
        buffer[n] = evalSample(buffer[n]);
}

}  // namespace ua176
