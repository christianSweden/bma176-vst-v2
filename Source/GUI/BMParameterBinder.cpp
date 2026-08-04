#include "BMParameterBinder.h"

namespace bm176
{
    BMKnobBinder::BMKnobBinder(juce::AudioProcessorValueTreeState& apvts,
                               const juce::String& paramID,
                               BMKnob& knobToBind,
                               Convert knobToParamFn,
                               Convert paramToKnobFn)
        : knob(knobToBind)
        , knobToParam(knobToParamFn ? std::move(knobToParamFn) : [](float v) { return v; })
        , paramToKnob(paramToKnobFn ? std::move(paramToKnobFn) : [](float v) { return v; })
        , attachment(*apvts.getParameter(paramID),
                     [this](float newParamValue)
                     {
                         knob.setValue(paramToKnob(newParamValue), juce::dontSendNotification);
                     })
    {
        auto* param = apvts.getParameter(paramID);
        jassert(param != nullptr);

        knob.setDefaultValue(paramToKnob(param->convertFrom0to1(param->getDefaultValue())));

        knob.onDragStart   = [this] { attachment.beginGesture(); };
        knob.onValueChange = [this](float v) { attachment.setValueAsPartOfGesture(knobToParam(v)); };
        knob.onDragEnd     = [this] { attachment.endGesture(); };

        attachment.sendInitialUpdate();
    }

    BMSwitchBinder::BMSwitchBinder(juce::AudioProcessorValueTreeState& apvts,
                                   const juce::String& paramID,
                                   BM176HardwareSwitch& switchToBind,
                                   bool inverted)
        : sw(switchToBind)
        , invert(inverted)
        , attachment(*apvts.getParameter(paramID),
                     [this](float v)
                     {
                         sw.setState(invert ? (v < 0.5f) : (v >= 0.5f), juce::dontSendNotification);
                     })
    {
        sw.setCallback([this](bool on)
        {
            attachment.setValueAsCompleteGesture(invert ? (on ? 0.0f : 1.0f)
                                                        : (on ? 1.0f : 0.0f));
        });

        attachment.sendInitialUpdate();
    }
}
