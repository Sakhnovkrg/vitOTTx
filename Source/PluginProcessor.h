/* Copyright 2021 Yegor Suslin
 *
 * vitOTT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vitOTT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with vitOTT.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>
#include "vital_dsp/compressor.h"
#include "vital_dsp/framework/value.h"

//==============================================================================
/**
*/
class VitOttAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    VitOttAudioProcessor();
    ~VitOttAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void writeAudio(vital::poly_float* comp_buf, juce::AudioSampleBuffer* buffer, int channels, int samples, int offset);
    void readAudio(vital::poly_float* comp_buf, juce::AudioSampleBuffer* buffer, int channels, int samples, int offset);

    void initVals();
    void updParams();

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return parameters; }

    juce::AudioProcessorParameter* getBypassParameter() const override
    {
        return parameters.getParameter("bypass");
    }

    // Real-time meter readouts (mean-squared per band, per stereo channel).
    // Updated from the audio thread, read by the GUI via a polling timer.
    // Index order: 0 = low, 1 = mid, 2 = high.
    float getInputMeanSquared(int bandIndex, int channel) const noexcept
    {
        return inputMeanSquared[bandIndex][channel].load(std::memory_order_relaxed);
    }

    float getOutputMeanSquared(int bandIndex, int channel) const noexcept
    {
        return outputMeanSquared[bandIndex][channel].load(std::memory_order_relaxed);
    }

private:
    //==============================================================================

    juce::AudioProcessorValueTreeState parameters;
    std::unique_ptr<vital::MultibandCompressor> comp;
    std::unique_ptr<vital::Output> sig_in;

    std::array <vital::Value*, 21> vals = {};

    double in_gain = 1.0f, out_gain = 1.0f;

    std::atomic<float> inputMeanSquared [3][2] {}; // [band][channel]
    std::atomic<float> outputMeanSquared[3][2] {}; // [band][channel]

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VitOttAudioProcessor)
};
