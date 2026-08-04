#pragma once

#include <functional>
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
        void mouseUp(const juce::MouseEvent& e) override;
        void mouseDoubleClick(const juce::MouseEvent& e) override;
        void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& d) override;

        void setValue(float newValue, juce::NotificationType n = juce::sendNotification);
        float getValue() const;
        void setDefaultValue(float v);

        void setIsBig(bool big);
        void setDiscrete(bool isDiscrete, int numPositions = 11);
        void setVernierMode(bool vernier);
        void setAngleRange(float minDeg, float maxDeg);
        void setValueRange(float minV, float maxV);

        std::function<void()>      onDragStart;
        std::function<void(float)> onValueChange;
        std::function<void()>      onDragEnd;

    protected:
        virtual float snapValue(float raw) const;

        float value = 0.5f;
        bool  isBig       = false;
        bool  isDiscrete  = false;
        bool  isVernier   = false;
        int   numPositions = 11;
        float minAngleDeg  = -150.0f;
        float maxAngleDeg  =  150.0f;
        float minVal       = 0.0f;
        float maxVal       = 10.0f;
        float defaultValue = 5.0f;

    private:
        static constexpr float DRAG_PIXELS_FULL_RANGE = 300.0f;

        float dragStartValue = 0.0f;

        float dragValueRange() const;
        float angleFromValue(float v) const;
        float knobRadius() const;

        void drawShadow(juce::Graphics& g);
        void drawSkirt(juce::Graphics& g);
        void drawKnurling(juce::Graphics& g);
        void drawRimHighlight(juce::Graphics& g);
        void drawTopFace(juce::Graphics& g);
        void drawPointer(juce::Graphics& g);
        void drawAO(juce::Graphics& g);

        void drawStaticLayers(juce::Graphics& g);

        juce::Image cachedStatic;
        bool staticCacheValid = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMKnob)
    };
}
