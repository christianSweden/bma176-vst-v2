#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Components/BMPanel.h"
#include "../Components/BMContinuousKnob.h"
#include "../Components/BMDiscreteKnob.h"
#include "../Components/BMToggleSwitch.h"
#include "../Components/BMVU.h"
#include "../Components/BMLED.h"
#include "../Components/BMJackSocket.h"

namespace bm176
{
    class BM176Editor : public juce::Component
    {
    public:
        BM176Editor();
        void resized() override;
        void paint(juce::Graphics&) override {}

    private:
        BMPanel panel;

        BMDiscreteKnob  ratioKnob;
        BMDiscreteKnob  sidechainKnob;
        BMContinuousKnob inputKnob;
        BMDiscreteKnob  meterKnob;
        BMContinuousKnob vernierInKnob;
        BMContinuousKnob thresholdKnob;
        BMContinuousKnob attackKnob;
        BMContinuousKnob vernierOutKnob;
        BMContinuousKnob outputKnob;
        BMContinuousKnob releaseKnob;

        BMVU vuMeter;

        BMToggleSwitch interstageSwitch;
        BMToggleSwitch bypassSwitch;
        BMToggleSwitch powerSwitch;

        BMLED onLamp;

        BMJackSocket inputJack;
        BMJackSocket hiZJack;
        BMJackSocket outputJack;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BM176Editor)
    };
}
