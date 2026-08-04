#pragma once

#include <atomic>
#include <memory>
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
#include "BMParameterBinder.h"

namespace bm176
{
    class BM176Editor : public juce::Component
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

        // Binders are constructed at the end of the constructor body, once every knob
        // has been configured (setDiscrete/setVernierMode/setAngleRange/setValueRange).
        // BMKnobBinder/BMSwitchBinder fire an initial host->GUI update synchronously from
        // their constructor, and BMKnob::setValue() clamps against isVernier/minVal/maxVal
        // as they stand at that moment — constructing a binder before its knob is
        // configured would clamp the vernier knobs' -1..1 default into their pre-config
        // range of [0, 10].
        std::unique_ptr<BMKnobBinder> ratioBinder;
        std::unique_ptr<BMKnobBinder> sidechainBinder;
        std::unique_ptr<BMKnobBinder> inputBinder;
        std::unique_ptr<BMKnobBinder> meterBinder;
        std::unique_ptr<BMKnobBinder> vernierInBinder;
        std::unique_ptr<BMKnobBinder> thresholdBinder;
        std::unique_ptr<BMKnobBinder> attackBinder;
        std::unique_ptr<BMKnobBinder> vernierOutBinder;
        std::unique_ptr<BMKnobBinder> outputBinder;
        std::unique_ptr<BMKnobBinder> releaseBinder;

        std::unique_ptr<BMSwitchBinder> interstageBinder;
        std::unique_ptr<BMSwitchBinder> attackOffBinder;
        std::unique_ptr<BMSwitchBinder> bypassBinder;
        std::unique_ptr<BMSwitchBinder> powerBinder;

        MeterSource getGainReductionFn;
        MeterSource getInputLevelFn;
        MeterSource getOutputLevelFn;

        const std::atomic<float>* pMeterMode = nullptr;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BM176Editor)
    };
}
