#include "xparameters.h"
#include "xscutimer.h"
#include "xuartps.h"
#include "xgpiops.h"
#include "xgpio.h"




#define AES_UNIT_BASEADDR 0x8AA00000
#define GPIO_BASEADDR 0x41200000
#define DDR_BASEADDR XPAR_PS7_DDR_0_S_AXI_BASEADDR

int oldmain()
{
    print("Starting main\n\r");
	// Do a test write to the DDR memory
    u32 val = Xil_In32(DDR_BASEADDR);
	Xil_Out32(DDR_BASEADDR, 0xdeadbeef);
	val = Xil_In32(DDR_BASEADDR);

	//Xil_Out32(AES_UNIT_BASEADDR+4, 0xacab);

	// Configure AXI GPIO
	XGpio_Config *gpioConfig = XGpio_LookupConfig(XPAR_AXI_GPIO_0_DEVICE_ID);
    XGpio gpio;
    XGpio_CfgInitialize(&gpio, gpioConfig, gpioConfig->BaseAddress);
    u32 dir = XGpio_GetDataDirection(&gpio, 1);
    dir = XGpio_GetDataDirection(&gpio, 2);
    XGpio_SetDataDirection(&gpio, 2, 0x00);

    while(1) {
    	val = XGpio_DiscreteRead(&gpio, 1);
    	XGpio_DiscreteWrite(&gpio, 2, val);
    }

	/*while (1) {
		val = Xil_In32(GPIO_BASEADDR);
	}*/


    int status;

    XUartPs_Config *uartConfig = XUartPs_LookupConfig(XPAR_PS7_UART_1_DEVICE_ID);
    XUartPs uart;
    status =  XUartPs_CfgInitialize(&uart, uartConfig, uartConfig->BaseAddress);

    status = XUartPs_SelfTest(&uart);
    //XUartPs_SetBaudRate(&uart, 9600);
    XUartPsFormat format;
    XUartPs_GetDataFormat(&uart, &format);
    u8 str[8] = {49, 50, 51, 52, 53, 54, 10, 13};
    XUartPs_Send(&uart, str, 8);

    // Configure LED as output
    XGpioPs_Config *GPIO_Config = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
    XGpioPs my_Gpio;
    status = XGpioPs_CfgInitialize(&my_Gpio, GPIO_Config, GPIO_Config->BaseAddr);
    XGpioPs_SetDirectionPin(&my_Gpio, 7, 1);
    XGpioPs_WritePin(&my_Gpio, 7, 1);


    // Configure timer
    XScuTimer_Config *timerConfig = XScuTimer_LookupConfig(XPAR_PS7_SCUTIMER_0_DEVICE_ID);
    XScuTimer timer;
    status = XScuTimer_CfgInitialize(&timer, timerConfig, timerConfig->BaseAddr);

    XScuTimer_LoadTimer(&timer, 100000000);
    //print("Starting timer...\n\r");
    XScuTimer_Start(&timer);

    // Switch LED on and off
    int isLedOn = 0;
    while(1)
    {
	    isLedOn = !isLedOn;
		XGpioPs_WritePin(&my_Gpio, 7, isLedOn);
	    XScuTimer_RestartTimer(&timer);
	    while (XScuTimer_GetCounterValue(&timer) > 0)
				;

     }



    xil_printf("Successfully ran the application");
    return 0;
}
