#include "DebugWindow.h"

// Give the window a color 
DebugWindow::DebugWindow()
    : juce::DocumentWindow ("Debug", juce::Colours::darkgrey, juce::DocumentWindow::allButtons)
{
        setUsingNativeTitleBar(true);
        setAlwaysOnTop(true);
        setResizable(true, false);
        
        // Initialize mono buffer with the window
        monoBuffer.setSize(1, 512);
        
        // Create the plot sources (configured later in prepareToPlay with sample rate)
        oscilloscope = magicState.createAndAddObject<foleys::MagicOscilloscope>("waveform");
        analyser = magicState.createAndAddObject<foleys::MagicAnalyser>("input");
        
        // Create and setup PlotContainer (private nested class) 
        // It has MagicPlotComponents as members
        container = std::make_unique<PlotContainer>();
        container->setOpaque(true);
        
        // Configure oscilloscope component
        container->oscilloscopeComponent.setPlotSource(oscilloscope);
        container->oscilloscopeComponent.setColour(foleys::MagicPlotComponent::plotColourId, juce::Colours::lime);
        container->oscilloscopeComponent.setColour(foleys::MagicPlotComponent::plotFillColourId, juce::Colours::transparentBlack);
        container->oscilloscopeComponent.setVisible(true);
        container->oscilloscopeComponent.setOpaque(false);
        container->addAndMakeVisible(container->oscilloscopeComponent);
        
        // Configure analyser component
        container->analyserComponent.setPlotSource(analyser);
        container->analyserComponent.setColour(foleys::MagicPlotComponent::plotColourId, juce::Colours::cyan);
        container->analyserComponent.setColour(foleys::MagicPlotComponent::plotFillColourId, juce::Colours::transparentBlack);
        container->analyserComponent.setVisible(true);
        container->analyserComponent.setOpaque(false);
        container->addAndMakeVisible(container->analyserComponent);
        
        // Set the container as the content component of the window
        // Content component will be deleted by DocumentWindow destructor
        // Content component is the main child component of the window.
        setContentNonOwned(container.get(), true);
        centreWithSize(600, 400);
        setVisible(true);
        
        // Start timer to trigger repaints
        startTimer(50); // 20 fps refresh rate
}

DebugWindow::~DebugWindow()
{
    stopTimer();
    clearContentComponent();
    /* 
    1) Stop the timer
    2) Clear the content component (detach container from window)
    3) Window destructs
    4) container unique_ptr destructs
    */
}

void DebugWindow::closeButtonPressed()
{
    setVisible(false);
}

void DebugWindow::timerCallback()
{
    // Trigger repaints
    if (container)
    {
        container->repaint();
        container->analyserComponent.repaint();
        container->oscilloscopeComponent.repaint();
    }
}

void DebugWindow::resized()
{
    // Guard against resized() being called before components are initialized
    // This prevents a segmentation fault that was happening ... so don't take it out!
    if (!container)
        return;

    // Container's resized() will handle layout of plot components
    container->setBounds(getLocalBounds());

    juce::DocumentWindow::resized();
}

void DebugWindow::prepareToPlay(double sampleRate, int samplesPerBlockExpected)
{
    oscilloscope->prepareToPlay(sampleRate, samplesPerBlockExpected);
    analyser->prepareToPlay(sampleRate, samplesPerBlockExpected);
}

void DebugWindow::pushAudioSamples(const juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumSamples() <= 0 || buffer.getNumChannels() <= 0)
        return;
    
        // leaving this try and catch in place to avoid any potential crashes
    try
    {
        // Ensure our mono buffer has the right size
        if (monoBuffer.getNumSamples() != buffer.getNumSamples())
            monoBuffer.setSize(1, buffer.getNumSamples(), false, true, false);
        
        monoBuffer.clear();
        
        // Get write pointer for our mono buffer
        float* monoData = monoBuffer.getWritePointer(0);
        
        if (monoData == nullptr)
            return;
        
        if (buffer.getNumChannels() == 1)
        {
            // Already mono, copy it directly
            const float* sourceData = buffer.getReadPointer(0);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                monoData[i] = sourceData[i];
        }
        else if (buffer.getNumChannels() >= 2)
        {
            // Mix first two channels to mono (average them)
            const float* channel0 = buffer.getReadPointer(0);
            const float* channel1 = buffer.getReadPointer(1);
            
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                monoData[i] = (channel0[i] + channel1[i]) * 0.5f;
        }
        
        // Push to oscilloscope and analyser 
        oscilloscope->pushSamples(monoBuffer);
        analyser->pushSamples(monoBuffer);
    }
    catch (const std::exception& e)
    {
        juce::Logger::writeToLog(juce::String("Error pushing audio samples: ") + e.what());
    }
}
