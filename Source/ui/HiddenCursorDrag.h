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
#include <optional>

namespace vitottx
{

class HiddenCursorDrag
{
public:
    void begin(juce::Component& host, const juce::MouseInputSource& source)
    {
        if (active)
            return;
        anchor = source.getScreenPosition();
        source.enableUnboundedMouseMovement(true, false);
        host.setMouseCursor(juce::MouseCursor::NoCursor);
        active = true;
    }

    void end(juce::Component& host,
             const juce::MouseInputSource& source,
             std::optional<juce::Point<float>> restoreOverride = std::nullopt)
    {
        if (!active)
            return;
        source.enableUnboundedMouseMovement(false, true);
        const auto pos = restoreOverride.value_or(anchor);
        juce::Desktop::getInstance().getMainMouseSource().setScreenPosition(pos);
        host.setMouseCursor(juce::MouseCursor::NormalCursor);
        active = false;
    }

    juce::Point<float> getAnchor() const noexcept { return anchor; }
    bool isActive() const noexcept { return active; }

private:
    juce::Point<float> anchor;
    bool active = false;
};

} // namespace vitottx
