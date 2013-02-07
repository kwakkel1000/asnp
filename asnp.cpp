//
//
// @ Project : asnp
// @ File Name : asnp.cpp
// @ Date : 05-02-2013
// @ Author : Gijs Kwakkel
//
//
// Copyright (c) 2013 Gijs Kwakkel
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//


#include "aux_globals.h"
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "communication/include/usart.h"
#include "communication/include/spi.h"
#include "communication/include/i2c.h"

#include "lcd/include/HD44780.h"

#define I2C_EEPROM_1 0b1010000
//#define I2C_EEPROM_1 0x50
#define SPI_ENC28J60 PB5

#define VREF 5

HD44780 lcd;

// functions

void checkButton();
void action();
uint16_t readADC(uint8_t ADCchannel);
uint8_t readEEPROM(uint16_t address);
void writeEEPROM(uint16_t address, uint8_t data);

//2-dimensional array for asigning the buttons and there high and low values
uint16_t g_Button[6][3] = {{1, 950, 1000}, // button 1
                     {2, 850, 900}, // button 2
                     {3, 700, 750}, // button 3
                     {4, 350, 400}, // button 4
                     {5, 200, 250}, // button 5
                     {6, 91, 92}}; // button 6

uint16_t g_ButtonValue = 0;
int label = 0;  // for reporting the button label
int counter = 0; // how many times we have seen new value
int debounce_count = 50; // number of millis/samples to consider before declaring a debounced input
int current_state = 0;  // the debounced input value

char szDisp[255] = {0};
uint8_t g_Menu = 0;
uint8_t g_MenuItem = 0;

// objects
i2c* g_I2C;


// CTC interrupt for Timer 1
volatile int interval1;
volatile int counter1 = 0;
float sec1 = 1;

ISR(TIMER1_COMPA_vect)
{
    interval1 = (int) (sec1 * 10);
    if (counter1 == interval1)
    {
        counter1 = 0;
        g_ButtonValue = readADC(0);
        if(g_ButtonValue == current_state && counter >0)
        {
            counter--;
        }
        if(g_ButtonValue != current_state)
        {
            counter++;
        }
        // If ButtonVal has shown the same value for long enough let's switch it
        if (counter >= debounce_count)
        {
            counter = 0;
            current_state = g_ButtonValue;
            //Checks which button or button combo has been pressed
            if (g_ButtonValue > 0)
            {
                checkButton();
            }
        }
    }
    counter1++;
}

void checkButton()
{
    // loop for scanning the button array.
    for(uint8_t buttonIndex = 0; buttonIndex <= 21; buttonIndex++)
    {
        // checks the ButtonVal against the high and low vales in the array
        if(g_ButtonValue >= g_Button[buttonIndex][1] && g_ButtonValue <= g_Button[buttonIndex][2])
        {
            label = g_Button[buttonIndex][0];
//            action();
        }
    }
}

uint8_t readEEPROM(uint16_t address)
{
    uint8_t data = 0x00;
    g_I2C->start();
    g_I2C->selectSlave(I2C_EEPROM_1, I2C_WRITE); // 0/1 reversed
    //if (g_I2C->selectSlave(I2C_EEPROM_1, I2C_WRITE) == SUCCESS)
    {
        sprintf(szDisp,"re sel: %X\n", g_I2C->getStatus());
        lcd.lcd_string(szDisp);
        //sprintf(szDisp,"read select success\n");
        //lcd.lcd_string(szDisp);
        g_I2C->write((uint8_t)address >> 8);
        g_I2C->write((uint8_t)address & 0xFF);
        if (g_I2C->getStatus() != 0x28)
        {
            sprintf(szDisp,"wr f: %X\n", g_I2C->getStatus());
            lcd.lcd_string(szDisp);
        }
        g_I2C->start();
        data = g_I2C->read(false); // read only 1 byte so ack = false
        //sprintf(szDisp,"read: %X\n", g_I2C->getStatus());
        //lcd.lcd_string(szDisp);
        if (g_I2C->getStatus() != 0x58) // 0x50 when ack = true
        //if (g_I2C->getStatus() != 0x50) // 0x50 when ack = true
        {
        }
    }
/*    else
    {
        sprintf(szDisp,"read select fail\n");
        lcd.lcd_string(szDisp);
        //l_Usart->putString((uint8_t*)"slaveSelect i2c slave 1 read failed. status: " + l_I2c->getStatus() + (uint8_t*)"\r\n");
    }*/
    g_I2C->stop();
    return data;
}

void writeEEPROM(uint16_t address, uint8_t data)
{
    g_I2C->start();
    g_I2C->selectSlave(I2C_EEPROM_1, I2C_READ); // 0/1 reversed
    //if (g_I2C->selectSlave(I2C_EEPROM_1, I2C_WRITE) == SUCCESS)
    {
        sprintf(szDisp,"wr sel: %X\n", g_I2C->getStatus());
        lcd.lcd_string(szDisp);
        g_I2C->write((uint8_t)address >> 8);
        g_I2C->write((uint8_t)address & 0xFF);
        if (g_I2C->getStatus() != 0x28)
        {
            sprintf(szDisp,"wr f: %X\n", g_I2C->getStatus());
            lcd.lcd_string(szDisp);
        }
        g_I2C->write(data);
        if (g_I2C->getStatus() != 0x28)
        {
        }
    }
/*    else
    {
        sprintf(szDisp,"write select fail\n");
        lcd.lcd_string(szDisp);
    }*/
    g_I2C->stop();
}

void action()
{
    if(label == 1)
    {
        sprintf(szDisp,"up button \n");
        lcd.lcd_string(szDisp);
    }
  if(label == 2)
  {
        sprintf(szDisp,"down button \n");
        lcd.lcd_string(szDisp);
  }
  if(label == 3)
  {
        sprintf(szDisp,"left button \n");
        lcd.lcd_string(szDisp);
  }
  if(label == 4)
  {
        sprintf(szDisp,"right button \n");
        lcd.lcd_string(szDisp);
  }
  if(label == 5)
  {
        sprintf(szDisp,"action button 1\n");
        lcd.lcd_string(szDisp);
  }
  if(label == 6)
  {
        sprintf(szDisp,"action button 2\n");
        lcd.lcd_string(szDisp);
  }
}

uint16_t readADC(uint8_t ADCchannel)
{
    //select ADC channel with safety mask
    ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
    //single conversion mode
    ADCSRA |= (1<<ADSC);
    // wait until ADC conversion is complete
    while( ADCSRA & (1<<ADSC) );
    return ADC;
}

int main(void)
{
    // lcd
    lcd.lcd_init(); // init the LCD screen
    lcd.lcd_clrscr(); // initial screen cleanup
    lcd.lcd_home();
    sprintf(szDisp,"booting\n");
    lcd.lcd_string(szDisp);
    // lcd

    // adc
    // Select Vref=AVcc
    ADMUX |= (1<<REFS0);
    //set prescaller to 128 and enable ADC
    ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);
    // adc


    // timer
    // setup timer 1 for CTC
    TCCR1B |= (1 << WGM12); // MAX counter = value OCR1A (Mode 4 / CTC)

    //TCCR1B |= 0x01; // prescaler = 1; // TCCR1B |= (1 << CS10);
    TCCR1B |= 0x02; // prescaler = 8; // TCCR1B |= (1 << CS11);
    //TCCR1B |= 0x03; // prescaler = 64; // TCCR1B |= (1 << CS11) | (1 << CS10);
    //TCCR1B |= 0x04; // prescaler = 256; // TCCR1B |= (1 << CS12);
    //TCCR1B |= 0x05; // prescaler = 1024; // TCCR1B |= (1 << CS12) | (1 << CS10);

    // setup period
    // when OCR1A = 2400 and prescaler = 8, TIMER1_COMPA_vect interrupt is triggered 1000 times/sec
    // because: 12000000 / 8 / 2400 = 1000;
    // 16000000 / 8 / 2000 = 1000
    OCR1A = 2000; // OCR1A is 16 bit, so max 65535

    // trigger interrupt when Timer1 == OCR1A
    TIMSK1 = 1 << OCIE1A;


    // start timer and interrupts
    sei();
    // timer


    /*
    // USART
    usart* l_Usart;
    uint8_t l_UsartReadBuf;
    //uint8_t l_UsartWriteBuf[512];

    l_Usart = new usart();
    l_Usart->init((F_CPU / (9600 * 16UL)));
    //strcpy(l_UsartWriteBuf, "usart init done\r\n");
    // *l_UsartWriteBuf = (uint8_t)*"usart init done\r\n";
    // l_Usart->write("char data");
    //l_Usart->putString(l_UsartWriteBuf);
    l_Usart->putString((uint8_t*)"usart init done\r\n");
    l_UsartReadBuf = l_Usart->read();
    */

/*
    // SPI
    spi* l_Spi;
    uint8_t l_SpiReadBuf;

    l_Spi = new spi();
    l_Spi->init(SPI_MODE_1, SPI_MSB, SPI_NO_INTERRUPT, SPI_MSTR_CLK8); // picked random mode, msb first, no interrupt, 1000000Hz

    //l_Usart->putString((uint8_t*)"spi init done\r\n");
    sprintf(szDisp,"spi init done\n");
    lcd.lcd_string(szDisp);

    l_Spi->selectSlave(SPI_SLAVE_1);
    //l_Spi->write((uint8_t)"HELLO SPI SLAVE 1");
    l_Spi->write(0x05);
    l_SpiReadBuf = l_Spi->write(0xFF); // 0xFF no data?
    l_Spi->deSelectSlave(SPI_SLAVE_1);

    //l_Usart->putString(l_SpiReadBuf + (uint8_t*)"\r\n");

    l_Spi->disableSpi();
    // spi
*/

    // I2C
    g_I2C = new i2c();
    //g_I2C->masterInit(0x02, I2C_PS1); // 8000000 / (16 + 2(07)*1) = 400000
    //g_I2C->masterInit(0x0C, I2C_PS1); // 16000000 / (16 + 2(12)*1) = 400000
    g_I2C->masterInit(0x2A, I2C_PS1); // 16000000 / (16 + 2(42)*1) = 100000
    sprintf(szDisp,"i2c master init done\n");
    lcd.lcd_string(szDisp);
/*
    l_I2c->start();
    if (l_I2c->selectSlave(I2C_EEPROM_1, I2C_WRITE) == SUCCESS)
    {
        sprintf(szDisp,"write success\n");
        lcd.lcd_string(szDisp);
        //l_I2c->write((uint8_t)*"HELLO I2C SLAVE 1");
        l_I2c->write(0x00);
        l_I2c->write(0x00);
        l_I2c->write(0b10101010);
        if (l_I2c->getStatus() != 0x28)
        {
            sprintf(szDisp,"write failed: %c\n", l_I2c->getStatus());
            lcd.lcd_string(szDisp);
            //l_Usart->putString((uint8_t*)"write to i2c slave 1 failed. status: " + l_I2c->getStatus() + (uint8_t*)"\r\n");
        }
    }
    else
    {
        sprintf(szDisp,"write fail\n");
        lcd.lcd_string(szDisp);
        //l_Usart->putString((uint8_t*)"slaveSelect i2c slave 1 write failed. status: " + l_I2c->getStatus() + (uint8_t*)"\r\n");
    }
    l_I2c->start();
    if (l_I2c->selectSlave(I2C_EEPROM_1, I2C_READ) == SUCCESS)
    {
        sprintf(szDisp,"read success\n");
        lcd.lcd_string(szDisp);
        l_I2cReadBuf = l_I2c->read(false); // read only 1 byte so ack = false
        if (l_I2c->getStatus() != 0x58) // 0x50 when ack = true
        {
            //l_Usart->putString((uint8_t*)"read from i2c slave 1 failed. status: " + l_I2c->getStatus() + (uint8_t*)"\r\n");
        }
    }
    else
    {
        sprintf(szDisp,"read fail\n");
        lcd.lcd_string(szDisp);
        //l_Usart->putString((uint8_t*)"slaveSelect i2c slave 1 read failed. status: " + l_I2c->getStatus() + (uint8_t*)"\r\n");
    }
    l_I2c->stop();
    // i2c
*/


    fcpu_delay_ms(5000);
    lcd.lcd_clrscr();
    sprintf(szDisp,"inits done\n");
    lcd.lcd_string(szDisp);
    fcpu_delay_ms(5000);
    lcd.lcd_clrscr();
    sprintf(szDisp,"write eeprom\n");
    sprintf(szDisp,"W:%X\n", 0x88);
    lcd.lcd_string(szDisp);
    writeEEPROM(0x0001, 0x88);
    writeEEPROM(0x0002, 0x66);
    writeEEPROM(0x0003, 0x44);
    fcpu_delay_ms(5000);
    lcd.lcd_clrscr();
    sprintf(szDisp,"read eeprom\n");
    lcd.lcd_string(szDisp);
    uint8_t eepromData = 0xEE;
    for(;;)
    {
        lcd.lcd_clrscr();
        eepromData = readEEPROM(0x0002);
        sprintf(szDisp,"R:%X\n", eepromData);
        lcd.lcd_string(szDisp);
        eepromData = readEEPROM(0x0003);
        eepromData = readEEPROM(0x0004);
        eepromData = readEEPROM(0x0005);
        eepromData = readEEPROM(0x0006);
        sprintf(szDisp,"R:%X\n", eepromData);
        lcd.lcd_string(szDisp);
/*        raw = readADC(3);
        button = VREF/1024*raw;
        lcd.lcd_clrscr();
        sprintf(szDisp,"%d/1024 %dV\n", raw, button);
        lcd.lcd_string(szDisp);
        fcpu_delay_ms(500);*/
    fcpu_delay_ms(1000);

        ;
    }
}
