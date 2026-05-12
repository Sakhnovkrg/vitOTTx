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
      inKnob     (theme),
      outKnob    (theme),
      mixKnob    (theme),
      lowKnob    (theme),
      bandKnob   (theme),
      highKnob   (theme),
      attackKnob (theme),
      releaseKnob(theme),
      lowCrossHandle (p.getAPVTS(), theme, CrossoverHandle::Side::Low),
      highCrossHandle(p.getAPVTS(), theme, CrossoverHandle::Side::High),
      bypassButton(p.getAPVTS(), theme),
      bypassOverlay(theme),
      readout(theme)
{
    addAndMakeVisible(lowBand);
    addAndMakeVisible(midBand);
    addAndMakeVisible(highBand);

    setupKnob(inKnob,      "in_gain",  "IN");
    setupKnob(outKnob,     "out_gain", "OUT");
    setupKnob(mixKnob,     "mix",      "MIX");
    setupKnob(lowKnob,     "lgain",    "LOW");
    setupKnob(bandKnob,    "mgain",    "BAND");
    setupKnob(highKnob,    "hgain",    "HIGH");
    setupKnob(attackKnob,  "att_time", "ATTACK");
    setupKnob(releaseKnob, "rel_time", "RELEASE");

    for (auto* k : { &inKnob, &outKnob, &mixKnob, &lowKnob, &bandKnob, &highKnob, &attackKnob, &releaseKnob })
        addAndMakeVisible(*k);

    addAndMakeVisible(lowCrossHandle);
    addAndMakeVisible(highCrossHandle);
    addChildComponent(bypassOverlay);
    bypassOverlay.setAlpha(0.0f);

    addAndMakeVisible(bypassButton);
    lastBypassState = bypassButton.isOn();

    auto applyBypassUi = [this](bool animated)
    {
        const bool on = bypassButton.isOn();
        auto& animator = juce::Desktop::getInstance().getAnimator();

        if (on)
        {
            bypassOverlay.setVisible(true);
            bypassOverlay.toFront(false);
            bypassButton.toFront(false);
            if (animated)
                animator.fadeIn(&bypassOverlay, kBypassFadeMs);
            else
                bypassOverlay.setAlpha(1.0f);
        }
        else
        {
            if (animated)
                animator.fadeOut(&bypassOverlay, kBypassFadeMs);
            else
            {
                bypassOverlay.setAlpha(0.0f);
                bypassOverlay.setVisible(false);
            }
        }
    };
    bypassButton.onToggled = [applyBypassUi]() { applyBypassUi(true); };
    applyBypassUi(false);

    addChildComponent(readout);

    auto wireReadout = [this](auto& control)
    {
        control.onShowReadout = [this](const juce::String& n, const juce::String& v) { showReadout(n, v); };
        control.onHideReadout = [this]() { hideReadout(); };
    };
    for (auto* k : { &inKnob, &outKnob, &mixKnob, &lowKnob, &bandKnob, &highKnob, &attackKnob, &releaseKnob })
        wireReadout(*k);
    wireReadout(lowCrossHandle);
    wireReadout(highCrossHandle);
    wireReadout(lowBand);
    wireReadout(midBand);
    wireReadout(highBand);

    auto fmtDb  = [](double v) { return juce::String(v, 1) + " dB"; };
    auto fmtPct = [](double v) { return juce::String(juce::roundToInt(v * 100.0)) + " %"; };

    inKnob.setValueFormat(fmtDb);
    outKnob.setValueFormat(fmtDb);
    mixKnob.setValueFormat(fmtPct);
    lowKnob.setValueFormat(fmtDb);
    bandKnob.setValueFormat(fmtDb);
    highKnob.setValueFormat(fmtDb);
    attackKnob.setValueFormat(fmtPct);
    releaseKnob.setValueFormat(fmtPct);
    lowCrossHandle.onFreqChange  = [this]() { resized(); };
    highCrossHandle.onFreqChange = [this]() { resized(); };

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

    g.setColour(theme.palette().panel);
    g.fillRoundedRectangle(panelBounds.toFloat(), (float) theme.panelCornerRadius());

    paintSidebar(g);
}

void VitOttAudioProcessorEditor::paintSidebar(juce::Graphics& g)
{
    const auto sidebar = sidebarBounds.toFloat();

    {
        juce::DropShadow shadow(juce::Colours::black.withAlpha(0.45f),
                                theme.scaledInt(BaseMetrics::kSidebarShadowRadius),
                                { theme.scaledInt(BaseMetrics::kSidebarShadowOffset), 0 });
        shadow.drawForRectangle(g, sidebarBounds);
    }

    g.setColour(theme.palette().sidebar);
    g.fillRect(sidebar);

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

    drawRotated("vitOTTx",
                juce::Font(juce::FontOptions(theme.scaled((float) BaseMetrics::kSidebarFontSize)).withStyle("Bold")),
                juce::Colours::white.withAlpha(0.55f),
                sidebar.getCentreY(),
                juce::Justification::centred,
                (int) sidebar.getHeight());

    drawRotated("0.1.0",
                juce::Font(juce::FontOptions(theme.scaled((float) BaseMetrics::kSidebarVersionFontSize))),
                juce::Colours::white.withAlpha(0.3f),
                sidebar.getBottom() - theme.scaled((float) BaseMetrics::kSidebarVersionBottomGap),
                juce::Justification::centredLeft,
                juce::jmax(0, (int) sidebar.getHeight() - 2 * sidebarTextGap));
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

    auto fullArea = getLocalBounds();
    const int sidebarW = theme.scaledInt(BaseMetrics::kSidebarWidth);
    sidebarBounds = fullArea.removeFromLeft(sidebarW);

    const int btnD       = theme.scaledInt(BaseMetrics::kBypassButtonDiameter);
    const int btnTopGap  = theme.scaledInt(BaseMetrics::kBypassButtonTopGap);
    bypassButton.setBounds(sidebarBounds.getCentreX() - btnD / 2,
                           sidebarBounds.getY() + btnTopGap,
                           btnD, btnD);

    bypassOverlay.setBounds(fullArea);
    if (bypassOverlay.isVisible())
    {
        bypassOverlay.toFront(false);
        bypassButton.toFront(false);
    }

    auto area = fullArea.reduced(outerPadding);
    auto leftArea  = area.removeFromLeft (leftSectionWidth);   area.removeFromLeft (gap);
    auto rightArea = area.removeFromRight(rightSectionWidth);  area.removeFromRight(gap);
    auto bandsArea = area;

    panelBounds = bandsArea;

    layoutLeftSection (leftArea);
    layoutRightSection(rightArea);

    using juce::FlexBox;
    using juce::FlexItem;

    const int bandsPadding    = theme.scaledInt(16);
    const int halfGap         = bandsPadding / 2;
    const int handleThickness = theme.scaledInt(4);
    const int minBandWidth    = theme.scaledInt(35);
    const int minMidWidth     = theme.scaledInt(35);

    auto bandsInner = bandsArea.reduced(bandsPadding, 0);
    const int rangeLeft  = bandsInner.getX();
    const int rangeRight = bandsInner.getRight();

    const float lowFreq  = lowCrossHandle.getFrequency();
    const float highFreq = highCrossHandle.getFrequency();

    const int lowX  = CrossoverHandle::freqToX(lowFreq,  rangeLeft, rangeRight);
    const int highX = CrossoverHandle::freqToX(highFreq, rangeLeft, rangeRight);

    const bool lowCollapsed  = (lowX  - rangeLeft) < minBandWidth;
    const bool highCollapsed = (rangeRight - highX) < minBandWidth;

    auto makeBandRect = [&](int leftX, int rightX) -> juce::Rectangle<int>
    {
        const int w = juce::jmax(0, rightX - leftX);
        return { leftX, bandsInner.getY(), w, bandsInner.getHeight() };
    };

    const int midLeftX  = lowCollapsed  ? rangeLeft  : (lowX  + halfGap);
    const int midRightX = highCollapsed ? rangeRight : (highX - halfGap);

    lowBand.setVisible(!lowCollapsed);
    if (!lowCollapsed)
        lowBand.setBounds(makeBandRect(rangeLeft, lowX - halfGap));

    midBand.setBounds(makeBandRect(midLeftX, midRightX));

    highBand.setVisible(!highCollapsed);
    if (!highCollapsed)
        highBand.setBounds(makeBandRect(highX + halfGap, rangeRight));

    const int lowHandleX  = lowCollapsed
                              ? panelBounds.getX()    + bandsPadding / 2
                              : lowX;
    const int highHandleX = highCollapsed
                              ? panelBounds.getRight() - bandsPadding / 2
                              : highX;

    lowCrossHandle.setRange(rangeLeft, rangeRight);
    lowCrossHandle.setSnapMinimumWidth(minBandWidth);
    lowCrossHandle.setMinMidWidth(minMidWidth);
    lowCrossHandle.setCounterpartX(highX);
    lowCrossHandle.setCounterpartCollapsed(highCollapsed);
    lowCrossHandle.setBounds(lowHandleX - handleThickness / 2, bandsInner.getY(),
                             handleThickness, bandsInner.getHeight());

    highCrossHandle.setRange(rangeLeft, rangeRight);
    highCrossHandle.setSnapMinimumWidth(minBandWidth);
    highCrossHandle.setMinMidWidth(minMidWidth);
    highCrossHandle.setCounterpartX(lowX);
    highCrossHandle.setCounterpartCollapsed(lowCollapsed);
    highCrossHandle.setBounds(highHandleX - handleThickness / 2, bandsInner.getY(),
                              handleThickness, bandsInner.getHeight());
}

void VitOttAudioProcessorEditor::showReadout(const juce::String& name, const juce::String& value)
{
    readout.setContent(name, value);

    const juce::Font f(juce::FontOptions(theme.scaled((float) BaseMetrics::kTooltipFontSize)));
    const int padX = theme.scaledInt(BaseMetrics::kTooltipPaddingX);
    const int padY = theme.scaledInt(BaseMetrics::kTooltipPaddingY);
    const int w = juce::GlyphArrangement::getStringWidthInt(f, name + "  " + value) + 2 * padX;
    const int h = juce::roundToInt(f.getHeight()) + 2 * padY;
    const int y = panelBounds.getY() + theme.scaledInt(BaseMetrics::kTooltipTopGap);
    const int x = panelBounds.getCentreX() - w / 2;

    readout.setBounds(x, y, w, h);
    readout.setVisible(true);
    readout.toFront(false);
}

void VitOttAudioProcessorEditor::hideReadout()
{
    readout.setVisible(false);
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
    top.items.add(FlexItem(inKnob ).withFlex(1.0f).withMargin({ 0, (float) gap, 0, 0 }));
    top.items.add(FlexItem(outKnob).withFlex(1.0f).withMargin({ 0, (float) gap, 0, 0 }));
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

    const bool bypassed = bypassButton.isOn();
    if (bypassed != lastBypassState)
    {
        lastBypassState = bypassed;
        bypassButton.syncFromState();

        auto& animator = juce::Desktop::getInstance().getAnimator();
        if (bypassed)
        {
            bypassOverlay.setVisible(true);
            bypassOverlay.toFront(false);
            bypassButton.toFront(false);
            animator.fadeIn(&bypassOverlay, kBypassFadeMs);
        }
        else
        {
            animator.fadeOut(&bypassOverlay, kBypassFadeMs);
        }
    }
}

} // namespace vitottx
