// Graphics library by ladyada/adafruit with init code from Rossum
// MIT license

#pragma once
#include <Adafruit_GFX.h>
#include <libmaple/gpio.h>
#include "DSO_config.h"
#include "MapleFreeRTOS1000_pp.h"

extern xMutex PortAMutex;


/*****************************************************************************/
// Define pins and Output Data Registers
/*****************************************************************************/
// Data port
#define TFT_DATA_PORT	GPIOB
// Data bits/pins
#define TFT_DATA_SHIFT 0 // take the lower bits/pins 0..7
//#define TFT_DATA_SHIFT 8 // take the higher bits/pins 8..15

// This is the DSO 150 mapping
#define TFT_RD			PA6
#define TFT_WR			PC15
#define TFT_RS			PC14
#define TFT_CS			PC13
#define TFT_RST			PB9

#define TFT_RD_MASK    (0x0001 << 6)
#define TFT_WR_MASK    (0x0001 << 15)
#define TFT_RS_MASK    (0x0001 << 14)
#define TFT_CS_MASK    (0x0001 << 13)
#define TFT_RST_MASK   (0x0001 << 9)

// use fast bit toggling, very fast speed!
#define RD_ACTIVE    { GPIOA->regs->BRR  = TFT_RD_MASK; }
#define RD_IDLE      { GPIOA->regs->BSRR = TFT_RD_MASK; }
#define WR_ACTIVE    { GPIOC->regs->BRR  = TFT_WR_MASK; }
#define WR_IDLE      { GPIOC->regs->BSRR = TFT_WR_MASK; }
#define CD_COMMAND   { GPIOC->regs->BRR  = TFT_RS_MASK; }
#define CD_DATA      { GPIOC->regs->BSRR = TFT_RS_MASK; }
#define RST_HIGH     { GPIOB->regs->BSRR = TFT_RST_MASK; }
#define RST_LOW      { GPIOB->regs->BRR  = TFT_RST_MASK; }
#define CS_IDLE	 { GPIOC->regs->BSRR = TFT_CS_MASK; }

#ifndef USE_RXTX_PIN_FOR_ROTARY

#define CS_ACTIVE  { intReg = EXTI_BASE->IMR;\
                    opReg = GPIOB->regs->ODR;\
                    EXTI_BASE->IMR = 0 ; \
                    GPIOB->regs->CRL = 0x33333333 ;\
                    GPIOC->regs->BRR  = TFT_CS_MASK; }

#define CS_IDLE    { GPIOB->regs->ODR = opReg;\
                    GPIOB->regs->CRL = 0x88888888; \
                    GPIOC->regs->BSRR = TFT_CS_MASK ; \
                    EXTI_BASE->IMR = intReg; }


#else // when RX/TX pins are used for rotary encoder, no need to mask interrupts
#define CS_ACTIVE  {  \        
                    PortAMutex.lock(); \
                    opReg = GPIOB->regs->ODR;\                    
                    GPIOB->regs->CRL = 0x33333333 ;\
                    GPIOC->regs->BRR  = TFT_CS_MASK; }

#define CS_IDLE    { \                    
                    GPIOB->regs->ODR = opReg;\
                    GPIOB->regs->CRL = 0x88888888; \
                    GPIOC->regs->BSRR = TFT_CS_MASK ; \
                    PortAMutex.unlock(); \
                    }


#endif

#define CS_ACTIVE_CD_COMMAND {CS_ACTIVE;CD_COMMAND}
#define WR_STROBE { WR_ACTIVE; WR_IDLE; }


#if (TFT_DATA_SHIFT==0)
    //#warning "Using lower data nibble..."
    // set the pins to input mode
    #define setReadDir() ( dataRegs->CRL = 0x88888888 )	// set the lower 8 bits as input
    // set the pins to output mode
    #define setWriteDir() ( dataRegs->CRL = 0x33333333 )	// set the lower 8 bits as output

    // set pins to output the 8 bit value

        //#define write8(c) { dataRegs->BSRR = (uint32_t)(0x00FF0000 + ((c)&0xFF)); WR_STROBE; }
       #define write8(c) { dataRegs->BSRR = (((c^0xFF)<<16) | (c)); WR_STROBE; }

        // inline void write8(uint8_t d) { dataRegs->BSRR = (uint32_t)(0x00FF0000 + d); WR_STROBE; };

#elif (TFT_DATA_SHIFT==8)
    #warning "Using high data nibble..."
    // set the pins to input mode
    #define setReadDir() ( dataRegs->CRH = 0x88888888 )	// set the upper 8 bits as input
    // set the pins to output mode
    #define setWriteDir() ( dataRegs->CRH = 0x33333333 )	// set the lower 8 bits as output


  #define write8(c) {  dataRegs->BSRR = (uint32_t)(0xFF000000 + (((c)&0xFF)<<TFT_DATA_SHIFT)); WR_STROBE; }
  // inline void write8(uint8_t d) { dataRegs->BSRR = (uint32_t)(0x00FF0000 + d); WR_STROBE; };

#else
  #error Invalid data shift selected! Please set to '0' for low nibble, or to '8' for high nibble
#endif


/*****************************************************************************/


#define read8(x) ( x = read8_() )

/**
 * 
 * @param r
 * @return 
 */     

extern uint16_t readReg(uint8_t r);
extern uint32_t readReg32(uint8_t r);
extern void writeCommand(uint16_t c);
extern void writeRegister8(uint16_t a, uint8_t d);
extern void writeRegister16(uint16_t a, uint16_t d);
extern void writeRegister24(uint16_t a, uint32_t d);
extern void writeRegister32(uint16_t a, uint32_t d);
extern void writeRegisterPair(uint16_t aH, uint16_t aL, uint16_t d);
extern uint8_t read8_(void);
/**
 */
extern uint32_t intReg;
extern uint32_t opReg;
extern gpio_reg_map * dataRegs;

// EOF


