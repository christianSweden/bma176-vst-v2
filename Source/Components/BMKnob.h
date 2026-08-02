#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace bm176
{
    class BMKnob : public juce::Component
    {
    public:
        BMKnob();

        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseDoubleClick(const juce::MouseEvent& e) override;
        void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& d) override;

        void setValue(float newValue);
        float getValue() const;

        void setIsBig(bool big);
        void setDiscrete(bool isDiscrete, int numPositions = 11);
        void setCentreDetent(bool centre);

    protected:
        virtual float snapValue(float raw) const;

        float value = 0.5f;
        bool  isBig       = false;
        bool  isDiscrete  = false;
        int   numPositions = 11;
        bool  centreDetent = false;

    private:
        float dragStartValue = 0.0f;
        int   dragStartY     = 0;

        float angleFromValue(float v) const;
        float valueFromAngle(float angle) const;
        float knobRadius() const;

        void drawShadow(juce::Graphics& g);
        void drawSkirt(juce::Graphics& g);
        void drawKnurling(juce::Graphics& g);
        void drawRimHighlight(juce::Graphics& g);
        void drawTopFace(juce::Graphics& g);
        void drawPointer(juce::Graphics& g);

        bool  snappedToDetent = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMKnob)
    };
}
