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

#include <cmath>

namespace vitottx
{

class HoverFader
{
public:
    explicit HoverFader(float decayRate = 0.30f) noexcept : decay(decayRate) {}

    void setTarget(bool active) noexcept { target = active ? 1.0f : 0.0f; }

    // Advance one tick. Returns true if the value moved (caller should repaint).
    bool tick() noexcept
    {
        const float diff = target - value;
        if (std::abs(diff) < kSettled)
        {
            if (value == target)
                return false;
            value = target;
            return true;
        }
        value += diff * decay;
        return true;
    }

    float getValue() const noexcept { return value; }
    bool  isMoving() const noexcept { return std::abs(target - value) >= kSettled; }

private:
    static constexpr float kSettled = 1.0f / 512.0f;
    float decay;
    float target = 0.0f;
    float value  = 0.0f;
};

} // namespace vitottx
