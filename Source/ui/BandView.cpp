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

#include "BandView.h"

#include <cmath>

namespace vitottx
{

namespace
{
    constexpr int   kDbSections             = 8;
    constexpr int   kRatioDbLines           = 14;
    constexpr float kStripeHeight           = 1.0f;
    constexpr float kStripeAlphaMult        = 1.8f;
    constexpr float kStripeAlphaMultOnHover = 3.0f;
    constexpr float kStripeInsetPx          = 3.0f;
    constexpr float kHoverEdgeHeight        = 3.0f;
    constexpr float kRatioActiveEpsilon     = 0.001f;
    constexpr float kMeterBarWidth          = 12.0f;
    constexpr float kMeterChannelGap        = 1.5f;
    constexpr float kMeterInputLineHeight   = 1.5f;

    juce::RangedAudioParameter& parameterRef(juce::AudioProcessorValueTreeState& apvts,
                                             const juce::String& id)
    {
        auto* p = apvts.getParameter(id);
        jassert(p != nullptr);
        return *p;
    }
}

BandView::BandView(juce::AudioProcessorValueTreeState& apvts,
                   const Theme& t,
                   ParamIds ids)
    : theme(t),
      paramIds(std::move(ids)),
      upperThresholdAttach(parameterRef(apvts, paramIds.upperThreshold),
                           [this](float v){ upperThreshold = v; repaint(); }),
      lowerThresholdAttach(parameterRef(apvts, paramIds.lowerThreshold),
                           [this](float v){ lowerThreshold = v; repaint(); }),
      upperRatioAttach    (parameterRef(apvts, paramIds.upperRatio),
                           [this](float v){ upperRatio = v;     repaint(); }),
      lowerRatioAttach    (parameterRef(apvts, paramIds.lowerRatio),
                           [this](float v){ lowerRatio = v;     repaint(); })
{
    upperThresholdAttach.sendInitialUpdate();
    lowerThresholdAttach.sendInitialUpdate();
    upperRatioAttach.sendInitialUpdate();
    lowerRatioAttach.sendInitialUpdate();
}

BandView::~BandView() = default;

//==============================================================================
// Geometry

juce::Rectangle<float> BandView::contentArea() const
{
    return getLocalBounds().toFloat().reduced(theme.scaled(BaseMetrics::kPanelOutlineWidth));
}

float BandView::dbToY(float db) const
{
    const auto area = contentArea();
    const float t = (BaseMetrics::kMaxDb - db) / (BaseMetrics::kMaxDb - BaseMetrics::kMinDb);
    return area.getY() + juce::jlimit(0.0f, 1.0f, t) * area.getHeight();
}

juce::Rectangle<float> BandView::upperBarRect() const
{
    const auto area = contentArea();
    const float yThr = dbToY(upperThreshold);
    return { area.getX(), area.getY(), area.getWidth(), juce::jmax(0.0f, yThr - area.getY()) };
}

juce::Rectangle<float> BandView::lowerBarRect() const
{
    const auto area = contentArea();
    const float yThr = dbToY(lowerThreshold);
    return { area.getX(), yThr, area.getWidth(), juce::jmax(0.0f, area.getBottom() - yThr) };
}

//==============================================================================
// Painting

void BandView::paint(juce::Graphics& g)
{
    const auto upper = upperBarRect();
    const auto lower = lowerBarRect();

    paintBar(g, upper, /*isUpper=*/true,  hoverUpperBodyFader.getValue());
    paintBar(g, lower, /*isUpper=*/false, hoverLowerBodyFader.getValue());
    paintMeters(g);

    paintHoverEdge(g, upper, /*isUpper=*/true,  hoverUpperEdgeFader.getValue());
    paintHoverEdge(g, lower, /*isUpper=*/false, hoverLowerEdgeFader.getValue());
}

void BandView::paintBar(juce::Graphics& g, juce::Rectangle<float> rect,
                        bool isUpper, float hoverBodyAlpha) const
{
    if (rect.getHeight() < 1.0f)
        return;
    paintBarFill(g, rect, isUpper);
    paintRatioStripes(g, rect, isUpper, hoverBodyAlpha);
}

void BandView::paintBarFill(juce::Graphics& g, juce::Rectangle<float> rect, bool isUpper) const
{
    const auto& pal   = theme.palette();
    const float ratio = isUpper ? upperRatio : lowerRatio;

    juce::Colour fill;
    if (isUpper)
        fill = (ratio < kRatioActiveEpsilon) ? pal.barFillDisabled : pal.barFillUpper;
    else
        fill = (ratio > kRatioActiveEpsilon) ? pal.barFillLower : pal.barFillUpper;

    g.setColour(fill);
    g.fillRect(rect);
}

void BandView::paintRatioStripes(juce::Graphics& g, juce::Rectangle<float> rect,
                                 bool isUpper, float hoverBodyAlpha) const
{
    if (rect.getHeight() < kStripeHeight * 3.0f)
        return;

    const float ratio     = isUpper ? upperRatio : lowerRatio;
    const float threshold = isUpper ? upperThreshold : lowerThreshold;

    const float dbRange = BaseMetrics::kMaxDb - BaseMetrics::kMinDb;
    const float dbStep  = dbRange / static_cast<float>(kDbSections);

    const float gridPos    = static_cast<float>(kDbSections) * (threshold - BaseMetrics::kMinDb) / dbRange;
    const int   gridIndex0 = isUpper ? static_cast<int>(std::ceil(gridPos))
                                     : static_cast<int>(std::floor(gridPos));
    const float gridDelta  = isUpper ? dbStep : -dbStep;

    const auto  area      = contentArea();
    const float inset     = juce::jmax(2.0f, theme.scaled(kStripeInsetPx));
    const float xLeft     = rect.getX() + inset;
    const float xRight    = rect.getRight() - inset;
    const float stripeW   = xRight - xLeft;
    const float pileLimit = isUpper ? rect.getBottom() : rect.getY();
    const float alphaMult = juce::jmap(hoverBodyAlpha, kStripeAlphaMult, kStripeAlphaMultOnHover);

    const auto& pal = theme.palette();

    float inputDb = static_cast<float>(gridIndex0) * dbStep + BaseMetrics::kMinDb;
    for (int i = 0; i < kRatioDbLines; ++i)
    {
        const float compressedDb = inputDb + ratio * (threshold - inputDb);
        const float yIdeal       = area.getY()
                                 + (BaseMetrics::kMaxDb - compressedDb) / dbRange * area.getHeight();

        const float yClamped = isUpper ? juce::jmin(yIdeal, pileLimit - kStripeHeight)
                                       : juce::jmax(yIdeal, pileLimit + kStripeHeight);

        const float yTopRaw = isUpper ? (yClamped - kStripeHeight) : yClamped;
        const float yBotRaw = isUpper ? yClamped : (yClamped + kStripeHeight);
        const float yTop    = juce::jmax(yTopRaw, rect.getY());
        const float yBot    = juce::jmin(yBotRaw, rect.getBottom());

        if (yBot > yTop)
        {
            const float alphaFactor = static_cast<float>(kRatioDbLines - i) * alphaMult
                                    / static_cast<float>(kRatioDbLines);
            g.setColour(pal.barStripe.withMultipliedAlpha(alphaFactor));
            g.fillRect(xLeft, yTop, stripeW, yBot - yTop);
        }

        inputDb += gridDelta;
    }
}

void BandView::paintHoverEdge(juce::Graphics& g, juce::Rectangle<float> rect, bool isUpper,
                              float hoverEdgeAlpha) const
{
    if (hoverEdgeAlpha <= 0.0f)
        return;
    g.setColour(theme.palette().hoverHandle.withMultipliedAlpha(hoverEdgeAlpha));
    if (isUpper)
        g.fillRect(rect.getX(), rect.getBottom() - kHoverEdgeHeight, rect.getWidth(), kHoverEdgeHeight);
    else
        g.fillRect(rect.getX(), rect.getY(), rect.getWidth(), kHoverEdgeHeight);
}

void BandView::paintMeters(juce::Graphics& g) const
{
    const auto area = contentArea();
    if (area.getHeight() <= 1.0f)
        return;

    const float centreX = area.getCentreX();
    const float barW    = theme.scaled(kMeterBarWidth);
    const float gap     = theme.scaled(kMeterChannelGap);
    const float leftX   = centreX - barW - gap * 0.5f;
    const float rightX  = centreX + gap * 0.5f;
    const float lineH   = juce::jmax(1.0f, theme.scaled(kMeterInputLineHeight));

    g.setColour(theme.palette().meter);

    const auto drawChannel = [&](float xLeft, float outputDb, float inputDb)
    {
        if (outputDb > BaseMetrics::kMinDb + 0.01f)
        {
            const float yTop = dbToY(outputDb);
            const float h    = juce::jmax(0.0f, area.getBottom() - yTop);
            if (h > 0.0f)
                g.fillRect(xLeft, yTop, barW, h);
        }
        if (inputDb > BaseMetrics::kMinDb + 0.01f)
        {
            const float yLine = dbToY(inputDb);
            g.fillRect(xLeft, yLine - lineH * 0.5f, barW, lineH);
        }
    };

    drawChannel(leftX,  outputLeftDb,  inputLeftDb);
    drawChannel(rightX, outputRightDb, inputRightDb);
}

void BandView::setInputLevels(float leftDb, float rightDb)
{
    const float l = juce::jlimit(BaseMetrics::kMinDb, BaseMetrics::kMaxDb, leftDb);
    const float r = juce::jlimit(BaseMetrics::kMinDb, BaseMetrics::kMaxDb, rightDb);
    if (l != inputLeftDb || r != inputRightDb)
    {
        inputLeftDb  = l;
        inputRightDb = r;
        repaint();
    }
}

void BandView::setOutputLevels(float leftDb, float rightDb)
{
    const float l = juce::jlimit(BaseMetrics::kMinDb, BaseMetrics::kMaxDb, leftDb);
    const float r = juce::jlimit(BaseMetrics::kMinDb, BaseMetrics::kMaxDb, rightDb);
    if (l != outputLeftDb || r != outputRightDb)
    {
        outputLeftDb  = l;
        outputRightDb = r;
        repaint();
    }
}

//==============================================================================
// Hit testing & hover

BandView::HitZone BandView::hitTest(juce::Point<int> p) const
{
    const float tolerance = static_cast<float>(theme.hitTolerance());
    const float yUpThr    = dbToY(upperThreshold);
    const float yLoThr    = dbToY(lowerThreshold);
    const float py        = static_cast<float>(p.getY());

    if (std::abs(py - yUpThr) <= tolerance) return HitZone::UpperEdge;
    if (std::abs(py - yLoThr) <= tolerance) return HitZone::LowerEdge;

    if (py < yUpThr) return HitZone::UpperBody;
    if (py > yLoThr) return HitZone::LowerBody;

    const float midpoint = (yUpThr + yLoThr) * 0.5f;
    return (py < midpoint) ? HitZone::UpperBody : HitZone::LowerBody;
}

void BandView::updateCursor(HitZone zone)
{
    switch (zone)
    {
        case HitZone::UpperEdge:
        case HitZone::LowerEdge:
            setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
            break;
        case HitZone::UpperBody:
        case HitZone::LowerBody:
            setMouseCursor(juce::MouseCursor::DraggingHandCursor);
            break;
        case HitZone::None:
            setMouseCursor(juce::MouseCursor::NormalCursor);
            break;
    }
}

void BandView::updateHover(juce::Point<int> p)
{
    const auto zone = hitTest(p);
    hoverUpperEdgeFader.setTarget(zone == HitZone::UpperEdge);
    hoverLowerEdgeFader.setTarget(zone == HitZone::LowerEdge);
    hoverUpperBodyFader.setTarget(zone == HitZone::UpperBody);
    hoverLowerBodyFader.setTarget(zone == HitZone::LowerBody);

    if (anyHoverFaderMoving())
        startTimerHz(60);
}

void BandView::clearHover()
{
    hoverUpperEdgeFader.setTarget(false);
    hoverLowerEdgeFader.setTarget(false);
    hoverUpperBodyFader.setTarget(false);
    hoverLowerBodyFader.setTarget(false);

    if (anyHoverFaderMoving())
        startTimerHz(60);
}

void BandView::timerCallback()
{
    bool moved = false;
    moved |= hoverUpperEdgeFader.tick();
    moved |= hoverLowerEdgeFader.tick();
    moved |= hoverUpperBodyFader.tick();
    moved |= hoverLowerBodyFader.tick();

    if (moved)
        repaint();
    if (!anyHoverFaderMoving())
        stopTimer();
}

bool BandView::anyHoverFaderMoving() const noexcept
{
    return hoverUpperEdgeFader.isMoving()
        || hoverLowerEdgeFader.isMoving()
        || hoverUpperBodyFader.isMoving()
        || hoverLowerBodyFader.isMoving();
}

void BandView::mouseMove(const juce::MouseEvent& e)
{
    updateCursor(hitTest(e.getPosition()));
    updateHover(e.getPosition());
}

void BandView::mouseEnter(const juce::MouseEvent& e)
{
    updateHover(e.getPosition());
}

void BandView::mouseExit(const juce::MouseEvent&)
{
    clearHover();
}

//==============================================================================
// Drag

void BandView::mouseDown(const juce::MouseEvent& e)
{
    activeZone = hitTest(e.getPosition());

    if (activeZone == HitZone::None)
    {
        updateCursor(activeZone);
        return;
    }

    dragStartY = static_cast<float>(e.position.y);

    switch (activeZone)
    {
        case HitZone::UpperEdge:
            dragStartValue = upperThreshold;
            upperThresholdAttach.beginGesture();
            lowerThresholdAttach.beginGesture();
            break;
        case HitZone::LowerEdge:
            dragStartValue = lowerThreshold;
            upperThresholdAttach.beginGesture();
            lowerThresholdAttach.beginGesture();
            break;
        case HitZone::UpperBody:
            dragStartValue = upperRatio;
            upperRatioAttach.beginGesture();
            break;
        case HitZone::LowerBody:
            dragStartValue = lowerRatio;
            lowerRatioAttach.beginGesture();
            break;
        default:
            break;
    }

    drag.begin(*this, e.source);

    emitReadoutForActiveZone();
}

void BandView::mouseDrag(const juce::MouseEvent& e)
{
    if (activeZone == HitZone::None)
        return;

    const auto area = contentArea();
    if (area.getHeight() <= 1.0f)
        return;

    const float pixelDelta = static_cast<float>(e.position.y) - dragStartY;
    const float dbRange    = BaseMetrics::kMaxDb - BaseMetrics::kMinDb;
    const float dyDb       = -pixelDelta * dbRange / area.getHeight()
                           * BaseMetrics::kThresholdDragMultiplier;

    switch (activeZone)
    {
        case HitZone::UpperEdge:
            applyUpperThreshold(juce::jlimit(BaseMetrics::kMinEditDb, BaseMetrics::kMaxEditDb,
                                             dragStartValue + dyDb));
            break;
        case HitZone::LowerEdge:
            applyLowerThreshold(juce::jlimit(BaseMetrics::kMinEditDb, BaseMetrics::kMaxEditDb,
                                             dragStartValue + dyDb));
            break;
        case HitZone::UpperBody:
        {
            const float dr = pixelDelta * BaseMetrics::kRatioDragMultiplier / area.getHeight();
            applyUpperRatio(juce::jlimit(0.0f, 1.0f, dragStartValue + dr));
            break;
        }
        case HitZone::LowerBody:
        {
            const float dr = -pixelDelta * BaseMetrics::kRatioDragMultiplier / area.getHeight();
            applyLowerRatio(juce::jlimit(-1.0f, 1.0f, dragStartValue + dr));
            break;
        }
        default:
            break;
    }

    emitReadoutForActiveZone();
}

void BandView::mouseUp(const juce::MouseEvent& e)
{
    if (activeZone == HitZone::None)
        return;

    switch (activeZone)
    {
        case HitZone::UpperEdge:
        case HitZone::LowerEdge:
            upperThresholdAttach.endGesture();
            lowerThresholdAttach.endGesture();
            break;
        case HitZone::UpperBody:
            upperRatioAttach.endGesture();
            break;
        case HitZone::LowerBody:
            lowerRatioAttach.endGesture();
            break;
        default:
            break;
    }

    std::optional<juce::Point<float>> restoreOverride;
    if (activeZone == HitZone::UpperEdge || activeZone == HitZone::LowerEdge)
    {
        const float yThr = (activeZone == HitZone::UpperEdge) ? dbToY(upperThreshold)
                                                              : dbToY(lowerThreshold);
        const auto screenPt = localPointToGlobal(juce::Point<float>(0.0f, yThr));
        restoreOverride = juce::Point<float>(drag.getAnchor().x, screenPt.y);
    }
    drag.end(*this, e.source, restoreOverride);

    activeZone = HitZone::None;
    updateCursor(hitTest(getMouseXYRelative()));

    if (onHideReadout) onHideReadout();
}

//==============================================================================
// Drag application

void BandView::applyUpperThreshold(float v)
{
    upperThresholdAttach.setValueAsPartOfGesture(v);
    upperThreshold = v;
    if (onParamChange) onParamChange(paramIds.upperThreshold, v);

    if (v < lowerThreshold)
    {
        lowerThresholdAttach.setValueAsPartOfGesture(v);
        lowerThreshold = v;
        if (onParamChange) onParamChange(paramIds.lowerThreshold, v);
    }
}

void BandView::applyLowerThreshold(float v)
{
    lowerThresholdAttach.setValueAsPartOfGesture(v);
    lowerThreshold = v;
    if (onParamChange) onParamChange(paramIds.lowerThreshold, v);

    if (v > upperThreshold)
    {
        upperThresholdAttach.setValueAsPartOfGesture(v);
        upperThreshold = v;
        if (onParamChange) onParamChange(paramIds.upperThreshold, v);
    }
}

void BandView::applyUpperRatio(float v)
{
    upperRatioAttach.setValueAsPartOfGesture(v);
    if (onParamChange) onParamChange(paramIds.upperRatio, v);
}

void BandView::emitReadoutForActiveZone()
{
    if (!onShowReadout)
        return;

    const auto db  = [](float v) { return juce::String(v, 1) + " dB"; };
    const auto pct = [](float v) { return juce::String(juce::roundToInt(v * 100.0f)) + " %"; };

    switch (activeZone)
    {
        case HitZone::UpperEdge: onShowReadout("UPPER THRES", db(upperThreshold));  break;
        case HitZone::LowerEdge: onShowReadout("LOWER THRES", db(lowerThreshold));  break;
        case HitZone::UpperBody: onShowReadout("UPPER RATIO", pct(upperRatio));     break;
        case HitZone::LowerBody: onShowReadout("LOWER RATIO", pct(lowerRatio));     break;
        default: break;
    }
}

void BandView::applyLowerRatio(float v)
{
    lowerRatioAttach.setValueAsPartOfGesture(v);
    if (onParamChange) onParamChange(paramIds.lowerRatio, v);
}

} // namespace vitottx
