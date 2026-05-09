/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "cmath"              //R1.00 Added library.

//==============================================================================
MakoBiteAudioProcessor::MakoBiteAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
    ),
    
    //R1.00 Define our VALUE TREE parameter variables. Min val, Max Val, default Val.
    parameters(*this, nullptr, "PARAMETERS", 
      {
        std::make_unique<juce::AudioParameterFloat>("gain","Gain", .0f, 1.0f, .15f),
        std::make_unique<juce::AudioParameterFloat>("ngate","Noise Gate", .0f, 1.0f, .0f),
        std::make_unique<juce::AudioParameterFloat>("drive","Drive", .0f, 1.0f, .2f),
        std::make_unique<juce::AudioParameterInt>("boom","Boom", 20, 300, 150),
        std::make_unique<juce::AudioParameterInt>("eq","EQ Band", 0, 10, 0),
        std::make_unique<juce::AudioParameterFloat>("eq1","EQ 1", -12.0f, 12.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("eq2","EQ 2", -12.0f, 12.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("eq3","EQ 3", -12.0f, 12.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("eq4","EQ 4", -12.0f, 12.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("eq5","EQ 5", -12.0f, 12.0f, 0.0f),
        std::make_unique<juce::AudioParameterInt>("amp","AMP Model", 0, 5, 1),
        std::make_unique<juce::AudioParameterInt>("ir","IR Model", 0, 7, 6),
        std::make_unique<juce::AudioParameterInt>("iron","IR On", 0, 1, 1),
        std::make_unique<juce::AudioParameterInt>("mono","Mono", 0, 1, 1),
        std::make_unique<juce::AudioParameterInt>("pedal","Pedal", 0, 10, 0),
        std::make_unique<juce::AudioParameterFloat>("thump","Thump", .0f, 1.0f, 0.2f),
        std::make_unique<juce::AudioParameterFloat>("air","Air", .0f, 1.0f, 0.2f),
        std::make_unique<juce::AudioParameterFloat>("power","Power", .0f, 1.0f, 0.4f),
        std::make_unique<juce::AudioParameterInt>("highcut","High Cut", 2000, 8000, 8000),
        std::make_unique<juce::AudioParameterInt>("lowcut","Low Cut", 20, 200, 20),
      }
    )   

#endif
{   
}

MakoBiteAudioProcessor::~MakoBiteAudioProcessor()
{
}

//==============================================================================
const juce::String MakoBiteAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MakoBiteAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MakoBiteAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MakoBiteAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MakoBiteAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MakoBiteAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MakoBiteAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MakoBiteAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MakoBiteAudioProcessor::getProgramName (int index)
{
    return {};
}

void MakoBiteAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MakoBiteAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    //R2.00 Get our Sample Rate for filter calculations.
    SampleRate = MakoBiteAudioProcessor::getSampleRate();
    if (SampleRate < 21000) SampleRate = 48000.0f;
    if (192000 < SampleRate) SampleRate = 48000.0f;

    //R2.00 PreCalc some RATIO values for Signal Averaging based on Sample Rate.
    AVG_Rate_High = .995f;
    AVG_Rate_Low = .005f;
    if (48001.0f < SampleRate)
    {
        AVG_Rate_High = .9975f;
        AVG_Rate_Low = .0025f;
    }
    if (96001.0f < SampleRate)
    {
        AVG_Rate_High = .99875f;
        AVG_Rate_Low = .00125f;
    }
        
    //R2.00 Calc our filters.
    Filter_LP4x_Coeffs(15000.0f, &makoF_4xFilter);    //R2.00 4x Anti-Aliasing filter.Low Pass. 
    Filter_FO_HP_Coeffs(50.0f, &makoF_Boom);          //R2.00 Increase this if bass is too mushy. 180,200, etc
    Filter_LP_Coeffs(8000.0f, &makoF_HighCut);        //R2.00 Reduce fizz from excessive harmonics.
    Filter_HP_Coeffs(20.0f, &makoF_LowCut);           //R2.00 Reduce fizz from excessive harmonics.
    Filter_LP_Coeffs(150.0f, &makoF_Thump);
    Filter_HP_Coeffs(150.0f, &makoF_Air);
    Filter_BP_Coeffs(9.0f, 1800.0f, .35f, &makoF_HighBoost);
    Filter_BP_Coeffs(6.0f, 800.0f, .35f, &makoF_MidBoost);
    Mako_Settings_Update(true);

    //R2.00 Create our initial Amp IR and set a volume factor for each stored IR.  
    Amp_VolAdjustVals[1] = .28f;
    Amp_VolAdjustVals[2] = .25f;
    Amp_VolAdjustVals[3] = .25f;
    Amp_VolAdjustVals[4] = .22f;
    Amp_VolAdjustVals[5] = .30f;
    Mako_Amp_Set();

    //R2.00 Create our initial Speaker IR and set a volume factor for each stored IR.  
    IR_VolAdjustVals[1] = .28f;
    IR_VolAdjustVals[2] = .25f;
    IR_VolAdjustVals[3] = .25f;
    IR_VolAdjustVals[4] = .22f;
    IR_VolAdjustVals[5] = .25f;
    IR_VolAdjustVals[6] = .25f;
    IR_VolAdjustVals[7] = .24f;
    Mako_IR_Set();
}

void MakoBiteAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MakoBiteAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//R2.00 This is the JUCE created function that handles audio processing.
void MakoBiteAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    //R2.00 Our defined variables.
    float tS;       //R2.00 Temporary working sample.
    float tS_Temp;  //R2.00 Sample that gets modified then added back to tS.

    //R1.00 Handle any changes to our Paramters.
    //R1.00 Handle any settings changes made in Editor. 
    if (0 < SettingsChanged) Mako_Settings_Update(false);

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        //R1.00 Process the AUDIO buffer data.
        if (Pedal_Mono && (channel == 1))
        {
            auto* channel0Data = buffer.getWritePointer(0);

            //R1.00 FORCE MONO - Put CHANNEL 0 data in CHANNEL 1.
            for (int samp = 0; samp < buffer.getNumSamples(); samp++) channelData[samp] = channel0Data[samp];
        }
        else
        {
            // ..do something to the data...
            for (int samp = 0; samp < buffer.getNumSamples(); samp++)
            {
                //R2.00 Get the current sample from the audio buffer and put it in tS. 
                tS = buffer.getSample(channel, samp);

                //R2.00 High Pass / Low Cut. May be better to remove some bass before the Gate.
                if (20.0f < Setting[e_Boom]) tS = Filter_Calc_BiQuad(tS, channel, &makoF_Boom);

                //R2.00 Noise gate.
                if (0.0f < Setting[e_NGate]) tS = Mako_FX_NoiseGate(tS, channel);
                
                //R1.00 Apply our Amplifier Simulation to the sample. 0 = Off.
                tS = Mako_FX_AmpSim(tS, channel);

                //R1.00 Impulse Response (IR). 0 = Off.
                if (0.0f < Setting[e_IR]) tS = Mako_CabSim(tS, channel);

                //R2.00 Air effect. 
                if (0.0f < Setting[e_Air])
                {
                    tS_Temp = Filter_Calc_BiQuad(tS, channel, &makoF_Air);
                    tS = tS + tanhf(tS_Temp * Setting[e_Air] * 4.0f);
                }

                //R2.00 Thump effect. 
                if (0.0f < Setting[e_Thump])
                {
                    tS_Temp = Filter_Calc_BiQuad(tS, channel, &makoF_Thump);
                    tS = tS + tanhf(tS_Temp * Setting[e_Thump] * 4.0f);
                }

                //R2.00 Apply EQ. Dont calc unless needed to save CPU cycles.    
                //R2.00 This will cause glitches if automating since buffers will not fill while at 0 (off).
                if (Setting[e_EQ1] != .0f) tS = Filter_Calc_BiQuad(tS, channel, &makoF_Band1);
                if (Setting[e_EQ2] != .0f) tS = Filter_Calc_BiQuad(tS, channel, &makoF_Band2);
                if (Setting[e_EQ3] != .0f) tS = Filter_Calc_BiQuad(tS, channel, &makoF_Band3);
                if (Setting[e_EQ4] != .0f) tS = Filter_Calc_BiQuad(tS, channel, &makoF_Band4);
                if (Setting[e_EQ5] != .0f) tS = Filter_Calc_BiQuad(tS, channel, &makoF_Band5);

                //R2.00 Power. 
                if (0.0f < Setting[e_Power]) tS = tS + tanhf(tS * Setting[e_Power] * 10.0f);

                //R2.00 LP - Fizz filter. Do this last to remove any additional aliasing picked up from Thump, Air, Power.
                if (20.0f < Setting[e_LowCut]) tS = Filter_Calc_BiQuad(tS, channel, &makoF_LowCut);
                if (Setting[e_HighCut] < 8000.0f) tS = Filter_Calc_BiQuad(tS, channel, &makoF_HighCut);

                //R2.00 See if we are clipping and flag the editor to show UI notice.
                Mako_CheckForClipping(tS);

                //R2.00 Write our modified sample back into the sample buffer.
                channelData[samp] = tS;
            }
        }
    }
}

//==============================================================================
bool MakoBiteAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MakoBiteAudioProcessor::createEditor()
{
    return new MakoBiteAudioProcessorEditor (*this);
}


//==============================================================================
void MakoBiteAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    //R1.00 Save our parameters to file/DAW.
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
   
    return;
}

void MakoBiteAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    //R1.00 Read our parameters from file/DAW.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));

    
    //R1.00 Force our variables to get updated.
    Setting[e_Gain] = makoGetParmValue_float("gain");
    Setting[e_NGate] = makoGetParmValue_float("ngate");
    Setting[e_Drive] = makoGetParmValue_float("drive");
    Setting[e_Boom] = makoGetParmValue_float("boom");
    Setting[e_EQ] = makoGetParmValue_float("eq");
    Setting[e_EQ1] = makoGetParmValue_float("eq1");
    Setting[e_EQ2] = makoGetParmValue_float("eq2");
    Setting[e_EQ3] = makoGetParmValue_float("eq3");
    Setting[e_EQ4] = makoGetParmValue_float("eq4");
    Setting[e_EQ5] = makoGetParmValue_float("eq5");
    Setting[e_Amp] = makoGetParmValue_float("amp");
    Setting[e_IR] = makoGetParmValue_float("ir");
    Setting[e_Mono] = makoGetParmValue_float("mono");    
    Setting[e_Thump] = makoGetParmValue_float("thump");
    Setting[e_Air] = makoGetParmValue_float("air");
    Setting[e_Power] = makoGetParmValue_float("power");
    Setting[e_HighCut] = makoGetParmValue_float("highcut");
    Setting[e_LowCut] = makoGetParmValue_float("lowcut");
    Setting[e_Pedal] = makoGetParmValue_float("pedal");
    
    return;
}


//R1.00 Parameter reading helper function.
int MakoBiteAudioProcessor::makoGetParmValue_int(juce::String Pstring)
{
    auto parm = parameters.getRawParameterValue(Pstring);
    if (parm != NULL)
        return int(parm->load());
    else
        return 0;
}

//R1.00 Parameter reading helper function.
float MakoBiteAudioProcessor::makoGetParmValue_float(juce::String Pstring)
{
    auto parm = parameters.getRawParameterValue(Pstring);
    if (parm != NULL)
        return float(parm->load());
    else
        return 0.0f;
}

//R1.00 Volume envelope based on average Signal volume.
float MakoBiteAudioProcessor::Mako_FX_NoiseGate(float tSample, int channel)
{
    float tS = tSample;
    //tS = (tS * .2f) + (NG_Last[channel] * .8f);

    //R1.00 Track our Input Signal Average (Absolute vals).
    Signal_AVG[channel] = (Signal_AVG[channel] * AVG_Rate_High) + (abs(tS) * AVG_Rate_Low);
    
    //R1.00 Create a volume envelope based on Signal Average.
    Pedal_NGate_Fac[channel] = Signal_AVG[channel] * 20000.0f * (1.1f - Setting[e_NGate]);

    //R1.00 Dont amplify the sound, just reduce when necessary.
    if (1.0f < Pedal_NGate_Fac[channel]) Pedal_NGate_Fac[channel] = 1.0f;

    return tSample * Pedal_NGate_Fac[channel];

    //float tLim = Setting[e_NGate] * .01f;
    //float tSA = abs(tSample);
    //float Gain = 1.0f;
    //if (tSA < tLim) Gain = (tSA / tLim);
    //return tSample * Gain;

    //R2.00 Expand gate.
    //float tLim = Setting[e_NGate];// *.01f;
    //float tS = tSample;
    //float Gain = 1.0f;
    //Gain = (1.0f - tLim) + (tS * tS * 100.0f * tLim);
    //return tSample * Gain;

    //2.00 Peak/Time based Gate.
    //float tLim = Setting[e_NGate] *.01f;
    //float tS = tSample;
    //float Gain = 0.0f;    
    //Signal_AVG[channel] = Signal_AVG[channel] * .99f;
    //if (tLim < abs(tS)) Signal_AVG[channel] = 1.0f;
    //return tSample * Signal_AVG[channel];

}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MakoBiteAudioProcessor();
}

//R1.00 Apply filter to a sample.
float MakoBiteAudioProcessor::Filter_Calc_BiQuad(float tSample, int channel, tp_filter* fn)
{
    float tS = tSample;

    fn->xn0[channel] = tS;
    tS = fn->a0 * fn->xn0[channel] + fn->a1 * fn->xn1[channel] + fn->a2 * fn->xn2[channel] - fn->b1 * fn->yn1[channel] - fn->b2 * fn->yn2[channel];
    fn->xn2[channel] = fn->xn1[channel]; fn->xn1[channel] = fn->xn0[channel]; fn->yn2[channel] = fn->yn1[channel]; fn->yn1[channel] = tS;

    return tS;
}

//R1.00 Second order parametric/peaking boost filter with constant-Q
void MakoBiteAudioProcessor::Filter_BP_Coeffs(float Gain_dB, float Fc, float Q, tp_filter* fn)
{    
    float K = pi2 * (Fc * .5f) / SampleRate;
    float K2 = K * K;
    float V0 = pow(10.0, Gain_dB / 20.0);

    float a = 1.0f + (V0 * K) / Q + K2;
    float b = 2.0f * (K2 - 1.0f);
    float g = 1.0f - (V0 * K) / Q + K2;
    float d = 1.0f - K / Q + K2;
    float dd = 1.0f / (1.0f + K / Q + K2);

    fn->a0 = a * dd;
    fn->a1 = b * dd;
    fn->a2 = g * dd;
    fn->b1 = b * dd;
    fn->b2 = d * dd;
    fn->c0 = 1.0f;
    fn->d0 = 0.0f;
}

//R2.00 Anti-Aliasing low pass filter. 
void MakoBiteAudioProcessor::Filter_LP4x_Coeffs(float fc, tp_filter* fn)
{
    float c = 1.0f / (tanf(pi * fc / (SampleRate * 4.0f)));
    fn->a0 = 1.0f / (1.0f + sqrt2 * c + (c * c));
    fn->a1 = 2.0f * fn->a0;
    fn->a2 = fn->a0;
    fn->b1 = 2.0f * fn->a0 * (1.0f - (c * c));
    fn->b2 = fn->a0 * (1.0f - sqrt2 * c + (c * c));
}


//R1.00 Second order LOW PASS filter. 
void MakoBiteAudioProcessor::Filter_LP_Coeffs(float fc, tp_filter* fn)
{    
    float c = 1.0f / (tanf(pi * fc / SampleRate));
    fn->a0 = 1.0f / (1.0f + sqrt2 * c + (c * c));
    fn->a1 = 2.0f * fn->a0;
    fn->a2 = fn->a0;
    fn->b1 = 2.0f * fn->a0 * (1.0f - (c * c));
    fn->b2 = fn->a0 * (1.0f - sqrt2 * c + (c * c));
}

//F1.00 Second order butterworth High Pass.
void MakoBiteAudioProcessor::Filter_HP_Coeffs(float fc, tp_filter* fn)
{    
    float c = tanf(pi * fc / SampleRate);
    fn->a0 = 1.0f / (1.0f + sqrt2 * c + (c * c));
    fn->a1 = -2.0f * fn->a0;
    fn->a2 = fn->a0;
    fn->b1 = 2.0f * fn->a0 * ((c * c) - 1.0f);
    fn->b2 = fn->a0 * (1.0f - sqrt2 * c + (c * c));
}

//R4.00 First Order High Pass.
void MakoBiteAudioProcessor::Filter_FO_HP_Coeffs(float fc, tp_filter* fn)
{
    float th = 2.0 * pi * fc / SampleRate;
    float g = cos(th) / (1.0 + sin(th));
    fn->a0 = (1.0 + g) / 2.0;
    fn->a1 = -((1.0 + g) / 2.0);
    fn->a2 = 0.0f;
    fn->b1 = -g;
    fn->b2 = 0.0f;
    fn->c0 = 0.0f;
    fn->d0 = 0.0f;
}


float MakoBiteAudioProcessor::Mako_FX_AmpSim(float tSample, int channel)
{   //R1.01 Set default values.
    float tS = tSample;
    float tS2 = 0.0f;
    float tS_Thump = 0.0f;
    float tS_Air = 0.0f;
    float tS_Peak = 0.0f;
    float tComp = 0.0f;
    float tS_Enh = 0.0f;

    //R1.00 Add some rectified signal.
    //tS = tS + (.05f * abs(tS));

    //R2.00 Pedal - Mid boost.
    if (0 < int(Setting[e_Pedal])) tS = Filter_Calc_BiQuad(tS, channel, &makoF_MidBoost);

    //2.00 Apply the AMP IR, gain, and clipping.    
    if (0 < int(Setting[e_Amp])) tS = Mako_Amp_InputIR_4x(tS, channel);
    
    //R1.00 Volume/Gain adjust. Reduction only.
    tS *= Setting[e_Gain] * .5f;

    return tS;
}

//R2.00 4x OVER SAMPLING version of amplifier, reduces aliasing.
//R2.00 The heart of MeatHead. This func applies an IR to incoming samples that represents
//R2.00 the amplifier preamp EQ.
float MakoBiteAudioProcessor::Mako_Amp_InputIR_4x(float tSample, int channel)
{
    int Index;
    int Amp_Qual_Max = 1024; //R2.00 Could only use 128 samples as most Amp IRs are not very big.
    int BitMask = Amp_Qual_Max - 1;
    float V = 0.0f;
    float tS = tSample;
    float tS_Temp;
    float tS_PostIR;    

    //R2.00 Get our current buffer index and store the new sample in our buffer.
    Index = InEQB_Ring1[channel];
    InEQ_AudioBuffer[channel][Index] = tS;

    //R2.0 Apply input EQ IR to the incoming sample.
    for (int t = 0; t < Amp_Qual_Max; t++)
    {
        V += (InEQ_AudioBuffer[channel][Index] * Amp_Final[t]);

        //R2.00 Increment index, mask off bits past Amp_Qual_Max to keep our index between 0-Amp_Qual_Max.
        Index = (Index + 1) & BitMask;
    }
    tS_PostIR = V; //R2.00 Store for LastSample.

    //R2.00 Decrement our ring buffer index and loop around at 0. 
    InEQB_Ring1[channel]--;
    if (InEQB_Ring1[channel] < 0) InEQB_Ring1[channel] = BitMask;

    //R2.00 Apply Drive.
    if (0.0f < Setting[e_Drive])
    {        
        float AG = Setting[e_Drive] * Setting[e_Drive]; //R2.00 Square Drive to get more range from the knob.
        float tvDelta = (V - AmpSample_Last) / 4.0f;    //R2.00 Calc interpolation value.

        //R2.00 4x Oversample. Interpolate beteen the current sample and last samples.
        for (int SampX = 0; SampX < 4; SampX++)
        {
            //R2.00 Need to interpolate between the samples here!!!
            V = AmpSample_Last + (tvDelta * SampX);

            //R2.00 Apply Drive/Gain.
            V = V * (1.0f + (AG * 1000.0f));

            //R2.00 Apply angled clipping curve.
            //R2.00 Could hard clip here for speed and crispier sound.
            tS_Temp = V;                                         //R2.00 Track Pos/Neg.
            V = abs(V);                                          //R2.00 Get positive value for math to work.
            if (0.75f < V) V = 0.75f + (V - 0.75f) * 0.35f;      //R2.00 .75 is clip start amplitude, .35 is the slope.
            if (.9999f < V) V = .9999f;                          //R2.00 Hard clip at max value.
            if (tS_Temp < 0.0f) V = -V;                          //R2.00 Restore polarity.

            //R2.00 4x Up Sampling filter.
            V = Filter_Calc_BiQuad(V, channel, &makoF_4xFilter); //R2.00 Remove highs to reduce aliasing.
        }
    }

    //R2.00 Store last value for next sample interpolation start.
    AmpSample_Last = tS_PostIR;

    //2.00 Reduce gain since we could be at MAX volume (-1,1).
    return (V * .2f);
}

void MakoBiteAudioProcessor::Mako_Amp_Set()
{
    int Amp_Model = int(Setting[e_Amp]);

    //R1.00 Put one of the preset IRs into the actual IR used for processing.
    //R1.00 We could just use an index into the current array, but this lets us do
    //R1.00 other modifications to the IRs as well. Like resampling, etc.
    switch (Amp_Model)
    {
    case 0: for (int t = 0; t < 1024; t++) Amp_Final[t] = 0.0f; Amp_Final[0] = 1.0f; break;
    case 1: for (int t = 0; t < 1024; t++) Amp_Final[t] = Amp_Stored_01[t]; break;
    case 2: for (int t = 0; t < 1024; t++) Amp_Final[t] = Amp_Stored_02[t]; break;
    case 3: for (int t = 0; t < 1024; t++) Amp_Final[t] = Amp_Stored_03[t]; break;
    case 4: for (int t = 0; t < 1024; t++) Amp_Final[t] = Amp_Stored_04[t]; break;
    default:for (int t = 0; t < 1024; t++) Amp_Final[t] = Amp_Stored_05[t]; break;
    }

    //R1.00 These volumes are estimated values defined in Prepare to play.
    //R1.00 Could do complicated math to get better values. Close enough for us.
    Amp_Final_VolAdjust = Amp_VolAdjustVals[Amp_Model];

    return;
}


//R1.00 We do changes here so we know the vars are not in use while we change them.
//R1.00 EDITOR sets SETTING flags and we make changes here.
void MakoBiteAudioProcessor::Mako_Settings_Update(bool ForceAll)
{    
    bool Force = ForceAll;

    //R2.00 BOOM - Low Cut to reduce bass entering amp section.
    Filter_FO_HP_Coeffs(Setting[e_Boom], &makoF_Boom);

    //R2.00 High Cut. 
    Filter_LP_Coeffs(Setting[e_HighCut], &makoF_HighCut);       

    //R2.00 Low Cut. 
    Filter_HP_Coeffs(Setting[e_LowCut], &makoF_LowCut);

    //R2.00 4x Anti-Aliasing filter.Low Pass. 
    Filter_LP4x_Coeffs(15000.0f, &makoF_4xFilter);

    //R2.00 Update our EQ Filters.
    Band_SetFilterValues();
    Filter_BP_Coeffs(Setting[e_EQ1], Band1_Freq, Band1_Q, &makoF_Band1);
    Filter_BP_Coeffs(Setting[e_EQ2], Band2_Freq, Band2_Q, &makoF_Band2);
    Filter_BP_Coeffs(Setting[e_EQ3], Band3_Freq, Band3_Q, &makoF_Band3);
    Filter_BP_Coeffs(Setting[e_EQ4], Band4_Freq, Band4_Q, &makoF_Band4);
    Filter_BP_Coeffs(Setting[e_EQ5], Band5_Freq, Band5_Q, &makoF_Band5);    
    
    //R2.00 Get the selected Amplifier IR.
    if ((Setting[e_Amp] != Setting_Last[e_Amp]) || Force)
    {
        Setting_Last[e_Amp] = Setting[e_Amp];
        Mako_Amp_Set();
    }
    
    //R2.00 Get the selected Speaker IR.
    if ((Setting[e_IR] != Setting_Last[e_IR]) || Force)
    {
        Setting_Last[e_IR] = Setting[e_IR];
        Mako_IR_Set();
    }

    //R2.00 Get the selected Pedal boost.
    if ((Setting[e_Pedal] != Setting_Last[e_Pedal]) || Force)
    {
        Setting_Last[e_Pedal] = Setting[e_Pedal];
        switch (int(Setting[e_Pedal]))
        {
        case 1:Filter_BP_Coeffs(6.0f, 450.0f, .35f, &makoF_MidBoost); break;
        case 2:Filter_BP_Coeffs(9.0f, 700.0f, .35f, &makoF_MidBoost); break;
        case 3:Filter_BP_Coeffs(12.0f, 900.0f, .35f, &makoF_MidBoost); break;
        case 4:Filter_BP_Coeffs(12.0f, 1200.0f, .25f, &makoF_MidBoost); break;
        case 5:Filter_BP_Coeffs(12.0f, 1500.0f, .25f, &makoF_MidBoost); break;
        case 6:Filter_BP_Coeffs(12.0f, 2500.0f, .25f, &makoF_MidBoost); break;
        case 7:Filter_BP_Coeffs(12.0f, 700.0f, 1.414f, &makoF_MidBoost); break;
        case 8:Filter_BP_Coeffs(12.0f, 900.0f, 1.414f, &makoF_MidBoost); break;
        case 9:Filter_BP_Coeffs(-6.0f, 1500.0f, 2.00f, &makoF_MidBoost); break;
        case 10:Filter_LP_Coeffs(3000.0f, &makoF_MidBoost); break;        
        }
    }

    //R1.00 RESET our settings flags.
    SettingsType = 0;
    SettingsChanged = false;
}

float MakoBiteAudioProcessor::Mako_CabSim(float tSample, int channel)
{
    int T1;
    float V = 0.0f;
    
    //R1.00 Get the next buffer position Index and store the new incoming Sample.
    T1 = IRB_Idx[channel];
    IRB[channel][T1] = tSample;

    //R1.00 Calculate the IR response by multiplying every IR sample by our audio buffer samples.
    //R1.00 Effectively it is a DELAY(comb filter) pedal with 1024 repeats in a very short time.
    //R1.00 The repeats will add and zero out signals due to phase which creates an EQ filter.
    //R1.00 The IR acts as both a delay and filter combined.
    //R1.00 For absolute best sound an IR should be 2048 samples. But the IR calc is very heavy on CPU usage.
    //R1.00 Using 1024 here for good sound and less CPU usage. Could add slider and make it even shorter.
    for (int t = 0; t < 1024; t++)
    {
        V += (IR_Final[t] * IRB[channel][T1]);

        //R1.00 Increment index, mask off bits past 1023 to keep our index between 0-1023.
        //R1.00 IR length must be a power of 2 for masking to work 1024(3FF), 2048(7FF) are standard sizes.
        T1 = (T1 + 1) & 0x3FF;
    }
    
    //R1.00 Decrement our buffer index and loop around at 0. 
    IRB_Idx[channel]--;
    if (IRB_Idx[channel] < 0) IRB_Idx[channel] = 1023;

    //R1.00 We usually gain volume here so reduce it.
    return V * IR_Final_VolAdjust;
}

void MakoBiteAudioProcessor::Mako_IR_Set()
{
    int IR_Model = int(Setting[e_IR]);

    //R1.00 Put one of the preset IRs into the actual IR used for processing.
    //R1.00 We could just use an index into the current array, but this lets us do
    //R1.00 other modifications to the IRs as well. Like resampling, etc.
    switch (IR_Model)
    {   
        case 1: for (int t = 0; t < 1024; t++) IR_Final[t] = IR_Stored_01[t]; break;
        case 2: for (int t = 0; t < 1024; t++) IR_Final[t] = IR_Stored_02[t]; break;
        case 3: for (int t = 0; t < 1024; t++) IR_Final[t] = IR_Stored_03[t]; break;
        case 4: for (int t = 0; t < 1024; t++) IR_Final[t] = IR_Stored_04[t]; break;
        case 5: for (int t = 0; t < 1024; t++) IR_Final[t] = IR_Stored_05[t]; break;
        case 6: for (int t = 0; t < 1024; t++) IR_Final[t] = IR_Stored_06[t]; break;
        case 7: for (int t = 0; t < 1024; t++) IR_Final[t] = IR_Stored_07[t]; break;
        default: for (int t = 0; t < 1024; t++) IR_Final[t] = 0.0f; IR_Final[0] = 1.0f; break;
    }

    //R1.00 These volumes are estimated values defined in Prepare to play.
    //R1.00 Could do complicated math to get better values. Close enough for us.
    IR_Final_VolAdjust = IR_VolAdjustVals[IR_Model];

    return;
}

void MakoBiteAudioProcessor::Mako_CheckForClipping(float tSample)
{
    //R2.00 Clip the signal to just below -1/1 so the audio engine does not crash. 
    //R2.00 Need a var here to let user know they are clipping. This is read in the
    //R2.00 EDITOR Timer code to show clipping on the UI.
    if (.9999f < tSample)
    {
        tSample = .9999f;
        AudioIsClipping = true;
    }
    else if (tSample < -.9999f)
    {
        tSample = -.9999f;
        AudioIsClipping = true;
    }    
}

//R2.00 This function is used so our filters are setup correctly on load.
//R2.00 Must match the same func in PluginEditor.cpp.
void MakoBiteAudioProcessor::Band_SetFilterValues()
{
    int EQ_Mode = int(Setting[e_EQ]);

    //R2.00 Define the user selected EQ mode.
    switch (EQ_Mode)
    {
    default:
    {
        Band1_Freq = 150.0f;
        Band2_Freq = 300.0f;
        Band3_Freq = 750.0f;
        Band4_Freq = 1500.0f;
        Band5_Freq = 3000.0f;
        Band1_Q = 1.414f;
        Band2_Q = .707f;
        Band3_Q = .350f;
        Band4_Q = .250f;
        Band5_Q = .35f;
        break;
    }
    case 1:
    {
        Band1_Freq = 150.0f;
        Band2_Freq = 450.0f;
        Band3_Freq = 900.0f;
        Band4_Freq = 1800.0f;
        Band5_Freq = 3500.0f;
        Band1_Q = .707f;
        Band2_Q = .707f;
        Band3_Q = .350f;
        Band4_Q = .250f;
        Band5_Q = .350f;
        break;
    }
    case 2:
    {
        Band1_Freq = 80.0f;
        Band2_Freq = 220.0f;
        Band3_Freq = 750.0f;
        Band4_Freq = 2200.0f;
        Band5_Freq = 6000.0f;
        Band1_Q = 1.414f;
        Band2_Q = .707f;
        Band3_Q = .350f;
        Band4_Q = .350f;
        Band5_Q = .35f;
        break;
    }
    case 3:
    {
        Band1_Freq = 80.0f;
        Band2_Freq = 350.0f;
        Band3_Freq = 900.0f;
        Band4_Freq = 1500.0f;
        Band5_Freq = 3000.0f;
        Band1_Q = .707f;
        Band2_Q = .707f;
        Band3_Q = .350f;
        Band4_Q = .707f;
        Band5_Q = .250f;
        break;
    }
    case 4:
    {
        Band1_Freq = 100.0f;
        Band2_Freq = 400.0f;
        Band3_Freq = 800.0f;
        Band4_Freq = 1600.0f;
        Band5_Freq = 3200.0f;
        Band1_Q = .707f;
        Band2_Q = .707f;
        Band3_Q = .35f;
        Band4_Q = .35f;
        Band5_Q = .25f;
        break;
    }
    case 5:
    {
        Band1_Freq = 120.0f;
        Band2_Freq = 330.0f;
        Band3_Freq = 660.0f;
        Band4_Freq = 1320.0f;
        Band5_Freq = 2500.0f;
        Band1_Q = .707f;
        Band2_Q = .707f;
        Band3_Q = .350f;
        Band4_Q = .250f;
        Band5_Q = .350f;
        break;
    }
    case 6:
    {
        Band1_Freq = 150.0f;
        Band2_Freq = 500.0f;
        Band3_Freq = 900.0f;
        Band4_Freq = 1800.0f;
        Band5_Freq = 5000.0f;
        Band1_Q = 1.414f;
        Band2_Q = .707f;
        Band3_Q = .350f;
        Band4_Q = .350f;
        Band5_Q = .250f;
        break;
    }
    case 7:
    {
        Band1_Freq = 80.0f;
        Band2_Freq = 300.0f;
        Band3_Freq = 650.0f;
        Band4_Freq = 1500.0f;
        Band5_Freq = 4500.0f;
        Band1_Q = 1.414f;
        Band2_Q = .707f;
        Band3_Q = .35f;
        Band4_Q = .707f;
        Band5_Q = .250f;
        break;
    }
    case 8:
    {
        Band1_Freq = 100.0f;
        Band2_Freq = 400.0f;
        Band3_Freq = 800.0f;
        Band4_Freq = 1600.0f;
        Band5_Freq = 4000.0f;
        Band1_Q = .707f;
        Band2_Q = .350f;
        Band3_Q = .350f;
        Band4_Q = .35f;
        Band5_Q = .25f;
        break;
    }
    case 9:
    {
        Band1_Freq = 200.0f;
        Band2_Freq = 600.0f;
        Band3_Freq = 1000.0f;
        Band4_Freq = 2000.0f;
        Band5_Freq = 5000.0f;
        Band1_Q = 1.00f;
        Band2_Q = .350f;
        Band3_Q = .350f;
        Band4_Q = .25f;
        Band5_Q = .35f;
        break;
    }
    case 10:
    {
        Band1_Freq = 80.0f;
        Band2_Freq = 250.0f;
        Band3_Freq = 900.0f;
        Band4_Freq = 1800.0f;
        Band5_Freq = 3600.0f;
        Band1_Q = .707f;
        Band2_Q = .707f;
        Band3_Q = .35f;
        Band4_Q = .25f;
        Band5_Q = .350f;
        break;
    }

    }

}
