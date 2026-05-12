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
#include <functional>

#include "HoverFader.h"
#include "Theme.h"

namespace vitottx
{

class Sidebar : public juce::Component, private juce::Timer
{
public:
    Sidebar(const Theme& t, juce::String title, juce::String version)
        : theme(t), titleText(std::move(title)), versionText(std::move(version))
    {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    std::function<void()> onTitleClick;

    void paint(juce::Graphics& g) override
    {
        const auto& pal = theme.palette();
        const auto sidebar = getLocalBounds().toFloat();

        {
            juce::DropShadow shadow(juce::Colours::black.withAlpha(0.45f),
                                    theme.scaledInt(BaseMetrics::kSidebarShadowRadius),
                                    { theme.scaledInt(BaseMetrics::kSidebarShadowOffset), 0 });
            shadow.drawForRectangle(g, getLocalBounds());
        }

        g.setColour(pal.sidebar);
        g.fillRect(sidebar);

        const auto titleColour = pal.sidebar.overlaidWith(
            juce::Colours::white.withAlpha(0.55f).interpolatedWith(pal.meter, fader.getValue()));

        const auto drawRotated = [&](const juce::String& text,
                                     juce::Font font,
                                     juce::Colour colour,
                                     float pivotY,
                                     juce::Justification justification,
                                     int textLen)
        {
            juce::Graphics::ScopedSaveState s(g);
            const float pivotX = sidebar.getCentreX();
            g.addTransform(juce::AffineTransform::translation(-pivotX, -pivotY)
                               .rotated(-juce::MathConstants<float>::halfPi)
                               .translated(pivotX, pivotY));
            g.setColour(colour);
            g.setFont(font);
            const int textThk = (int) sidebar.getWidth();
            const int x = justification == juce::Justification::centred
                              ? (int) pivotX - textLen / 2
                              : (int) pivotX;
            const juce::Rectangle<int> rect(x, (int) pivotY - textThk / 2, textLen, textThk);
            g.drawText(text, rect, justification);
        };

        const int sidebarTextGap = theme.scaledInt(BaseMetrics::kSidebarVersionBottomGap);

        drawRotated(titleText,
                    titleFont(),
                    titleColour,
                    sidebar.getCentreY(),
                    juce::Justification::centred,
                    (int) sidebar.getHeight());

        drawRotated(versionText,
                    versionFont(),
                    pal.sidebar.overlaidWith(juce::Colours::white.withAlpha(0.3f)),
                    sidebar.getBottom() - theme.scaled((float) BaseMetrics::kSidebarVersionBottomGap),
                    juce::Justification::centredLeft,
                    juce::jmax(0, (int) sidebar.getHeight() - 2 * sidebarTextGap));
    }

    bool hitTest(int x, int y) override
    {
        return titleHitZone().contains(x, y);
    }

    void mouseEnter(const juce::MouseEvent&) override
    {
        fader.setTarget(true);
        startTimerHz(60);
    }

    void mouseExit(const juce::MouseEvent&) override
    {
        fader.setTarget(false);
        startTimerHz(60);
    }

    void mouseDown(const juce::MouseEvent&) override
    {
        if (onTitleClick) onTitleClick();
    }

private:
    void timerCallback() override
    {
        if (fader.tick())
            repaint();
        if (! fader.isMoving())
            stopTimer();
    }

    juce::Font titleFont() const
    {
        return juce::Font(juce::FontOptions(theme.scaled((float) BaseMetrics::kSidebarFontSize))
                              .withStyle("Bold"));
    }

    juce::Font versionFont() const
    {
        return juce::Font(juce::FontOptions(theme.scaled((float) BaseMetrics::kSidebarVersionFontSize)));
    }

    juce::Rectangle<int> titleHitZone() const
    {
        const int textW = juce::GlyphArrangement::getStringWidthInt(titleFont(), titleText);
        const int halfH = textW / 2 + theme.scaledInt(3);
        const auto b = getLocalBounds();
        return { b.getX(), b.getCentreY() - halfH, b.getWidth(), halfH * 2 };
    }

    const Theme& theme;
    juce::String titleText;
    juce::String versionText;
    HoverFader   fader { 0.18f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sidebar)
};

} // namespace vitottx
