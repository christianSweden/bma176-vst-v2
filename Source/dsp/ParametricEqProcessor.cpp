#include "ParametricEqProcessor.h"

namespace ua176 {

void ParametricEqProcessor::setParams(const float* p, double sampleRate) noexcept
{
    lowShelf_.computeCoeffs(BiquadType::LowShelf, p[0], p[1], p[2], sampleRate);
    band0_.computeCoeffs(BiquadType::Peaking, p[3], p[4], p[5], sampleRate);
    band1_.computeCoeffs(BiquadType::Peaking, p[6], p[7], p[8], sampleRate);
    band2_.computeCoeffs(BiquadType::Peaking, p[9], p[10], p[11], sampleRate);
    highShelf_.computeCoeffs(BiquadType::HighShelf, p[12], p[13], p[14], sampleRate);
}

void ParametricEqProcessor::process(float* buffer, int numSamples) noexcept
{
    for (int n = 0; n < numSamples; ++n)
    {
        float x = buffer[n];
        x = lowShelf_.processSample(x);
        x = band0_.processSample(x);
        x = band1_.processSample(x);
        x = band2_.processSample(x);
        x = highShelf_.processSample(x);
        buffer[n] = x;
    }
}

void ParametricEqProcessor::reset() noexcept
{
    lowShelf_.reset();
    band0_.reset();
    band1_.reset();
    band2_.reset();
    highShelf_.reset();
}

}  // namespace ua176
