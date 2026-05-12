/* Copyright 2026 Alexandr Sakhnov
 *
 * This file is part of vitOTTx.
 *
 * vitOTTx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vitOTTx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with vitOTTx. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Knob.h"

#include <cmath>

namespace vitottx
{

namespace
{
    constexpr float kStartAngle = -juce::MathConstants<float>::pi * 0.75f;
    constexpr float kEndAngle   =  juce::MathConstants<float>::pi * 0.75f;

    constexpr float kArcThickness     = 4.5f;
    constexpr float kBodyInset        = 7.0f;
    constexpr float kPointerThickness = 2.5f;
    constexpr float kPointerInnerFrac = 0.30f;
    constexpr float kPointerOuterPad  = 4.0f;
    constexpr float kShadowRadius     = 6.0f;
    constexpr float kShadowYOffset    = 2.0f;
    constexpr float kShadowAlpha      = 0.55f;
}

Knob::Knob(const Theme& t) : theme(t)
{
    setSliderStyle(juce::Slider::RotaryVerticalDrag);
    setRotaryParameters(kStartAngle, kEndAngle, true);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
}

void Knob::setCaption(const juce::String& text)
{
    caption = text;
    repaint();
}

void Knob::attachTo(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
{
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, paramId, *this);
    parameter = apvts.getParameter(paramId);
}

juce::String Knob::formatValueText() const
{
    if (valueFormatter)
        return valueFormatter(getValue());
    if (parameter != nullptr)
        return parameter->getCurrentValueAsText();
    return juce::String(getValue(), 2);
}

void Knob::resized()
{
    auto bounds = getLocalBounds();
    const int captionH = theme.labelHeight();
    captionArea = bounds.removeFromBottom(captionH);

    const int rotaryDiameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
    const int x = bounds.getX() + (bounds.getWidth() - rotaryDiameter) / 2;
    const int y = bounds.getBottom() - rotaryDiameter;
    rotaryArea = { x, y, rotaryDiameter, rotaryDiameter };
}

bool Knob::hitTest(int x, int y)
{
    return rotaryArea.contains(x, y);
}

void Knob::paint(juce::Graphics& g)
{
    const auto bounds  = rotaryArea.toFloat();
    const float side   = juce::jmin(bounds.getWidth(), bounds.getHeight());
    const float radius = side * 0.5f;
    const float cx     = bounds.getCentreX();
    const float cy     = bounds.getCentreY();

    const float arcThickness = theme.scaled(kArcThickness);
    const float bodyInset    = theme.scaled(kBodyInset);
    const float bodyRadius   = juce::jmax(1.0f, radius - bodyInset);
    const float arcRadius    = radius - arcThickness * 0.5f;

    const float value     = static_cast<float>(valueToProportionOfLength(getValue()));
    const float currentA  = kStartAngle + (kEndAngle - kStartAngle) * value;

    const auto& pal = theme.palette();

    juce::Path bodyShape;
    bodyShape.addEllipse(cx - bodyRadius, cy - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f);

    juce::DropShadow shadow(juce::Colours::black.withAlpha(kShadowAlpha),
                            juce::roundToInt(theme.scaled(kShadowRadius)),
                            { 0, juce::roundToInt(theme.scaled(kShadowYOffset)) });
    shadow.drawForPath(g, bodyShape);

    juce::Path track;
    track.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f,
                        kStartAngle, kEndAngle, true);
    g.setColour(pal.knobArcTrack);
    g.strokePath(track, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));

    if (value > 0.0001f)
    {
        juce::Path active;
        active.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f,
                             kStartAngle, currentA, true);
        g.setColour(pal.knobArcActive);
        g.strokePath(active, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    g.setColour(pal.knobBody);
    g.fillEllipse(cx - bodyRadius, cy - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f);

    const float pointerThickness = theme.scaled(kPointerThickness);
    const float pointerInner     = bodyRadius * kPointerInnerFrac;
    const float pointerOuter     = bodyRadius - theme.scaled(kPointerOuterPad);
    const float dx = std::sin(currentA);
    const float dy = -std::cos(currentA);

    juce::Path pointer;
    pointer.startNewSubPath(cx + dx * pointerInner, cy + dy * pointerInner);
    pointer.lineTo         (cx + dx * pointerOuter, cy + dy * pointerOuter);
    g.setColour(pal.knobPointer);
    g.strokePath(pointer, juce::PathStrokeType(pointerThickness, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    if (caption.isNotEmpty())
    {
        g.setColour(pal.textSecondary);
        g.setFont(juce::FontOptions(theme.fontLabel()));
        g.drawText(caption, captionArea, juce::Justification::centredTop, false);
    }
}

//==============================================================================
// Drag

void Knob::mouseDown(const juce::MouseEvent& e)
{
    juce::Slider::mouseDown(e);
    if (!isEnabled())
        return;

    dragStartProp = static_cast<float>(valueToProportionOfLength(getValue()));
    dragStartY    = e.position.y;
    wasShift      = e.mods.isShiftDown();
    drag.begin(*this, e.source);

    if (onShowReadout) onShowReadout(caption, formatValueText());
}

void Knob::mouseDrag(const juce::MouseEvent& e)
{
    if (!isEnabled())
        return;

    const bool shift = e.mods.isShiftDown();
    if (shift != wasShift)
    {
        dragStartProp = static_cast<float>(valueToProportionOfLength(getValue()));
        dragStartY    = e.position.y;
        wasShift      = shift;
    }

    const float sens   = shift ? BaseMetrics::kKnobDragSensitivityFine
                               : BaseMetrics::kKnobDragSensitivity;
    const float delta  = (dragStartY - e.position.y) * sens;
    const float target = juce::jlimit(0.0f, 1.0f, dragStartProp + delta);
    setValue(proportionOfLengthToValue(target), juce::sendNotificationSync);

    if (onShowReadout) onShowReadout(caption, formatValueText());
}

void Knob::mouseUp(const juce::MouseEvent& e)
{
    juce::Slider::mouseUp(e);
    if (!isEnabled())
        return;

    drag.end(*this, e.source);

    if (onHideReadout) onHideReadout();
}

} // namespace vitottx
