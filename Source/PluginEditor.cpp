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

#include "PluginEditor.h"

namespace vitottx
{

static BandView::ParamIds makeIds(const char* prefix)
{
    return BandView::ParamIds {
        juce::String(prefix) + "_upper_thres",
        juce::String(prefix) + "_lower_thres",
        juce::String(prefix) + "_upper_ratio",
        juce::String(prefix) + "_lower_ratio"
    };
}

VitOttAudioProcessorEditor::VitOttAudioProcessorEditor(VitOttAudioProcessor& p)
    : juce::AudioProcessorEditor(&p),
      processor(p),
      lowBand (p.getAPVTS(), theme, makeIds("low")),
      midBand (p.getAPVTS(), theme, makeIds("band")),
      highBand(p.getAPVTS(), theme, makeIds("high"))
{
    addAndMakeVisible(lowBand);
    addAndMakeVisible(midBand);
    addAndMakeVisible(highBand);

    hint.setColour(juce::Label::backgroundColourId, juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.0f, 0.55f));
    hint.setColour(juce::Label::textColourId, theme.palette().textPrimary);
    hint.setJustificationType(juce::Justification::centred);
    hint.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(hint);

    auto bind = [this](BandView& b) {
        b.onParamChange = [this](const juce::String& id, float v) { showParam(id, v); };
    };
    bind(lowBand);
    bind(midBand);
    bind(highBand);

    setResizable(true, true);
    setResizeLimits(BaseMetrics::kReferenceWidth,
                    BaseMetrics::kReferenceHeight,
                    BaseMetrics::kReferenceWidth  * 3,
                    BaseMetrics::kReferenceHeight * 3);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio((double) BaseMetrics::kReferenceWidth
                               / (double) BaseMetrics::kReferenceHeight);
    setSize(BaseMetrics::kReferenceWidth, BaseMetrics::kReferenceHeight);
}

VitOttAudioProcessorEditor::~VitOttAudioProcessorEditor() = default;

void VitOttAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(theme.palette().background);

    auto bounds = panelBounds.toFloat();
    const float r = (float) theme.panelCornerRadius();
    g.setColour(theme.palette().panel);
    g.fillRoundedRectangle(bounds, r);
    g.setColour(theme.palette().panelOutline);
    g.drawRoundedRectangle(bounds.reduced(theme.panelOutlineWidth() * 0.5f), r, theme.panelOutlineWidth());
}

void VitOttAudioProcessorEditor::resized()
{
    const float xScale = (float) getWidth()  / (float) BaseMetrics::kReferenceWidth;
    const float yScale = (float) getHeight() / (float) BaseMetrics::kReferenceHeight;
    theme.setScale(juce::jmin(xScale, yScale));

    auto area = getLocalBounds().reduced(theme.padding());
    const int hintH = theme.labelHeight() + theme.smallPadding();
    hint.setBounds(area.removeFromBottom(hintH));
    hint.setFont(juce::FontOptions(theme.fontValue()));
    area.removeFromBottom(theme.smallPadding());

    area.removeFromLeft(theme.scaledInt(BaseMetrics::kLeftSectionWidth));
    area.removeFromRight(theme.scaledInt(BaseMetrics::kRightSectionWidth));

    panelBounds = area;

    auto inner = area.reduced(theme.smallPadding());
    const int gap = juce::jmax(2, theme.smallPadding() / 2);
    const int totalGap  = gap * 2;
    const int bandWidth = (inner.getWidth() - totalGap) / 3;

    lowBand .setBounds(inner.removeFromLeft(bandWidth));
    inner.removeFromLeft(gap);
    midBand .setBounds(inner.removeFromLeft(bandWidth));
    inner.removeFromLeft(gap);
    highBand.setBounds(inner);
}

void VitOttAudioProcessorEditor::showParam(const juce::String& id, float value)
{
    auto* p = processor.getAPVTS().getParameter(id);
    const juce::String name = (p != nullptr) ? p->getName(64) : id;
    hint.setText(name + " = " + juce::String(value, 2), juce::dontSendNotification);
}

} // namespace vitottx
