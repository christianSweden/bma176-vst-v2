#pragma once

#include "BMKnob.h"

namespace bm176
{
    class BMDiscreteKnob : public BMKnob
    {
    public:
        BMDiscreteKnob();
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMDiscreteKnob)
    };
}
