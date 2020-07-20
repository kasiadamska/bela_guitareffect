# bela_guitareffect
Real-time distortion and delay guitar effect implemented using Bela.


Distortion
Distortion effect has four adjustable parameters, controlled using Bela’s GUI:
1. Range - determines gain amount (min = 0, max = 20)
2. Gain - determines gain amount (min = 0, max = 10)
3. Blend - applies gain to the signal (min = 0, max = 1)
4. Volume - output volume (min = 0, max = 1)

Delay effect has three adjustable parameters, controlled using Bela’s GUI:
1. Delay Amount - sets delay time (min = 0, max = 2s)
2. Feedback Amount - how much of the delay output should be sent back ( min = 0, max = 0.995)
3. Pre-delay Input - amount of time between the original sound and the audible onset of delays (min = 0, max = 1)

High-pass Filter
Finally, the high-pass has been implemented using a preset function from Bela’s Biquad library. Cut-off frequency is equal to 200Hz, and Q is 0.707.

Setup
• Lab Kit (Bela Starter Kit and cables) 
• 1 x button
• 1 x LED
• 1 x 220Ω resistor for LED • 1 x 1kΩ resistor for button
Upon running the code in Bela’s IDE we hear the original guitar sample playing. Pressing the button activates the “Distortion & Delay” filter, turning the LED on to indicate that the filter has been applied. We can simply turn the filter off by pressing the button again.
