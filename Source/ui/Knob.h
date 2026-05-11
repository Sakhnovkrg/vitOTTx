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

#include "HiddenCursorDrag.h"
#include "Theme.h"

namespace vitottx
{

class Knob : public juce::Slider
{
public:
    explicit Knob(const Theme& theme);
    ~Knob() override = default;

    void setCaption(const juce::String& text);
    void attachTo  (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId);

    void paint   (juce::Graphics&) override;
    void resized () override;
    bool hitTest (int x, int y) override;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp   (const juce::MouseEvent&) override;
    void modifierKeysChanged(const juce::ModifierKeys&) override {}

private:
    const Theme& theme;
    juce::String caption;

    juce::Rectangle<int> rotaryArea;
    juce::Rectangle<int> captionArea;

    HiddenCursorDrag drag;
    float dragStartProp = 0.0f;
    float dragStartY    = 0.0f;
    bool  wasShift      = false;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Knob)
};

} // namespace vitottx
