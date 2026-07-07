/*When writing a dishwasher application with STM32 using C++,

Classes:

Button: Reads the status of each button.
TemperatureSensor: Reads data from the temperature sensor.
WaterLevelSensor: Measures the water level.
Relay: Provides relay control.
Motor: Provides motor control.
State Machine: A state machine to manage states such as washing, rinsing, and spinning.
OLED Display: Displays status information, water level, temperature, and error conditions on a color OLED screen.
Error Management: Detects all errors in the device (e.g., over-temperature, low water level) and displays them on the OLED.*/



#include "main.h"
#include <string>
#include <iostream>

// Durum makinesi durumları
enum class MachineState {
    Idle,
    Washing,
    Rinsing,
    Spinning,
    Error
};

// --- Sınıf Tanımları ---
class Button {
private:
    GPIO_TypeDef* port;
    uint16_t pin;
    volatile bool pressed;

public:
    Button(GPIO_TypeDef* port, uint16_t pin) : port(port), pin(pin), pressed(false) {}

    void checkPress() {
        if (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET) {
            pressed = true;
        }
    }

    bool isPressed() {
        if (pressed) {
            pressed = false;
            return true;
        }
        return false;
    }
};

class TemperatureSensor {
private:
    ADC_HandleTypeDef* hadc;
    uint32_t channel;

public:
    TemperatureSensor(ADC_HandleTypeDef* hadc, uint32_t channel) : hadc(hadc), channel(channel) {}

    float readTemperature() {
        ADC_ChannelConfTypeDef sConfig = {0};
        sConfig.Channel = channel;
        sConfig.Rank = 1;
        sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
        HAL_ADC_ConfigChannel(hadc, &sConfig);
        HAL_ADC_Start(hadc);
        if (HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY) == HAL_OK) {
            return HAL_ADC_GetValue(hadc) * (3.3 / 4096) * 100; // Örnek ölçekleme
        }
        return 0.0;
    }
};

class WaterLevelSensor {
private:
    ADC_HandleTypeDef* hadc;
    uint32_t channel;

public:
    WaterLevelSensor(ADC_HandleTypeDef* hadc, uint32_t channel) : hadc(hadc), channel(channel) {}

    float readWaterLevel() {
        ADC_ChannelConfTypeDef sConfig = {0};
        sConfig.Channel = channel;
        sConfig.Rank = 1;
        sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
        HAL_ADC_ConfigChannel(hadc, &sConfig);
        HAL_ADC_Start(hadc);
        if (HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY) == HAL_OK) {
            return HAL_ADC_GetValue(hadc) * (3.3 / 4096); // Örnek ölçekleme
        }
        return 0.0;
    }
};

class Relay {
private:
    GPIO_TypeDef* port;
    uint16_t pin;

public:
    Relay(GPIO_TypeDef* port, uint16_t pin) : port(port), pin(pin) {}

    void set(bool state) {
        HAL_GPIO_WritePin(port, pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
};

class Motor {
private:
    GPIO_TypeDef* port;
    uint16_t pin;

public:
    Motor(GPIO_TypeDef* port, uint16_t pin) : port(port), pin(pin) {}

    void start() {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    }

    void stop() {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    }
};

// --- Global Nesneler ---
Button onOffButton(GPIOA, GPIO_PIN_0);
Button startStopButton(GPIOA, GPIO_PIN_1);
Button program1Button(GPIOA, GPIO_PIN_2);
Button program2Button(GPIOA, GPIO_PIN_3);

TemperatureSensor tempSensor(&hadc1, ADC_CHANNEL_1);
WaterLevelSensor waterSensor(&hadc1, ADC_CHANNEL_2);

Relay waterPump(GPIOB, GPIO_PIN_0);
Relay heater(GPIOB, GPIO_PIN_1);

Motor washMotor(GPIOB, GPIO_PIN_2);

// Durum ve diğer değişkenler
MachineState currentState = MachineState::Idle;
float waterLevel = 0.0;
float temperature = 0.0;

// OLED ekran fonksiyonu
void displayStatus(const std::string& status, float water, float temp, const std::string& error = "") {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Status: %s\nWater: %.1f\nTemp: %.1f\nError: %s",
             status.c_str(), water, temp, error.c_str());
    OLED_Display(buffer);
}

// Hata kontrol fonksiyonu
bool checkForErrors() {
    if (waterLevel < 0.5) return true;  // Düşük su seviyesi
    if (temperature > 80.0) return true;  // Aşırı sıcaklık
    return false;
}

// Durum makinesi
void runStateMachine() {
    switch (currentState) {
        case MachineState::Idle:
            displayStatus("Idle", waterLevel, temperature);
            if (onOffButton.isPressed()) currentState = MachineState::Washing;
            break;

        case MachineState::Washing:
            displayStatus("Washing", waterLevel, temperature);
            waterPump.set(true);
            washMotor.start();
            if (checkForErrors()) currentState = MachineState::Error;
            if (waterLevel > 1.0) currentState = MachineState::Rinsing;
            break;

        case MachineState::Rinsing:
            displayStatus("Rinsing", waterLevel, temperature);
            waterPump.set(false);
            washMotor.start();
            if (checkForErrors()) currentState = MachineState::Error;
            if (temperature < 40.0) currentState = MachineState::Spinning;
            break;

        case MachineState::Spinning:
            displayStatus("Spinning", waterLevel, temperature);
            washMotor.stop();
            if (checkForErrors()) currentState = MachineState::Error;
            if (waterLevel < 0.5) currentState = MachineState::Idle;
            break;

        case MachineState::Error:
            displayStatus("Error", waterLevel, temperature, "Critical!");
            washMotor.stop();
            waterPump.set(false);
            break;
    }
}

// Ana döngü
int main() {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();
    OLED_Init();

    while (true) {
        waterLevel = waterSensor.readWaterLevel();
        temperature = tempSensor.readTemperature();
        onOffButton.checkPress();
        startStopButton.checkPress();
        program1Button.checkPress();
        program2Button.checkPress();
        runStateMachine();
        HAL_Delay(500);
    }
}


