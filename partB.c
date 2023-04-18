#include "VirtualSerial.h"
#include <xmc_gpio.h>
#include <stdio.h>
#include <string.h>


#define TICKS_PER_SECOND 1000

volatile size_t ticks = 0;
volatile size_t Button1_last = 0;
volatile size_t Button1_second_last = 0;
volatile size_t timeDifference = 0;
volatile size_t numToSend = 0;

const char englishMessage[] = "I CAN MORSE";
char *morseCode;

// function to be called every 1/1000 s
void SysTick_Handler(void)
{
  ticks++;

  if (ticks == 1000000)
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
    while (localTicks4 + 100 > ticks)
    {
      XMC_GPIO_SetOutputHigh(XMC_GPIO_PORT1, 1);
    }
  }
  else if (theDotOrDash == '-')
  {
    // output high for 3 dot
    size_t localTicks5 = ticks;
    while (localTicks5 + 300 > ticks)
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
      while (localTicks3 + 100 > ticks)
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

/* Clock configuration */
XMC_SCU_CLOCK_CONFIG_t clock_config = {
    .syspll_config.p_div = 2,
    .syspll_config.n_div = 80,
    .syspll_config.k_div = 4,
    .syspll_config.mode = XMC_SCU_CLOCK_SYSPLL_MODE_NORMAL,
    .syspll_config.clksrc = XMC_SCU_CLOCK_SYSPLLCLKSRC_OSCHP,
    .enable_oschp = true,
    .calibration_mode = XMC_SCU_CLOCK_FOFI_CALIBRATION_MODE_FACTORY,
    .fsys_clksrc = XMC_SCU_CLOCK_SYSCLKSRC_PLL,
    .fsys_clkdiv = 1,
    .fcpu_clkdiv = 1,
    .fccu_clkdiv = 1,
    .fperipheral_clkdiv = 1};

void SystemCoreClockSetup(void)
{
  /* Setup settings for USB clock */
  XMC_SCU_CLOCK_Init(&clock_config);

  XMC_SCU_CLOCK_EnableUsbPll();
  XMC_SCU_CLOCK_StartUsbPll(2, 64);
  XMC_SCU_CLOCK_SetUsbClockDivider(4);
  XMC_SCU_CLOCK_SetUsbClockSource(XMC_SCU_CLOCK_USBCLKSRC_USBPLL);
  XMC_SCU_CLOCK_EnableClock(XMC_SCU_CLOCK_USB);

  SystemCoreClockUpdate();
}

int integerLength(size_t x)
{
  if (x >= 1000000000)
    return 10;
  if (x >= 100000000)
    return 9;
  if (x >= 10000000)
    return 8;
  if (x >= 1000000)
    return 7;
  if (x >= 100000)
    return 6;
  if (x >= 10000)
    return 5;
  if (x >= 1000)
    return 4;
  if (x >= 100)
    return 3;
  if (x >= 10)
    return 2;
  return 1;
}

/* GPIO configuration */
#define GPIO_LED2 XMC_GPIO_PORT1, 0
#define GPIO_LED1 XMC_GPIO_PORT1, 1
#define GPIO_BUTTON1 XMC_GPIO_PORT1, 14
#define GPIO_BUTTON2 XMC_GPIO_PORT1, 15

const XMC_GPIO_CONFIG_t out_config =
    {.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
     .output_level = XMC_GPIO_OUTPUT_LEVEL_LOW,
     .output_strength = XMC_GPIO_OUTPUT_STRENGTH_STRONG_SHARP_EDGE};
const XMC_GPIO_CONFIG_t in_config =
    {.mode = XMC_GPIO_MODE_INPUT_TRISTATE,
     .output_level = XMC_GPIO_OUTPUT_LEVEL_LOW,
     .output_strength = XMC_GPIO_OUTPUT_STRENGTH_STRONG_SHARP_EDGE};

/**
 * Main program entry point. This routine configures the hardware required by
 * the application, then enters a loop to run the application tasks in sequence.
 */
int main(void)
{
  uint16_t Bytes = 0;

  XMC_GPIO_Init(GPIO_LED1, &out_config);
  XMC_GPIO_Init(GPIO_LED2, &out_config);
  XMC_GPIO_Init(GPIO_BUTTON1, &in_config);
  XMC_GPIO_Init(GPIO_BUTTON2, &in_config);
  USB_Init();

  /* Start sending periodic message */
  SysTick_Config(SystemCoreClock / TICKS_PER_SECOND);

  while (1)
  {
    /* Check if data received */
    Bytes = CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface);

    while (Bytes > 0)
    {
      /* Send data back to the host */
      CDC_Device_SendByte(&VirtualSerial_CDC_Interface,
                          CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface));
      --Bytes;
    }

    if (XMC_GPIO_GetInput(GPIO_BUTTON1) == 0)
    {
      // Once the button1 is pressed, the time point is recorded
      Button1_last = ticks;
      timeDifference = Button1_last - Button1_second_last;
      Button1_second_last = Button1_last;

      // string is sent only when Button1 is pressed down
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
              while (localTicks1 + 300 > ticks)
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
          while (localTicks2 + 600 > ticks)
          {
            XMC_GPIO_SetOutputLow(XMC_GPIO_PORT1, 1);
          }
        }
      }
      size_t localTicks6 = ticks;
      while (localTicks6 + 5000 > ticks)
      {
        XMC_GPIO_SetOutputLow(XMC_GPIO_PORT1, 1);
      }
    }
    else
    {
      XMC_GPIO_SetOutputLow(GPIO_LED1);
    }
    if (XMC_GPIO_GetInput(GPIO_BUTTON2) == 0)
    {
      // Set up functionality of Button2
      // Debug use: XMC_GPIO_SetOutputHigh(GPIO_LED2);
      // if Button1 has not been pressed yet, send 0
      if (Button1_last == 0)
      {
        numToSend = 0;
      }
      // if Button1 has only been pressed once
      else if (Button1_last == timeDifference)
      {
        numToSend = Button1_last;
      }
      // if Button1 has been pressed twice or more
      else
      {
        numToSend = timeDifference;
      }
      // convert numToSend from size_t to string
      char int_string[integerLength(numToSend)];
      sprintf(int_string, "%d", numToSend);
      // send milliseconds
      for (int32_t i = 0; i < strlen(int_string); i++)
      {
        // if it's not space bar, then it should be letter
        if (int_string[i] != ' ')
        {
          if (i + 1 < strlen(int_string))
          {
            if (int_string[i + 1] != ' ')
            {
              morseCode = morseEncoder(int_string[i]);
              // output high and low for the i-th letter
              flashLetter(morseCode);
              // output low for 3 dots for seperating two letters
              size_t localTicks7 = ticks;
              while (localTicks7 + 300 > ticks)
              {
                XMC_GPIO_SetOutputLow(XMC_GPIO_PORT1, 1);
              }
            }
            else if (int_string[i + 1] == ' ')
            {
              morseCode = morseEncoder(int_string[i]);
              // output high and low for the i-th letter
              flashLetter(morseCode);
            }
          }
          else
          {
            morseCode = morseEncoder(int_string[i]);
            // output high and low for the i-th letter
            flashLetter(morseCode);
          }
        }
        else if (int_string[i] == ' ')
        {
          // output low for 6 dots
          size_t localTicks8 = ticks;
          while (localTicks8 + 600 > ticks)
          {
            XMC_GPIO_SetOutputLow(XMC_GPIO_PORT1, 1);
          }
        }
      }
    }
    else
    {
      XMC_GPIO_SetOutputLow(GPIO_LED2);
    }

    CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
  }
  return 0;
}
