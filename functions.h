//header includes
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xil_exception.h"

//these are lab specific registers (given on the project specs sheet)
#define UART1_CON_Addr 0xE0001000
#define UART1_Mode_Addr 0xE0001004
#define UART1_INT_EN_Addr 0xE0001008
#define UART1_INT_DIS_Addr 0xE000100C
#define UART1_INT_MASK_Addr 0xE0001010
#define UART1_ISR_Addr 0xE0001014
#define UART1_Buad_Gen_Addr 0xE0001018
#define UART1_RT_Addr 0xE000101C
#define UART1_RFIFO_LEV_Addr 0xE0001020
#define UART1_Modem_CNT_Addr 0xE0001024
#define UART1_Modem_Stat_Addr 0xE0001028
#define UART1_C_Stat_Addr 0xE000102C
#define UART1_FIFO_Addr 0xE0001030
#define UART1_Baud_DIV_Addr 0xE0001034
#define UART1_FCD_Addr 0xE0001038
#define UART1_TFIFO_LEV_Addr 0xE0001044


//Various Registers
#define ICCPMR_BASEADDR 0xF8F00104 // Interrupt Priority Mask Register
#define ICCICR_BASEADDR 0xF8F00100 // CPU Interface Control Register
#define ICDDCR_BASEADDR 0xF8F01000 // Distributor Control Register
#define ICDISER_BASEADDR 0xF8F01100 // Interrupt Set Enable Register
#define ICDICER_BASEADDR 0xF8F01180 // Interrupt Clear/Enable Register
#define ICDIPR_BASEADDR 0xF8F01400 // Interrupt Priority Register Base address
#define ICDIPTR_BASEADDR 0xF8F01800 // Interrupt Processor Targets Register
#define ICDICFR_BASEADDR 0xF8F01C00 // Interrupt Configuration Register
#define ICCIAR_BASEADDR 0xF8F0010C // Interrupt Acknowledge Register
#define ICCEOIR_BASEADDR 0xF8F00110 // End Of Interrupt Register
#define GPIO_MTDATA_OUT_0 0xE000A004 // Maskable data out in bank 0
#define GPIO_INT_DIS_0 0xE000A214 // Interrupt disable bank 0
#define GPIO_INT_EN_1 0xE000A250 // Interrupt enable bank 1
#define GPIO_INT_DIS_1 0xE000A254 // Interrupt disable bank 1
#define GPIO_INT_STAT_1 0xE000A258 // Interrupt status bank 1
#define GPIO_INT_TYPE_1 0xE000A25C // Interrupt type bank 1
#define GPIO_INT_POL_1 0xE000A260 // Interrupt polarity bank 1
#define GPIO_INT_ANY_1 0xE000A264 // Interrupt any edge sensitive bank 1
#define GT_COUNTER0_ADDRESS 0xF8F00200
#define GT_COUNTER1_ADDRESS 0xF8F00204
#define GT_CONTROL_ADDRESS 0xF8F00208
#define GT_INTSTAT_ADDRESS 0xF8F0020C
#define GT_COMP0_ADDRESS 0xF8F00210
#define GT_COMP1_ADDRESS 0xF8F00214
#define GT_AUTOINC_ADDRESS 0xF8F00218
#define SVN_SEG_CTRL 0X4BB03000
#define DIG1_ADDRESS (0X4BB03000 + 4)
#define DIG2_ADDRESS (0X4BB03000 + 8)
#define DIG3_ADDRESS (0X4BB03000 + 12)
#define DIG4_ADDRESS (0X4BB03000 + 16)
#define SVN_SEG_DP (0X4BB03000 + 20)
#define MIO_PIN_16 0xF8000740
#define MIO_PIN_17 0xF8000744
#define MIO_PIN_18 0xF8000748
#define MIO_PIN_50 0xF80007C8
#define MIO_PIN_51 0xF80007CC
#define GPIO_DIRM_0 0xE000A204 // Direction mode bank 0
#define GPIO_OUTE_0 0xE000A208 // Output enable bank 0
#define GPIO_DIRM_1 0xE000A244 // Direction mode bank 1

//global variables
uint8_t D1 =0;
uint8_t D2 =0;
uint8_t D3 =0;
uint8_t D4 =0;
uint8_t START =0;


//declaration of functions
void Initialize_SVD();
void Initialize_UART1();
void disable_interrupts();
void configure_GIC_GPIO();
void IRQ_Handler(void *data);
void MY_GPIO_IRQ(uint32_t button_press);
void delay(int i);
void Initialize_GPIO_Interrupts();
void enable_interrupts();
void SendChar(uint8_t C);
void MY_GT_IRQ();

void Initialize_SVD()
{
	*((uint32_t*)SVN_SEG_CTRL) = 0x9;
	*((uint32_t*)DIG1_ADDRESS) = 0x0;
	*((uint32_t*)DIG2_ADDRESS) = 0x0;
	*((uint32_t*)DIG3_ADDRESS) = 0x0;
	*((uint32_t*)DIG4_ADDRESS) = 0x0;
	*((uint32_t*)SVN_SEG_DP) = 0x1;
	*((uint32_t*)D1) =0;
	*((uint32_t*)D2) =0;
	*((uint32_t*)D3) =0;
	*((uint32_t*)D4) =0;
	START=0;
	//I think I need to add some code here that
	return;
}


void Initialize_UART1(){
*((uint32_t*) UART1_CON_Addr) = 0x0000000; // disable transmitter and receiver
*((uint32_t*) UART1_Buad_Gen_Addr) = 0x7C; // Baud Gen
*((uint32_t*) UART1_Baud_DIV_Addr) = 0x6; // Baud Divider
*((uint32_t*) UART1_CON_Addr) = 0x0000117; // disable transmitter and receiver
// Normal mode, single stop bit, no parity 8 bits data
*((uint32_t*) UART1_Mode_Addr) = 0x0000030;
*((uint32_t*) UART1_INT_DIS_Addr) = 0x000FFFF; // Disable interrupt
*((uint32_t*) UART1_RT_Addr) = 0xFF; // Reciever Time out
}


void disable_interrupts()
{

	uint32_t mode = 0xDF; // System mode [4:0] and IRQ disabled [7], D == 1101, F == 1111
	//what does this mean???
	uint32_t read_cpsr=0; // used to read previous CPSR value, read status register values
	uint32_t bit_mask = 0xFF; // used to clear bottom 8 bits
	__asm__ __volatile__("mrs %0, cpsr\n" : "=r" (read_cpsr) ); // execute the assembly instruction MSR
	__asm__ __volatile__("msr cpsr,%0\n" : : "r" ((read_cpsr & (~bit_mask))| mode)); // only change the
	//lower 8 bits
	return;
}

void configure_GIC_GPIO()
{
	*((uint32_t*) ICDIPTR_BASEADDR+13) = 0x00000000;
	*((uint32_t*) ICDICER_BASEADDR+1) = 0x00000000;
	*((uint32_t*) ICDDCR_BASEADDR) = 0x0;
	*((uint32_t*) ICDIPR_BASEADDR+13) = 0x000000A0;
	*((uint32_t*) ICDIPTR_BASEADDR+13) = 0x00000001;
	*((uint32_t*) ICDICFR_BASEADDR+3) = 0x55555555;
	*((uint32_t *) ICDISER_BASEADDR+1) = 0x00100000;
	*((uint32_t*) ICCPMR_BASEADDR) = 0xFF;
	*((uint32_t*) ICCICR_BASEADDR) = 0x3;
	*((uint32_t*) ICDDCR_BASEADDR) = 0x1;
	return;
//add your own code here

}

//this function needs to be edited
void IRQ_Handler(void *data)
{
uint32_t interrupt_ID = *((uint32_t*)ICCIAR_BASEADDR);
	if (interrupt_ID == 52) //checking if the interrupt is from the GPIO
	{
	uint32_t GPIO_INT = *((uint32_t*)GPIO_INT_STAT_1);
	uint32_t button_press = 0xC0000 & GPIO_INT;
	MY_GPIO_IRQ( button_press); // Deals with BTNs
	}
	else if (interrupt_ID == 27)
	{
	MY_GT_IRQ(); // updates the seven segment display.
	}
*((uint32_t*)ICCEOIR_BASEADDR) = interrupt_ID; // Clears the GIC flag bit.
}

//this function was given to us
void MY_GPIO_IRQ(uint32_t button_press){
uint32_t BTN5=0x80000;
uint32_t BTN4=0x40000;
delay(100000); // button rebounce
if (button_press == BTN4){
uint8_t C = 13;
SendChar(C);
C='E';SendChar(C);
C='C';SendChar(C);
C='H';SendChar(C);
C='O';SendChar(C);
C='>';SendChar(C);
C='>';SendChar(C);
}
else if (button_press == BTN5){ // used for debugging
uint8_t C = 'T';
SendChar(C);
C = 'E';
SendChar(C);
C = 'S';
SendChar(C);
C = 'T';
SendChar(C);
}
*((uint32_t*)GPIO_INT_STAT_1) = 0xFFFFFF;
}

//not sure if i need this function
void delay(int i)
{
int k = 0;
while (k<i)
k++;
}

void Initialize_GPIO_Interrupts(){
*((uint32_t*) GPIO_INT_DIS_1) = 0xFFFFFFFF;
*((uint32_t*) GPIO_INT_DIS_0) = 0xFFFFFFFF;
*((uint32_t*) GPIO_INT_STAT_1) = 0xFFFFFFFF; // Clear Status register
*((uint32_t*) GPIO_INT_TYPE_1) = 0x0C0000; // Type of interrupt rising edge
*((uint32_t*) GPIO_INT_POL_1) = 0x0C0000; // Polarity of interrupt
*((uint32_t*) GPIO_INT_ANY_1) = 0x000000; // Interrupt any edge sensitivity
*((uint32_t*) GPIO_INT_EN_1) = 0x0C0000; // Enable interrupts in bank 0
return;
}

void enable_interrupts(){
uint32_t read_cpsr=0; // used to read previous CPSR value
uint32_t mode = 0x5F; // System mode [4:0] and IRQ enabled [7]
uint32_t bit_mask = 0xFF; // used to clear bottom 8 bits
__asm__ __volatile__("mrs %0, cpsr\n" : "=r" (read_cpsr) );
__asm__ __volatile__("msr cpsr,%0\n" : : "r" ((read_cpsr & (~bit_mask))| mode));
return;
}

void SendChar(uint8_t C){
*((uint32_t*) UART1_FIFO_Addr) = C; // Disable interrupt
}

void MY_GT_IRQ(){
 D4=D4+1;
 if (D4<10)
	 *((uint32_t*) DIG4_ADDRESS) = D4;
 else{
	 D4=0;
	 *((uint32_t*) DIG4_ADDRESS) = D4;
	 D3=D3+1;
	 if (D3<10)
		 *((uint32_t*) DIG3_ADDRESS) = D3;
	 else{
		 D3=0;
		 *((uint32_t*) DIG3_ADDRESS) = D3;
		 D2=D2+1;
		 if (D2<10)
			 *((uint32_t*) DIG2_ADDRESS) = D2;
		 else{
			 D2=0;
			 *((uint32_t*) DIG2_ADDRESS) = D2;
			 D1= D1+1;
			 if (D1<10)
				 *((uint32_t*) DIG1_ADDRESS) = D1;
			 else {
				 D1=0;
				 *((uint32_t*) DIG1_ADDRESS) = D1;
			 }
		 }
	 }
 }
*((uint32_t*) GT_COUNTER0_ADDRESS) = 0x00000000; // reset Counter
*((uint32_t*) GT_COUNTER1_ADDRESS) = 0x00000000;
*((uint32_t*) GT_CONTROL_ADDRESS) = 0x010F; // Start Timer
*((uint32_t*) GT_INTSTAT_ADDRESS) = 0x1; // clear Global Timer Interrupt Flag bit.
}
