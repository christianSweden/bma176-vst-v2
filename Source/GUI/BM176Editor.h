#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../Components/BMPanel.h"
#include "../Components/BMContinuousKnob.h"
#include "../Components/BMDiscreteKnob.h"
#include "../Components/BMToggleSwitch.h"
#include "../Components/BM176HardwareSwitch.h"
#include "../Components/BMVU.h"
#include "../Components/BMLED.h"
#include "../Components/BMJackSocket.h"

namespace bm176
{
    class BM176Editor : public juce::Component,
                        private juce::AudioProcessorValueTreeState::Listener
    {
    public:
        BM176Editor(juce::AudioProcessorValueTreeState& apvts);
        ~BM176Editor() override;
        void resized() override;
        void paint(juce::Graphics&) override {}

        using MeterSource = std::function<float()>;
        void setMeterSources(MeterSource gainReduction, MeterSource inputLevel,
                             MeterSource outputLevel);

        void timerCallback();

    private:
        void parameterChanged(const juce::String& parameterID, float newValue) override;

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

        BM176HardwareSwitch interstageSwitch;
        BM176HardwareSwitch attackOffSwitch;
        BM176HardwareSwitch powerSwitch;
        BM176HardwareSwitch bypassSwitch;

        BMLED onLamp;

        BMJackSocket inputJack;
        BMJackSocket hiZJack;
        BMJackSocket outputJack;

        juce::AudioProcessorValueTreeState& apvts;

        MeterSource getGainReductionFn;
        MeterSource getInputLevelFn;
        MeterSource getOutputLevelFn;

        int meterModeIdx = 1;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BM176Editor)
    };
}
