#include "stm32f1xx_hal.h"

UART_HandleTypeDef huart1;  // UART configuration

#define RS485_DE_PIN          GPIO_PIN_8
#define RS485_RE_PIN          GPIO_PIN_9
#define RS485_GPIO_PORT       GPIOB
#define START_BUTTON_PIN      GPIO_PIN_0
#define STOP_BUTTON_PIN       GPIO_PIN_1
#define INCREASE_BUTTON_PIN   GPIO_PIN_2
#define DECREASE_BUTTON_PIN   GPIO_PIN_3

uint16_t setpoint = 100;  // Initial setpoint value(enter a value 0-1000!!!)

uint16_t USPoint(uint16_t setpoint){ // user set point value = IN(40101)*16384/50
   uint16_t UserSetPoint =0;
   /*  if(setpoint < 0 || setpoint > 1000){
    	printf(" hyz de?erini do?ru giriniz!");
	} */
	if(setpoint < 0){
		setpoint=0;
	}if else(setpoint > 1000){
		setpoint=1000;
	}else{
		UserSetPoint = (setpoint*16384)/50;
	}

  return UserSetPoint;
}

void RS485_Transmit_Enable(void) {
    HAL_GPIO_WritePin(RS485_GPIO_PORT, RS485_DE_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RS485_GPIO_PORT, RS485_RE_PIN, GPIO_PIN_RESET);
}

void RS485_Receive_Enable(void) {
    HAL_GPIO_WritePin(RS485_GPIO_PORT, RS485_DE_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RS485_GPIO_PORT, RS485_RE_PIN, GPIO_PIN_SET);
}

void UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();    
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

uint16_t calculate_crc(uint8_t *data, uint8_t length)
{
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void modbus_write_register(uint8_t address, uint16_t register_addr, uint16_t data)
{
    uint8_t packet[8];
    packet[0] = address;
    packet[1] = 0x06;  
    packet[2] = (register_addr >> 8) & 0xFF;
    packet[3] = register_addr & 0xFF;
    packet[4] = (data >> 8) & 0xFF;
    packet[5] = data & 0xFF;

    uint16_t crc = calculate_crc(packet, 6);
    packet[6] = crc & 0xFF;
    packet[7] = (crc >> 8) & 0xFF;

    RS485_Transmit_Enable();
    HAL_UART_Transmit(&huart1, packet, 8, HAL_MAX_DELAY);
    RS485_Receive_Enable();
}

uint16_t modbus_read_register(uint8_t address, uint16_t register_addr)
{
    uint8_t packet[8];
    uint8_t response[7];
    packet[0] = address;
    packet[1] = 0x03; 
    packet[2] = (register_addr >> 8) & 0xFF;
    packet[3] = register_addr & 0xFF;
    packet[4] = 0x00;
    packet[5] = 0x01;

    uint16_t crc = calculate_crc(packet, 6);
    packet[6] = crc & 0xFF;
    packet[7] = (crc >> 8) & 0xFF;

    RS485_Transmit_Enable();
    HAL_UART_Transmit(&huart1, packet, 8, HAL_MAX_DELAY);
    RS485_Receive_Enable();

    HAL_UART_Receive(&huart1, response, 7, HAL_MAX_DELAY);
    return (response[3] << 8) | response[4];
}

void Error_Handler(void) {
    while(1) {}
}

int main(void)
{
	
    HAL_Init();
    UART_Init();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = START_BUTTON_PIN | STOP_BUTTON_PIN | INCREASE_BUTTON_PIN | DECREASE_BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    uint16_t motor_speed=0;
    uint16_t actuel_motor_speed=0;
    uint16_t calculatedSpeed=0;
    
    while (1) {
    	
        if (HAL_GPIO_ReadPin(GPIOA, START_BUTTON_PIN) == GPIO_PIN_RESET) {
            modbus_write_register(1, 0x0100, 0x047F); 
            HAL_Delay(1000);
        }

        if (HAL_GPIO_ReadPin(GPIOA, STOP_BUTTON_PIN) == GPIO_PIN_RESET) {
            modbus_write_register(1, 0x0100, 0x047E);
            HAL_Delay(1000);
        }

        if (HAL_GPIO_ReadPin(GPIOA, INCREASE_BUTTON_PIN) == GPIO_PIN_RESET) {
            setpoint += 10;
            calculatedSpeed=USPoint(setpoint);
            modbus_write_register(1, 0x0101, calculatedSpeed);
            HAL_Delay(500);
        }

        if (HAL_GPIO_ReadPin(GPIOA, DECREASE_BUTTON_PIN) == GPIO_PIN_RESET) {
            setpoint -= 10;
            calculatedSpeed=USPoint(setpoint);
            modbus_write_register(1, 0x0101, calculatedSpeed);
            HAL_Delay(500);
        }

        motor_speed = modbus_read_register(1, 0x0111);
        actuel_motor_speed = (motor_speed*50)/16384;   // Actual motor speed = IN(40111)*50/16384
        // Print the received value
        printf("Read Data: %d\n", actuel_motor_speed);
         
        HAL_Delay(1000);
    }
}


