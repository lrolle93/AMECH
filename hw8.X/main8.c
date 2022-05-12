// based on adafruit and sparkfun libraries


#include <string.h> // for memset

#include<sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include <xc.h> // for the core timer delay
#include "ssd1306.h"
#include "i2chw8.h"//include i2chw8
#include "font.h"//include font


// I2C pins need pull-up resistors, 2k-10k

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

unsigned char ssd1306_write = 0b01111000; // i2c address
unsigned char ssd1306_read = 0b01111001; // i2c address
unsigned char ssd1306_buffer[512]; // 128x32/8. Every bit is a pixel
void ssd1306_drawLetter (unsigned char x, unsigned char y, unsigned char letter);
void ssd1306_drawMessage (char x, char y, char *message);

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
    
    TRISAbits.TRISA4 = 0;
    // Sets A4 to an output
    i2c_master_setup();
    ssd1306_setup();
    
    char message[50];//20 letters
    float fps;
    
    
    
    while(1){
        
       
        _CP0_SET_COUNT(0);
            
        
               
         
        sprintf (message, "This FPS");
        //ssd1306_drawMessage (10, 10, message);
        ssd1306_drawMessage (10, 10, message);
        ssd1306_update();
              
        fps = 24000000.0/_CP0_GET_COUNT();
        sprintf (message, "%f", fps);
        ssd1306_drawMessage (50, 23, message);
        ssd1306_update();
        
        //life++;
        //ssd1306_drawPixel(10,15,1); draws one pixel
        //ssd1306_drawLetter (60, 10, 66); //Outputs letter B
        //ssd1306_update();
        
        
        /*
        //blink
        LATAbits.LATA4 = 1; 
        //A4 goes high
        _CP0_SET_COUNT(0); 
        //start timer        
        while (_CP0_GET_COUNT()<24000000)  {}
        // While timer is less than 0.5 seconds, 24 mil is 1 second
        LATAbits.LATA4 = 0;
        //A4 goes low    
        _CP0_SET_COUNT(0);
        //restart timer
        while (_CP0_GET_COUNT()<24000000){}
         */
           
    }
}

void ssd1306_setup() {
    // give a little delay for the ssd1306 to power up
    _CP0_SET_COUNT(0);
    while (_CP0_GET_COUNT() < 48000000 / 2 / 50) {
    }
    ssd1306_command(SSD1306_DISPLAYOFF);
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);
    ssd1306_command(0x80);
    ssd1306_command(SSD1306_SETMULTIPLEX);
    ssd1306_command(0x1F); // height-1 = 31
    ssd1306_command(SSD1306_SETDISPLAYOFFSET);
    ssd1306_command(0x0);
    ssd1306_command(SSD1306_SETSTARTLINE);
    ssd1306_command(SSD1306_CHARGEPUMP);
    ssd1306_command(0x14);
    ssd1306_command(SSD1306_MEMORYMODE);
    ssd1306_command(0x00);
    ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);
    ssd1306_command(SSD1306_SETCOMPINS);
    ssd1306_command(0x02);
    ssd1306_command(SSD1306_SETCONTRAST);
    ssd1306_command(0x8F);
    ssd1306_command(SSD1306_SETPRECHARGE);
    ssd1306_command(0xF1);
    ssd1306_command(SSD1306_SETVCOMDETECT);
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYON);
    ssd1306_clear();
    ssd1306_update();
}

// send a command instruction (not pixel data)
void ssd1306_command(unsigned char c) {
    i2c_master_start();
    i2c_master_send(ssd1306_write);
    i2c_master_send(0x00); // bit 7 is 0 for Co bit (data bytes only), bit 6 is 0 for DC (data is a command))
    i2c_master_send(c);
    i2c_master_stop();
}

// update every pixel on the screen
void ssd1306_update() {
    ssd1306_command(SSD1306_PAGEADDR);
    ssd1306_command(0);
    ssd1306_command(0xFF);
    ssd1306_command(SSD1306_COLUMNADDR);
    ssd1306_command(0);
    ssd1306_command(128 - 1); // Width

    unsigned short count = 512; // WIDTH * ((HEIGHT + 7) / 8)
    unsigned char * ptr = ssd1306_buffer; // first address of the pixel buffer
    i2c_master_start();
    i2c_master_send(ssd1306_write);
    i2c_master_send(0x40); // send pixel data
    // send every pixel
    while (count--) {
        i2c_master_send(*ptr++);
    }
    i2c_master_stop();
}

// set a pixel value. Call update() to push to the display)
void ssd1306_drawPixel(unsigned char x, unsigned char y, unsigned char color) {
    if ((x < 0) || (x >= 128) || (y < 0) || (y >= 32)) {
        return;
    }
    if (color == 1) {
        ssd1306_buffer[x + (y / 8)*128] |= (1 << (y & 7));
    } else {
        ssd1306_buffer[x + (y / 8)*128] &= ~(1 << (y & 7));
    }
}

void ssd1306_drawLetter (unsigned char x, unsigned char y, unsigned char letter){
    
    for (int col =0; col <=4; col++){
        for (int row =0; row <=7; row++){
            if ((((ASCII[letter-0x20][col])>>row)&1) == 0b1){
                ssd1306_drawPixel (x+col, y+row, 1);
            }
            else {
                ssd1306_drawPixel (x+col, y+row, 0);
            }
        }
    }
}


void ssd1306_drawMessage (char x, char y, char *message){
    int i=0;
    while (message[i] !=0){
       ssd1306_drawLetter (x,y,message[i]);
       i++;
       x=x+6;
    }
}

// zero every pixel value
void ssd1306_clear() {
    memset(ssd1306_buffer, 0, 512); // make every bit a 0, memset in string.h
}


void i2c_master_setup(void) {
    // using a large BRG to see it on the nScope, make it smaller after verifying that code works
    // look up TPGD in the datasheet
    I2C1BRG = 30; // I2CBRG = [1/(2*Fsck) - TPGD]*Pblck - 2 (TPGD is the Pulse Gobbler Delay)
    I2C1CONbits.ON = 1; // turn on the I2C1 module
}

void i2c_master_start(void) {
    I2C1CONbits.SEN = 1; // send the start bit
    while (I2C1CONbits.SEN) {
        ;
    } // wait for the start bit to be sent
}

void i2c_master_restart(void) {
    I2C1CONbits.RSEN = 1; // send a restart 
    while (I2C1CONbits.RSEN) {
        ;
    } // wait for the restart to clear
}

void i2c_master_send(unsigned char byte) { // send a byte to slave
    I2C1TRN = byte; // if an address, bit 0 = 0 for write, 1 for read
    while (I2C1STATbits.TRSTAT) {
        ;
    } // wait for the transmission to finish
    if (I2C1STATbits.ACKSTAT) { // if this is high, slave has not acknowledged
        // ("I2C1 Master: failed to receive ACK\r\n");
        while(1){} // get stuck here if the chip does not ACK back
    }
}

unsigned char i2c_master_recv(void) { // receive a byte from the slave
    I2C1CONbits.RCEN = 1; // start receiving data
    while (!I2C1STATbits.RBF) {
        ;
    } // wait to receive the data
    return I2C1RCV; // read and return the data
}

void i2c_master_ack(int val) { // sends ACK = 0 (slave should send another byte)
    // or NACK = 1 (no more bytes requested from slave)
    I2C1CONbits.ACKDT = val; // store ACK/NACK in ACKDT
    I2C1CONbits.ACKEN = 1; // send ACKDT
    while (I2C1CONbits.ACKEN) {
        ;
    } // wait for ACK/NACK to be sent
}

void i2c_master_stop(void) { // send a STOP:
    I2C1CONbits.PEN = 1; // comm is complete and master relinquishes bus
    while (I2C1CONbits.PEN) {
        ;
    } // wait for STOP to complete
}


void setpin (unsigned char address, unsigned char reg, unsigned char value){
    //Write function
    i2c_master_start();
    i2c_master_send(address);
    i2c_master_send(reg);
    i2c_master_send(value);
    i2c_master_stop();
}


void blink (void){
     TRISAbits.TRISA4 = 0;
       // Sets A4 to an output   
    LATAbits.LATA4 = 1; 
      //A4 goes high
      _CP0_SET_COUNT(0); 
      //start timer        
      while (_CP0_GET_COUNT()<1200000)  {
           // While timer is less than 0.5 seconds, 24 mil is 1 second
                   
       }
      LATAbits.LATA4 = 0;
      //A4 goes low    
       _CP0_SET_COUNT(0);
       //restart timer
       
        while (_CP0_GET_COUNT()<1200000){
           // While timer is less than 0.5 seconds, 24 mil is 1 second
          
                 }
        
   LATAbits.LATA4 = 1;
   //a4 goes high
   _CP0_SET_COUNT(0);
   //restart timer
}
