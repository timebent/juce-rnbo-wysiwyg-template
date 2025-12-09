#pragma once

#include <JuceHeader.h>
#include <foleys_gui_magic/foleys_gui_magic.h>

// Need the timer to periodically update the plots
class DebugWindow : public juce::DocumentWindow, private juce::Timer
{
public:
    DebugWindow();
    ~DebugWindow() override;

    void closeButtonPressed() override;
    void resized() override;
    void timerCallback() override;
    
    void prepareToPlay(double sampleRate, int samplesPerBlockExpected);
    void pushAudioSamples(const juce::AudioBuffer<float>& buffer);
    
private:
    // Simple container component that paints a background
    // and lays out the two plot components
    class PlotContainer : public juce::Component
    {
    public:
        foleys::MagicPlotComponent oscilloscopeComponent;
        foleys::MagicPlotComponent analyserComponent;
        
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colours::black);
            g.setColour(juce::Colours::darkgrey);
            auto bounds = getLocalBounds();
            g.drawLine(0, bounds.getHeight() / 2.0f, bounds.getWidth(), bounds.getHeight() / 2.0f, 2.0f);
        }
        
        void resized() override
        {
            // Flow layout - top half for oscilloscope, bottom half for analyser
            auto area = getLocalBounds();
            auto oscArea = area.removeFromTop(area.getHeight() / 2);
            oscilloscopeComponent.setBounds(oscArea);
            analyserComponent.setBounds(area);
        }
    };
    
    // the members we need from foleys_gui_magic
    // and a mono buffer for the visualizers
    foleys::MagicGUIState magicState;
    foleys::MagicOscilloscope* oscilloscope = nullptr;
    foleys::MagicPlotSource* analyser = nullptr;
    std::unique_ptr<PlotContainer> container;
    juce::AudioBuffer<float> monoBuffer;
};
