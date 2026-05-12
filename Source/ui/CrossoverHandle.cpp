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

#include "CrossoverHandle.h"

#include <cmath>

namespace vitottx
{

namespace
{
    juce::RangedAudioParameter& paramForSide(juce::AudioProcessorValueTreeState& apvts,
                                             CrossoverHandle::Side side)
    {
        const auto* id = side == CrossoverHandle::Side::Low ? "low_cross_freq" : "high_cross_freq";
        auto* p = apvts.getParameter(id);
        jassert(p != nullptr);
        return *p;
    }
}

CrossoverHandle::CrossoverHandle(juce::AudioProcessorValueTreeState& apvts,
                                  const Theme& t,
                                  Side s)
    : theme(t),
      side(s),
      attach(paramForSide(apvts, s),
             [this](float v)
             {
                 currentFreq = v;
                 if (onFreqChange) onFreqChange();
                 repaint();
             })
{
    attach.sendInitialUpdate();
    setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
}

void CrossoverHandle::setRange(int left, int right)
{
    rangeLeft  = left;
    rangeRight = right;
}

void CrossoverHandle::setSnapMinimumWidth(int pixels)
{
    snapMinWidth = pixels;
}

void CrossoverHandle::setCounterpartX(int x)
{
    counterpartX = x;
}

void CrossoverHandle::setCounterpartCollapsed(bool collapsed)
{
    counterpartCollapsed = collapsed;
}

void CrossoverHandle::setMinMidWidth(int pixels)
{
    minMidWidth = pixels;
}

int CrossoverHandle::freqToX(float freq, int rangeLeft, int rangeRight) noexcept
{
    const float clamped = juce::jlimit(kMinFreq, kMaxFreq, freq);
    const float t = (std::log(clamped) - std::log(kMinFreq))
                  / (std::log(kMaxFreq) - std::log(kMinFreq));
    return rangeLeft + juce::roundToInt(t * static_cast<float>(rangeRight - rangeLeft));
}

float CrossoverHandle::xToFreq(float x, int rangeLeft, int rangeRight) noexcept
{
    const float t = juce::jlimit(0.0f, 1.0f,
                                 (x - static_cast<float>(rangeLeft))
                                 / static_cast<float>(rangeRight - rangeLeft));
    return std::exp(std::log(kMinFreq) + t * (std::log(kMaxFreq) - std::log(kMinFreq)));
}

void CrossoverHandle::paint(juce::Graphics& g)
{
    const auto& pal = theme.palette();
    const float alpha = hovered ? 0.85f : 0.35f;
    g.setColour(pal.hoverHandle.withMultipliedAlpha(alpha));
    g.fillRect(getLocalBounds().toFloat());
}

void CrossoverHandle::mouseEnter(const juce::MouseEvent&)
{
    if (!hovered) { hovered = true; repaint(); }
}

void CrossoverHandle::mouseExit(const juce::MouseEvent&)
{
    if (hovered) { hovered = false; repaint(); }
}

static juce::String formatFreq(float hz)
{
    if (hz >= 1000.0f)
        return juce::String(hz / 1000.0f, 2) + " kHz";
    return juce::String(juce::roundToInt(hz)) + " Hz";
}

juce::String CrossoverHandle::readoutName() const
{
    if (counterpartCollapsed)
        return "LOW / HIGH";
    return side == Side::Low ? "LOW / MID" : "MID / HIGH";
}

juce::String CrossoverHandle::readoutValue() const
{
    if (side == Side::Low  && currentFreq <= CrossoverHandle::kMinFreq + 1.0f)  return "OFF";
    if (side == Side::High && currentFreq >= CrossoverHandle::kMaxFreq - 50.0f) return "OFF";
    return formatFreq(currentFreq);
}

void CrossoverHandle::mouseDown(const juce::MouseEvent& e)
{
    dragStartFreq      = currentFreq;
    dragStartSourcePos = e.source.getScreenPosition();
    attach.beginGesture();
    drag.begin(*this, e.source);

    if (onShowReadout) onShowReadout(readoutName(), readoutValue());
}

void CrossoverHandle::mouseDrag(const juce::MouseEvent& e)
{
    if (rangeRight <= rangeLeft)
        return;

    const int   startX  = freqToX(dragStartFreq, rangeLeft, rangeRight);
    const float deltaX  = (e.source.getScreenPosition().x - dragStartSourcePos.x)
                          * BaseMetrics::kCrossoverDragMultiplier;
    int         targetX = startX + juce::roundToInt(deltaX);

    if (counterpartX >= 0)
    {
        if (side == Side::Low)
            targetX = juce::jmin(targetX, counterpartX - minMidWidth);
        else
            targetX = juce::jmax(targetX, counterpartX + minMidWidth);
    }

    const int resistance = BaseMetrics::kCrossoverCollapseResistance;
    if (side == Side::Low)
    {
        const int stickEdge = rangeLeft + snapMinWidth;
        if (targetX < stickEdge)
        {
            const int overshoot = stickEdge - targetX;
            targetX = (overshoot >= resistance) ? rangeLeft : stickEdge;
        }
    }
    else
    {
        const int stickEdge = rangeRight - snapMinWidth;
        if (targetX > stickEdge)
        {
            const int overshoot = targetX - stickEdge;
            targetX = (overshoot >= resistance) ? rangeRight : stickEdge;
        }
    }

    targetX = juce::jlimit(rangeLeft, rangeRight, targetX);
    const float newFreq = juce::jlimit(kMinFreq, kMaxFreq,
                                       xToFreq(static_cast<float>(targetX), rangeLeft, rangeRight));
    attach.setValueAsPartOfGesture(newFreq);

    // ParameterAttachment dispatches its callback via AsyncUpdater, which can
    // starve during continuous dragging. Update currentFreq synchronously so
    // the Editor's resized() (called from onFreqChange) sees the just-set
    // value via getFrequency(), without waiting on the APVTS round-trip.
    if (std::abs(newFreq - currentFreq) > 0.0001f)
    {
        currentFreq = newFreq;
        if (onFreqChange) onFreqChange();
        repaint();
    }

    if (onShowReadout) onShowReadout(readoutName(), readoutValue());
}

void CrossoverHandle::mouseUp(const juce::MouseEvent& e)
{
    attach.endGesture();

    // Restore the cursor at the handle's final screen X (keeping Y from the
    // drag anchor) so it doesn't snap back to the click origin after a drag.
    const float screenX = static_cast<float>(getScreenBounds().getCentreX());
    const juce::Point<float> restorePos(screenX, drag.getAnchor().y);
    drag.end(*this, e.source, restorePos);

    // HiddenCursorDrag::end forces NormalCursor on the host; reinstate the
    // resize cursor so subsequent hovers show the correct affordance.
    setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);

    if (onHideReadout) onHideReadout();
}

} // namespace vitottx
