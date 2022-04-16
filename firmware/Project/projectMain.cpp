#include "main.h"
#include <Project/FlashCheck.h>
#include <Project/SWO.h>
#include <Project/projectMain.h>
#include <array>
#include <span>

struct Led
{
    explicit Led(GPIO_TypeDef* t_gpio, uint16_t t_pin)
        : gpio(t_gpio)
        , pin(t_pin)
    {
    }
    void on()
    {
        HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_SET);
    }
    void off()
    {
        HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_RESET);
    }
    void toggle()
    {
        HAL_GPIO_TogglePin(gpio, pin);
    }
    auto state() -> bool
    {
        return HAL_GPIO_ReadPin(gpio, pin);
    }

private:
    GPIO_TypeDef* gpio;
    uint16_t pin;
};

static uint32_t const __attribute__((__used__)) myval = 0xffffffff;

void projectMain()
{
    Led led(GPIOD, GPIO_PIN_15);

    // checkFlashCpp = C++ implementaiton, true ok
    // checkFlashC = C implementation, 0 ok
    if (!checkFlashCpp())
    {
        while (1)
        {
            // Failure, lock the device
            SWO_Print("Flash check failed!\n");
            led.toggle();
            HAL_Delay(1000);
        }
    }
    else
    {
        SWO_Print("Flash check successfull!\n");
    }

    while (true)
    {
        led.toggle();
        HAL_Delay(100);
    }
}
