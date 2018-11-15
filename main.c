#include "functions.h"
int main()
{
 init_platform();
 Initialize_UART1();
 disable_interrupts();
 configure_GIC_GPIO();
 Initialize_GPIO_Interrupts();
 enable_interrupts();
 SEND_READY(); //not sure about this function in general, idk what it's meant to do
 Xil_ExceptionRegisterHandler(5, IRQ_Handler, NULL);// build-in
 while(1){
 uint32_t R= *((uint32_t*) UART1_C_Stat_Addr);
 if ((R && 0x0002)== 0x0){
 uint8_t C = *((uint32_t*) UART1_FIFO_Addr);
 SendChar(C);
 }
 }
 cleanup_platform();
 return 0;
}
