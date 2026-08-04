#pragma once

#include <functional>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../Components/BMKnob.h"
#include "../Components/BM176HardwareSwitch.h"

namespace bm176
{
    // Binds a BMKnob to an APVTS parameter: forwards user gestures to the host with
    // begin/end automation brackets, and mirrors host/automation changes back onto the
    // knob on the message thread without re-triggering onValueChange.
    class BMKnobBinder
    {
    public:
        using Convert = std::function<float(float)>;

        BMKnobBinder(juce::AudioProcessorValueTreeState& apvts,
                     const juce::String& paramID,
                     BMKnob& knobToBind,
                     Convert knobToParamFn = nullptr,
                     Convert paramToKnobFn = nullptr);

    private:
        BMKnob&  knob;
        Convert  knobToParam;
        Convert  paramToKnob;
        juce::ParameterAttachment attachment;   // must be declared last

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMKnobBinder)
    };

    // Binds a BM176HardwareSwitch to a bool APVTS parameter. `inverted` handles switches
    // where the on-screen "on" state maps to a parameter value of 0 (e.g. power).
    class BMSwitchBinder
    {
    public:
        BMSwitchBinder(juce::AudioProcessorValueTreeState& apvts,
                       const juce::String& paramID,
                       BM176HardwareSwitch& switchToBind,
                       bool inverted = false);

    private:
        BM176HardwareSwitch& sw;
        bool invert;
        juce::ParameterAttachment attachment;   // must be declared last

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMSwitchBinder)
    };
}
