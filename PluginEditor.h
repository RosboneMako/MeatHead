/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/

//R1.00 Structure to hold our slider/knob screen positions.
struct t_KnobInfo {
    float x;
    float y;
    float sizex;
    float sizey;
    int DataType;
    juce::String Suffix;
    juce::String Name;
};

//*******************************************************************************************************************
//R1.00 Create a new LOOK AND FEEL class based on Juces LnF class.
//R1.00 We will override the SLIDER drawing routine.
//*******************************************************************************************************************
//R1.00 This lets us modify how objects are drawn to the screen.
class MakoLookAndFeel : public juce::LookAndFeel_V4
{
public:
    //R1.00 Let the user select a knob style.
    float Kpts[32];
    juce::Path pathKnob;

private:
    //R1.00 Ten tick mark angles around a slider.
    float TICK_Angle[11] = { 8.79645920, 8.29380417, 7.79114914, 7.28849411, 6.78583908, 6.28318405, 5.78052902, 5.27787399, 4.77521896, 4.27256393, 3.76 }; //3.76990914
    float TICK_Cos[11] = {};
    float TICK_Sin[11] = {};

    juce::Image imgSwitchOn;
    juce::Image imgSwitchOff;

public:
    MakoLookAndFeel()
    {
        //imgSwitchOff = juce::ImageCache::getFromMemory(BinaryData::switchoff01_png, BinaryData::switchoff01_pngSize);
        //imgSwitchOn = juce::ImageCache::getFromMemory(BinaryData::switchon01_png, BinaryData::switchon01_pngSize);

        //R1.00 Do some PRECALC on Sin/Cos since they are expensive on CPU.
        for (int t = 0; t < 11; t++)
        {
            TICK_Cos[t] = std::cosf(TICK_Angle[t]);
            TICK_Sin[t] = std::sinf(TICK_Angle[t]);
        }

        //R1.00 Define the Path points to make a knob (Style 3).
        Kpts[0] = -2.65325243300477f;
        Kpts[1] = 8.60001462363607f;
        Kpts[2] = 0.0f;
        Kpts[3] = 10.0f;
        Kpts[4] = 2.65277678639377f;
        Kpts[5] = 8.60016135439157f;
        Kpts[6] = 7.81826556234706f;
        Kpts[7] = 6.23495979109873f;
        Kpts[8] = 8.3778301945593f;
        Kpts[9] = 3.28815468479365f;
        Kpts[10] = 9.74931428347318f;
        Kpts[11] = -2.22505528067641f;
        Kpts[12] = 7.79431009355225f;
        Kpts[13] = -4.4998589050713f;
        Kpts[14] = 4.3390509473009f;
        Kpts[15] = -9.00958583269659f;
        Kpts[16] = 1.34161181197136f;
        Kpts[17] = -8.89944255254108f;
        Kpts[18] = -4.33855264588318f;
        Kpts[19] = -9.00982579958681f;
        Kpts[20] = -6.12133095297134f;
        Kpts[21] = -6.59767439058605f;
        Kpts[22] = -9.74919120703023f;
        Kpts[23] = -2.22559448434896f;
        Kpts[24] = -8.97486228392824f;
        Kpts[25] = .672195644527914f;
        Kpts[26] = -7.81861038843018f;
        Kpts[27] = 6.23452737534543f;
        Kpts[28] = -5.07025014121689f;
        Kpts[29] = 7.4358969536627f;
        Kpts[30] = -2.65325243300477f;
        Kpts[31] = 8.60001462363607f;

        //R1.00 Create the actual PATH for our KNOB style 3.
        pathKnob.startNewSubPath(Kpts[0], Kpts[1]);
        for (int t = 0; t < 32; t += 2)
        {
            pathKnob.lineTo(Kpts[t], Kpts[t + 1]);
        }
        pathKnob.closeSubPath();

        //R1.00 Recreate our points with smoothed corners.
        //pathKnob = pathKnob.createPathWithRoundedCorners(4.0f);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle, juce::Slider& sld) override
    {
        //R1.00 Are using this func to draw Switches (On (1)/Off(0) only).
        if (sld.getValue() < .5f)
            g.drawImageAt(imgSwitchOff, x, y);
        else
            g.drawImageAt(imgSwitchOn, x, y);
    }

    //R1.00 Override the Juce SLIDER drawing function so our code gets called instead of Juces code.
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& sld) override
    {
        //R1.00 Most of these are from JUCE demo code. Could be reduced if not used.
        //R1.00 Could PRECALC if they were all the same size control. 
        auto radius = (float)juce::jmin(width / 2, height / 2) - 8.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle); //R1.00 Bizarre values here 216(36) to 504(324).

        //R1.00 Mako Var defs.
        float sinA;
        float cosA;
        juce::ColourGradient ColGrad;
        int col;
        juce::Colour C1;

        //R1.00 Used when creating background image.
        //g.setColour(juce::Colour(0xFF000000));
        //g.fillEllipse(rx, ry, rw, rw);
        //return;

        //1.00 Draw the KNOB face.
        ColGrad = juce::ColourGradient(juce::Colour(0xFF404040), 0.0f, y, juce::Colour(0xFF202020), 0.0f, y + height, false);
        g.setGradientFill(ColGrad);
        g.fillEllipse(rx, ry, rw, rw);

        //R2.00 Draw shading around knob face.
        g.setColour(juce::Colour(0xFF404040)); 
        g.drawEllipse(rx, ry, rw, rw, 1.0f);

        //R1.00 Dont draw anymore objects if the control is disabled.
        if (sld.isEnabled() == false) return;

        //R1.00 Copy our predefined KNOB PATH, scale it, and then transform it to the centre position.
        //R1.00 The knob SIZE must be performed first. It is then ROTATED around its center. Then moved (TRANSLATED) to the screen knob position.
        juce::Path pK = pathKnob;
        pK.applyTransform(juce::AffineTransform::scale(radius / 11.0f).followedBy(juce::AffineTransform::rotation(angle).translated(centreX, centreY)));
        ColGrad = juce::ColourGradient(juce::Colour(0xFFC0C0C0), 0.0f, y, juce::Colour(0xFF000000), 0.0f, y + height, false);
        g.setGradientFill(ColGrad);
        g.strokePath(pK, juce::PathStrokeType(2.0f));

        //R1.00 Knob notches. Not used but left here for others to use.
        //ColGrad = juce::ColourGradient(juce::Colour(0xFFE0E0E0), 0.0f, y, juce::Colour(0xFF404040), 0.0f, y + height, false);
        //g.setGradientFill(ColGrad);
        //for (float ang = .7854f; ang < 6.0f; ang += .7854f)
        //{
        //    sinA = std::sinf(ang + angle) * radius;
        //    cosA = std::cosf(ang + angle) * radius;
        //    g.drawLine(centreX + (sinA * .9f), centreY - (cosA * .9f), centreX + sinA , centreY - cosA, 1.0f);
        //}

        //R1.00 TICK marks on background.
        //R1.00 We are cheating and using the rotarySliderOutlineColourId as a tick mark style selector.
        g.setColour(juce::Colour(0xFFFFFFFF));
        C1 = sld.findColour(juce::Slider::rotarySliderOutlineColourId);
        if (C1 == juce::Colour(0x1))
        {
            for (int t = 0; t < 11; t++)
            {
                sinA = TICK_Sin[t] * radius;
                cosA = TICK_Cos[t] * radius;
                g.drawLine(centreX + (sinA * 1.2f), centreY - (cosA * 1.2f), centreX + sinA * 1.1f, centreY - cosA * 1.1f, 1.0f);
            }
        }
        if (C1 == juce::Colour(0x2))
        {
            sinA = TICK_Sin[0] * radius; cosA = TICK_Cos[0] * radius; g.drawLine(centreX + (sinA * 1.2f), centreY - (cosA * 1.2f), centreX + sinA * 1.1f, centreY - cosA * 1.1f, 1.0f);
            sinA = TICK_Sin[5] * radius; cosA = TICK_Cos[5] * radius; g.drawLine(centreX + (sinA * 1.2f), centreY - (cosA * 1.2f), centreX + sinA * 1.1f, centreY - cosA * 1.1f, 1.0f);
            sinA = TICK_Sin[10] * radius; cosA = TICK_Cos[10] * radius; g.drawLine(centreX + (sinA * 1.2f), centreY - (cosA * 1.2f), centreX + sinA * 1.1f, centreY - cosA * 1.1f, 1.0f);
        }
        if (C1 == juce::Colour(0x3))
        {
            for (int t = 0; t < 11; t+=2)
            {
                sinA = TICK_Sin[t] * radius;
                cosA = TICK_Cos[t] * radius;
                g.drawLine(centreX + (sinA * 1.2f), centreY - (cosA * 1.2f), centreX + sinA * 1.1f, centreY - cosA * 1.1f, 1.0f);
            }
        }

        //R1.00 Draw finger adjust dent/indicator.
        sinA = std::sinf(angle);
        cosA = std::cosf(angle);
        g.setColour(sld.findColour(juce::Slider::thumbColourId));        
        g.drawLine(centreX + sinA * radius * .5f, centreY - cosA * radius * .5f, centreX + sinA * radius, centreY - cosA * radius, 4.0f);


        if (sld.hasKeyboardFocus(true))
        {
            g.setColour(juce::Colour(0xFFFF0000));
            g.fillEllipse(centreX - 4.0f, centreY - 4.0f, 8.0f, 8.0f);            
        }
       
    }
};


//*******************************************************************************************************************
//R1.00 Add SLIDER listener. BUTTON or TIMER listeners also go here if needed. Must add ValueChanged overrides!
//*******************************************************************************************************************
class MakoBiteAudioProcessorEditor  : public juce::AudioProcessorEditor , public juce::Slider::Listener, public juce::Timer //, public juce::Button::Listener , public juce::Timer
{
public:
    MakoBiteAudioProcessorEditor (MakoBiteAudioProcessor&);
    ~MakoBiteAudioProcessorEditor() override;

    //R1.00 Override the TIMER so we can capture it and executes our UI code.
    void timerCallback() override;

    //R1.00 OUR override functions.
    void sliderValueChanged(juce::Slider* slider) override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MakoBiteAudioProcessor& audioProcessor;

    MakoLookAndFeel myLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MakoBiteAudioProcessorEditor)

    juce::Image imgBackground;
    juce::Label labInfo1;
    juce::Label labInfo2;
    juce::Label labInfo3;
    juce::Label labInfo4;
    float SampleRate = 0.0f; //R2.00 Added must be 0.

    //R1.00 Track our Clipping state so we dont redraw the screen every timer tick.
    bool STATE_Clip = false;

    int GUI_Width = 738;  
    int GUI_Height = 255; 

    void GUI_Init_Rotary(juce::Slider* slider, float Val, float Vmin, float Vmax, float Vinterval, juce::String Suffix, int TickStyle, int ThumbColor);
    void GUI_Init_Slider(juce::Slider* slider, float Val, float Vmin, float Vmax, float Vinterval, juce::String Suffix);
    
    //R1.00 Set the EQ filter values. This is mirrored in the PROCESSOR! Any changes need to be done in both places.
    void Band_SetFilterValues(bool ForcePaint);

    //R1.00 Define our UI Juce Slider controls.
    int Knob_Cnt = 0;
    juce::Slider sldKnob[20];
    juce::Slider jsP1_Mono;

    //R1.00 Define the coords and text for our knobs. Not JUCE related. 
    t_KnobInfo Knob_Info[20] = {};    
    void KNOB_DefinePosition(int t, float x, float y, float sizex, float sizey, int datatype, juce::String name, juce::String suffix);
    void KNOB_ShowValue(int t);

    //R2.00 These are the indexes into our Settings var. Must be recreated in Processor. 
    enum { e_Gain, e_NGate, e_Drive, e_Boom, e_EQ, e_EQ1, e_EQ2, e_EQ3, e_EQ4, e_EQ5, e_Amp, e_IR, e_Mono, e_Thump, e_Air, e_Power, e_HighCut, e_LowCut, e_Pedal };

public:
    
    //R1.00 Define our SLIDER attachment variables.
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> ParAtt[20];
    
};
