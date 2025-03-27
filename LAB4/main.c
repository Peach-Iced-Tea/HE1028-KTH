#include "gd32vf103.h"
#include "drivers.h"
#include "lcd.h"
int main(void){
    int ms=0, s=0, key, pKey=-1, c=0, idle=0;
    int lookUpTbl[16]={1,4,7,14,2,5,8,0,3,6,9,15,10,11,12,13};
    int dac=0, speed=-100;
    int adcr, tmpr;
    int colors[9]={WHITE, BLACK, BLUE, RED, MAGENTA, GREEN, CYAN, YELLOW, DARKBLUE};
    char digits[10][10]={"Zero ","One  ","Two  ","Three","Four ","Five ","Six  ","Seven","Eight","Nine "};
    int entryMade = 0;
    int temp[2] = {0,0};
    int posA[2] = {0,0};
    int posB[2] = {0,0};
    int col = 0;
    int keyNum;
    int entryNr = 0;
    t5omsi();                               // Initialize timer5 1kHz
    colinit();                              // Initialize column toolbox
    l88init();                              // Initialize 8*8 led toolbox
    keyinit();                              // Initialize keyboard toolbox
    Lcd_SetType(LCD_NORMAL);//INVERTED);                // or use LCD_INVERTED!
    Lcd_Init();
    LCD_Clear(RED);
    LCD_ShowStr(10, 10, "HEARTBEAT:", WHITE, TRANSPARENT);
    LCD_Fill(10,30, 20, 40, BLUE);


    while (1) {
        idle++;                             // Manage Async events
        LCD_WR_Queue();                     // Manage LCD com queue!

        if (t5expq()) {                     // Manage periodic tasks
            l88row(colset());               // ...8*8LED and Keyboard
            if (++ms==1000){                // ...One second heart beat
              ms=0;
              LCD_ShowStr(80, 10, digits[s%10], WHITE, OPAQUE);
              l88mem(0,s++);
            }
            if ((key=keyscan())>=0) {       // ...Any key pressed?
              if (pKey==key) c++; else {c=0; pKey=key;}
              keyNum = lookUpTbl[key];
              l88mem(1,keyNum+(c<<4));
              if (entryNr > 0){
                switch(keyNum){
                  case 10:
                    posA[0] = temp[0];
                    posA[1] = temp[1];
                    entryNr == 0;
                    break;
                  case 11:
                    posB[0] = temp[0];
                    posB[1] = temp[1];
                    entryNr == 0;
                    break;
                  case 12:
                    col = temp[0];
                    entryNr == 0;
                    entryMade = 1;
                    break;
                  case 13:
                    break;
                  case 14:
                    break;
                  case 15:
                    break;
                  default:
                    if (entryNr < 2){
                      entryNr++;
                      temp[1] = keyNum;
                    }
                    break;
                }
              } else {
                switch(keyNum){
                  case 10:
                    LCD_Clear(RED);
                    break;
                  case 11:
                    LCD_ShowStr(144,72, "NF", WHITE, OPAQUE);
                  break;
                  case 12:break;
                  case 13:
                    if (entryMade == 1){
                      LCD_DrawLine(posA[0], posA[1], posB[0], posB[1], colors[col]);
                      entryMade = 0;
                    }
                    break;
                  case 14:
                    if (entryMade == 1){
                      Draw_Circle(posA[0], posA[1], posB[0], colors[col]);
                      entryMade = 0;
                    }
                    break;
                  case 15:
                    LCD_Fill(posA[0], posA[1], posB[0], posB[1], colors[col]);
                    break;
                  default:
                    entryNr++;
                    temp[0] = keyNum;
                    entryMade = 0;
                    break;
                }
              }
            }
            l88mem(2,idle>>8);              // ...Performance monitor
            l88mem(3,idle); idle=0;
        }
    }
}