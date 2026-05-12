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
#include "ui/BandView.h"
#include "ui/CrossoverHandle.h"
#include "ui/Knob.h"
#include "ui/Theme.h"

namespace vitottx
{

class BypassButton : public juce::Component
{
public:
    BypassButton(juce::AudioProcessorValueTreeState& a, Theme& th) : apvts(a), theme(th) {}

    bool isOn() const
    {
        if (auto* v = apvts.getRawParameterValue("bypass"))
            return v->load() >= 0.5f;
        return false;
    }

    std::function<void()> onToggled;

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        const bool active = !isOn();
        const auto accent = theme.palette().knobArcActive;
        g.setColour(active ? accent : accent.withAlpha(0.25f));
        g.fillEllipse(b);
    }

    void mouseDown(const juce::MouseEvent&) override
    {
        if (auto* p = apvts.getParameter("bypass"))
        {
            p->beginChangeGesture();
            p->setValueNotifyingHost(isOn() ? 0.0f : 1.0f);
            p->endChangeGesture();
        }
        repaint();
        if (onToggled) onToggled();
    }

private:
    juce::AudioProcessorValueTreeState& apvts;
    Theme& theme;
};

class BypassOverlay : public juce::Component
{
public:
    BypassOverlay(Theme& th) : theme(th)
    {
        setInterceptsMouseClicks(true, false);
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(theme.palette().background.withAlpha(0.7f));
        g.fillRect(getLocalBounds());
    }

private:
    Theme& theme;
};

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
    bool lastBypassState = false;

    void setupKnob(Knob& slot, const char* paramId, const juce::String& caption);
    void layoutLeftSection (juce::Rectangle<int> area);
    void layoutRightSection(juce::Rectangle<int> area);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VitOttAudioProcessorEditor)
};

} // namespace vitottx
