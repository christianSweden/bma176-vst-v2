#include "GainProcessor.h"

#include <cmath>

namespace ua176 {

void GainProcessor::setGainDb(float gainDb) noexcept
{
    gainLin_ = std::pow(10.0f, gainDb / 20.0f);
}

void GainProcessor::process(float* buffer, int numSamples) const noexcept
{
    for (int n = 0; n < numSamples; ++n)
        buffer[n] *= gainLin_;
}

}  // namespace ua176
