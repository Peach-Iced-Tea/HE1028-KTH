#include "gd32vf103.h"
#include "drivers.h"
#include "adc.h"
#include "pwm.h"

int main(void)
{
  int key, curCol = 0, adcr, tmpr;
  int keyMap[16] = {1, 4, 7, 14, 2, 5, 8, 0, 3, 6, 9, 15, 10, 11, 12, 13};
  int entryIndex = 0, entryMap[2] = {0, 0};
  int lightLevel = 0;
  int prefCounter = 0;
  int calc = 0;

  t5omsi();           // Initialize timer5 1kHz
  colinit();          // Initialize column toolbox
  l88init();          // Initialize 8*8 led toolbox
  keyinit();          // Initialize keyboard toolbox
  ADC3powerUpInit(0); // Initialize ADC0, Ch3
  T1powerUpInitPWM(0x2); // Timer #1, Ch #2 & 3 PWM
  T1setPWMmotorB(0);
  while (1)
  { // Manage Async events
    prefCounter++;
    if (adc_flag_get(ADC0, ADC_FLAG_EOC) == SET)
    { // ...ADC done?
      if (adc_flag_get(ADC0, ADC_FLAG_EOIC) == SET)
      { //...ch3 or ch16?
        tmpr = adc_inserted_data_read(ADC0, ADC_INSERTED_CHANNEL_0);
        adc_flag_clear(ADC0, ADC_FLAG_EOC);
        adc_flag_clear(ADC0, ADC_FLAG_EOIC);
      }
      else
      {
        adcr = adc_regular_data_read(ADC0); // ......get data
        adc_flag_clear(ADC0, ADC_FLAG_EOC); // ......clear IF
      }
    }

    if (t5expq())
    {                   // Manage periodic tasks
      l88row(colset()); // ...8*8LED and Keyboard
      if ((key = keyscan()) >= 0)
      { // ...Any key pressed?
        switch (keyMap[key])
        {
        case 10: // A pressed
          entryIndex = 0;
          T1setPWMmotorB(100);
          break;
        case 11:
          break;
        case 12: // C pressed
          entryIndex = 0;
          T1setPWMmotorB(0);
          break;
        case 13: // D pressed
          entryIndex = 0;
          T1setPWMmotorB(entryMap[0] * 10 + entryMap[1]);
          break;
        case 14: // * pressed
          if (entryIndex > 0)
          {
            entryIndex--;
          }
          break;
        case 15: // # pressed
          entryMap[0] = 0;
          entryMap[1] = 0;
          entryIndex = 0;
          break;
        default:
          if (entryIndex < 2)
          {
            entryMap[entryIndex++] = keyMap[key];
          }
          break;
        }
      }
      l88mem(7, entryIndex);
      l88mem(6, entryMap[1] + (entryMap[0] << 4));
    }
    l88mem(0,prefCounter>>8);                   // ...Performance monitor
    l88mem(1,prefCounter); prefCounter=0;
  }
}