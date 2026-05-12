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

class BypassOverlay : public juce::Component
{
public:
    explicit BypassOverlay(const Theme& th) : theme(th)
    {
        setInterceptsMouseClicks(true, false);
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(theme.palette().background.withAlpha(0.7f));
        g.fillRect(getLocalBounds());
    }

private:
    const Theme& theme;
};

} // namespace vitottx
