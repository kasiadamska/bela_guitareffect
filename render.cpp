// Assignment 3: Final Project: Distortion & Delay Guitar Effect with a preset high-pass filter
//
// ECS7012U - MUSIC AND AUDIO PROGRAMMING 
// School of Electronic Engineering and Computer Science
// Queen Mary University of London
// Spring 2020

//Import Libraries and files
#include <Bela.h>
#include <SampleLoader.h>
#include <libraries/Biquad/Biquad.h>
#include <cmath>
#include <algorithm>
#include <libraries/Gui/Gui.h>
#include <libraries/GuiController/GuiController.h>

#define DELAY_BUFFER_SIZE 44100 //delay buffer size

// Browser-based GUI to adjust parameters
Gui gui;
GuiController controller;

Biquad hpFilter;	// Biquad high-pass frequency;

float gHPfreq =  200;  // Cut-off frequency for high-pass filter (Hz)
float gFilterQ = 0.707; // Quality factor for the biquad filters to provide a Butterworth response


// Load guitar sample 
string gFilename = "guitar2.wav"; // Name of the sound file (in project folder)
float *gSampleBuffer;			 // Buffer that holds the sound file
int gSampleBufferLength;		 // The length of the buffer in frames
int gReadPointer = 0;			 // Position of the last frame we played 

// Initialise global variables for Distortion
float gDistortionRange = 0;
float gDistortionGain = 0;
float gDistortionBlend = 0;
float gDistortionVolume = 0;

// Initialise global variables for Delay
float gDelayBuffer[DELAY_BUFFER_SIZE] = {0}; //Buffer holding previous samples
int gDelayBufWritePtr = 0; //Write pointer
float gDelayAmount = 0; //Amount of delay
float gDelayFeedbackAmount = 0; //Amout of delay feedback
float gDelayAmountPre = 0; //Pre-delay input 
int gDelayInSamples = 22050; //Amount of delay in samples -> 44100/2 = 22050


//Initialise analog inputs and outputs
const int kLEDPin = 0;    // digital pin P8_07 - check the pin diagram in the IDE
const int kButtonPin = 1;	// Button on P8 pin 8
int gLastButtonValue = 1;	// Keep track of the previous button state in render()
int gtoneState = 0; //is the tone on or off

int gAudioFramesPerAnalogFrame = 0;


bool setup(BelaContext *context, void *userData)
{
	// Button press turns the effect on
	pinMode(context, 0, kButtonPin, INPUT);
	// LED is on when the effect is on
	pinMode(context, 0, kLEDPin, OUTPUT);

	gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;


	// Check the length of the audio file and allocate memory
    gSampleBufferLength = getNumFrames(gFilename);
    
    if(gSampleBufferLength <= 0) {
    	rt_printf("Error loading audio file '%s'\n", gFilename.c_str());
    	return false;
    }
    
    gSampleBuffer = new float[gSampleBufferLength];
    
    // Make sure the memory allocated properly
    if(gSampleBuffer == 0) {
    	rt_printf("Error allocating memory for the audio buffer.\n");
    	return false;
    }
    
    // Load the sound into the file (note: this example assumes a mono audio file)
    getSamples(gFilename, gSampleBuffer, 0, 0, gSampleBufferLength);

    rt_printf("Loaded the audio file '%s' with %d frames (%.1f seconds)\n", 
    			gFilename.c_str(), gSampleBufferLength,
    			gSampleBufferLength / context->audioSampleRate);
    			
 	// Set up the GUI
	gui.setup(context->projectName);
	controller.setup(&gui, "Filter Controller");	
	
	// Arguments: name, default value, minimum, maximum, increment
	// Create sliders for Distortion and Delay parameters
	controller.addSlider("Range", 8, 0, 20, 0);
	controller.addSlider("Gain", 3, 0, 10, 0);
	controller.addSlider("Blend", 0.5, 0, 1, 0);
	controller.addSlider("Volume", 0.7, 0, 1, 0);
	controller.addSlider("Delay Amount", 0.1, 0, 2, 0);
	controller.addSlider("Feedback Amount", 0.005, 0, 0.995, 0);
	controller.addSlider("Pre-delay Amount", 0.01, 0, 1, 0);
	
	//Create highpass filter 
	hpFilter.setup(gHPfreq, context->audioSampleRate, Biquad::highpass, gFilterQ, 0);


	return true;
}

void render(BelaContext *context, void *userData)
{

	for (unsigned int n = 0; n < context->audioFrames; n++) 
	{
		//Initialise outputs
		float distortion = 0; //distortion on original signal
		float output = 0; //output from distortion and delay
		float out = 0; //final output 
		
		float input = gSampleBuffer[gReadPointer];
        if(++gReadPointer >= gSampleBufferLength){
        	gReadPointer = 0;
        }
        
        // Increment delay buffer write pointer
        if(++gDelayBufWritePtr>DELAY_BUFFER_SIZE){
            gDelayBufWritePtr = 0;
        }
        
        //Read from button	
    	int buttonValue = digitalRead(context, n, kButtonPin);
    	
    	//Check if button has been pressed
    	if(buttonValue == 0 && gLastButtonValue != 0)
    	{
    		
    		if(gtoneState == 0)
				gtoneState = 1; // if button is pressed apply distortion and delay effect 
			else
				gtoneState = 0;
    	}
    	else{
    		out = input; //if button is not pressed play the original sample 
    	}
    	
    	gLastButtonValue = buttonValue; // keep track of the button state 


    	//If button pressed
    	if(gtoneState !=0) {

			if(gReadPointer < gSampleBufferLength)
			{
				//read distortion and delay parameters from the sliders in GUI
				gDistortionRange = controller.getSliderValue(0);
				gDistortionGain = controller.getSliderValue(1);
				gDistortionBlend = controller.getSliderValue(2);
				gDistortionVolume = controller.getSliderValue(3);
				gDelayAmount = controller.getSliderValue(4);
				gDelayFeedbackAmount = controller.getSliderValue(5);
				gDelayAmountPre = controller.getSliderValue(6);
			
				//applay gain and range to input signal
				distortion = gDistortionRange * gDistortionGain * input +.01;
				//compute distortion 
				output = ( ( ( ((2/M_PI) * std::atan(distortion)) * gDistortionBlend) + (input * (1.f - gDistortionBlend)) ) / 2.f) * gDistortionVolume;

				//compute delay
				float delay_out = (gDelayAmountPre * output + gDelayBuffer[(gDelayBufWritePtr-gDelayInSamples+DELAY_BUFFER_SIZE)%DELAY_BUFFER_SIZE] * gDelayFeedbackAmount);
        		//update buffer
        		gDelayBuffer[gDelayBufWritePtr] = delay_out;
        		//write delay to output buffer
    			 output += gDelayBuffer[(gDelayBufWritePtr-gDelayInSamples+DELAY_BUFFER_SIZE)%DELAY_BUFFER_SIZE] * gDelayAmount;

				//apply highpass filter 
				out = hpFilter.process(output);

				// LED on when effect is turned on
				digitalWrite(context, n, kLEDPin, 1);
			}
			
		}
		else {
			// Button wasn't pushed. Turn the LED off, no sound
			digitalWrite(context, n, kLEDPin, 0);
		}
    	

	// Store the sample in the audio output buffer
		for(unsigned int channel = 0; channel <context->audioOutChannels; channel++) 
		{
				audioWrite(context, n, channel, out);
		}
	}
}
void cleanup(BelaContext *context, void *userData)
{
}