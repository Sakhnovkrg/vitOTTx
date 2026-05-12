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

#include <juce_graphics/juce_graphics.h>

namespace vitottx
{

struct Palette
{
    juce::Colour background       { 0xff0d1218 };
    juce::Colour sidebar          { 0xff141a22 };
    juce::Colour panel            { 0xff06090d };
    juce::Colour panelOutline     { 0xff000004 };
    juce::Colour barFillUpper     { 0xff168f78 };
    juce::Colour barFillLower     { 0xff083a2f };
    juce::Colour barFillDisabled  { 0xff2c3038 };
    juce::Colour barStripe        { 0x33ffffff };
    juce::Colour hoverHandle      { 0xccffffff };
    juce::Colour meter            { 0xff1de9b6 };
    juce::Colour knobBody         { 0xff10161c };
    juce::Colour knobArcTrack     { 0xff1f262e };
    juce::Colour knobArcActive    { 0xff1de9b6 };
    juce::Colour knobPointer      { 0xffffffff };
    juce::Colour textPrimary      { 0xffc8d4d8 };
    juce::Colour textSecondary    { 0xff7a8890 };
};

struct BaseMetrics
{
    static constexpr float kPanelCornerRadius = 6.0f;
    static constexpr float kPanelOutlineWidth = 1.0f;
    static constexpr float kBarCornerRadius   = 3.0f;
    static constexpr float kBarEdgeThickness  = 3.0f;
    static constexpr int   kPadding           = 8;
    static constexpr int   kSmallPadding      = 4;
    static constexpr int   kLabelHeight       = 14;
    static constexpr int   kFontSizeLabel     = 11;
    static constexpr int   kFontSizeValue     = 13;
    static constexpr int   kHitTolerance      = 6;

    static constexpr float kMinDb        = -80.0f;
    static constexpr float kMaxDb        =   0.0f;
    static constexpr float kDbEditBuffer =   1.0f;
    static constexpr float kMinEditDb    = kMinDb + kDbEditBuffer;
    static constexpr float kMaxEditDb    = kMaxDb - kDbEditBuffer;

    static constexpr float kThresholdDragMultiplier = 0.5f;
    static constexpr float kRatioDragMultiplier     = 0.6f;
    static constexpr float kCrossoverDragMultiplier = 0.5f;
    static constexpr int   kCrossoverCollapseResistance = 24;

    // Pixel-rate sensitivity for rotary drags. 0.0024 ≈ full sweep over 420 px.
    static constexpr float kKnobDragSensitivity     = 0.0024f;
    static constexpr float kKnobDragSensitivityFine = 0.00048f;

    static constexpr int   kSidebarWidth          = 30;
    static constexpr int   kSidebarTextBottomGap  = 10;
    static constexpr int   kSidebarFontSize       = 18;
    static constexpr int   kSidebarVersionFontSize = 10;
    static constexpr int   kSidebarShadowRadius   = 6;
    static constexpr int   kSidebarShadowOffset   = 2;
    static constexpr int   kBypassButtonTopGap    = 10;
    static constexpr int   kBypassButtonDiameter  = 14;
    static constexpr int   kReferenceWidth        = 640 + kSidebarWidth;
    static constexpr int   kReferenceHeight       = 180;
};

class Theme
{
public:
    Theme() = default;

    void setScale(float newScale) noexcept { scale = juce::jmax(0.25f, newScale); }
    float getScale() const noexcept        { return scale; }

    int    scaledInt(int base)   const noexcept { return juce::roundToInt(base * scale); }
    float  scaled(float base)    const noexcept { return base * scale; }

    int panelCornerRadius() const noexcept { return scaledInt(juce::roundToInt(BaseMetrics::kPanelCornerRadius)); }
    int barCornerRadius()   const noexcept { return scaledInt(juce::roundToInt(BaseMetrics::kBarCornerRadius)); }
    float barEdgeThickness() const noexcept { return scaled(BaseMetrics::kBarEdgeThickness); }
    int padding()       const noexcept { return scaledInt(BaseMetrics::kPadding); }
    int smallPadding()  const noexcept { return scaledInt(BaseMetrics::kSmallPadding); }
    int labelHeight()   const noexcept { return scaledInt(BaseMetrics::kLabelHeight); }
    int hitTolerance()  const noexcept { return scaledInt(BaseMetrics::kHitTolerance); }
    float fontLabel()   const noexcept { return scaled((float) BaseMetrics::kFontSizeLabel); }
    float fontValue()   const noexcept { return scaled((float) BaseMetrics::kFontSizeValue); }
    float panelOutlineWidth() const noexcept { return scaled(BaseMetrics::kPanelOutlineWidth); }

    const Palette& palette() const noexcept { return paletteData; }

private:
    Palette paletteData;
    float   scale = 1.0f;
};

} // namespace vitottx
