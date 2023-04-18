# XMC4500_Morse
Send a given string in Morse code via LED and use Button for control

Requirements:

Part A:        
In this part the program should be able to send a given string in Morse code via LED1. Refer to the ITU standard M.1677-1 for details on the Morse Code. The punctuation character does not have to be implemented. Letters and numbers according to Section 1.1.1 and 1.1.2 are sufficient. The length of a dot has to be 100 ms ± 10 ms. Send out the string repeatedly while the device is on, without any calling or invitation signals but a pause of 5 s after each transmission. The string to send out is: I CAN MORSE

Part B:       
Extend Part A as follows:        
* The string is not sent continuously, but only once when Button1 is pressed down (not when it is released).
* Once Button2 is pressed down (not when it is released), the time between the last and second last press of Button1 in milliseconds is sent once in Morse code via LED1.
    If Button1 has not been pressed yet, send a single 0 digit.
    If Button1 has only been pressed once
