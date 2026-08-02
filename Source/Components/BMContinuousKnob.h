#pragma once

#include "BMKnob.h"

namespace bm176
{
    class BMContinuousKnob : public BMKnob
    {
    public:
        BMContinuousKnob();
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMContinuousKnob)
    };
}
