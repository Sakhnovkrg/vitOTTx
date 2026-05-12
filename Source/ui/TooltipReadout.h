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

#include <juce_gui_basics/juce_gui_basics.h>

#include "Theme.h"

namespace vitottx
{

class TooltipReadout : public juce::Component
{
public:
    explicit TooltipReadout(const Theme& th) : theme(th)
    {
        setInterceptsMouseClicks(false, false);
    }

    void setContent(const juce::String& n, const juce::String& v)
    {
        captionText = n;
        valueText   = v;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour(juce::Colours::black.withAlpha(0.65f));
        g.fillRoundedRectangle(b, theme.scaled(3.0f));

        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(theme.scaled((float) BaseMetrics::kTooltipFontSize)));
        g.drawText(captionText + "  " + valueText, getLocalBounds(), juce::Justification::centred);
    }

private:
    const Theme& theme;
    juce::String captionText;
    juce::String valueText;
};

} // namespace vitottx
