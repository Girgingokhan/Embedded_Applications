/*STM32 ile bulaşık makinesi uygulamasını C++ kullanarak yazarken, 
Sınıflar:

Button: Her butonun durumunu okur.
TemperatureSensor: Sıcaklık sensöründen veri okur.
WaterLevelSensor: Su seviyesini ölçer.
Relay: Röle kontrolünü sağlar.
Motor: Motor kontrolünü sağlar.
State Machine: Yıkama, durulama, sıkma gibi durumları yönetmek için bir durum makinesi.
OLED Ekran: Durum bilgileri, su seviyesi, sıcaklık ve hata durumlarını renkli OLED ekranda gösterir.
Hata Yönetimi: Cihazdaki tüm hataları (örneğin, aşırı sıcaklık, düşük su seviyesi) algılar ve OLED'de gösterir.
*/
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
/*
OOP Kullanımı: Her bir bileşen için sınıf oluşturuldu. Her sınıfın görev ve yetenekleri net şekilde ayrıldı.
State Machine: enum class ile durumlar tanımlandı ve geçişler yapıldı.
Hata Yönetimi: Hatalar kontrol edilip ekranda görüntüleniyor.
OLED Entegrasyonu: Ekranda cihazın durumu, su seviyesi ve sıcaklık değerleri gösteriliyor. */



/*
STM32 mikrodenetleyicisini kullanarak bulaşık makinesi kontrol uygulamasını Modern C++ 
Sınıflar:
Button: Volatile buton durumlarını yönetir, callback kullanır.
TemperatureSensor: NTC sensör değerlerini okur.
Relay: Röle kontrolünü sağlar.
Motor: PWM tabanlı motor kontrolü.
OLED: Renkli OLED ekrana veri yazmak için.
ErrorManager: Hataları yönetir.
Durum Makinesi (State Machine):
Yıkama, durulama, sıkma gibi durumları kontrol eder.

FreeRTOS Görevleri:
ButtonTask: Butonların durumlarını okur.
ControlTask: Durum makinesini yönetir.
DisplayTask: OLED ekrana veri yazdırır.
ErrorTask: Hata yönetimi.
STM32 HAL ile Donanım Entegrasyonu:
GPIO, PWM, ADC ve I2C kullanımı.  */

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <functional>
#include <string>

// Sınıflar
class Button {
private:
    volatile bool state;
    GPIO_TypeDef* gpioPort;
    uint16_t gpioPin;
    std::function<void()> callback;  // void (*callback)(); // C++03

public:
    Button(GPIO_TypeDef* port, uint16_t pin) : gpioPort(port), gpioPin(pin), state(false), callback(nullptr) {}

    void setCallback(std::function<void()> cb) { callback = cb; } //void setCallback( void (*cb)() ){callback = cb;} //C++03

    void checkState() {
        bool currentState = HAL_GPIO_ReadPin(gpioPort, gpioPin);
        if (currentState && !state) {
            state = true;
            if (callback) callback();
        } else if (!currentState && state) {
            state = false;
        }
    }
};

class TemperatureSensor {
private:
    ADC_HandleTypeDef* adc;
    uint32_t channel;

public:
    TemperatureSensor(ADC_HandleTypeDef* adcHandle, uint32_t adcChannel)
        : adc(adcHandle), channel(adcChannel) {}

    float readTemperature() {
        ADC_ChannelConfTypeDef sConfig = {};
        sConfig.Channel = channel;
        sConfig.Rank = 1;
        sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
        HAL_ADC_ConfigChannel(adc, &sConfig);
        HAL_ADC_Start(adc);
        if (HAL_ADC_PollForConversion(adc, 100) == HAL_OK) {
            uint32_t value = HAL_ADC_GetValue(adc);
            return (value * 3.3f / 4096) * 100; // Örnek sıcaklık hesaplama
        }
        return -1; // Hata durumunda
    }
};

class Relay {
private:
    GPIO_TypeDef* gpioPort;
    uint16_t gpioPin;

public:
    Relay(GPIO_TypeDef* port, uint16_t pin) : gpioPort(port), gpioPin(pin) {}

    void on() { HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET); }
    void off() { HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET); }
};

class Motor {
private:
    TIM_HandleTypeDef* timer;
    uint32_t channel;

public:
    Motor(TIM_HandleTypeDef* timerHandle, uint32_t timerChannel)
        : timer(timerHandle), channel(timerChannel) {}

    void setSpeed(uint16_t pwmValue) {
        __HAL_TIM_SET_COMPARE(timer, channel, pwmValue);
    }
};

class OLED {
public:
    void display(const std::string& message) {
        // OLED'e veri yazdırma (I2C/SPI ile entegre edilebilir)
        printf("OLED: %s\n", message.c_str());
    }
};

class ErrorManager {
public:
 // Hata kontrol fonksiyonu
  bool checkForErrors() {
    if (waterLevel < 0.5) return true;  // Düşük su seviyesi
    if (temperature > 80.0) return true;  // Aşırı sıcaklık
    return false;
  }
};


// Durum Makinesi
enum class WashingState { IDLE, FILLING, WASHING, RINSING, SPINNING, DRAINING, ERROR };

class WashingMachine {
private:
    WashingState state;
    Motor& motor;
    Relay& heater;
    TemperatureSensor& tempSensor;
    OLED& display;

public:
    WashingMachine(Motor& motor, Relay& heater, TemperatureSensor& tempSensor, OLED& display)
        : state(WashingState::IDLE), motor(motor), heater(heater), tempSensor(tempSensor), display(display) {}

    void setState(WashingState newState) {
        state = newState;
        displayState();
    }

    void displayState() {
        switch (state) {
        case WashingState::IDLE: display.display("State: IDLE"); break;
        case WashingState::FILLING: display.display("State: FILLING"); break;
        case WashingState::WASHING: display.display("State: WASHING"); break;
        case WashingState::RINSING: display.display("State: RINSING"); break;
        case WashingState::SPINNING: display.display("State: SPINNING"); break;
        case WashingState::DRAINING: display.display("State: DRAINING"); break;
        case WashingState::ERROR: display.display("State: ERROR"); break;
        }
    }

    void run() {
        switch (state) {
        case WashingState::WASHING:
            motor.setSpeed(80); // Örnek PWM değeri
            break;
        case WashingState::ERROR:
            display.display("ERROR OCCURRED!");
            break;
        default:
            break;
        }
    }
};

// FreeRTOS Görevleri
void ButtonTask(void* param) {
    Button* button = static_cast<Button*>(param);
    while (1) {
        button->checkState();
        vTaskDelay(pdMS_TO_TICKS(50)); // 50 ms kontrol süresi
    }
}

void ControlTask(void* param) {
    WashingMachine* machine = static_cast<WashingMachine*>(param);
    while (1) {
        machine->run();
        vTaskDelay(pdMS_TO_TICKS(100)); // 100 ms kontrol süresi
    }
}

void DisplayTask(void* param) {
    OLED* display = static_cast<OLED*>(param);
    while (1) {
        display->display("Bulaşık makinesi çalışıyor...");
        vTaskDelay(pdMS_TO_TICKS(500)); // OLED güncelleme süresi
    }
}

int main() {
    // Simüle edilmiş donanım
    Button onOffButton(GPIOA, GPIO_PIN_0);
    ADC_HandleTypeDef adc1;
    TemperatureSensor tempSensor(&adc1, ADC_CHANNEL_1);
    Relay heater(GPIOB, GPIO_PIN_0);
    TIM_HandleTypeDef timer2;
    Motor motor(&timer2, TIM_CHANNEL_1);
    OLED display;

    WashingMachine machine(motor, heater, tempSensor, display);

    // FreeRTOS Görevlerini Başlat
    xTaskCreate(ButtonTask, "ButtonTask", 128, &onOffButton, 1, nullptr);
    xTaskCreate(ControlTask, "ControlTask", 256, &machine, 1, nullptr);
    xTaskCreate(DisplayTask, "DisplayTask", 128, &display, 1, nullptr);

    // FreeRTOS Scheduler
    vTaskStartScheduler();

    while (1) {}
}
/*
Açıklamalar:
Button: volatile değişkenler ile GPIO durumu kontrol edilir.
TemperatureSensor: NTC sensöründen sıcaklık okur.
Relay: Röle açma/kapama kontrolü sağlar.
Motor: PWM ile motor hızını ayarlar.
OLED: Ekranda durumu gösterir.
FreeRTOS Görevleri: Görevler modüler yapıyı destekler. */



