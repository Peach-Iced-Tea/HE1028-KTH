#include "gd32vf103.h"
#include "drivers.h"
#include "adc.h"
#include "pwm.h"

int main(void)
{
  int ms = 0, s = 0, key, shift = 0, pKey = -1, c = 0, idle = 0, adcr, tmpr, cStep = 0, pPos = 128, worldTime = 0, timeLim = 1000, pPts = 0, playing = 1;
  int worldMap[48] = {0, 3, 3, 99, 104, 104, 96, 98, 2, 0, 48, 48, 52, 4, 112, 83, 115, 3, 8, 136, 128, 2, 98, 96, 108, 108, 97, 1, 12, 204, 204, 12, 12, 64, 64, 18, 18, 0, 48, 49, 49, 0, 0, 124, 68, 84, 68, 124};

  t5omsi();           // Initialize timer5 1kHz
  colinit();          // Initialize column toolbox
  l88init();          // Initialize 8*8 led toolbox
  ADC3powerUpInit(0); // Initialize ADC0, Ch3

  while (1)
  {
    idle++; // Manage Async events

    if (adc_flag_get(ADC0, ADC_FLAG_EOC) == SET)
    { // ...ADC done?
      if (adc_flag_get(ADC0, ADC_FLAG_EOIC) == SET)
      {                                                              //...ch3 or ch16?
        tmpr = adc_inserted_data_read(ADC0, ADC_INSERTED_CHANNEL_0); // ......(view each ms)
        adc_flag_clear(ADC0, ADC_FLAG_EOC);
        adc_flag_clear(ADC0, ADC_FLAG_EOIC);
      }
      else
      {
        adcr = adc_regular_data_read(ADC0); // ......get data
        adc_flag_clear(ADC0, ADC_FLAG_EOC); // ......clear IF
        pPos = 128;
        for (int i = 0; i < 7; i++)
        {
          if (adcr < 512 + 512*i){
            shift = i;
            break;
          }
        }
        pPos = pPos >> shift;
        if (cStep > 0)
        {
          if (worldMap[(cStep - 1)%48] & pPos && playing){
            for (int i = 0; i < 7; i++)
            {
              l88mem(i, 0);
            }
            playing = 0;
          }
          l88mem(7, worldMap[(cStep - 1) % 48] | pPos);
        }
      }
    }
    if (t5expq())
    {                   // Manage periodic tasks
      l88row(colset()); // ...8*8LED and Keyboard
      worldTime++;      // ...One second heart beat
      if (playing){

      if (worldTime == timeLim)
      {
        worldTime = 0;
        pPts++;
        if (timeLim > 250)
        {
          timeLim -= 5;
        }
        for (int i = 0; i < 8; i++)
        {
          if (i > 0)
          {
            l88mem(7 - i, worldMap[(cStep + i) % 48]);
          }
          else
          {
            l88mem(7 - i, worldMap[(cStep + i) % 48] | pPos);
          }
        }
        cStep++;
      } 
      } else {
        l88mem(2, pPts);
      }
      adc_software_trigger_enable(ADC0, // Trigger another ADC conversion!
                                  ADC_REGULAR_CHANNEL);
    }
  }
}