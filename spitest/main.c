//******************************************************************************
//                      Motion Detection
//                        MSP430F2013
//                       time: 2014.09
//                       threshold: 50
//******************************************************************************
//******************************************************************************
#include  <msp430x20x3.h>
//BIT define
#define   LED_OUT         BIT0
#define   SPI_CS          BIT4
#define   SENSOR_PWR      BIT7              // power to sensor 2.7

//consts define
#define   THRESHOLD       30             // Threshold for motion
#define   DELAYNUM        100
#define   DELAYNUM2       200
#define   DATANUM         32

#define LED_1 (P1OUT |= BIT0)
#define LED_0 (P1OUT &= ~BIT0)
//spi define
#define SDOUT_1 (P1OUT |= BIT6)
#define SDOUT_0 (P1OUT &= ~BIT6)        //P1.6 dout
#define SCK_1 (P1OUT |= BIT5)
#define SCK_0 (P1OUT &= ~BIT5)        //P1.5
#define CS_1 (P1OUT |= BIT4)
#define CS_0 (P1OUT &= ~BIT4)        //P1.4

//globle veriable
static unsigned int rid = 0;         // Storage for last conversion
static unsigned int result_old = 0;         // Storage for last conversion
static unsigned int result_new = 0;         // storage for this conversion
static unsigned int result = 0;             // |result_new-result_old
static unsigned int i=0;
static unsigned int k;
static unsigned int block = 0x00;
static unsigned int sector = 0x00;
static unsigned int addr = 0x00;

//static unsigned int data[DATANUM];
unsigned char Wbuff[DATANUM];

//SPI function
void Init_SPI(void);
unsigned char Readbyte(unsigned char wdata);
void SendByte(unsigned char wdata);
void ReadId(void);
void Read(unsigned int addr1,unsigned int addr2,unsigned int addr3);
void writePage(unsigned int addr1,unsigned int addr2,unsigned int addr3);
void writeEnable(void);
void writeEnd(void);

void writeSR(void);
void readSR(void);

void EraseBlock();
//SPI function

void main(void)
{
  //aclk=vlo/4,vlo=12kHz,mclk=dco,smclk=dco
  WDTCTL = WDTPW+WDTTMSEL+WDTCNTCL+WDTSSEL; // timer,ACLK*2^15  定时器，确定频率

  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO to 1MHz
  DCOCTL = CALDCO_1MHZ;
  BCSCTL1 |= DIVA_2;                        // ACLK = VLO/4
  BCSCTL3 |= LFXT1S_2;

  P1OUT = 0x10;                             // P1OUTs P1.4为输出，默认out为1，其他均默认输出0
  P1DIR = 0xFF;                             // Unused pins as outputs  P1.4为spi片选端,P1.0为输出
  P1REN |= 0x10;                          // P1.4 pullup
  P1SEL = 0x08;                             // Select VREF function   功能选择P1.3为VREF

  P2OUT = 0x00 + SENSOR_PWR;                // P2.7=1
  P2SEL &= ~SENSOR_PWR;
  P2DIR = 0xFF;

  SD16CTL = SD16VMIDON + SD16REFON + SD16SSEL_1;// 1.2V ref, SMCLK
  SD16INCTL0 = SD16GAIN_32 + SD16INCH_4;     // PGA = 16x, Diff inputs A4- & A4+
  SD16CCTL0 =  SD16SNGL + SD16IE;           // Single, 256OSR, Int enable
  SD16CTL &= ~SD16VMIDON;                   // VMID off: used to settle ref cap
  SD16AE = SD16AE1 + SD16AE2;               // P1.1 & P1.2: A4+/- SD16_A inputs

  Init_SPI();

  // Wait for PIR sensor to settle: 1st WDT+ interval
  P1SEL |= LED_OUT;                         // Turn LED on with ACLK
  while(!(IFG1 & WDTIFG));                  // ~5.4s delay: PIR sensor settling
  P1SEL &= ~LED_OUT;                        // Turn LED off with ACLK

  WDTCTL = WDTPW+WDTTMSEL+WDTCNTCL+WDTSSEL+WDTIS1+WDTIS0;// timer,ACLK*2^6
  BCSCTL1 |= DIVA_2;                        // ACLK = VLO/8
  IE1 |= WDTIE;                             // Enable WDT interrupt

  i=0;

  for(k=0;k<DELAYNUM;k++);
  ReadId();

  for(k=0;k<DELAYNUM;k++);
  EraseBlock();


  SD16CTL |= SD16REFON;                   // If no, turn on SD16_A ref
  SD16CCTL0 |= SD16SC;                    // Set bit to start new conversion


  _BIS_SR(LPM0_bits + GIE);                 // Enter LPM3 with interrupts
}
//******************************************************************************
//                      SD16_A interrupt service
//******************************************************************************
#pragma vector = SD16_VECTOR
__interrupt void SD16ISR(void)
{
  SD16CTL &= ~SD16REFON;                    // Turn off SD16_A ref
  result_new = SD16MEM0; // Save result (clears IFG)

  if (result_new > result_old)               // Get difference between samples
    result = result_new - result_old;
  else
    result = result_old - result_new;
  if (result > THRESHOLD)               // If motion detected...
     LED_1;                      // Turn LED on --LED high

  Wbuff[i] = result_new & 0x00FF;
  //Wbuff[i+1] = result_new >> 8;
  if(i == DATANUM -1){
	  writePage(block,sector,addr);
	  if(addr == 0xE0 ){
		  addr = 0x00;
		  if(sector == 0xFF ){
			  sector = 0x00;
			  block = block + 0x01;
		  }else{
			  sector = sector + 0x01;
		  }
	  }else{
		  addr = addr + 0x20;
	  }
	  i=0;
  }else{
    i++;
  }
  result_old = result_new;                    // Save last conversion

  SD16CTL |= SD16REFON;                   // If no, turn on SD16_A ref
  SD16CCTL0 |= SD16SC;                    // Set bit to start new conversion

  __bis_SR_register_on_exit(SCG0);     // Return to LPM3 after reti
}

//******************************************************************************
//                      Watchdog Timer interrupt service
//******************************************************************************
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{

    LED_0;                      // If yes, turn off LED, next loop --LED high

}

// ***********************************
// * SPI function
// ***********************************
void Init_SPI(void)
{
    P1DIR &= ~BIT7;                 //SO din
    P1DIR |= BIT4 + BIT5 + BIT6;    //CS CLK SI dout
    P1OUT |= BIT4 + BIT5 + BIT6;
    CS_1;                            //CS High
    SCK_0;                           //CLK low
}
unsigned char Readbyte(unsigned char wdata)
{
    unsigned char i;
    unsigned char rdata=0;

    for (i=0; i<8; i++)
    {
        if (wdata & BIT7)
            SDOUT_1;
        else
            SDOUT_0;
        SCK_1;
        wdata<<=1;
        _NOP();
        rdata<<=1;
        if (P1IN&BIT7)
          rdata |= 0x01;
        else
          rdata &= ~0x01;
        SCK_0;
    }
    return rdata;
}

void SendByte(unsigned char wdata)
{
    unsigned char i;
    unsigned char rdata=0;

    for (i=0; i<8; i++)
    {
        if (wdata & BIT7)
            SDOUT_1;
        else
            SDOUT_0;
        SCK_1;
        wdata<<=1;
        _NOP();
        rdata<<=1;
        if (P1IN&BIT7)
          rdata |= 0x01;
        else
          rdata &= ~0x01;
        SCK_0;
    }
}

void ReadId(void){

  CS_0;

  for(k=0;k<DELAYNUM;k++);
  SendByte(0x9F);
  SendByte(0x00);
  SendByte(0x00);
  SendByte(0x00);
  for(k=0;k<DELAYNUM2;k++);

  CS_1;

}


void Read(unsigned int addr1,unsigned int addr2,unsigned int addr3){

  CS_0;
  
  for(k=0;k<DELAYNUM;k++);
  SendByte(0x03);

  for(k=0;k<10;k++);
  SendByte(addr1);
  SendByte(addr2);
  SendByte(addr3);
  for(k=0;k<256;k++){
    SendByte(0xFF);
  }
  for(k=0;k<DELAYNUM2;k++);

  CS_1;
}

void writePage(unsigned int addr1,unsigned int addr2,unsigned int addr3)
{

  writeSR();
  for(k=0;k<DELAYNUM;k++);
  readSR();

  for(k=0;k<DELAYNUM;k++);
  writeEnable();

  for(k=0;k<DELAYNUM;k++);
  CS_0;
  for(k=0;k<DELAYNUM;k++);
  SendByte(0x02);
  for(k=0;k<10;k++);
  SendByte( addr1 );
  SendByte( addr2 );
  SendByte( addr3 );
  for(k=0;k<DATANUM;k++){
      SendByte(Wbuff[k]);
    }

  for(k=0;k<DELAYNUM2;k++);
  CS_1;

  for(k=0;k<DELAYNUM;k++);
  writeEnd();
}


void writeEnable(void){
  CS_0;

  for(k=0;k<DELAYNUM;k++);
  SendByte(0x06);
  for(k=0;k<DELAYNUM2;k++);

  CS_1;
}

void writeEnd(void){
  CS_0;

  for(k=0;k<DELAYNUM;k++);
  SendByte(0x04);
  for(k=0;k<DELAYNUM2;k++);

  CS_1;
}

//************************************************

void writeSR(void){
  CS_0;

  for(k=0;k<DELAYNUM;k++);
  SendByte(0x01);
  SendByte(0x01);
  for(k=0;k<DELAYNUM;k++);

  CS_1;
}

void readSR(void){
  unsigned char srdata = 0xFF;
  CS_0;

  while(srdata){
  for(k=0;k<DELAYNUM;k++);
  SendByte(0x05);
  SendByte(0x03);
  srdata = Readbyte(0x03);
  }
  for(k=0;k<DELAYNUM2;k++);
  CS_1;
}

void EraseBlock()//64k擦除
{
  for(k=0;k<100;k++);
  CS_0;
  SendByte(0x06);
  CS_1;

  for(k=0;k<100;k++);
  CS_0;
  SendByte(0xD8);
  SendByte(0x00);
  SendByte(0x00);SendByte(0x00);

  CS_1;

  for(k=0;k<100;k++);
  writeEnd();

  for(k=0;k<100;k++);
  readSR();
}


















