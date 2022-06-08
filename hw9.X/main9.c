// WS2812B library

#include "ws2812b.h"
// other includes if necessary for debugging
#include<xc.h>
// Timer2 delay times, you can tune these if necessary
#include<sys/attribs.h>  // __ISR macro
#include <stdio.h>

#define LOWTIME 15 // number of 48MHz cycles to be low for 0.35uS
#define HIGHTIME 65 // number of 48MHz cycles to be high for 1.65uS

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL // use fast frc oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = OFF // primary osc disabled
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt value
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz fast rc internal oscillator
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

#define PIC32_SYS_FREQ 48000000ul // 48 million Hz
#define PIC32_DESIRED_BAUD 230400 // Baudrate for RS232

void ws2812b_setup();
void ws2812b_setColor(wsColor * c, int numLEDs);
void blink();


int main(){
    
    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    __builtin_enable_interrupts();
    
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 1;
    

    ws2812b_setup();
    //float w[6] = {0,60,120,180,240,300};
    //float w[3] = {0,120,240};
    wsColor c[4];
    //FOR setting each individual LED
//    c[0].r = 255;
//    c[0].b=0;
//    c[0].g=0;
//    c[1].r = 0;
//    c[1].b=255;
//    c[1].g=0;
//    c[2].r = 0;
//    c[2].b=0;
//    c[2].g=255;
//    c[2].r = 0;
//    c[2].b=0;
//    c[2].g=255;
//    c[3].r = 255;
//    c[3].b=255;
//    c[3].g=255;
    
    c[0] = HSBtoRGB( 0, 1 , 0.5);
    c[1] = HSBtoRGB(60 , 1 , 0.5);
    c[2] = HSBtoRGB(120 , 1 , 0.5);
    c[3] = HSBtoRGB(180 , 1 , 0.5);
    
 int i =0;
int j =0;
int k=0;
int m=0;
   
   
    
   while (1) {
 
        blink(); 
        
        for (k=0 ; k <= 360 ; k++ ) { 
          for (m=0; m<=3; m++){  
//              if (k > 360){
//                  k=k/360;
//              }
//              c[m]= HSBtoRGB((60*m)+k , 1 , 0.5);//% is MOD, which means /360 and keep the remainder
            c[m]= HSBtoRGB(((60*m)+k)%360 , 1 , 0.5);//% is MOD, which means /360 and keep the remainder
            ws2812b_setColor(c, 4) ; 
             
        }
          _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT()<24000000/200){}
        }
       
        
                
                //ws2812b_setColor(c, 4) ; 
        //}
        //}
        //ws2812b_setColor(c, 4) ; 
        
        
    }
    
    

}


// setup Timer2 for 48MHz, and setup the output pin
void ws2812b_setup() {
    T2CONbits.TCKPS = 000; // Timer2 prescaler N=1 (1:1)
    PR2 = 65535; // maximum period
    TMR2 = 0; // initialize Timer2 to 0
    T2CONbits.ON = 1; // turn on Timer2
    TRISBbits.TRISB2 = 0;
    LATBbits.LATB2 = 0;
    
}

// build an array of high/low times from the color input array, then output the high/low bits
void ws2812b_setColor(wsColor * c, int numLEDs) {
    int i = 0; int j = 0; // for loops
    int numBits = 2 * 3 * 8 * numLEDs; // the number of high/low bits to store, 2 per color bit
    //volatile unsigned int delay_times[2*3*8 * 5]; // I only gave you 5 WS2812B, adjust this if you get more somewhere
    volatile unsigned int delay_times[2*3*8 * 4]; // I only gave you 5 WS2812B, I used 4
    // start at time at 0
    delay_times[0] = 0;
    
    int nB = 1; // which high/low bit you are storing, 2 per color bit, 24 color bits per WS2812B
	
    // loop through each WS2812B
    //RED
    for (i = 0; i < numLEDs; i++) {
        // loop through each color bit, MSB first
        for (j = 7; j >= 0; j--) {
            // if the bit is a 1
            if ((c[i].r >> j) & 0b1)/* identify the bit in c[].r, is it 1 */ {
                // the high is longer
                delay_times[nB] = delay_times[nB - 1] + HIGHTIME;
                nB++;
                delay_times[nB] = delay_times[nB - 1] + LOWTIME;
                nB++;
            } 
            // if the bit is a 0
            else {
                // the low is longer
                delay_times[nB] = delay_times[nB - 1] + LOWTIME;
                nB++;
                delay_times[nB] = delay_times[nB - 1] + HIGHTIME;
                nB++;
            }
        }
    
        //GREEN
        
        for (j = 7; j >= 0; j--) {
            // if the bit is a 1
            if ((c[i].g >> j) & 0b1)/* identify the bit in c[].g, is it 1 */ {
                // the high is longer
                delay_times[nB] = delay_times[nB - 1] + HIGHTIME;
                nB++;
                delay_times[nB] = delay_times[nB - 1] + LOWTIME;
                nB++;
            } 
            // if the bit is a 0
            else {
                // the low is longer
                delay_times[nB] = delay_times[nB - 1] + LOWTIME;
                nB++;
                delay_times[nB] = delay_times[nB - 1] + HIGHTIME;
                nB++;
            }
        }
        
        //BLUE
        
        for (j = 7; j >= 0; j--) {
            // if the bit is a 1
            if ((c[i].b >> j) & 0b1)/* identify the bit in c[].r, is it 1 */ {
                // the high is longer
                delay_times[nB] = delay_times[nB - 1] + HIGHTIME;
                nB++;
                delay_times[nB] = delay_times[nB - 1] + LOWTIME;
                nB++;
            } 
            // if the bit is a 0
            else {
                // the low is longer
                delay_times[nB] = delay_times[nB - 1] + LOWTIME;
                nB++;
                delay_times[nB] = delay_times[nB - 1] + HIGHTIME;
                nB++;
            }
        }
       
    }

    // turn on the pin for the first high/low
    
    LATBbits.LATB2 =1;
    TMR2 = 0; // start the timer
    for (i = 1; i < numBits; i++) {
        while (TMR2 < delay_times[i]) {
        }
        LATBINV = 0b00000100; // invert B2
    }
    LATBbits.LATB2 = 0;
    //LATBbits.LATB2 = 0;
    TMR2 = 0;
    while(TMR2 < 2400){} // wait 50uS, reset condition
}
    



// adapted from https://forum.arduino.cc/index.php?topic=8498.0
// hue is a number from 0 to 360 that describes a color on the color wheel
// sat is the saturation level, from 0 to 1, where 1 is full color and 0 is gray
// brightness sets the maximum brightness, from 0 to 1
wsColor HSBtoRGB(float hue, float sat, float brightness) {
    float red = 0.0;
    float green = 0.0;
    float blue = 0.0;

    if (sat == 0.0) {
        red = brightness;
        green = brightness;
        blue = brightness;
    } else {
        if (hue == 360.0) {
            hue = 0;
        }

        int slice = hue / 60.0;
        float hue_frac = (hue / 60.0) - slice;

        float aa = brightness * (1.0 - sat);
        float bb = brightness * (1.0 - sat * hue_frac);
        float cc = brightness * (1.0 - sat * (1.0 - hue_frac));

        switch (slice) {
            case 0:
                red = brightness;
                green = cc;
                blue = aa;
                break;
            case 1:
                red = bb;
                green = brightness;
                blue = aa;
                break;
            case 2:
                red = aa;
                green = brightness;
                blue = cc;
                break;
            case 3:
                red = aa;
                green = bb;
                blue = brightness;
                break;
            case 4:
                red = cc;
                green = aa;
                blue = brightness;
                break;
            case 5:
                red = brightness;
                green = aa;
                blue = bb;
                break;
            default:
                red = 0.0;
                green = 0.0;
                blue = 0.0;
                break;
        }
    }

    unsigned char ired = red * 255.0;
    unsigned char igreen = green * 255.0;
    unsigned char iblue = blue * 255.0;

    wsColor c;
    c.r = ired;
    c.g = igreen;
    c.b = iblue;
    return c;
}

void blink(){
    LATAbits.LATA4 = 1;
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<24000000/2/20){}
    LATAbits.LATA4 = 0;
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<24000000/2/20){}
}