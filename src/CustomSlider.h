#pragma once

#include <JuceHeader.h>
#include <foleys_gui_magic/foleys_gui_magic.h>

#include "ComponentContainer.h"

struct CustomSlider : public Slider
{
    void paint(Graphics& g) override {g.setColour(Colours::blue); g.fillAll();}
};

struct CustomSliderContainer : public ComponentContainer<Slider, SliderParameterAttachment>
{
    FOLEYS_DECLARE_GUI_FACTORY(CustomSliderContainer);

    CustomSliderContainer(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node)
        : ComponentContainer(builder, node)
    {
        setContainedComponent(&slider);
    }
private:
    CustomSlider slider;
    std::unique_ptr<SliderParameterAttachment> attachment;
};
