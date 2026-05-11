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

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "HiddenCursorDrag.h"
#include "HoverFader.h"
#include "Theme.h"

namespace vitottx
{

class BandView : public juce::Component, private juce::Timer
{
public:
    struct ParamIds
    {
        juce::String upperThreshold;
        juce::String lowerThreshold;
        juce::String upperRatio;
        juce::String lowerRatio;
    };

    BandView(juce::AudioProcessorValueTreeState& apvts,
             const Theme& theme,
             ParamIds ids);

    ~BandView() override;

    std::function<void(const juce::String& paramId, float value)> onParamChange;

    void setInputLevels (float leftDb, float rightDb);
    void setOutputLevels(float leftDb, float rightDb);

    void paint(juce::Graphics&) override;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp   (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    enum class HitZone { None, UpperEdge, UpperBody, LowerEdge, LowerBody };

    // Geometry
    juce::Rectangle<float> contentArea()   const;
    juce::Rectangle<float> upperBarRect()  const;
    juce::Rectangle<float> lowerBarRect()  const;
    float dbToY(float db) const;

    // Painting
    void paintBar         (juce::Graphics&, juce::Rectangle<float> rect, bool isUpper,
                           float hoverBodyAlpha) const;
    void paintBarFill     (juce::Graphics&, juce::Rectangle<float> rect, bool isUpper) const;
    void paintRatioStripes(juce::Graphics&, juce::Rectangle<float> rect, bool isUpper,
                           float hoverBodyAlpha) const;
    void paintHoverEdge   (juce::Graphics&, juce::Rectangle<float> rect, bool isUpper,
                           float hoverEdgeAlpha) const;
    void paintMeters      (juce::Graphics&) const;

    // Hit testing & hover
    HitZone hitTest(juce::Point<int> p) const;
    void    updateCursor(HitZone zone);
    void    updateHover(juce::Point<int> p);
    void    clearHover();
    void    timerCallback() override;
    bool    anyHoverFaderMoving() const noexcept;

    // Drag handlers — each enforces threshold coupling and parameter range.
    void applyUpperThreshold(float v);
    void applyLowerThreshold(float v);
    void applyUpperRatio    (float v);
    void applyLowerRatio    (float v);

    const Theme& theme;
    ParamIds paramIds;

    juce::ParameterAttachment upperThresholdAttach;
    juce::ParameterAttachment lowerThresholdAttach;
    juce::ParameterAttachment upperRatioAttach;
    juce::ParameterAttachment lowerRatioAttach;

    float upperThreshold = -28.0f;
    float lowerThreshold = -35.0f;
    float upperRatio     =  0.9f;
    float lowerRatio     =  0.8f;

    HitZone activeZone     = HitZone::None;
    float   dragStartY     = 0.0f;
    float   dragStartValue = 0.0f;
    HiddenCursorDrag drag;

    HoverFader hoverUpperEdgeFader;
    HoverFader hoverLowerEdgeFader;
    HoverFader hoverUpperBodyFader;
    HoverFader hoverLowerBodyFader;

    float inputLeftDb   = BaseMetrics::kMinDb;
    float inputRightDb  = BaseMetrics::kMinDb;
    float outputLeftDb  = BaseMetrics::kMinDb;
    float outputRightDb = BaseMetrics::kMinDb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandView)
};

} // namespace vitottx
