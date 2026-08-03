// 5 cascaded biquads in the exact order nablafx/nablafx/processors/ddsp.py::ParametricEQ builds
// its sos stack: low_shelf, band0(peaking), band1(peaking), band2(peaking), high_shelf. The 15
// input params must be in the SAME order as VariMuDRC's param_ranges dict iteration order (the
// controller's own output-channel order): low_shelf_{gain_db,cutoff_freq,q_factor},
// band0_{...}, band1_{...}, band2_{...}, high_shelf_{...}.
#pragma once

#include "Biquad.h"

namespace ua176 {

class ParametricEqProcessor
{
public:
    // p[0..14] in the order documented above.
    void setParams(const float* p, double sampleRate) noexcept;
    void process(float* buffer, int numSamples) noexcept;
    void reset() noexcept;

private:
    Biquad lowShelf_, band0_, band1_, band2_, highShelf_;
};

}  // namespace ua176
