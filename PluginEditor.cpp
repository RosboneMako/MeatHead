/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MakoBiteAudioProcessorEditor::MakoBiteAudioProcessorEditor (MakoBiteAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{    
    char Mess[40];

    //R1.00 Create SLIDER ATTACHMENTS so our parameter vars get adjusted automatically for Get/Set states.
    ParAtt[e_Gain] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "gain", sldKnob[e_Gain]);
    ParAtt[e_NGate] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "ngate", sldKnob[e_NGate]);
    ParAtt[e_Drive] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "drive", sldKnob[e_Drive]);
    ParAtt[e_Boom] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "boom", sldKnob[e_Boom]);
    ParAtt[e_EQ] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "eq", sldKnob[e_EQ]);
    ParAtt[e_EQ1] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "eq1", sldKnob[e_EQ1]);
    ParAtt[e_EQ2] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "eq2", sldKnob[e_EQ2]);
    ParAtt[e_EQ3] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "eq3", sldKnob[e_EQ3]);
    ParAtt[e_EQ4] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "eq4", sldKnob[e_EQ4]);
    ParAtt[e_EQ5] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "eq5", sldKnob[e_EQ5]);
    ParAtt[e_Amp] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "amp", sldKnob[e_Amp]);
    ParAtt[e_IR] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "ir", sldKnob[e_IR]);
    ParAtt[e_Mono] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "mono", sldKnob[e_Mono]);
    ParAtt[e_Thump] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "thump", sldKnob[e_Thump]);
    ParAtt[e_Air] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "air", sldKnob[e_Air]);
    ParAtt[e_Power] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "power", sldKnob[e_Power]);
    ParAtt[e_HighCut] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "highcut", sldKnob[e_HighCut]);
    ParAtt[e_LowCut] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "lowcut", sldKnob[e_LowCut]);
    ParAtt[e_Pedal] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "pedal", sldKnob[e_Pedal]);
        
    imgBackground = juce::ImageCache::getFromMemory(BinaryData::meatheadback_png, BinaryData::meatheadback_pngSize);

    //R1.00 Start our Timer so we can tell the user they are clipping. Could draw VU Meters here, etc.
    startTimerHz(2);  //R1.00 have our Timer get called twice per second.

    //****************************************************************************************
    //R1.00 Add GUI CONTROLS
    //****************************************************************************************
    GUI_Init_Rotary(&sldKnob[e_Gain], audioProcessor.Setting[e_Gain],0.0f, 1.0f,.01f,"", 1, 0xFFFF8000);    
    GUI_Init_Rotary(&sldKnob[e_Drive], audioProcessor.Setting[e_Drive], 0.0f, 1.0f, .01f, "", 1, 0xFFFF8000);
    GUI_Init_Rotary(&sldKnob[e_Boom], audioProcessor.Setting[e_Boom], 20, 300, 10, "", 2, 0xFFFF8000);
    GUI_Init_Rotary(&sldKnob[e_Power], audioProcessor.Setting[e_Power], 0.0f, 1.0f, .01f, "", 2, 0xFFFF8000);

    GUI_Init_Rotary(&sldKnob[e_EQ1], audioProcessor.Setting[e_EQ1], -12.0f, 12.0f, .1f, "", 2, 0xFFC0C0B0);
    GUI_Init_Rotary(&sldKnob[e_EQ2], audioProcessor.Setting[e_EQ2], -12.0f, 12.0f, .1f, "", 2, 0xFFC0C0B0);
    GUI_Init_Rotary(&sldKnob[e_EQ3], audioProcessor.Setting[e_EQ3], -12.0f, 12.0f, .1f, "", 2, 0xFFC0C0B0);
    GUI_Init_Rotary(&sldKnob[e_EQ4], audioProcessor.Setting[e_EQ4], -12.0f, 12.0f, .1f, "", 2, 0xFFC0C0B0);
    GUI_Init_Rotary(&sldKnob[e_EQ5], audioProcessor.Setting[e_EQ5], -12.0f, 12.0f, .1f, "", 2, 0xFFC0C0B0);

    GUI_Init_Rotary(&sldKnob[e_Thump], audioProcessor.Setting[e_Thump], 0.0f, 1.0f, .01f, "", 2, 0xFFFF8000);
    GUI_Init_Rotary(&sldKnob[e_Air], audioProcessor.Setting[e_Air], 0.0f, 1.0f, .01f, "", 2, 0xFFFF8000);
    GUI_Init_Rotary(&sldKnob[e_HighCut], audioProcessor.Setting[e_HighCut], 2000, 8000, 100, "", 2, 0xFFFF8000);
    GUI_Init_Rotary(&sldKnob[e_LowCut], audioProcessor.Setting[e_LowCut], 20, 200, 10, "", 2, 0xFFFF8000);
            
    GUI_Init_Slider(&sldKnob[e_Amp], audioProcessor.Setting[e_Amp], 0, 5, 1, "");
    GUI_Init_Slider(&sldKnob[e_IR], audioProcessor.Setting[e_IR], 0, 7, 1, "");
    GUI_Init_Slider(&sldKnob[e_Mono], audioProcessor.Setting[e_Mono], 0, 1, 1, "");
    GUI_Init_Slider(&sldKnob[e_EQ], audioProcessor.Setting[e_EQ], 0, 10, 1, "");
    GUI_Init_Slider(&sldKnob[e_Pedal], audioProcessor.Setting[e_Pedal], 0, 10, 1, "");
    GUI_Init_Slider(&sldKnob[e_NGate], audioProcessor.Setting[e_NGate], 0.0f, 1.0f, .01f, "");

    //R1.00 Update the Look and Feel (Global colors) so drop down menu is the correct color. 
    getLookAndFeel().setColour(juce::DocumentWindow::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::DocumentWindow::textColourId, juce::Colour(255, 255, 255));
    getLookAndFeel().setColour(juce::DialogWindow::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(192, 0, 0));
    getLookAndFeel().setColour(juce::TextButton::buttonOnColourId, juce::Colour(192, 0, 0));
    getLookAndFeel().setColour(juce::TextButton::buttonColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::ComboBox::backgroundColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::ListBox::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::Label::backgroundColourId, juce::Colour(32, 32, 32));


    //R2.00 LARGE IMAGE - Define our control positions to make drawing easier.
    KNOB_DefinePosition(e_Gain,    95,  35, 50, 80, 0, "Gain", "");
    KNOB_DefinePosition(e_Drive,  155,  35, 50, 80, 0, "Drive", "");
    KNOB_DefinePosition(e_Boom,  95, 135, 50, 80, 5, "Boom", " Hz");
    KNOB_DefinePosition(e_Power,  155, 135, 50, 80, 0, "Power", "");

    KNOB_DefinePosition(e_EQ1, 215, 85, 60, 80, 0, "150", " dB");
    KNOB_DefinePosition(e_EQ2, 275, 85, 60, 80, 0, "300", " dB");
    KNOB_DefinePosition(e_EQ3, 335, 85, 60, 80, 0, "750", " dB");
    KNOB_DefinePosition(e_EQ4, 395, 85, 60, 80, 0, "1500", " dB");
    KNOB_DefinePosition(e_EQ5, 455, 85, 60, 80, 0, "3000", " dB");

    KNOB_DefinePosition(e_Air,     525,  35, 50, 80, 0, "Air", "");
    KNOB_DefinePosition(e_Thump,   525, 135, 50, 80, 0, "Thump", "");
    KNOB_DefinePosition(e_HighCut, 585,  35, 50, 80, 3, "High Cut", " Hz");
    KNOB_DefinePosition(e_LowCut,  585, 135, 50, 80, 9, "Low Cut", " Hz");
        
    KNOB_DefinePosition(e_Pedal, 208, 177, 60, 20, 8, "Boost", "");
    KNOB_DefinePosition(e_Amp,   293, 177, 60, 20, 6, "Amp", "");
    KNOB_DefinePosition(e_IR,    378, 177, 60, 20, 7, "Speaker", "");
    KNOB_DefinePosition(e_EQ,    464, 177, 60, 20, 1, "EQ Band", "");

    KNOB_DefinePosition(e_NGate, 208, 207, 60, 20, 0, "Gate", "");
    KNOB_DefinePosition(e_Mono,  464, 207, 60, 20, 4, "Mono", "");

    Knob_Cnt = 19;

    //R2.00 Adjustment information label.
    labInfo1.setJustificationType(juce::Justification::centredLeft);
    labInfo1.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF000000));
    labInfo1.setColour(juce::Label::textColourId, juce::Colour(0xFFFF8000));
    labInfo1.setColour(juce::Label::outlineColourId, juce::Colour(0xFF402000));
    addAndMakeVisible(labInfo1);
    
    //R2.00 Clipping label.
    labInfo2.setJustificationType(juce::Justification::centredRight);
    labInfo2.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF000000));
    labInfo2.setColour(juce::Label::textColourId, juce::Colour(0xFFFF8000));
    labInfo2.setColour(juce::Label::outlineColourId, juce::Colour(0xFF402000));
    addAndMakeVisible(labInfo2);

    //R2.00 Sample Rate label.
    labInfo3.setJustificationType(juce::Justification::centredRight);
    labInfo3.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF000000));
    labInfo3.setColour(juce::Label::textColourId, juce::Colour(0xFFFF8000));
    labInfo3.setColour(juce::Label::outlineColourId, juce::Colour(0xFF402000));
    addAndMakeVisible(labInfo3);

    //R2.00 Version label.
    labInfo4.setJustificationType(juce::Justification::centredRight);
    labInfo4.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF000000));
    labInfo4.setColour(juce::Label::textColourId, juce::Colour(0xFFFF8000));
    labInfo4.setColour(juce::Label::outlineColourId, juce::Colour(0xFF402000));
    addAndMakeVisible(labInfo4);
    labInfo4.setText("v2.00", juce::dontSendNotification);
    
    //R1.00 Update our filter text strings.
    Band_SetFilterValues(false);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    //R1.00 Set the window size.
    setSize(GUI_Width, GUI_Height);
}

MakoBiteAudioProcessorEditor::~MakoBiteAudioProcessorEditor()
{
}

//R1.00 This func gets called twice a second to monitor processor.
//R1.00 Could be used to draw VU meters, etc. 
void MakoBiteAudioProcessorEditor::timerCallback()
{
    //R1.00 Check processor if audio is clipping.
    //R1.00 Track the Label stats so we are not redrawing the control twice a second.
    if (audioProcessor.AudioIsClipping)
    {
        audioProcessor.AudioIsClipping = false;
        STATE_Clip = true;
        labInfo2.setText("CLIPPING", juce::sendNotification);
    }
    else
    {
        if (STATE_Clip) labInfo2.setText("", juce::sendNotification);
        STATE_Clip = false;
    }

    //R2.00 Show the current sample rate.
    if (SampleRate != audioProcessor.SampleRate)
    {
        SampleRate = audioProcessor.SampleRate;
        switch (int(SampleRate))
        {
        case 44100: labInfo3.setText("44.1 k", juce::sendNotification); break;
        case 48000: labInfo3.setText("48 k", juce::sendNotification); break;
        case 96000: labInfo3.setText("96 k", juce::sendNotification); break;
        case 192000: labInfo3.setText("192 k", juce::sendNotification); break;
        default: labInfo3.setText("", juce::sendNotification); break;
        }
    }

}

//==============================================================================
void MakoBiteAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    bool UseImage = true;   

    if (UseImage)
    {
        g.drawImageAt(imgBackground, 0, 0);        
        
        //R1.00 Use to get control positions when making an image background.
        //g.setColour(juce::Colours::white);
        //g.fillRect(0, 0, GUI_Width, GUI_Height);
    }
    else
    {        
        //R2.00 Draw our GUI.
        //R2.00 Background.
        juce::ColourGradient ColGrad;
        //ColGrad = juce::ColourGradient(juce::Colour(0xFF202020), 0.0f, 0.0f, juce::Colour(0xFF808080), 0.0f, GUI_Height, false);
        //g.setGradientFill(ColGrad);
        g.setColour(juce::Colour::Colour(0xFF404040));
        g.fillRect(0, 0, GUI_Width, GUI_Height);

        //R2.00 Draw LOGO text.
        g.setColour(juce::Colours::black);
        g.fillRect(244, 32, 240, 40);
        g.setFont(18.0f);
        g.setColour(juce::Colours::white);
        g.drawFittedText("MEATHEAD", 244, 32, 240, 18, juce::Justification::centred, 1);
        g.setFont(14.0f);  

        //R2.00 Draw amp head outline.
        g.setColour(juce::Colour::Colour(0xFF000000));
        g.drawRoundedRectangle(15.0f, 5.0f, 707.0f, 245.0f, 4.0f, 2.0f);
    }
    
    //R1.00 DRAW TEXT.
    //R1.00 Most of these could be done on the image to speed up painting.
    //R1.00 But the EQ frequencies need to update. So we are doing all the text
    //R1.00 here so the font matches no matter what in the future.
    g.setFont(12.0f);
    g.setColour(juce::Colours::orange);
    for (int t = 0; t < Knob_Cnt; t++)
    {
       g.drawFittedText(Knob_Info[t].Name, Knob_Info[t].x, Knob_Info[t].y - 10, Knob_Info[t].sizex, 15, juce::Justification::centred, 1);
    }
    
}

void MakoBiteAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    //R1.00 Use RETURN here to have a blank window drawn when creating a GUI image.
    //R1.00 The controls are only visible if their BOUNDS are defined.
    //return;

    //R1.00 Draw all of the defined KNOBS.
    for (int t = 0; t < Knob_Cnt; t++) sldKnob[t].setBounds(Knob_Info[t].x, Knob_Info[t].y, Knob_Info[t].sizex, Knob_Info[t].sizey);

    //R2.00 Information labels. 
    labInfo1.setBounds(248, 55, 80, 17);
    labInfo4.setBounds(248, 33, 50, 17);
    labInfo2.setBounds(403, 55, 80, 17);
    labInfo3.setBounds(433, 33, 50, 17);
}


//R2.00 Setup the KNOB control edit values, Text Suffix (if any), UI tick marks, and Indicator Color.
void MakoBiteAudioProcessorEditor::GUI_Init_Rotary(juce::Slider* slider, float Val, float Vmin, float Vmax, float Vinterval, juce::String Suffix, int TickStyle, int ThumbColor)
{
    //R1.00 Setup the slider edit parameters.
    slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    slider->setTextValueSuffix(Suffix);
    slider->setRange(Vmin, Vmax, Vinterval);
    slider->setValue(Val);
    slider->addListener(this);
    addAndMakeVisible(slider);

    //R1.00 Override the default Juce drawing routines and use ours.
    slider->setLookAndFeel(&myLookAndFeel);

    //R1.00 Setup the type and colors for the sliders.
    slider->setSliderStyle(juce::Slider::SliderStyle::Rotary);
    slider->setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFC08000));
    slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::textBoxHighlightColourId, juce::Colour(0xFF804000));
    slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0x00000000));    //R1.00 Make this SEE THRU. Alpha=0.
    slider->setColour(juce::Slider::thumbColourId, juce::Colour(ThumbColor));

    //R1.00 Cheat: We are using this color as a Tick Mark style selector in our drawing function.
    slider->setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(TickStyle));
}

//R2.00 Setup the SLIDER control edit values, Text Suffix (if any), UI tick marks, and Indicator Color.
void MakoBiteAudioProcessorEditor::GUI_Init_Slider(juce::Slider* slider, float Val, float Vmin, float Vmax, float Vinterval, juce::String Suffix)
{
    //R1.00 Setup the slider edit parameters.
    slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 60, 40);
    slider->setRange(Vmin, Vmax, Vinterval);
    slider->setTextValueSuffix(Suffix);
    slider->setValue(Val);
    slider->addListener(this);
    addAndMakeVisible(slider);

    //R1.00 Setup the type and colors for the sliders.
    slider->setSliderStyle(juce::Slider::LinearHorizontal);
    slider->setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFA0A0A0));
    slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF202020));
    slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::textBoxHighlightColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::trackColourId, juce::Colour(0xFF603000));
    slider->setColour(juce::Slider::backgroundColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::thumbColourId, juce::Colour(0xFFC0C0B0)); //E02020
}

//R2.00 Function to make declarations easier.
void MakoBiteAudioProcessorEditor::KNOB_DefinePosition(int idx,float x, float y, float sizex, float sizey, int datatype, juce::String name, juce::String suffix)
{
    Knob_Info[idx].x = x;
    Knob_Info[idx].y = y;
    Knob_Info[idx].sizex = sizex;
    Knob_Info[idx].sizey = sizey;
    Knob_Info[idx].DataType = datatype;
    Knob_Info[idx].Suffix = suffix;
    Knob_Info[idx].Name = name;
}

//R1.00 Function to format our SLIDER/KNOB settings to print on the UI (labInfo1).
void MakoBiteAudioProcessorEditor::KNOB_ShowValue(int t)
{
    char Mess[40];
    juce::String Mess2;
    
    switch (Knob_Info[t].DataType)
    {
      case 0: //R1.00 Generic FLOAT value.
        sprintf(Mess, "%1.2f", audioProcessor.Setting[t]);
        labInfo1.setText(Mess + Knob_Info[t].Suffix, juce::dontSendNotification);
        break;

      case 1: //R1.00 Generic INT value.
         Mess2 = std::to_string(int(audioProcessor.Setting[t]));
         labInfo1.setText(Mess2 + Knob_Info[t].Suffix, juce::sendNotification);
         break;

      case 2:  //R1.00 Print Q val with 2 decimal places.
        sprintf(Mess, "%1.2f", audioProcessor.Setting[t]);
        labInfo1.setText(Mess + Knob_Info[t].Suffix, juce::dontSendNotification);
        break;
    
      case 3: //R1.00 Special formatting for HIGH CUT. 
          if (audioProcessor.Setting[t] < 8000.0f)
          {
              Mess2 = std::to_string(int(audioProcessor.Setting[t]));
              labInfo1.setText(Mess2 + Knob_Info[t].Suffix, juce::sendNotification);
          }
          else
              labInfo1.setText("HC off", juce::sendNotification);
          break;
          
      case 4: //R1.00 Special formatting for Stereo / Mono.
          if (audioProcessor.Setting[t] < 1.0f)
              labInfo1.setText("Stereo", juce::sendNotification);
          else
              labInfo1.setText("Mono", juce::sendNotification);
          break;

      case 5: //R2.00 Special formatting for Low CUT. 
          if (20.0f < audioProcessor.Setting[t])
          {
              Mess2 = std::to_string(int(audioProcessor.Setting[t]));
              labInfo1.setText(Mess2 + Knob_Info[t].Suffix, juce::sendNotification);
          }
          else
              labInfo1.setText("Boom off", juce::sendNotification);
          break;
      case 6: //R2.00 Show the current amplifier selection.
          switch (int(audioProcessor.Setting[t]))
          {
             default: labInfo1.setText("0 Amp off", juce::sendNotification); break;
             case 1:labInfo1.setText("1 Bogo", juce::sendNotification); break;
             case 2:labInfo1.setText("2 Revell", juce::sendNotification); break;
             case 3:labInfo1.setText("3 Cali4nia", juce::sendNotification); break;
             case 4:labInfo1.setText("4 515 III", juce::sendNotification); break;
             case 5:labInfo1.setText("5 J800", juce::sendNotification); break;
          }
          break;
      case 7: //R2.00 Show the current amplifier selection.
          switch (int(audioProcessor.Setting[t]))
          {
          default: labInfo1.setText("0 Cab off", juce::sendNotification); break;
          case 1:labInfo1.setText("1 Bright", juce::sendNotification); break;
          case 2:labInfo1.setText("2 Normal", juce::sendNotification); break;
          case 3:labInfo1.setText("3 Tight", juce::sendNotification); break;
          case 4:labInfo1.setText("4 Beefy", juce::sendNotification); break;
          case 5:labInfo1.setText("5 Combo", juce::sendNotification); break;
          case 6:labInfo1.setText("6 Juiced", juce::sendNotification); break;
          case 7:labInfo1.setText("7 4x12", juce::sendNotification); break;
          }
          break;
      case 8: //R2.00 Show the current Boost selection.
          switch (int(audioProcessor.Setting[t]))
          {
          default: labInfo1.setText("Boost off", juce::sendNotification); break;
          case 1:labInfo1.setText("450 Hz", juce::sendNotification); break;
          case 2:labInfo1.setText("700 Hz", juce::sendNotification); break;
          case 3:labInfo1.setText("900 Hz", juce::sendNotification); break;
          case 4:labInfo1.setText("1200 Hz", juce::sendNotification); break;
          case 5:labInfo1.setText("1500 Hz", juce::sendNotification); break;
          case 6:labInfo1.setText("2500 Hz", juce::sendNotification); break;
          case 7:labInfo1.setText("700 HiQ", juce::sendNotification); break;
          case 8:labInfo1.setText("900 HiQ", juce::sendNotification); break;
          case 9:labInfo1.setText("1.5k Cut", juce::sendNotification); break;
          case 10:labInfo1.setText("High Cut", juce::sendNotification); break;
          }
          break;
      case 9: //R1.00 Special formatting for LOW CUT. 
          if (20.0f < audioProcessor.Setting[t])
          {
              Mess2 = std::to_string(int(audioProcessor.Setting[t]));
              labInfo1.setText(Mess2 + Knob_Info[t].Suffix, juce::sendNotification);
          }
          else
              labInfo1.setText("LC off", juce::sendNotification);
          break;
    }
}

//R1.00 Override SLIDER control to capture setting changes.
void MakoBiteAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{       
    //R1.00 Catch the EQ BAND change here so we can update the UI and frequencies.
    if (slider == &sldKnob[e_EQ])
    {   
        audioProcessor.Setting[e_EQ] = float(sldKnob[e_EQ].getValue());
        Band_SetFilterValues(true);
        audioProcessor.SettingsChanged += 1;        
        KNOB_ShowValue(e_EQ);
        return; 
    }
       
    //R1.00 When a slider is adjusted, this func gets called. Capture the new edits
    //R1.00  and flag the processor when it needs to recalc things.
    //R1.00 Check which slider has been adjusted.
    for (int t = 0; t < Knob_Cnt; t++)
    {
        if (slider == &sldKnob[t])
        {   
            //R1.00 Update the actual processor variable being edited.
            audioProcessor.Setting[t] = float(sldKnob[t].getValue());

            //R1.00 We need to update settings in processor.
            //R1.00 Increment changed var to be sure every change gets made. Changed var is decremented in processor.
            audioProcessor.SettingsChanged += 1;

            //R1.00 Print the value to the UI.
            KNOB_ShowValue(t);

            //R1.00 We have captured the correct slider change, exit this function.
           return;
        }
    }
    
    return;
}

//R1.00 Define the user selected EQ mode.
//R1.00 These values must match the twin function in the PROCESSOR.
void MakoBiteAudioProcessorEditor::Band_SetFilterValues(bool ForcePaint)
{
    int EQVals[6] = {};
    int EQ_Mode = int(audioProcessor.Setting[e_EQ]);
        
    switch (EQ_Mode)
    {
    default:
    {
        EQVals[1] = 150;
        EQVals[2] = 300;
        EQVals[3] = 750;
        EQVals[4] = 1500;
        EQVals[5] = 3000;        
        break;
    }
    case 1:
    {
        EQVals[1] = 150;
        EQVals[2] = 450;
        EQVals[3] = 900;
        EQVals[4] = 1800;
        EQVals[5] = 3500;
        break;
    }
    case 2:
    {
        EQVals[1] = 80;
        EQVals[2] = 220;
        EQVals[3] = 750;
        EQVals[4] = 2200;
        EQVals[5] = 6000;
        break;
    }
    case 3:
    {
        EQVals[1] = 80;
        EQVals[2] = 350;
        EQVals[3] = 900;
        EQVals[4] = 1500;
        EQVals[5] = 3000;
        break;
    }
    case 4:
    {
        EQVals[1] = 100;
        EQVals[2] = 400;
        EQVals[3] = 800;
        EQVals[4] = 1600;
        EQVals[5] = 3200;
        break;
    }
    case 5:
    {
        EQVals[1] = 120;
        EQVals[2] = 330;
        EQVals[3] = 660;
        EQVals[4] = 1320;
        EQVals[5] = 2500;
        break;
    }
    case 6:
    {
        EQVals[1] = 150;
        EQVals[2] = 500;
        EQVals[3] = 900;
        EQVals[4] = 1800;
        EQVals[5] = 5000;
        break;
    }
    case 7:
    {
        EQVals[1] = 80;
        EQVals[2] = 300;
        EQVals[3] = 650;
        EQVals[4] = 1500;
        EQVals[5] = 4500;        
        break;
    }
    case 8:
    {
        EQVals[1] = 100;
        EQVals[2] = 400;
        EQVals[3] = 800;
        EQVals[4] = 1600;
        EQVals[5] = 4000;
        break;
    }
    case 9:
    {
        EQVals[1] = 200;
        EQVals[2] = 600;
        EQVals[3] = 1000;
        EQVals[4] = 2000;
        EQVals[5] = 5000;
        break;
    }
    case 10:
    {
        EQVals[1] = 80;
        EQVals[2] = 250;
        EQVals[3] = 900;
        EQVals[4] = 1800;
        EQVals[5] = 3600;
        break;
    }

    }

    //R1.00 Create the UI label strings.    
    Knob_Info[e_EQ1].Name = std::to_string(EQVals[1]);
    Knob_Info[e_EQ2].Name = std::to_string(EQVals[2]);
    Knob_Info[e_EQ3].Name = std::to_string(EQVals[3]);
    Knob_Info[e_EQ4].Name = std::to_string(EQVals[4]);
    Knob_Info[e_EQ5].Name = std::to_string(EQVals[5]);

    //R1.01 We changed some stuff so refresh the screen/UI.
    if (ForcePaint) repaint();

}
