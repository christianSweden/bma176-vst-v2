#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace bm176
{
    class BM176HardwareSwitch : public juce::Component, private juce::Timer
    {
    public:
        BM176HardwareSwitch();
        ~BM176HardwareSwitch() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;

        void setState(bool on);
        bool getState() const;

        void setLabels(const juce::String& title,
                       const juce::String& topLabel,
                       const juce::String& bottomLabel);

        void setCallback(std::function<void(bool)> cb);

        void timerCallback() override;

    private:
        bool state = true;
        float actuatorY = 0.0f;
        float velocity  = 0.0f;
        float targetY   = 0.0f;
        bool  dragging  = false;
        float dragOrigin = 0.0f;
        float dragY      = 0.0f;

        juce::String title;
        juce::String topLabel;
        juce::String bottomLabel;
        std::function<void(bool)> onChange;

        float slotTopY() const;
        float slotBottomY() const;
        float slotTravel() const;
        float clampSlot(float y) const;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BM176HardwareSwitch)
    };
}
