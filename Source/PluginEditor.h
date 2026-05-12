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

#include "PluginProcessor.h"
#include "Settings.h"
#include "ui/BandView.h"
#include "ui/BypassButton.h"
#include "ui/BypassOverlay.h"
#include "ui/CrossoverHandle.h"
#include "ui/Knob.h"
#include "ui/Theme.h"
#include "ui/TooltipReadout.h"

namespace vitottx
{

class VitOttAudioProcessorEditor : public juce::AudioProcessorEditor,
                                   private juce::Timer
{
public:
    explicit VitOttAudioProcessorEditor(VitOttAudioProcessor&);
    ~VitOttAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;


    VitOttAudioProcessor& processor;

    Theme    theme;
    BandView lowBand;
    BandView midBand;
    BandView highBand;

    juce::Rectangle<int> panelBounds;
    juce::Rectangle<int> sidebarBounds;

    Knob inKnob;
    Knob outKnob;
    Knob mixKnob;
    Knob lowKnob;
    Knob bandKnob;
    Knob highKnob;
    Knob attackKnob;
    Knob releaseKnob;

    CrossoverHandle lowCrossHandle;
    CrossoverHandle highCrossHandle;

    BypassButton bypassButton;
    BypassOverlay bypassOverlay;
    TooltipReadout readout;
    bool lastBypassState = false;
    bool persistSize     = false;

    static constexpr int kBypassFadeMs = 180;

    void showReadout(const juce::String& name, const juce::String& value);
    void hideReadout();

    void setupKnob(Knob& slot, const char* paramId, const juce::String& caption);
    void layoutLeftSection (juce::Rectangle<int> area);
    void layoutRightSection(juce::Rectangle<int> area);
    void paintSidebar      (juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VitOttAudioProcessorEditor)
};

} // namespace vitottx
