#include "stm32f1xx_hal.h"

UART_HandleTypeDef huart1;  // UART yapýlandýrma

#define RS485_DE_PIN          GPIO_PIN_8
#define RS485_RE_PIN          GPIO_PIN_9
#define RS485_GPIO_PORT       GPIOB
#define START_BUTTON_PIN      GPIO_PIN_0
#define STOP_BUTTON_PIN       GPIO_PIN_1
#define INCREASE_BUTTON_PIN   GPIO_PIN_2
#define DECREASE_BUTTON_PIN   GPIO_PIN_3

uint16_t setpoint = 100;  // Baþlangýç setpoint deðeri (0-1000 arasý deger gir!!!)

uint16_t USPoint(uint16_t setpoint){ // user set point value = IN(40101)*16384/50
   uint16_t UserSetPoint =0;
   /*  if(setpoint < 0 || setpoint > 1000){
    	printf(" hýz deðerini doðru giriniz!");
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
        actuel_motor_speed = (motor_speed*50)/16384;   // aktuel motor hýzý = IN(40111)*50/16384
        // Okunan veriyi ekrana yazdýr
        printf("Read Data: %d\n", actuel_motor_speed);
         
        HAL_Delay(1000);
    }
}



/////////////////////// C++ kodu yazýlým programý //////////////////////////
#include "stm32f1xx_hal.h"

class RS485Modbus {
private:
    UART_HandleTypeDef huart1;  // UART yapýlandýrma
    uint16_t setpoint;
    
public:
    RS485Modbus() : setpoint(1000) {
        // Constructor: setpoint varsayýlan deðeri 1000
    }

    void UART_Init(void) {
        __HAL_RCC_USART1_CLK_ENABLE();    // USART1 saatini etkinleþtir
        __HAL_RCC_GPIOB_CLK_ENABLE();     // GPIOB saatini etkinleþtir

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

        if (HAL_UART_Init(&huart1) != HAL_OK) {
            Error_Handler();
        }
    }

    uint16_t calculate_crc(uint8_t *data, uint8_t length) {
        uint16_t crc = 0xFFFF;
        uint8_t i, j;
        for (i = 0; i < length; i++) {
            crc ^= data[i];
            for (j = 8; j > 0; j--) {
                if (crc & 0x0001) {
                    crc = (crc >> 1) ^ 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }

    void modbus_write_register(uint8_t address, uint16_t register_addr, uint16_t data) {
        uint8_t packet[8];
        packet[0] = address;
        packet[1] = 0x06;  // Write Single Register
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

    void modbus_read_register(uint8_t address, uint16_t register_addr, uint16_t *data) {
        uint8_t packet[8];
        uint8_t response[5];

        packet[0] = address;
        packet[1] = 0x03;  // Read Holding Registers
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
        HAL_UART_Receive(&huart1, response, 5, HAL_MAX_DELAY);

        *data = (response[3] << 8) | response[4];
    }

    void RS485_Transmit_Enable(void) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
    }

    void RS485_Receive_Enable(void) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
    }

    void Error_Handler(void) {
        while (1) { }  // Hata durumunda sonsuz döngüye gir
    }

    void set_setpoint_from_flash() {
        // Flash belleðinden setpoint deðerini oku
        uint32_t flash_value = *(__IO uint32_t*)0x0800FC00;  // Flash belleði adresi
        setpoint = (uint16_t)(flash_value & 0xFFFF);  // Sadece düþük 16 bit
    }

    void save_setpoint_to_flash() {
        HAL_FLASH_Unlock();
        // Setpoint'i Flash belleðe yaz
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 0x0800FC00, setpoint);
        HAL_FLASH_Lock();
    }

    uint16_t get_setpoint() {
        return setpoint;
    }

    void set_setpoint(uint16_t value) {
        setpoint = value;
    }
};

volatile uint8_t start_button_state = 0;
volatile uint8_t stop_button_state = 0;
volatile uint8_t increase_button_state = 0;
volatile uint8_t decrease_button_state = 0;

int main(void) {
    HAL_Init();

    RS485Modbus modbus;
    modbus.UART_Init();
    modbus.set_setpoint_from_flash();  // Flash belleðinden setpoint'i oku

    // GPIO pinlerini baþlatmak için gerekli yapýlandýrmalarý yapýn 
    __HAL_RCC_GPIOA_CLK_ENABLE(); // GPIOA saatini etkinleþtir
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;  // Butonlarý pull-up ile kullanýyoruz
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    uint16_t read_data;
    modbus.modbus_read_register(1, 0x0100, &read_data);  // 400100 adresinden aktüel hýz okuma iþlemi

    while (1) {
        // Buton durumlarýný oku
        start_button_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
        stop_button_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);
        increase_button_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2);
        decrease_button_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);

        // Start butonuna basýldýysa 40001 adresine 16#047E yaz
        if (start_button_state == GPIO_PIN_RESET) {
            modbus.modbus_write_register(1, 0x0000, 0x047E);  // 40001 adresine 16#047E gönder
            HAL_Delay(1000);  // Butonun basýlý kalmasýný engellemek için kýsa bir gecikme
        }

        // Stop butonuna basýldýysa 40001 adresine 16#047F yaz
        if (stop_button_state == GPIO_PIN_RESET) {
            modbus.modbus_write_register(1, 0x0000, 0x047F);  // 40001 adresine 16#047F gönder
            HAL_Delay(1000);  // Butonun basýlý kalmasýný engellemek için kýsa bir gecikme
        }

        // Setpoint artýrma butonuna basýldýysa setpoint deðerini artýr
        if (increase_button_state == GPIO_PIN_RESET) {
            modbus.set_setpoint(modbus.get_setpoint() + 10);  // Setpoint'i 10 artýr
            modbus.modbus_write_register(1, 0x0002, modbus.get_setpoint());  // Yeni setpoint deðerini yaz
            HAL_Delay(500);  // Butonun basýlý kalmasýný engellemek için kýsa bir gecikme
        }

        // Setpoint azaltma butonuna basýldýysa setpoint deðerini azalt
        if (decrease_button_state == GPIO_PIN_RESET) {
            modbus.set_setpoint(modbus.get_setpoint() - 10);  // Setpoint'i 10 azalt
            modbus.modbus_write_register(1, 0x0002, modbus.get_setpoint());  // Yeni setpoint deðerini yaz
            HAL_Delay(500);  // Butonun basýlý kalmasýný engellemek için kýsa bir gecikme
        }
    }
}

