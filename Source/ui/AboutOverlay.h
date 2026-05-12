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

#include "Theme.h"

namespace vitottx
{

class AboutOverlay : public juce::Component
{
public:
    AboutOverlay(const Theme& t, juce::String version)
        : theme(t), versionText(std::move(version))
    {
        setInterceptsMouseClicks(true, true);

        configureLink(githubLink,  "GitHub",  "https://github.com/Sakhnovkrg/vitOTTx");
        configureLink(websiteLink, "Website", "https://dsgdnb.com");
        configureLink(donateLink,  "Donate",  "https://dsgdnb.com/donate");

        addAndMakeVisible(githubLink);
        addAndMakeVisible(websiteLink);
        addAndMakeVisible(donateLink);
    }

    std::function<void()> onClose;

    void paint(juce::Graphics& g) override
    {
        const auto& pal = theme.palette();
        g.fillAll(juce::Colour(0xee000000));

        const auto L = layout();
        const float scale = theme.getScale();

        const juce::Font titleFont  (juce::FontOptions(22.0f * scale).withStyle("Bold"));
        const juce::Font versionFont(juce::FontOptions(11.0f * scale));
        const juce::Font authorFont (juce::FontOptions(12.0f * scale));
        const juce::Font creditsFont(juce::FontOptions(10.5f * scale));

        g.setFont(titleFont);
        g.setColour(juce::Colours::white.withAlpha(0.95f));
        g.drawText("vitOTTx", L.title, juce::Justification::centred);

        g.setFont(versionFont);
        g.setColour(pal.textSecondary);
        g.drawText("v" + versionText, L.version, juce::Justification::centred);

        g.setFont(authorFont);
        g.setColour(juce::Colours::white.withAlpha(0.75f));
        g.drawText("by Alexandr Sakhnov (DsgDnB)", L.author, juce::Justification::centred);

        g.setFont(creditsFont);
        g.setColour(pal.textSecondary.withAlpha(0.75f));
        g.drawText("Based on Vital by Matt Tytel", L.credits1, juce::Justification::centred);
        g.drawText("via vitOTT by Yegor Suslin",   L.credits2, juce::Justification::centred);
    }

    void resized() override
    {
        const auto L = layout();
        const float scale = theme.getScale();
        const juce::Font linkFont(juce::FontOptions(12.0f * scale));

        githubLink .setBounds(L.github);
        websiteLink.setBounds(L.website);
        donateLink .setBounds(L.donate);

        githubLink .setFont(linkFont, false, juce::Justification::centred);
        websiteLink.setFont(linkFont, false, juce::Justification::centred);
        donateLink .setFont(linkFont, false, juce::Justification::centred);
    }

    void mouseDown(const juce::MouseEvent&) override
    {
        if (onClose) onClose();
    }

private:
    struct Layout
    {
        juce::Rectangle<int> title, version, author;
        juce::Rectangle<int> github, website, donate;
        juce::Rectangle<int> credits1, credits2;
    };

    Layout layout() const
    {
        const float scale = theme.getScale();
        const int titleH   = juce::roundToInt(24.0f * scale);
        const int versionH = juce::roundToInt(14.0f * scale);
        const int authorH  = juce::roundToInt(15.0f * scale);
        const int linkH    = juce::roundToInt(16.0f * scale);
        const int creditH  = juce::roundToInt(14.0f * scale);

        const int gapTinies  = juce::roundToInt(1.0f  * scale);
        const int gapBetween = juce::roundToInt(14.0f * scale);

        const int totalH = titleH + gapTinies + versionH + gapTinies + authorH
                         + gapBetween + linkH
                         + gapBetween + creditH + creditH;

        const auto a = getLocalBounds();
        int y = juce::jmax(0, (a.getHeight() - totalH) / 2);

        Layout L;
        L.title   = { a.getX(), y, a.getWidth(), titleH   }; y += titleH   + gapTinies;
        L.version = { a.getX(), y, a.getWidth(), versionH }; y += versionH + gapTinies;
        L.author  = { a.getX(), y, a.getWidth(), authorH  }; y += authorH  + gapBetween;

        const int linkW    = juce::roundToInt(70.0f * scale);
        const int linkColGap = 10;
        const int rowW     = linkW * 3 + linkColGap * 2;
        int xRow = a.getCentreX() - rowW / 2;
        L.github  = { xRow, y, linkW, linkH }; xRow += linkW + linkColGap;
        L.website = { xRow, y, linkW, linkH }; xRow += linkW + linkColGap;
        L.donate  = { xRow, y, linkW, linkH };
        y += linkH + gapBetween;

        L.credits1 = { a.getX(), y, a.getWidth(), creditH }; y += creditH;
        L.credits2 = { a.getX(), y, a.getWidth(), creditH };
        return L;
    }

    void configureLink(juce::HyperlinkButton& b,
                       const juce::String& text,
                       const juce::String& url)
    {
        b.setURL(juce::URL(url));
        b.setButtonText(text);
        b.setJustificationType(juce::Justification::centred);
        b.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        b.setColour(juce::HyperlinkButton::textColourId, theme.palette().meter);
    }

    const Theme& theme;
    juce::String versionText;

    juce::HyperlinkButton githubLink;
    juce::HyperlinkButton websiteLink;
    juce::HyperlinkButton donateLink;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AboutOverlay)
};

} // namespace vitottx
