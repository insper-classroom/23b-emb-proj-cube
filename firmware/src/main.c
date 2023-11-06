/************************************************************************
* 5 semestre - Eng. da Computao - Insper
*
* 2021 - Exemplo com HC05 com RTOS
*
*/

#include <asf.h>
#include "conf_board.h"
#include <string.h>

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LEDs
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      19
#define LED_IDX_MASK (1 << LED_IDX)

// botao de acionamento
#define BUT_PIO     PIOA
#define BUT_PIO_ID  ID_PIOA
#define BUT_PIO_PIN 11
#define BUT_PIO_PIN_MASK (1u << BUT_PIO_PIN)

#define BUT_PIO_1		PIOD
#define BUT_PIO_ID_1		ID_PIOD
#define BUT_PIO_IDX_1		11
#define BUT_PIO_IDX_MASK_1	(1u<< BUT_PIO_IDX_1)

#define BUT_PIO_2		PIOA
#define BUT_PIO_ID_2		ID_PIOA
#define BUT_PIO_IDX_2		6
#define BUT_PIO_IDX_MASK_2	(1u<< BUT_PIO_IDX_2)

#define BUT_PIO_3		PIOA
#define BUT_PIO_ID_3		ID_PIOA
#define BUT_PIO_IDX_3		24
#define BUT_PIO_IDX_MASK_3	(1u<< BUT_PIO_IDX_3)

#define BUT_PIO_4		PIOD
#define BUT_PIO_ID_4		ID_PIOD
#define BUT_PIO_IDX_4		26
#define BUT_PIO_IDX_MASK_4	(1u<< BUT_PIO_IDX_4)

#define BUT_PIO_5		PIOA
#define BUT_PIO_ID_5		ID_PIOA
#define BUT_PIO_IDX_5		4
#define BUT_PIO_IDX_MASK_5	(1u<< BUT_PIO_IDX_5)

#define BUT_PIO_6		PIOA
#define BUT_PIO_ID_6		ID_PIOA
#define BUT_PIO_IDX_6		3
#define BUT_PIO_IDX_MASK_6	(1u<< BUT_PIO_IDX_6)

//AFEC-MOS
#define AFEC_POT AFEC0
#define AFEC_POT_ID ID_AFEC0

#define AFEC_POT_CHANNEL_2 6 // PB3
#define AFEC_POT_CHANNEL_3 7 // PB4



// usart (bluetooth ou serial)
// Descomente para enviar dados
// pela serial debug

//#define DEBUG_SERIAL

#ifdef DEBUG_SERIAL
#define USART_COM USART1
#define USART_COM_ID ID_USART1
#else
#define USART_COM USART0
#define USART_COM_ID ID_USART0
#endif

/************************************************************************/
/* RTOS                                                                 */
/************************************************************************/

#define TASK_BLUETOOTH_STACK_SIZE            (4096/sizeof(portSTACK_TYPE))
#define TASK_BLUETOOTH_STACK_PRIORITY        (tskIDLE_PRIORITY)

#define TASK_ADC_STACK_SIZE (1024 * 10 / sizeof(portSTACK_TYPE))
#define TASK_ADC_STACK_PRIORITY (tskIDLE_PRIORITY)

#define TASK_PROC_STACK_SIZE (1024 * 10 / sizeof(portSTACK_TYPE))
#define TASK_PROC_STACK_PRIORITY (tskIDLE_PRIORITY)



#define TASK_TRIGGER_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_TRIGGER_STACK_PRIORITY            (tskIDLE_PRIORITY)


#define TASK_TC_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_TC_STACK_PRIORITY            (tskIDLE_PRIORITY)

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);
static void USART1_init(void);
static void config_AFEC(Afec *afec, uint32_t afec_id);
static void configure_console(void);
static void but_callback1(void);
static void but_callback2(void);
static void but_callback3(void);
static void but_callback4(void);
static void but_callback5(void);
static void but_callback6(void);
void task_bluetooth(void);
volatile int 	flag_rtt;




/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/



/************************************************************************/
/* recursos RTOS                                                        */
/************************************************************************/

TimerHandle_t xTimer;

/** Queue for msg log send data */
QueueHandle_t xQueueADC;

QueueHandle_t xQueueMSG;



typedef struct {
	uint id;
	uint value;
} adcData;

/************************************************************************/
/* RTOS application HOOK                                                */
/************************************************************************/

/** Semaforo a ser usado pela task led */
SemaphoreHandle_t xSemaphoreBut1;
SemaphoreHandle_t xSemaphoreBut2;
SemaphoreHandle_t xSemaphoreBut3;
SemaphoreHandle_t xSemaphoreBut4;
SemaphoreHandle_t xSemaphoreBut5;
SemaphoreHandle_t xSemaphoreBut6;
SemaphoreHandle_t xSemaphoreHANDSHAKE;


/* Called if stack overflow during execution */
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName) {
	/*	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);*/
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	* identify which task has overflowed its stack.
	*/
	for (;;) {
	}
}

/* This function is called by FreeRTOS idle task */
extern void vApplicationIdleHook(void) {
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
}

/* This function is called by FreeRTOS each tick */
extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

	/* Force an assert. */
	configASSERT( ( volatile void * ) NULL );
}


/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/

static void AFEC_5_callback(void) {
	adcData adc;
	adc.id = 5;
	adc.value = afec_channel_get_value(AFEC0, 5);
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	xQueueSendFromISR(xQueueADC, &adc, &xHigherPriorityTaskWoken);
}

static void AFEC_2_callback(void) {
	adcData adc;
	adc.id = 2;
	
	adc.value = afec_channel_get_value(AFEC0, 2); //PB3
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	xQueueSendFromISR(xQueueADC, &adc, &xHigherPriorityTaskWoken);
}

static void AFEC_1_callback(void) {
	adcData adc;
	adc.id = 1;
	adc.value = afec_channel_get_value(AFEC0, 1);
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	xQueueSendFromISR(xQueueADC, &adc, &xHigherPriorityTaskWoken);
}



static void but_callback1(void){

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreBut1, &xHigherPriorityTaskWoken);
	};

static void but_callback2(void){
	  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	  xSemaphoreGiveFromISR(xSemaphoreBut2, &xHigherPriorityTaskWoken);
};

static void but_callback3(void){
	  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	  xSemaphoreGiveFromISR(xSemaphoreBut3, &xHigherPriorityTaskWoken);
};

static void but_callback4(void){
	  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	  xSemaphoreGiveFromISR(xSemaphoreBut4, &xHigherPriorityTaskWoken);
};

static void but_callback5(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreBut5, &xHigherPriorityTaskWoken);
};

static void but_callback6(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreBut6, &xHigherPriorityTaskWoken);
};



/************************************************************************/
/* funcoes                                                              */
/************************************************************************/


static void config_AFEC(Afec *afec, uint32_t afec_id) {
	/*************************************
	* Ativa e configura AFEC
	*************************************/
	/* Ativa AFEC - 0 */
	afec_enable(afec);

	/* struct de configuracao do AFEC */
	struct afec_config afec_cfg;

	/* Carrega parametros padrao */
	afec_get_config_defaults(&afec_cfg);

	/* Configura AFEC */
	afec_init(afec, &afec_cfg);

	/* Configura trigger por software */
	afec_set_trigger(afec, AFEC_TRIG_SW);
	


}

void io_init(void) {

	// Ativa PIOs
	pmc_enable_periph_clk(LED_PIO_ID);

	// Configura Pinos
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT | PIO_DEBOUNCE);
}

void but_init(void){
		pmc_enable_periph_clk(BUT_PIO_ID_1);
		pio_configure(BUT_PIO_1, PIO_INPUT, BUT_PIO_IDX_MASK_1, PIO_PULLUP);

		pio_handler_set(BUT_PIO_1,
		BUT_PIO_ID_1,
		BUT_PIO_IDX_MASK_1,
		PIO_IT_EDGE,
		but_callback1);

		pio_enable_interrupt(BUT_PIO_1, BUT_PIO_IDX_MASK_1);
		pio_get_interrupt_status(BUT_PIO_1);


		NVIC_EnableIRQ(BUT_PIO_ID_1);
		NVIC_SetPriority(BUT_PIO_ID_1, 4);
		
		//BOTAO 2
		
		pmc_enable_periph_clk(BUT_PIO_ID_2);
		pio_configure(BUT_PIO_2, PIO_INPUT, BUT_PIO_IDX_MASK_2, PIO_PULLUP);

		pio_handler_set(BUT_PIO_2,
		BUT_PIO_ID_2,
		BUT_PIO_IDX_MASK_2,
		PIO_IT_EDGE,
		but_callback2);

		pio_enable_interrupt(BUT_PIO_2, BUT_PIO_IDX_MASK_2);
		pio_get_interrupt_status(BUT_PIO_2);


		NVIC_EnableIRQ(BUT_PIO_ID_2);
		NVIC_SetPriority(BUT_PIO_ID_2, 4);
	
		// BOTAO 3
		pmc_enable_periph_clk(BUT_PIO_ID_3);
		pio_configure(BUT_PIO_3, PIO_INPUT, BUT_PIO_IDX_MASK_3, PIO_PULLUP);

		pio_handler_set(BUT_PIO_3,
		BUT_PIO_ID_3,
		BUT_PIO_IDX_MASK_3,
		PIO_IT_EDGE,
		but_callback3);

		pio_enable_interrupt(BUT_PIO_3, BUT_PIO_IDX_MASK_3);
		pio_get_interrupt_status(BUT_PIO_3);


		NVIC_EnableIRQ(BUT_PIO_ID_3);
		NVIC_SetPriority(BUT_PIO_ID_3, 4);

		// BOTAO 4
		pmc_enable_periph_clk(BUT_PIO_ID_4);
		pio_configure(BUT_PIO_4, PIO_INPUT, BUT_PIO_IDX_MASK_4, PIO_PULLUP);

		pio_handler_set(BUT_PIO_4,
		BUT_PIO_ID_4,
		BUT_PIO_IDX_MASK_4,
		PIO_IT_EDGE,
		but_callback4);

		pio_enable_interrupt(BUT_PIO_4, BUT_PIO_IDX_MASK_4);
		pio_get_interrupt_status(BUT_PIO_4);


		NVIC_EnableIRQ(BUT_PIO_ID_4);
		NVIC_SetPriority(BUT_PIO_ID_4, 4);

	   // BOTAO REC
		pmc_enable_periph_clk(BUT_PIO_ID_5);
		pio_configure(BUT_PIO_5, PIO_INPUT, BUT_PIO_IDX_MASK_5, PIO_PULLUP);

		pio_handler_set(BUT_PIO_5,
		BUT_PIO_ID_5,
		BUT_PIO_IDX_MASK_5,
		PIO_IT_EDGE,
		but_callback5);

		pio_enable_interrupt(BUT_PIO_5, BUT_PIO_IDX_MASK_5);
		pio_get_interrupt_status(BUT_PIO_5);


		NVIC_EnableIRQ(BUT_PIO_ID_5);
		NVIC_SetPriority(BUT_PIO_ID_5, 4);


		//BOTAO PAUSE
		pmc_enable_periph_clk(BUT_PIO_ID_6);
		pio_configure(BUT_PIO_6, PIO_INPUT, BUT_PIO_IDX_MASK_6, PIO_PULLUP);

		pio_handler_set(BUT_PIO_6,
		BUT_PIO_ID_6,
		BUT_PIO_IDX_MASK_6,
		PIO_IT_EDGE,
		but_callback6);

		pio_enable_interrupt(BUT_PIO_6, BUT_PIO_IDX_MASK_6);
		pio_get_interrupt_status(BUT_PIO_6);


		NVIC_EnableIRQ(BUT_PIO_ID_6);
		NVIC_SetPriority(BUT_PIO_ID_6, 4);
		

}

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits = CONF_UART_STOP_BITS,
	};

	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	setbuf(stdout, NULL);
}

uint32_t usart_puts(uint8_t *pstring) {
	uint32_t i ;

	while(*(pstring + i))
	if(uart_is_tx_empty(USART_COM))
	usart_serial_putchar(USART_COM, *(pstring+i++));
}

void usart_put_string(Usart *usart, char str[]) {
	usart_serial_write_packet(usart, str, strlen(str));
}

int usart_get_string(Usart *usart, char buffer[], int bufferlen, uint timeout_ms) {
	uint timecounter = timeout_ms;
	uint32_t rx;
	uint32_t counter = 0;

	while( (timecounter > 0) && (counter < bufferlen - 1)) {
		if(usart_read(usart, &rx) == 0) {
			buffer[counter++] = rx;
		}
		else{
			timecounter--;
			vTaskDelay(1);
		}
	}
	buffer[counter] = 0x00;
	return counter;
}

void usart_send_command(Usart *usart, char buffer_rx[], int bufferlen,
char buffer_tx[], int timeout) {
	usart_put_string(usart, buffer_tx);
	usart_get_string(usart, buffer_rx, bufferlen, timeout);
}

void config_usart0(void) {
	sysclk_enable_peripheral_clock(ID_USART0);
	usart_serial_options_t config;
	config.baudrate = 9600;
	config.charlength = US_MR_CHRL_8_BIT;
	config.paritytype = US_MR_PAR_NO;
	config.stopbits = false;
	usart_serial_init(USART0, &config);
	usart_enable_tx(USART0);
	usart_enable_rx(USART0);

	// RX - PB0  TX - PB1
	pio_configure(PIOB, PIO_PERIPH_C, (1 << 0), PIO_DEFAULT);
	pio_configure(PIOB, PIO_PERIPH_C, (1 << 1), PIO_DEFAULT);
}

int hc05_init(void) {
	char buffer_rx[128];
	usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT+NAMEMido", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT+PIN9999", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
}

void led_on(Pio *pio, uint32_t mask){
	pio_set(pio,mask);
}



void led_off(Pio *pio, uint32_t mask){
	pio_clear(pio, mask);
}


/************************************************************************/
/* TASKS                                                                */
/************************************************************************/



void task_bluetooth(void) {
	// 	printf("Task Bluetooth started \n");
	//
	// 	printf("Inicializando HC05 \n");
  #ifndef DEBUG_SERIAL
  config_usart0();
  #endif
  hc05_init();
	// configura LEDs e Botões
	io_init();
	int flag = 0;

	char handshake = 's';

	int valoradc;
	adcData adc;
	//NOTAS
	char b0 = '0';
	char b1 = '0';
	char b2 = '0';
	char b3 = '0';
	char b4 = '0';
	char b5 = '0';
	char b6 = '0';
	char b7 = '0';
	char b8 = '0';
	char b9 = '0';
	char b10 = '0';
	char b11 = '0';
	char b12 = '0';
	char b13 = '0';
	char b14 = '0';
	char b15 = '0';
	char b16 = '0';
	char b17 = '0';
	char b18 = '0';
	char b19 = '0';
	char b20 = '0';
	char b21 = '0';
	int on = 0;
	char eof = 'X';
	int i = 0;
	int msg;
	float d;
	char rx;

	// Task não deve retornar.
	while(1) {
		if(flag == 1){
			//printf("o porra");
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1 / portTICK_PERIOD_MS);
			}
			usart_write(USART_COM, 's');
			
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1 / portTICK_PERIOD_MS);
			}
			
			usart_write(USART_COM, eof);
			
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(1 / portTICK_PERIOD_MS);
			}
			
			rx = usart_is_tx_ready(USART_COM);
			
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(1 / portTICK_PERIOD_MS);
			}
			
			usart_write(USART_COM, rx);
			if(rx == '2'){
				xSemaphoreGiveFromISR(xSemaphoreHANDSHAKE,xHigherPriorityTaskWoken);
			}
			if(xSemaphoreTake(xSemaphoreHANDSHAKE, 0)){
				flag = 1;
			}
		}		

		if(flag == 0){
		// atualiza valor do botão
			if (xQueueReceive(xQueueMSG, &(adc), 1000)) {
				((adc.value >> 8) & 0x0f)
				(adc.value)
				//printf("ADC: %d, %d \n",adc.id, adc.value);
				if(adc.id == 5){
					if ( adc.value > 10 &&  adc.value <= 341){
						b7 = '0';
						b8 = '0';
						b9 = '0';
						b10 = '1';
					}
					else if ( adc.value <= 682){
						b7 = '0';
						b8 = '0';
						b9 = '1';
						b10 = '0';
					}
					else if ( adc.value <= 1023){
						b7 = '0';
						b8 = '0';
						b9 = '1';
						b10 = '1';
					}
					else if (  adc.value <= 1364){
						b7 = '0';
						b8 = '1';
						b9 = '0';
						b10 = '0';
					}
					else if (  adc.value <= 1705){
						b7 = '0';
						b8 = '1';
						b9 = '0';
						b10 = '1';
					}
					else if ( adc.value <= 2046){
						b7 = '0';
						b8 = '1';
						b9 = '1';
						b10 = '0';
					}
					else if ( adc.value  <= 2387){
						b7 = '0';
						b8 = '1';
						b9 = '1';
						b10 = '1';
					}
					else if ( adc.value  <= 2728){
						b7 = '1';
						b8 = '0';
						b9 = '0';
						b10 = '0';
					}
					else if ( adc.value  <= 3069){
						b7 = '1';
						b8 = '0';
						b9 = '0';
						b10 = '1';
					}
					else if ( adc.value  <= 3410){
						b7 = '1';
						b8 = '0';
						b9 = '1';
						b10 = '0';
					}
					else if ( adc.value  <= 3410){
						b7 = '1';
						b8 = '0';
						b9 = '1';
						b10 = '1';
					}
					else if ( adc.value  <= 3751){
						b7 = '1';
						b8 = '1';
						b9 = '0';
						b10 = '0';
					}
					else if ( adc.value  <= 4092){
						b7 = '1';
						b8 = '1';
						b9 = '0';
						b10 = '1';
					}
					else{
						b7 = '0';
						b8 = '0';
						b9 = '0';
						b10 = '0';
					}
				
				}
			
				if(adc.id == 2){
					if ( adc.value > 10 &&  adc.value <= 341){
						b3 = '0';
						b4 = '0';
						b5 = '0';
						b6 = '1';
					}
					else if ( adc.value <= 682){
						b3 = '0';
						b4 = '0';
						b5 = '1';
						b6 = '0';
					}
					else if ( adc.value <= 1023){
						b3 = '0';
						b4 = '0';
						b5 = '1';
						b6 = '1';
					}
					else if (  adc.value <= 1364){
						b3 = '0';
						b4 = '1';
						b5 = '0';
						b6 = '0';
					}
					else if (  adc.value <= 1705){
						b3 = '0';
						b4 = '1';
						b5 = '0';
						b6 = '1';
					}
					else if ( adc.value <= 2046){
						b3 = '0';
						b4 = '1';
						b5 = '1';
						b6 = '0';
					}
					else if ( adc.value  <= 2387){
						b3 = '0';
						b4 = '1';
						b5 = '1';
						b6 = '1';
					}
					else if ( adc.value  <= 2728){
						b3 = '1';
						b4 = '0';
						b5 = '0';
						b6 = '0';
					}
					else if ( adc.value  <= 3069){
						b3 = '1';
						b4 = '0';
						b5 = '0';
						b6 = '1';
					}
					else if ( adc.value  <= 3410){
						b3 = '1';
						b4 = '0';
						b5 = '1';
						b6 = '0';
					}
					else if ( adc.value  <= 3410){
						b3 = '1';
						b4 = '0';
						b5 = '1';
						b6 = '1';
					}
					else if ( adc.value  <= 3751){
						b3 = '1';
						b4 = '1';
						b5 = '0';
						b6 = '0';
					}
					else if ( adc.value  <= 4092){
						b3 = '1';
						b4 = '1';
						b5 = '0';
						b6 = '1';
					}
					else{
						b3 = '0';
						b4 = '0';
						b5 = '0';
						b6 = '0';
					}
				}
				if(adc.id == 1){
					if ( adc.value > 10 &&  adc.value <= 341){
						b14 = '0';
						b15 = '0';
						b16 = '0';
						b17 = '1';
					}
					else if ( adc.value <= 682){
						b14 = '0';
						b15 = '0';
						b16 = '1';
						b17 = '0';
					}
					else if ( adc.value <= 1023){
						b14 = '0';
						b15 = '0';
						b16 = '1';
						b17 = '1';
					}
					else if (  adc.value <= 1364){
						b14 = '0';
						b15 = '1';
						b16 = '0';
						b17 = '0';
					}
					else if (  adc.value <= 1705){
						b14 = '0';
						b15 = '1';
						b16 = '0';
						b17 = '1';
					}
					else if ( adc.value <= 2046){
						b14 = '0';
						b15 = '1';
						b16 = '1';
						b17 = '0';
					}
					else if ( adc.value  <= 2387){
						b14 = '0';
						b15 = '1';
						b16 = '1';
						b17 = '1';
					}
					else if ( adc.value  <= 2728){
						b14 = '1';
						b15 = '0';
						b16 = '0';
						b17 = '0';
					}
					else if ( adc.value  <= 3069){
						b14 = '1';
						b15 = '0';
						b16 = '0';
						b17 = '1';
					}
					else if ( adc.value  <= 3410){
						b14 = '1';
						b15 = '0';
						b16 = '1';
						b17 = '0';
					}
					else if ( adc.value  <= 3410){
						b14 = '1';
						b15 = '0';
						b16 = '1';
						b17 = '1';
					}
					else if ( adc.value  <= 3751){
						b14 = '1';
						b15 = '1';
						b16 = '0';
						b17 = '0';
					}
					else if ( adc.value  <= 4092){
						b14 = '1';
						b15 = '1';
						b16 = '0';
						b17 = '1';
					}
					else{
						b14 = '0';
						b15 = '0';
						b16 = '0';
						b17 = '0';
					}
				
				}
			
			
			
			
			}
		
			// Aciona Acompanhamento da musica
			if(xSemaphoreTake(xSemaphoreBut1, 0)){
				if(!pio_get(BUT_PIO_1, PIO_INPUT, BUT_PIO_IDX_MASK_1)){
					b0 = '1';
				}
				else{
					b0 = '0';
				}
			}
			if(xSemaphoreTake(xSemaphoreBut2, 0)){
				if(!pio_get(BUT_PIO_2, PIO_INPUT, BUT_PIO_IDX_MASK_2)){
					b1 = '1';
				}
				else{
					b1 = '0';
				}
			}
			if(xSemaphoreTake(xSemaphoreBut3, 0)){
				if(!pio_get(BUT_PIO_3, PIO_INPUT, BUT_PIO_IDX_MASK_3)){
					b2 = '1';
				}
				else{
					b2 = '0';
				}
			}
			if(xSemaphoreTake(xSemaphoreBut4, 0)){
				if(!pio_get(BUT_PIO_4, PIO_INPUT, BUT_PIO_IDX_MASK_4)){
					b11 = '1';
				}
				else{
					b11 = '0';
				}
			}
			if(xSemaphoreTake(xSemaphoreBut5, 0)){
				
				led_off(LED_PIO, LED_IDX_MASK);
				if(!pio_get(BUT_PIO_5, PIO_INPUT, BUT_PIO_IDX_MASK_5)){
					b12 = '1';
				}
				else{
					b12 = '0';
				}
				
			}
			if(xSemaphoreTake(xSemaphoreBut6, 0)){
				led_on(LED_PIO, LED_IDX_MASK);
				if(!pio_get(BUT_PIO_6, PIO_INPUT, BUT_PIO_IDX_MASK_6)){
					b13 = '1';
				}
				else{
					b13 = '0';
				}
			}
		

			//Botaão rec
		
			//Botão Pause

			// envia status botão

		
		
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(1 / portTICK_PERIOD_MS);
			}
			usart_write(USART_COM, b0);

			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(1 / portTICK_PERIOD_MS);
			}
			usart_write(USART_COM, b1);

			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(1/ portTICK_PERIOD_MS);
			}
			usart_write(USART_COM, b2);


		

			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1/ portTICK_PERIOD_MS);
			}
			usart_write(USART_COM, b3);


			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1/ portTICK_PERIOD_MS);
			}
			usart_write(USART_COM, b4);


			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1 / portTICK_PERIOD_MS);
			}
			usart_write(USART_COM, b5);

			//
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1/ portTICK_PERIOD_MS);
			}
			usart_write(USART_COM, b6);


			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1/ portTICK_PERIOD_MS);
			}

			usart_write(USART_COM, b7);

			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1/ portTICK_PERIOD_MS);
			}

			usart_write(USART_COM, b8);

			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1/ portTICK_PERIOD_MS);
			}

			usart_write(USART_COM, b9);
		
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1/ portTICK_PERIOD_MS);
			}

			usart_write(USART_COM, b10);
		
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1/ portTICK_PERIOD_MS);
			}

			usart_write(USART_COM, b11);
		
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1/ portTICK_PERIOD_MS);
			}

			usart_write(USART_COM, b12);
		
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1/ portTICK_PERIOD_MS);
			}

			usart_write(USART_COM, b13);

			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1/ portTICK_PERIOD_MS);
			}

			usart_write(USART_COM, b14);

			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1/ portTICK_PERIOD_MS);
			}

			usart_write(USART_COM, b15);
		
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1/ portTICK_PERIOD_MS);
			}

			usart_write(USART_COM, b16);
		
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1/ portTICK_PERIOD_MS);
			}

			usart_write(USART_COM, b17);
			// envia fim de pacote
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1 / portTICK_PERIOD_MS);
			}
			usart_write(USART_COM, b18);
			// envia fim de pacote
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1 / portTICK_PERIOD_MS);
			}
			usart_write(USART_COM, b19);
			// envia fim de pacote
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1 / portTICK_PERIOD_MS);
			}
			usart_write(USART_COM, b20);
			// envia fim de pacote
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1 / portTICK_PERIOD_MS);
			}
			usart_write(USART_COM, b21);
			// envia fim de pacote
			
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(0.1 / portTICK_PERIOD_MS);
			}
		
		
		
		
			usart_write(USART_COM, eof);

			// dorme por 500 ms
			vTaskDelay( 1 / portTICK_PERIOD_MS);

		}

	}

}








static void task_afec(void *pvParameters){
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	config_AFEC(AFEC0, ID_AFEC0);
	
	struct afec_ch_config afec_ch_cfg;
	afec_ch_get_config_defaults(&afec_ch_cfg);
	afec_ch_cfg.gain = AFEC_GAINVALUE_0;
	afec_ch_set_config(AFEC0, 5, &afec_ch_cfg);
	afec_channel_set_analog_offset(AFEC0, 5, 0x200);
	afec_set_callback(AFEC0, 5, AFEC_5_callback, 1);

	afec_ch_set_config(AFEC0, 2, &afec_ch_cfg);
	afec_channel_set_analog_offset(AFEC0, 2, 0x200);
	afec_set_callback(AFEC0, 2, AFEC_2_callback, 1);
	
	afec_ch_set_config(AFEC0, 1, &afec_ch_cfg);
	afec_channel_set_analog_offset(AFEC0, 1, 0x200);
	afec_set_callback(AFEC0, 1, AFEC_1_callback, 1);
	

	NVIC_SetPriority(ID_AFEC0, 4);
	NVIC_EnableIRQ(ID_AFEC0);
	
	adcData adc;
	
	for(;;){
		afec_channel_enable(AFEC0, 5);
		afec_start_software_conversion(AFEC0);
		vTaskDelay(50);
		afec_channel_enable(AFEC0, 2);
		afec_start_software_conversion(AFEC0);
		vTaskDelay(50);
		afec_channel_enable(AFEC0, 1);
		afec_start_software_conversion(AFEC0);
		vTaskDelay(50);
		
		while(xQueueReceive(xQueueADC, &adc, 0)){
			//printf("%d, %d \n", adc.id, adc.value);
			xQueueSendFromISR(xQueueMSG, &adc, &xHigherPriorityTaskWoken );
		}

		
	}
	
}


// 
// static void task_afec(void *pvParameters){
// 	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
// 	int channel = 0;	config_AFEC(AFEC0, ID_AFEC0);
// 	
// 	struct afec_ch_config afec_ch_cfg;
// 	afec_ch_get_config_defaults(&afec_ch_cfg);
// 	afec_ch_cfg.gain = AFEC_GAINVALUE_0;
// 	afec_ch_set_config(AFEC0, 5, &afec_ch_cfg);
// 	afec_channel_set_analog_offset(AFEC0, 5, 0x200);
// 	afec_set_callback(AFEC0, 5, AFEC_5_callback, 1);
// 
// 	afec_ch_set_config(AFEC0, 2, &afec_ch_cfg);
// 	afec_channel_set_analog_offset(AFEC0, 2, 0x200);
// 	afec_set_callback(AFEC0, 2, AFEC_2_callback, 1);
// 	
// 	afec_ch_set_config(AFEC0, 1, &afec_ch_cfg);
// 	afec_channel_set_analog_offset(AFEC0, 1, 0x200);
// 	afec_set_callback(AFEC0, 1, AFEC_1_callback, 1);
// 
// 	NVIC_SetPriority(ID_AFEC0, 4);
// 	NVIC_EnableIRQ(ID_AFEC0);
// 	
// 	adcData adc;
// 	int channel = {5, 2, 1};
// 	int channel_len = 3;
// 	int channel_id = 0;
// 	
// 	for(;;){
// 		afec_channel_enable(AFEC0, channel[channel_id++]);
// 		if (channel_id < channel_len)
// 		channel_id = 0;
// 		
// 		while(xQueueReceive(xQueueADC, &adc, 100)){
// 			//printf("%d, %d \n", adc.id, adc.value);
// 			xQueueSendFromISR(xQueueMSG, &adc, &xHigherPriorityTaskWoken );
// 		}
// 	}
// 	
// }
/************************************************************************/
/* main                                                                 */
/************************************************************************/

int main(void) {
	
	sysclk_init();
	board_init();
	configure_console();
	but_init();
	/* Initialize the SAM system */
	  xSemaphoreBut1 = xSemaphoreCreateBinary();
	  if (xSemaphoreBut1 == NULL){
		  printf("falha em criar o semaforo \n");
	  }
	  xSemaphoreBut2 = xSemaphoreCreateBinary();
	  if(xSemaphoreBut2 == NULL){
		  printf("nao rolou criar o Semaforo 2");
	  }
	  
	  xSemaphoreBut3 = xSemaphoreCreateBinary();
	  if(xSemaphoreBut3 == NULL){
		  printf("Boa tentativa, mas o semaforo 3 nao foi gerado");
	  }
	  xSemaphoreBut4= xSemaphoreCreateBinary();
	  if(xSemaphoreBut4 == NULL){
		  printf("Boa tentativa, mas o semaforo 3 nao foi gerado");
	  }
	  xSemaphoreBut5= xSemaphoreCreateBinary();
	  if(xSemaphoreBut5 == NULL){
		  printf("Boa tentativa, mas o semaforo 3 nao foi gerado");
	  }

	  xSemaphoreBut6= xSemaphoreCreateBinary();
	  if(xSemaphoreBut6 == NULL){
		  printf("Boa tentativa, mas o semaforo 3 nao foi gerado");
	  }
	  ;
	 xSemaphoreHANDSHAKE = xSemaphoreCreateBinary();
	 if(xSemaphoreHANDSHAKE == NULL){
	 	printf("Boa tentativa, mas o semaforo 3 nao foi gerado");
	 }
	
	xQueueADC = xQueueCreate(100, sizeof(adcData));
	if (xQueueADC == NULL)
	printf("falha em criar a queue xQueueADC \n");
	xQueueMSG = xQueueCreate(100, sizeof(adcData));
	if (xQueueMSG == NULL)
	printf("falha em criar a queue xQueueADC \n");


	xTaskCreate(task_afec, "afec", TASK_BLUETOOTH_STACK_SIZE, NULL,	TASK_BLUETOOTH_STACK_PRIORITY, NULL);
	xTaskCreate(task_bluetooth, "afec", TASK_BLUETOOTH_STACK_SIZE, NULL,	TASK_BLUETOOTH_STACK_PRIORITY, NULL);

	/* Start the scheduler. */
	vTaskStartScheduler();

	while(1){

	}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}
