#include "MainComponent.h"

/* my code additions begin - (1) */
#include <chrono> //for latency reporting
#define GUI_WD 520 //pixels
#define GUI_HT 380 //pixels
#define DB_LOW -60.0f //decibels
#define DB_HIGH 12.0f //decibels
#define INIT 0.0f //initialize
#define INC 0.1f //increment
/* my code additions end - (1) */

MainComponent::MainComponent()
{
    /* GUI - my code additions begin - (2) */
    addAndMakeVisible(reset_button);
    reset_button.setButtonText("reset");
    reset_button.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    reset_button.addListener(this);
    // input gain slider
    addAndMakeVisible(gain_in_slider);
    gain_in_slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    gain_in_slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
    gain_in_slider.setRange(DB_LOW, DB_HIGH, INC);
    gain_in_slider.setValue(INIT);
    gain_in_slider.setTextValueSuffix(" dB");
    gain_in_slider.setColour(juce::Slider::backgroundColourId, juce::Colours::darkgrey);
    gain_in_slider.setColour(juce::Slider::trackColourId, juce::Colours::darkred);
    gain_in_slider.setColour(juce::Slider::thumbColourId, juce::Colours::red);
    gain_in_slider.addListener(this);
    // clipper knob
    addAndMakeVisible(clipper_knob);
    clipper_knob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    clipper_knob.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
    clipper_knob.setRange(-30.0f, 6.0f, INC);
    clipper_knob.setValue(INIT);
    clipper_knob.setTextValueSuffix(" dB");
    clipper_knob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::darkgrey);
    clipper_knob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkred);
    clipper_knob.setColour(juce::Slider::thumbColourId, juce::Colours::red);
    clipper_knob.addListener(this);
    // mute toggle
    addAndMakeVisible(mute_toggle);
    mute_toggle.setButtonText("MUTE");
    mute_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    mute_toggle.addListener(this);
    // high-pass filter toggle
    addAndMakeVisible(highpass_toggle);
    highpass_toggle.setButtonText("HIGH-PASS");
    highpass_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    highpass_toggle.addListener(this);
    // low-pass filter toggle
    addAndMakeVisible(lowpass_toggle);
    lowpass_toggle.setButtonText("LOW-PASS");
    lowpass_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    lowpass_toggle.addListener(this);
    // output gain slider
    addAndMakeVisible(gain_out_slider);
    gain_out_slider.setSliderStyle(juce::Slider::Slider::LinearVertical);
    gain_out_slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
    gain_out_slider.setRange(DB_LOW, DB_HIGH, INC);
    gain_out_slider.setValue(INIT);
    gain_out_slider.setTextValueSuffix(" dB");
    gain_out_slider.setColour(juce::Slider::backgroundColourId, juce::Colours::darkgrey);
    gain_out_slider.setColour(juce::Slider::trackColourId, juce::Colours::darkred);
    gain_out_slider.setColour(juce::Slider::thumbColourId, juce::Colours::red);
    gain_out_slider.addListener(this);
    // automatic output gain toggle
    addAndMakeVisible(auto_gain_toggle);
    auto_gain_toggle.setButtonText("AUTO");
    auto_gain_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    auto_gain_toggle.addListener(this);
    // safe-mode toggle
    addAndMakeVisible(safe_toggle);
    safe_toggle.setButtonText("safe");
    safe_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    safe_toggle.addListener(this);
    // latency reporting button
    addAndMakeVisible(get_latency);
    get_latency.setButtonText("latency");
    get_latency.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    get_latency.addListener(this);
    // latency report GUI display label
    addAndMakeVisible(latency_label);
    latency_label.setText(" ", juce::dontSendNotification);
    latency_label.attachToComponent(&get_latency, true);
    // default application window dimensions
    setSize(GUI_WD, GUI_HT);
    // default typeface
    font.setTypefaceName("Futura");
    /* my code additions end - (2) */
    
    /* stock code from JUCE */
    // Some platforms require permissions to open input channels
    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio) && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
                                          [&](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels to open
        setAudioChannels(2, 2);
    }
    
    /* my code additions begin - (3) */
    // initialize the register arrays
    for (int k = 0; k < 50; ++k)
    {
        hp_reg_l[k] = 0.0; //highpass filter left channel
        hp_reg_r[k] = 0.0; //highpass filter right channel
        lp_reg_l[k] = 0.0; //lowpass filter left channel
        lp_reg_r[k] = 0.0; //lowpass filter right channel
    }
    for (int k = 0; k < 100; ++k) latency_reg[k] = 0.0; //latency register
    /* my code additions end - (3) */
}

/* stock code from JUCE */
MainComponent::~MainComponent()
{
    // Shut down the audio device and clear the audio source
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill)
{
    /* my code additions begin - (4) */
    using namespace std::chrono;
    auto start = high_resolution_clock::now(); //capture time at beginning of block
    /* my code additions end - (4) */
    
    /* stock code from JUCE begins */
    auto *device = deviceManager.getCurrentAudioDevice();
    auto activeInputChannels = device->getActiveInputChannels();
    auto activeOutputChannels = device->getActiveOutputChannels();
    auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
    auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

    for (int channel = 0; channel < maxOutputChannels; ++channel)
    {
        if ((!activeOutputChannels[channel]) || maxInputChannels == 0)
        {
            bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
        }
        else
        {
            auto actualInputChannel = channel % maxInputChannels;

            if (!activeInputChannels[channel])
            {
                bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
            }
            else
            {
                auto *inBuffer = bufferToFill.buffer->getReadPointer(actualInputChannel, bufferToFill.startSample);
                auto *outBuffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
                /* stock code from JUCE ends */
                
                /* DSP PROCESSING BLOCK - my code additions begin - (5) */
                // declare and define `N` as the length of the buffer
                int N = bufferToFill.numSamples;
                // declare register and filter variables
                double h;
                int i, n, k;
                // declare and initialize temporary buffers
                double temp_buff_1[N],
                    temp_buff_2[N],
                    temp_buff_3[N];
                for (i = 0; i < N; ++i)
                {
                    temp_buff_1[i] = 0.0;
                    temp_buff_2[i] = 0.0;
                    temp_buff_3[i] = 0.0;
                }
                
                // core processing loop 1 of 2 - loop over each sample in the buffer
                for (n = 0; n < N; ++n)
                {
                    // break loop if mute toggle is on
                    if (mute) break;

                    // apply input gain and pass values into the first temporary buffer
                    temp_buff_1[n] = inBuffer[n] * pow(10, gain_in_db/20); //dB to scalar

                    // high-pass filter audio if toggle is on
                    if (highpass)
                    {
                        if (!channel) //process the left channel
                        {
                            // shift register values
                            for (k = hp_K; k > 0; --k)
                            {
                                hp_reg_l[k] = hp_reg_l[k - 1];
                            }
                            // denominator
                            hp_reg_l[0] = temp_buff_1[n];
                            for (k = 1; k <= hp_K; ++k)
                            {
                                hp_reg_l[0] -= hp_a[k] * hp_reg_l[k];
                            }
                            // numerator
                            h = 0;
                            for (k = 0; k <= hp_K; ++k)
                            {
                                h += hp_b[k] * hp_reg_l[k];
                            }
                        }
                        else //process the right channel
                        {
                            // shift register values
                            for (k = hp_K; k > 0; --k)
                            {
                                hp_reg_r[k] = hp_reg_r[k - 1];
                            }
                            // denominator
                            hp_reg_r[0] = temp_buff_1[n];
                            for (k = 1; k <= hp_K; ++k)
                            {
                                hp_reg_r[0] -= hp_a[k] * hp_reg_r[k];
                            }
                            // numerator
                            h = 0;
                            for (k = 0; k <= hp_K; ++k)
                            {
                                h += hp_b[k] * hp_reg_r[k];
                            }
                        }
                        // output filtered values to next temporary buffer
                        temp_buff_2[n] = h;
                    }
                    // otherwise, pass directly to next temporary buffer
                    else temp_buff_2[n] = temp_buff_1[n];

                    // clip peaks above threshold value
                    double clipper_mod = pow(10, clipper_threshold/20); //dB to scalar
                    if (abs(temp_buff_2[n]) > clipper_mod)
                    {
                        // calculate polarity and then scale to threshold
                        temp_buff_2[n] = temp_buff_2[n] / abs(temp_buff_2[n]) * clipper_mod;
                    }
                    
                    // low-pass filter audio if toggle is on
                    if (lowpass)
                    {
                        if (!channel) //process left channel
                        {
                            // shift register values
                            for (k = lp_K; k > 0; --k)
                            {
                                lp_reg_l[k] = lp_reg_l[k - 1];
                            }
                            // denominator
                            lp_reg_l[0] = temp_buff_2[n];
                            for (k = 1; k <= lp_K; ++k)
                            {
                                lp_reg_l[0] -= lp_a[k] * lp_reg_l[k];
                            }
                            // numerator
                            h = 0;
                            for (k = 0; k <= lp_K; ++k)
                            {
                                h += lp_b[k] * lp_reg_l[k];
                            }
                        }
                        else //process right channel
                        {
                            // shift register values
                            for (k = lp_K; k > 0; --k)
                            {
                                lp_reg_r[k] = lp_reg_r[k - 1];
                            }
                            // denominator
                            lp_reg_r[0] = temp_buff_2[n];
                            for (k = 1; k <= lp_K; ++k)
                            {
                                lp_reg_r[0] -= lp_a[k] * lp_reg_r[k];
                            }
                            // numerator
                            h = 0.0;
                            for (k = 0; k <= lp_K; ++k)
                            {
                                h += lp_b[k] * lp_reg_r[k];
                            }
                        }
                        // output filtered values to next temporary buffer
                        temp_buff_3[n] = h;
                    }
                    // otherwise, pass directly to next temporary buffer
                    else temp_buff_3[n] = temp_buff_2[n];
                    
                    // auto-set output gain if toggle is on
                    if (auto_gain) gain_out_db = -0.5 * clipper_threshold - gain_in_db;
                    
                    // apply output gain
                    temp_buff_3[n] *= pow(10, gain_out_db/20); //dB to scalar
                    
                    // set max output to +-0.99 if safe mode is on
                    if (safe)
                    {
                        if (abs(temp_buff_3[n]) > 0.99)
                        {
                            temp_buff_3[n] = 0.99 * temp_buff_3[n] / abs(temp_buff_3[n]);
                        }
                    }
                }
                // core processing loop 2 of 2
                for (n = 0; n < N; ++n)
                {
                    // set outputs to `0` if toggle is on
                    if (mute)
                    {
                        outBuffer[n] = 0.0;
                        continue;
                    }
                    // feed final processing buffer back into channelData output buffer
                    outBuffer[n] = temp_buff_3[n];
                }
            }
        }
    }
    // latency reporting
    auto stop = high_resolution_clock::now(); //capture time at end of block
    auto latency = duration_cast<microseconds>(stop - start); //cast difference to latency in microseconds
    // display latency in debugger output window (comment out if not in use--adds to latency when engaged)
//    std::cout << "Processing Latency: " << latency.count() << " microseconds" << std::endl;
    // set latency register index
    ++lri;
    lri = lri % 100;
    latency_reg[lri] = latency.count(); //insert current block's latency value into register
    /* my code additions end - (5) */
}

void MainComponent::releaseResources()
{

}

void MainComponent::paint(juce::Graphics &g)
{
    /* my code additions begin - (6) */
    /* set GUI look and feel */
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);
    g.setFont(font);
    
    g.setFont(15.0f);
    g.drawText("IN", getWidth() * 0.125, getHeight() * 0.20, getWidth() * 0.125, getHeight() * 0.10, juce::Justification::centredTop);
    g.setFont(22.0f);
    g.drawText("TIPPER", getWidth() * 0.375, getHeight() * 0.10, getWidth() * 0.25, getHeight() * 0.10, juce::Justification::centredTop);
    g.setFont(24.0f);
    g.drawText("CLIPPER", getWidth() * 0.375, getHeight() * 0.14, getWidth() * 0.25, getHeight() * 0.10, juce::Justification::centredTop);
    g.setFont(15.0f);
    g.drawText("THRESHOLD", getWidth() * 0.375, getHeight() * 0.86, getWidth() * 0.25, getHeight() * 0.10, juce::Justification::centredTop);
    g.setFont(15.0f);
    g.drawText("OUT", getWidth() * 0.75, getHeight() * 0.20, getWidth() * 0.125, getHeight() * 0.10, juce::Justification::centredTop);
    /* my code additions end - (6) */
}

void MainComponent::resized()
{
    /* my code additions begin - (7) */
    /* set relative positions of GUI elements */
    reset_button.setBounds(5, 5, 50, 20);
    
    gain_in_slider.setBounds(getWidth() * 0.125, getHeight() * 0.3, getWidth() * 0.125, getHeight() * 0.45);

    clipper_knob.setBounds(getWidth() * 0.25, getHeight() * 0.25, getWidth() * 0.5, getHeight() * 0.60);

    mute_toggle.setBounds(getWidth() * 0.125, getHeight() * 0.775, getWidth() * 0.125, getHeight() * 0.1);
    
    highpass_toggle.setBounds(getWidth() * 0.125, getHeight() * 0.9, getWidth() * 0.125, 20);
    
    lowpass_toggle.setBounds(getWidth() * 0.75, getHeight() * 0.9, getWidth() * 0.125, 20);
    
    gain_out_slider.setBounds(getWidth() * 0.75, getHeight() * 0.3, getWidth() * 0.125, getHeight() * 0.45);
    
    auto_gain_toggle.setBounds(getWidth() * 0.75, getHeight() * 0.775, getWidth() * 0.125, getHeight() * 0.1);
    
    safe_toggle.setBounds(60, 5, 50, 20);
    
    get_latency.setBounds(getWidth() - 55, 5, 50, 20);
}

void MainComponent::sliderValueChanged(juce::Slider *slider)
{
    /* slider actions */
    // set input gain with GUI slider
    if (slider == &gain_in_slider) gain_in_db = gain_in_slider.getValue();
    // set clipper threshold with GUI slider
    if (slider == &clipper_knob) clipper_threshold = clipper_knob.getValue();
    // disengage `auto_gain` if output gain GUI slider is interacted with
    // set output gain with GUI slider
    if (slider == &gain_out_slider)
    {
        if (auto_gain) auto_gain = 0;
        auto_gain_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        gain_out_db = gain_out_slider.getValue();
        if (!mute)
        {
            gain_out_slider.setColour(juce::Slider::thumbColourId, juce::Colours::red);
            gain_out_slider.setColour(juce::Slider::trackColourId, juce::Colours::darkred);
        }
    }
}

void MainComponent::buttonClicked(juce::Button *button)
{
    /* button actions */
    if (button == &reset_button)
    {
        // `reset_button` re-initializes all parameters (except for safe-mode)
        gain_in_slider.setValue(INIT);
        gain_out_slider.setValue(INIT);
        clipper_knob.setValue(INIT);
        mute = 0;
        mute_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        highpass = 0;
        highpass_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        lowpass = 0;
        lowpass_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        auto_gain = 0;
        auto_gain_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        gain_out_db = gain_out_slider.getValue();
        gain_in_slider.setColour(juce::Slider::thumbColourId, juce::Colours::red);
        gain_in_slider.setColour(juce::Slider::trackColourId, juce::Colours::darkred);
        clipper_knob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkred);
        clipper_knob.setColour(juce::Slider::thumbColourId, juce::Colours::red);
        gain_out_slider.setColour(juce::Slider::thumbColourId, juce::Colours::red);
        gain_out_slider.setColour(juce::Slider::trackColourId, juce::Colours::darkred);
        latency_label.setText(" ", juce::dontSendNotification);
        setSize(GUI_WD, GUI_HT);
    }
    if (button == &mute_toggle)
    {
        // `mute_toggle` toggles the `mute` statement in the DSP block and changes the GUI accordingly
        // when engaged, DSP calculations are bypassed and audio output signal values are set to zero
        if (mute == 0)
        {
            mute = 1;
            mute_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
            gain_in_slider.setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
            gain_in_slider.setColour(juce::Slider::trackColourId, juce::Colours::darkgrey);
            clipper_knob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
            clipper_knob.setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
            gain_out_slider.setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
            gain_out_slider.setColour(juce::Slider::trackColourId, juce::Colours::darkgrey);
        }
        else
        {
            mute = 0;
            mute_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
            gain_in_slider.setColour(juce::Slider::thumbColourId, juce::Colours::red);
            gain_in_slider.setColour(juce::Slider::trackColourId, juce::Colours::darkred);
            clipper_knob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkred);
            clipper_knob.setColour(juce::Slider::thumbColourId, juce::Colours::red);
            if (auto_gain == 0)
            {
                gain_out_slider.setColour(juce::Slider::thumbColourId, juce::Colours::red);
                gain_out_slider.setColour(juce::Slider::trackColourId, juce::Colours::darkred);
            }
        }
    }
    if (button == &highpass_toggle)
    {
        // `highpass_toggle` toggles the `highpass` filter statement in the DSP block
        // when engaged, feeds audio data through an IIR high-pass filter
        if (!highpass)
        {
            highpass = 1;
            highpass_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
        }
        else
        {
            highpass = 0;
            highpass_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        }
    }
    if (button == &lowpass_toggle)
    {
        // `lowpass_toggle` toggles the `lowpass` filter statement in the DSP block
        // when engaged, feeds audio data through an IIR low-pass filter
        if (!lowpass)
        {
            lowpass = 1;
            lowpass_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
        }
        else
        {
            lowpass = 0;
            lowpass_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        }
    }
    if (button == &auto_gain_toggle)
    {
        // `auto_gain_toggle` toggles the `auto_gain` statement in the DSP block
        // when engaged, automatically sets output gain values to mirror input gain/clipper threshold
        if (auto_gain == 0)
        {
            auto_gain = 1;
            auto_gain_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
            gain_out_slider.setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
            gain_out_slider.setColour(juce::Slider::trackColourId, juce::Colours::darkgrey);
        }
        else
        {
            auto_gain = 0;
            auto_gain_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
            gain_out_db = gain_out_slider.getValue();
            if (!mute)
            {
                gain_out_slider.setColour(juce::Slider::thumbColourId, juce::Colours::red);
                gain_out_slider.setColour(juce::Slider::trackColourId, juce::Colours::darkred);
            }
        }
    }
    if (button == &safe_toggle)
    {
        // `safe_toggle` toggles the `safe` statement in the DSP block
        // safe-mode stops outputs from exceeding +-0.99
        if (safe == 0)
        {
            safe = 1;
            safe_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
        }
        else
        {
            safe = 0;
            safe_toggle.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
        }
    }
    if (button == &get_latency)
    {
        // on click of `get_latency`, compute mean latency over a sample of recent blocks and display in GUI
        using namespace std;
        float sum = 0.0; //calculate mean latency over 100 recent blocks
        for (int i = 0; i < 100; ++i) sum += latency_reg[i];
        float mean_latency = sum / 100;
        float latency_ms = mean_latency / 1000; //convert mean latency to milliseconds
        if (latency_ms >= 1.0) //display overload warning if latency calculated is >= 1 ms
        {
            char overload[] = "OVERLOAD\0";
            // display warning in `orangered`
            latency_label.setColour(juce::Label::textColourId, juce::Colours::orangered);
            latency_label.setText(overload, juce::dontSendNotification);
        }
        else //otherwise, display latency values in corresponding colors
        {
            string str = to_string(latency_ms); //convert latency value to a string
            char dest[9] = "      "; //declare and define destination string
            for (int i = 0; i < 5; ++i) dest[i] = str[i]; //copy first 5 values into destination string
            strcat(dest, "ms\0"); //append unit of measurement
            // display in `green` if latency is <= 0.2 ms
            if (latency_ms <= 0.2) latency_label.setColour(juce::Label::textColourId, juce::Colours::limegreen);
            // display in `yellow` if latency is > 0.2 ms and <= 0.3 ms
            else if (latency_ms <= 0.3) latency_label.setColour(juce::Label::textColourId, juce::Colours::yellow);
            // otherwise, display in `coral`
            else latency_label.setColour(juce::Label::textColourId, juce::Colours::coral);
            latency_label.setText(dest, juce::dontSendNotification);
        }
    }
}
/* my code additions end - (7) */
