// Matches nablafx/nablafx/processors/ddsp.py::Gain.process(): y = x * 10^(gain_db/20). Stateless
// (no persistent audio state, so no reset() -- GreyBoxChain::reset() does not call into this).
#pragma once

namespace ua176 {

class GainProcessor
{
public:
    void setGainDb(float gainDb) noexcept;
    void process(float* buffer, int numSamples) const noexcept;

private:
    float gainLin_ = 1.0f;
};

}  // namespace ua176
