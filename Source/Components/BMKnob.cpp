#include "BMKnob.h"
#include "../GUI/BM176Colours.h"

namespace bm176
{
    inline constexpr float TWO_PI = juce::MathConstants<float>::twoPi;
    inline constexpr float DEG2RAD = juce::MathConstants<float>::pi / 180.0f;

    BMKnob::BMKnob()
    {
        setRepaintsOnMouseActivity(false);
        setWantsKeyboardFocus(false);
    }

    float BMKnob::knobRadius() const
    {
        return juce::jmin(getWidth(), getHeight()) * 0.5f;
    }

    void BMKnob::setIsBig(bool b)    { isBig = b; staticCacheValid = false; }
    void BMKnob::setDiscrete(bool d, int n) { isDiscrete = d; numPositions = juce::jmax(2, n); }
    void BMKnob::setVernierMode(bool v)   { isVernier = v; }
    void BMKnob::setAngleRange(float minDeg, float maxDeg) { minAngleDeg = minDeg; maxAngleDeg = maxDeg; staticCacheValid = false; }
    void BMKnob::setValueRange(float minV, float maxV) { minVal = minV; maxVal = maxV; }
    void BMKnob::setDefaultValue(float v) { defaultValue = v; }

    float BMKnob::dragValueRange() const
    {
        return isVernier ? 2.0f : (maxVal - minVal);
    }

    float BMKnob::angleFromValue(float v) const
    {
        if (isVernier)
            return v * maxAngleDeg;
        const float range = maxAngleDeg - minAngleDeg;
        return minAngleDeg + (v / 10.0f) * range;
    }

    float BMKnob::snapValue(float raw) const
    {
        if (isVernier)
        {
            raw = juce::jlimit(-1.0f, 1.0f, raw);
            if (std::abs(raw) < 0.15f) return 0.0f;
            return raw;
        }
        if (!isDiscrete) return juce::jlimit(0.0f, 10.0f, raw);
        const int n = numPositions - 1;
        const int idx = juce::roundToInt(raw / 10.0f * static_cast<float>(n));
        return juce::jlimit(0.0f, 10.0f,
            static_cast<float>(juce::jlimit(0, n, idx)) / static_cast<float>(n) * 10.0f);
    }

    void BMKnob::setValue(float newValue, juce::NotificationType n)
    {
        if (isVernier)
            newValue = juce::jlimit(-1.0f, 1.0f, snapValue(newValue));
        else
            newValue = juce::jlimit(minVal, maxVal, snapValue(newValue));
        if (!juce::approximatelyEqual(value, newValue))
        {
            value = newValue;
            repaint();
            if (n != juce::dontSendNotification && onValueChange)
                onValueChange(value);
        }
    }

    float BMKnob::getValue() const { return value; }

    void BMKnob::mouseDown(const juce::MouseEvent& e)
    {
        dragStartValue = value;
        e.source.enableUnboundedMouseMovement(true);
        setMouseCursor(juce::MouseCursor::NoCursor);
        if (onDragStart) onDragStart();
    }

    void BMKnob::mouseDrag(const juce::MouseEvent& e)
    {
        float sensitivity = dragValueRange() / DRAG_PIXELS_FULL_RANGE;
        if (e.mods.isShiftDown() || e.mods.isCommandDown() || e.mods.isCtrlDown())
            sensitivity *= 0.2f;

        setValue(dragStartValue - static_cast<float>(e.getDistanceFromDragStartY()) * sensitivity);
    }

    void BMKnob::mouseUp(const juce::MouseEvent&)
    {
        setMouseCursor(juce::MouseCursor::NormalCursor);
        juce::Desktop::getInstance().getMainMouseSource()
            .setScreenPosition(localPointToGlobal(getLocalBounds().toFloat().getCentre()));
        if (onDragEnd) onDragEnd();
    }

    void BMKnob::mouseDoubleClick(const juce::MouseEvent&)
    {
        if (onDragStart) onDragStart();
        setValue(defaultValue);
        if (onDragEnd) onDragEnd();
    }

    void BMKnob::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& d)
    {
        const float dy = ! juce::approximatelyEqual(d.deltaY, 0.0f) ? d.deltaY : d.deltaX;
        if (juce::approximatelyEqual(dy, 0.0f)) return;
        const float dir = dy > 0.0f ? 1.0f : -1.0f;

        float step = isDiscrete ? 10.0f / static_cast<float>(numPositions - 1)
                                : dragValueRange() / 50.0f;
        if (e.mods.isShiftDown()) step *= 0.2f;

        if (onDragStart) onDragStart();
        setValue(value + dir * step);
        if (onDragEnd) onDragEnd();
    }

    void BMKnob::resized()
    {
        staticCacheValid = false;
    }

    void BMKnob::paint(juce::Graphics& g)
    {
        if (!staticCacheValid || !cachedStatic.isValid()
            || cachedStatic.getWidth() != getWidth() || cachedStatic.getHeight() != getHeight())
        {
            cachedStatic = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
            juce::Graphics cg(cachedStatic);
            drawStaticLayers(cg);
            staticCacheValid = true;
        }
        g.drawImageAt(cachedStatic, 0, 0);
        drawPointer(g);
    }

    void BMKnob::drawStaticLayers(juce::Graphics& g)
    {
        drawShadow(g);
        drawSkirt(g);
        drawKnurling(g);
        drawRimHighlight(g);
        drawTopFace(g);
        drawAO(g);
    }

    void BMKnob::drawAO(juce::Graphics& g)
    {
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f;
        const float R = knobRadius();
        const float aoR = R * 1.02f;

        juce::ColourGradient ao(
            juce::Colours::black.withAlpha(0.28f), cx, cy + aoR,
            juce::Colours::black.withAlpha(0.0f), cx, cy, false);
        g.setGradientFill(ao);
        g.fillEllipse(cx - aoR, cy - aoR, aoR * 2.0f, aoR * 2.0f);
    }

    void BMKnob::drawShadow(juce::Graphics& g)
    {
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f;
        const float R = knobRadius();
        float shadowAlpha = isBig ? 0.50f : 0.45f;
        int   shadowBlur  = isBig ? 14 : 9;

        juce::Path sp;
        sp.addEllipse(juce::Rectangle<float>(cx - R, cy - R + 3.0f, R * 2.0f, R * 2.0f));
        juce::DropShadow ds(
            juce::Colours::black.withAlpha(shadowAlpha), shadowBlur,
            isBig ? juce::Point<int>(1, 6) : juce::Point<int>(1, 4));
        ds.drawForPath(g, sp);
    }

    void BMKnob::drawSkirt(juce::Graphics& g)
    {
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f;
        const float R = knobRadius();

        juce::Path skirt;
        skirt.addEllipse(juce::Rectangle<float>(cx - R, cy - R, R * 2.0f, R * 2.0f));
        juce::ColourGradient skirtGrad(knobLight, cx - R * 0.5f, cy - R * 0.5f,
                                       knobDark, cx + R * 0.8f, cy + R * 0.8f, true);
        g.setGradientFill(skirtGrad);
        g.fillPath(skirt);

        juce::ColourGradient topLight(
            juce::Colours::white.withAlpha(0.08f), cx, cy - R * 0.8f,
            juce::Colours::white.withAlpha(0.0f),  cx, cy, false);
        g.setGradientFill(topLight);
        g.fillPath(skirt);

        g.setColour(juce::Colours::black.withAlpha(0.25f));
        g.drawEllipse(cx - R, cy - R, R * 2.0f, R * 2.0f, 1.2f);
    }

    void BMKnob::drawKnurling(juce::Graphics& g)
    {
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f;
        const float R = knobRadius();
        const int numFlutes = isBig ? 72 : 48;
        const float innerR = R * 0.86f;

        for (int i = 0; i < numFlutes; ++i)
        {
            const float a = static_cast<float>(i) * TWO_PI / static_cast<float>(numFlutes);
            const float x1 = cx + innerR * std::cos(a);
            const float y1 = cy + innerR * std::sin(a);
            const float x2 = cx + R * std::cos(a);
            const float y2 = cy + R * std::sin(a);

            const float topLight = std::max(0.0f, -std::sin(a));
            const float alpha = topLight * 0.08f;

            g.setColour((i % 2 == 0)
                ? knobFluteLight.withAlpha(0.30f + alpha)
                : knobFluteDark.withAlpha(0.40f + alpha));
            g.drawLine(x1, y1, x2, y2, 1.4f);
        }
    }

    void BMKnob::drawRimHighlight(juce::Graphics& g)
    {
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f;
        const float R = knobRadius();

        juce::Path arc;
        arc.addCentredArc(cx, cy, R * 0.97f, R * 0.97f, 0.0f,
            juce::degreesToRadians(200.0f), juce::degreesToRadians(340.0f), true);
        g.setColour(knobRimHi.withAlpha(0.35f));
        g.strokePath(arc, juce::PathStrokeType(2.0f));
    }

    void BMKnob::drawTopFace(juce::Graphics& g)
    {
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f;
        const float R = knobRadius();
        const float faceR = R * 0.80f;

        juce::Path face;
        face.addEllipse(juce::Rectangle<float>(cx - faceR, cy - faceR, faceR * 2.0f, faceR * 2.0f));
        juce::ColourGradient faceGrad(knobFaceTop, cx - faceR * 0.3f, cy - faceR * 0.3f,
                                      knobFaceBottom, cx + faceR * 0.3f, cy + faceR * 0.3f, false);
        g.setGradientFill(faceGrad);
        g.fillPath(face);

        juce::ColourGradient metalInlay(
            screwTop.brighter(0.1f).withAlpha(0.15f), cx - faceR * 0.5f, cy - faceR * 0.5f,
            screwBottom.darker(0.1f).withAlpha(0.05f), cx + faceR * 0.5f, cy + faceR * 0.5f, false);
        g.setGradientFill(metalInlay);
        g.fillPath(face);

        g.setColour(juce::Colours::black.withAlpha(0.45f));
        g.drawEllipse(cx - faceR, cy - faceR, faceR * 2.0f, faceR * 2.0f, 2.0f);

        g.setColour(juce::Colours::white.withAlpha(0.04f));
        g.drawEllipse(cx - faceR + 1.0f, cy - faceR + 1.0f, (faceR - 1.0f) * 2.0f, (faceR - 1.0f) * 2.0f, 1.0f);
    }

    void BMKnob::drawPointer(juce::Graphics& g)
    {
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f;
        const float R = knobRadius();
        const float angleDeg = angleFromValue(value);
        const float angle = angleDeg * DEG2RAD;
        const float ptrW = isBig ? 6.0f : 4.0f;

        const float x1 = cx + R * 0.20f * std::sin(angle);
        const float y1 = cy - R * 0.20f * std::cos(angle);
        const float x2 = cx + R * 0.90f * std::sin(angle);
        const float y2 = cy - R * 0.90f * std::cos(angle);

        juce::Path ptr;
        ptr.addLineSegment(juce::Line<float>(x1, y1, x2, y2), 0.0f);
        juce::PathStrokeType stroke(ptrW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);

        juce::Path shadowPtr;
        stroke.createStrokedPath(shadowPtr, ptr, juce::AffineTransform::translation(0.0f, 1.5f));
        g.setColour(juce::Colours::black.withAlpha(0.40f));
        g.fillPath(shadowPtr);

        g.setColour(pointerWhite.darker(0.05f));
        g.strokePath(ptr, stroke);

        juce::Path bevelPtr;
        juce::PathStrokeType bevelStroke(ptrW * 0.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
        juce::Path bv;
        bevelStroke.createStrokedPath(bv, ptr, juce::AffineTransform::translation(0.0f, -ptrW * 0.2f));
        g.setColour(pointerWhite.brighter(0.15f).withAlpha(0.5f));
        g.fillPath(bv);
    }
}
