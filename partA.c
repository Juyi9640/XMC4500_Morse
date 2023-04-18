#include <xmc_gpio.h>
#include <stdio.h>
#include <string.h>


#define TICKS_PER_SECOND 10
//#define TICKS_WAIT 1000

volatile size_t ticks = 0;
const char englishMessage[] = "I CAN MORSE";
char *morseCode;

// function to be called every 1/10 s
// which means it ticks at 10 Hz
void SysTick_Handler(void)
{
  ticks++;

  if (ticks == 10000)
    ticks = 0;
}

// function to convert an alphabet to Morse code
char *morseEncoder(char x)
{
  switch (x)
  {
  case 'A':return ".-";break;
  case 'B':return "-...";break;
  case 'C':return "-.-.";break;
  case 'D':return "-..";break;
  case 'E':return ".";break;
  case 'F':return "..-.";break;
  case 'G':return "--.";break;
  case 'H':return "....";break;
  case 'I':return "..";break;
  case 'J':return ".---";break;
  case 'K':return "-.-";break;
  case 'L':return ".-..";break;
  case 'M':return "--";break;
  case 'N':return "-.";break;
  case 'O':return "---";break;
  case 'P':return ".--.";break;
  case 'Q':return "--.-";break;
  case 'R':return ".-.";break;
  case 'S':return "...";break;
  case 'T':return "-";break;
  case 'U':return "..-";break;
  case 'V':return "...-";break;
  case 'W':return ".--";break;
  case 'X':return "-..-";break;
  case 'Y':return "-.--";break;
  case 'Z':return "--..";break;
  case '1':return ".----";break;
  case '2':return "..---";break;
  case '3':return "...--";break;
  case '4':return "....-";break;
  case '5':return ".....";break;
  case '6':return "-....";break;
  case '7':return "--...";break;
  case '8':return "---..";break;
  case '9':return "----.";break;
  case '0':return "-----";break;
  default:return 0;
  }
}

void flashTheDotOrDash(char theDotOrDash)
{
  if (theDotOrDash == '.')
  {
    // output high for 1 dot
    size_t localTicks4 = ticks;
    while (localTicks4 + 1 > ticks)
    {
      XMC_GPIO_SetOutputHigh(XMC_GPIO_PORT1, 1);
    }
  }
  else if (theDotOrDash == '-')
  {
    // output high for 3 dot
    size_t localTicks5 = ticks;
    while (localTicks5 + 3 > ticks)
    {
      XMC_GPIO_SetOutputHigh(XMC_GPIO_PORT1, 1);
    }
  }
}

void flashLetter(char *letterSequence)
{
  for (int32_t j = 0; j < strlen(letterSequence); j++)
  {
    if (j + 1 < strlen(letterSequence))
    {
      flashTheDotOrDash(letterSequence[j]);
      // output low for 1 dot for seperating dot or dash
      size_t localTicks3 = ticks;
      while (localTicks3 + 1 > ticks)
      {
        XMC_GPIO_SetOutputLow(XMC_GPIO_PORT1, 1);
      }
    }
    else if (j + 1 == strlen(letterSequence))
    {
      flashTheDotOrDash(letterSequence[j]);
    }
  }
}

int main(void)
{
  const XMC_GPIO_CONFIG_t LED_config =
      {.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
       .output_level = XMC_GPIO_OUTPUT_LEVEL_LOW,
       .output_strength = XMC_GPIO_OUTPUT_STRENGTH_STRONG_SHARP_EDGE};

  XMC_GPIO_Init(XMC_GPIO_PORT1, 1, &LED_config);

  /* Start sending periodic message */
  SysTick_Config(SystemCoreClock / TICKS_PER_SECOND);

  while (1)
  {
    for (int32_t i = 0; i < strlen(englishMessage); i++)
    {
      // if it's not space bar, then it should be letter
      if (englishMessage[i] != ' ')
      {
        if (i + 1 < strlen(englishMessage))
        {
          if (englishMessage[i + 1] != ' ')
          {
            morseCode = morseEncoder(englishMessage[i]);
            // output high and low for the i-th letter
            flashLetter(morseCode);
            // output low for 3 dots for seperating two letters
            size_t localTicks1 = ticks;
            while (localTicks1 + 3 > ticks)
            {
              XMC_GPIO_SetOutputLow(XMC_GPIO_PORT1, 1);
            }
          }
          else if (englishMessage[i + 1] == ' ')
          {
            morseCode = morseEncoder(englishMessage[i]);
            // output high and low for the i-th letter
            flashLetter(morseCode);
          }
        }
        else
        {
          morseCode = morseEncoder(englishMessage[i]);
          // output high and low for the i-th letter
          flashLetter(morseCode);
        }
      }
      else if (englishMessage[i] == ' ')
      {
        // output low for 6 dots
        size_t localTicks2 = ticks;
        while (localTicks2 + 6 > ticks)
        {
          XMC_GPIO_SetOutputLow(XMC_GPIO_PORT1, 1);
        }
      }
    }
    size_t localTicks6 = ticks;
            while (localTicks6 + 50 > ticks)
        {
          XMC_GPIO_SetOutputLow(XMC_GPIO_PORT1, 1);
        }
  }
  return 0;
}

/*if (ticks == TICKS_WAIT){
  XMC_GPIO_SetOutputHigh(XMC_GPIO_PORT1,1);
}
if (ticks == TICKS_WAIT + dotLength){
  XMC_GPIO_SetOutputLow(XMC_GPIO_PORT1,1);
}*/
// for(int32_t waiter=(1<<20); waiter >= 0; waiter--);
// XMC_GPIO_ToggleOutput(XMC_GPIO_PORT1, 1);
// /opt/XMClib/XMC_Peripheral_Library_v2.1.16/XMCLib/examples/XMC4500_series/SPI/SPI_TRANSMIT/main.c
