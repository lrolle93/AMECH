#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include"hw5.h" //include hw5

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

//void readUART1(char * string, int maxLength);
//void writeUART1(const char * string);
void initSPI();
unsigned char spi_io(unsigned char o);
unsigned short voltage(unsigned char ab, unsigned char q);

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

// FOR UART 
    /*
            // turn on UART3 without an interrupt    
          U1MODEbits.BRGH = 0; // set baud to NU32_DESIRED_BAUD
          U1BRG = ((48000000 / 230400) / 16) - 1;

          // 8 bit, no parity bit, and 1 stop bit (8N1 setup)
          U1MODEbits.PDSEL = 0;
          U1MODEbits.STSEL = 0;

          // configure TX & RX pins as output & input pins
          U1STAbits.UTXEN = 1;
          U1STAbits.URXEN = 1;

          // enable the uart
          U1MODEbits.ON = 1;

           int u =0;

           TRISAbits.TRISA4 = 0;
               // Sets A4 to an output 
               LATAbits.LATA4 = 0;
               // Sets Value of A4 to 0 i.e. off
               TRISBbits.TRISB4 = 1;
                // Sets B4 to an input

               RPB3Rbits.RPB3R = 0b0001;
               // MAKE B3 U1TX

               U1RXRbits.U1RXR = 0b0000;
               // MAKE A2 U1RX

               char m [100];

                while (1) {
                // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
                // remember the core timer runs at half the sysclk


               if (PORTBbits.RB4 == 0) {
                   // if Button is pushed
 
               LATAbits.LATA4 = 1;
               // Turn LED on

               _CP0_SET_COUNT(0);
               // Start your timer

               while (_CP0_GET_COUNT()<12000000)  {
                   // While timer is less than 0.5 seconds, 24 mil is 1 second
               }

                LATAbits.LATA4 = 0;
                // turn LED off, repeat process to blink

           _CP0_SET_COUNT(0);

               while (_CP0_GET_COUNT()<12000000)  {

               }

           LATAbits.LATA4 = 1;

            _CP0_SET_COUNT(0);

               while (_CP0_GET_COUNT()<12000000)  {

               }

           LATAbits.LATA4 = 0;

           sprintf(m,"Blink %d\r\n",u);
           writeUART1(m);
           u++;



                }

               }  

         */     
    initSPI();
    //Initialize SPI
    float t = 0;
    int v = 0;
    int down = 0;
    
    while (1){
        // For A output
        unsigned char volta = ((sin(2*3.14*2*t)+1)*127);
        unsigned short vala = voltage (0, volta);
        LATBbits.LATB5 = 0; //Bring CS Low
        unsigned short a1 = vala>>8;
        unsigned short a2 = vala;
        spi_io(a1); //write the byte
        spi_io(a2); //write the byte
        LATBbits.LATB5 = 1; //Bring cs high
            /*
            i++;
            if (i==100){
                i=0;
            }
            */
        
        //FOR B OUTPUT
        unsigned char voltb = (v);
        unsigned short valb = voltage (1, voltb);
        LATBbits.LATB5 = 0; //Bring CS Low
        unsigned short b1 = valb>>8;
        unsigned short b2 = valb;
        spi_io(b1); //write the byte
        spi_io(b2); //write the byte
        LATBbits.LATB5 = 1; //Bring cs high
        
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT()<48000000/100){
        
        }
        t= t+0.01;
        
        if (down==0){
            v=v+10;
            if(v>=255){
                down=1;
            }
        }
            
        if (down==1){
            v=v-10;
            if (v<=0){
                down=0;
            }
          }
        
        
            
        
        
       
        
        
        
}
   
}

// initialize SPI1
void initSPI() {
  // do your TRIS and LAT commands here
   
    ANSELAbits.ANSA1 = 0;
    // Turn off A1 Analog
    
    TRISBbits.TRISB5 = 0;
    //Sets B5 to an output for CS
    
    LATBbits.LATB5 = 1;
    //Sets  B5 high
    
    TRISAbits.TRISA1 = 0;
    //Sets A1 to an output
   
    RPA1Rbits.RPA1R = 0b0011;
    //Make A1 SDO1
         
    TRISBbits.TRISB14 =0;
    //Sets B14 to an output.

    // setup SPI1
    SPI1CON = 0; // turn off the spi module and reset it
    SPI1BUF; // clear the rx buffer by reading from it
    SPI1BRG = 500; // 1000 for 24kHz, 1 for 12MHz; // baud rate to 10 MHz [SPI1BRG = (48000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0; // clear the overflow bit
    SPI1CONbits.CKE = 1; // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1; // master operation
    SPI1CONbits.ON = 1; // turn on spi 
}


// send a byte via spi and return the response
unsigned char spi_io(unsigned char o) {
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}

/*
 // Read from UART1
// block other functions until you get a '\r' or '\n'
// send the pointer to your char array and the number of elements in the array
void readUART1(char * message, int maxLength) {
  char data = 0;
  int complete = 0, num_bytes = 0;
  // loop until you get a '\r' or '\n'
  while (!complete) {
    if (U1STAbits.URXDA) { // if data is available
      data = U1RXREG;      // read the data
      if ((data == '\n') || (data == '\r')) {
        complete = 1;
      } else {
        message[num_bytes] = data;
        ++num_bytes;
        // roll over if the array is too small
        if (num_bytes >= maxLength) {
          num_bytes = 0;
        }
      }
    }
  }
  // end the string
  message[num_bytes] = '\0';
}

// Write a character array using UART1
void writeUART1(const char * string) {
  while (*string != '\0') {
    while (U1STAbits.UTXBF) {
      ; // wait until tx buffer isn't full
    }
    U1TXREG = *string;
    ++string;
  }
}
*/

unsigned short voltage(unsigned char ab, unsigned char q) {
    unsigned short p = 0;
    p = ab << 15;
    p= p | (0b111<<12);
    p = p | (q << 4);
    return p;
}