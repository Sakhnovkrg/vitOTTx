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
#include <functional>

#include "HiddenCursorDrag.h"
#include "Theme.h"

namespace vitottx
{

// Vertical drag handle that lives between two adjacent BandViews and edits a
// crossover-frequency parameter on a log scale. When dragged close enough to
// the edge that the adjacent edge-band would become narrower than
// snapMinimumWidth pixels, the freq snaps to its absolute min/max so the band
// fully collapses (driving the DSP into LowBand / HighBand / SingleBand mode).
class CrossoverHandle : public juce::Component
{
public:
    enum class Side { Low, High };

    static constexpr float kMinFreq = 20.0f;
    static constexpr float kMaxFreq = 18000.0f;

    CrossoverHandle(juce::AudioProcessorValueTreeState& apvts,
                    const Theme& theme,
                    Side side);

    void setRange(int leftX, int rightX);          // pixel extent in parent coords
    void setSnapMinimumWidth(int pixels);
    void setCounterpartX(int x);                   // x of the other crossover handle (-1 = none)
    void setCounterpartCollapsed(bool collapsed);
    void setMinMidWidth(int pixels);               // min gap between this handle and counterpart

    std::function<void()> onFreqChange;
    std::function<void(const juce::String&, const juce::String&)> onShowReadout;
    std::function<void()> onHideReadout;

    float getFrequency() const noexcept { return currentFreq; }

    void paint(juce::Graphics&) override;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp   (const juce::MouseEvent&) override;
    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    static int   freqToX (float freq, int rangeLeft, int rangeRight) noexcept;
    static float xToFreq (float x,    int rangeLeft, int rangeRight) noexcept;

    juce::String readoutName()  const;
    juce::String readoutValue() const;

private:
    const Theme& theme;
    Side side;

    juce::ParameterAttachment attach;
    HiddenCursorDrag drag;

    float currentFreq         = 1000.0f;
    float dragStartFreq       = 0.0f;
    juce::Point<float> dragStartSourcePos;
    int   rangeLeft           = 0;
    int   rangeRight     = 0;
    int   snapMinWidth   = 30;
    int   counterpartX           = -1;
    bool  counterpartCollapsed   = false;
    int   minMidWidth            = 30;
    bool  hovered                = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrossoverHandle)
};

} // namespace vitottx
