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

#include "Theme.h"

namespace vitottx
{

class BypassButton : public juce::Component, private juce::Timer
{
public:
    BypassButton(juce::AudioProcessorValueTreeState& a, const Theme& th)
        : apvts(a), theme(th)
    {
        fade = targetFade = isOn() ? 0.0f : 1.0f;
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    bool isOn() const
    {
        if (auto* v = apvts.getRawParameterValue(paramId))
            return v->load() >= 0.5f;
        return false;
    }

    void syncFromState()
    {
        const float wanted = isOn() ? 0.0f : 1.0f;
        if (! juce::approximatelyEqual(targetFade, wanted))
        {
            targetFade = wanted;
            startTimerHz(60);
        }
    }

    std::function<void()> onToggled;

    void paint(juce::Graphics& g) override
    {
        const auto accent = theme.palette().knobArcActive;
        const float alpha = juce::jmap(fade, kInactiveAlpha, 1.0f);
        g.setColour(accent.withAlpha(alpha));
        g.fillEllipse(getLocalBounds().toFloat());
    }

    void mouseDown(const juce::MouseEvent&) override
    {
        if (auto* p = apvts.getParameter(paramId))
        {
            p->beginChangeGesture();
            p->setValueNotifyingHost(isOn() ? 0.0f : 1.0f);
            p->endChangeGesture();
        }
        syncFromState();
        if (onToggled) onToggled();
    }

private:
    void timerCallback() override
    {
        fade += (targetFade - fade) * kEasing;
        if (std::abs(targetFade - fade) < 0.005f)
        {
            fade = targetFade;
            stopTimer();
        }
        repaint();
    }

    static constexpr const char* paramId = "bypass";
    static constexpr float kInactiveAlpha = 0.25f;
    static constexpr float kEasing        = 0.35f;

    juce::AudioProcessorValueTreeState& apvts;
    const Theme& theme;

    float fade       = 1.0f;
    float targetFade = 1.0f;
};

} // namespace vitottx
