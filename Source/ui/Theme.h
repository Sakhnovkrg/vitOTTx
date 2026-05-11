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
    juce::Colour background       { 0xff2a2d31 };
    juce::Colour panel            { 0xff1a1d21 };
    juce::Colour panelOutline     { 0xff0a0c0e };
    juce::Colour barFillUpper     { 0xff4ed4af };
    juce::Colour barFillLower     { 0xff2e8f76 };
    juce::Colour barFillDisabled  { 0xff5a5e64 };
    juce::Colour barStripe        { 0x33ffffff };
    juce::Colour edgeHandle       { 0xff5fd9ba };
    juce::Colour hoverHandle      { 0xffffffff };
    juce::Colour textPrimary      { 0xffe6e8eb };
    juce::Colour textSecondary    { 0xff9aa0a6 };
    juce::Colour accent           { 0xffe04848 };
};

struct BaseMetrics
{
    static constexpr float kPanelCornerRadius = 6.0f;
    static constexpr float kPanelOutlineWidth = 1.0f;
    static constexpr float kBarCornerRadius   = 3.0f;
    static constexpr float kBarEdgeThickness  = 3.0f;
    static constexpr float kBarTextureSpacing = 4.0f;
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

    static constexpr int   kReferenceWidth        = 640;
    static constexpr int   kReferenceHeight       = 230;
    static constexpr int   kLeftSectionWidth      = 175;
    static constexpr int   kRightSectionWidth     = 117;
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
    float barTextureSpacing() const noexcept { return scaled(BaseMetrics::kBarTextureSpacing); }
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
