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

#include <cmath>

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
      highBand(p.getAPVTS(), theme, makeIds("high")),
      mixKnob    (theme),
      lowKnob    (theme),
      bandKnob   (theme),
      highKnob   (theme),
      attackKnob (theme),
      releaseKnob(theme)
{
    addAndMakeVisible(lowBand);
    addAndMakeVisible(midBand);
    addAndMakeVisible(highBand);

    setupKnob(mixKnob,     "mix",      "MIX");
    setupKnob(lowKnob,     "lgain",    "LOW");
    setupKnob(bandKnob,    "mgain",    "BAND");
    setupKnob(highKnob,    "hgain",    "HIGH");
    setupKnob(attackKnob,  "att_time", "ATTACK");
    setupKnob(releaseKnob, "rel_time", "RELEASE");

    for (auto* k : { &mixKnob, &lowKnob, &bandKnob, &highKnob, &attackKnob, &releaseKnob })
        addAndMakeVisible(*k);

    setResizable(true, true);
    setResizeLimits(BaseMetrics::kReferenceWidth,
                    BaseMetrics::kReferenceHeight,
                    BaseMetrics::kReferenceWidth  * 3,
                    BaseMetrics::kReferenceHeight * 3);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio((double) BaseMetrics::kReferenceWidth
                               / (double) BaseMetrics::kReferenceHeight);
    setSize(BaseMetrics::kReferenceWidth, BaseMetrics::kReferenceHeight);

    startTimerHz(30);
}

VitOttAudioProcessorEditor::~VitOttAudioProcessorEditor() = default;

void VitOttAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(theme.palette().background);

    auto bounds = panelBounds.toFloat();
    const float r = (float) theme.panelCornerRadius();
    g.setColour(theme.palette().panel);
    g.fillRoundedRectangle(bounds, r);
}

void VitOttAudioProcessorEditor::resized()
{
    const float xScale = (float) getWidth()  / (float) BaseMetrics::kReferenceWidth;
    const float yScale = (float) getHeight() / (float) BaseMetrics::kReferenceHeight;
    theme.setScale(juce::jmin(xScale, yScale));

    const int outerPadding = theme.scaledInt(10);
    const int gap          = theme.scaledInt(10);
    const int knobSlot     = theme.scaledInt(60);

    const int leftSectionWidth  = 3 * knobSlot + 2 * gap;
    const int rightSectionWidth = knobSlot;

    auto area = getLocalBounds().reduced(outerPadding);
    auto leftArea  = area.removeFromLeft (leftSectionWidth);   area.removeFromLeft (gap);
    auto rightArea = area.removeFromRight(rightSectionWidth);  area.removeFromRight(gap);
    auto bandsArea = area;

    panelBounds = bandsArea;

    layoutLeftSection (leftArea);
    layoutRightSection(rightArea);

    using juce::FlexBox;
    using juce::FlexItem;

    const int bandsPadding = theme.scaledInt(16);
    auto bandsInner = bandsArea.reduced(bandsPadding, 0);

    FlexBox bands;
    bands.flexDirection = FlexBox::Direction::row;
    bands.items.add(FlexItem(lowBand ).withFlex(1.0f).withMargin({ 0, (float) bandsPadding, 0, 0 }));
    bands.items.add(FlexItem(midBand ).withFlex(1.0f).withMargin({ 0, (float) bandsPadding, 0, 0 }));
    bands.items.add(FlexItem(highBand).withFlex(1.0f));
    bands.performLayout(bandsInner.toFloat());
}

void VitOttAudioProcessorEditor::setupKnob(Knob& slot,
                                           const char* paramId,
                                           const juce::String& caption)
{
    slot.setCaption(caption);
    slot.attachTo(processor.getAPVTS(), paramId);
}

void VitOttAudioProcessorEditor::layoutLeftSection(juce::Rectangle<int> area)
{
    using juce::FlexBox;
    using juce::FlexItem;

    const int gap = theme.scaledInt(10);
    const int topRowDrop = theme.scaledInt(5);
    const int rowH = (area.getHeight() - gap) / 2;
    auto topRow = area.removeFromTop(rowH);
    topRow.removeFromTop(topRowDrop);
    area.removeFromTop(gap);
    auto botRow = area;

    FlexBox top;
    top.flexDirection = FlexBox::Direction::row;
    top.items.add(FlexItem().withFlex(1.0f).withMargin({ 0, (float) gap, 0, 0 }));
    top.items.add(FlexItem().withFlex(1.0f).withMargin({ 0, (float) gap, 0, 0 }));
    top.items.add(FlexItem(mixKnob).withFlex(1.0f));
    top.performLayout(topRow.toFloat());

    FlexBox bot;
    bot.flexDirection = FlexBox::Direction::row;
    bot.items.add(FlexItem(lowKnob ).withFlex(1.0f).withMargin({ 0, (float) gap, 0, 0 }));
    bot.items.add(FlexItem(bandKnob).withFlex(1.0f).withMargin({ 0, (float) gap, 0, 0 }));
    bot.items.add(FlexItem(highKnob).withFlex(1.0f));
    bot.performLayout(botRow.toFloat());
}

void VitOttAudioProcessorEditor::layoutRightSection(juce::Rectangle<int> area)
{
    using juce::FlexBox;
    using juce::FlexItem;

    const int gap = theme.scaledInt(10);
    const int topRowDrop = theme.scaledInt(5);
    area.removeFromTop(topRowDrop);

    FlexBox col;
    col.flexDirection = FlexBox::Direction::column;
    col.items.add(FlexItem(attackKnob ).withFlex(1.0f).withMargin({ 0, 0, (float) gap, 0 }));
    col.items.add(FlexItem(releaseKnob).withFlex(1.0f));
    col.performLayout(area.toFloat());
}

void VitOttAudioProcessorEditor::timerCallback()
{
    static constexpr float kMsFloor = 1.0e-8f; // ~-80 dB

    const auto msToDb = [](float ms) -> float
    {
        return 10.0f * std::log10(juce::jmax(kMsFloor, ms));
    };

    const auto pushBand = [&](BandView& band, int bandIndex)
    {
        band.setInputLevels (msToDb(processor.getInputMeanSquared (bandIndex, 0)),
                             msToDb(processor.getInputMeanSquared (bandIndex, 1)));
        band.setOutputLevels(msToDb(processor.getOutputMeanSquared(bandIndex, 0)),
                             msToDb(processor.getOutputMeanSquared(bandIndex, 1)));
    };

    pushBand(lowBand,  0);
    pushBand(midBand,  1);
    pushBand(highBand, 2);
}

} // namespace vitottx
