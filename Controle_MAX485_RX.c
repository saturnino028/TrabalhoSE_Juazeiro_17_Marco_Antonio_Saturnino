/**
 * Sistema de Comando
 * TX → DI → 8 (Data In do MAX485)
 * 9 ← RX ← RO (Receiver Output do MAX485)
 * DE (Driver Enable) → Ativa o transmissor. Nível ALTO = transmitir.
 * RE (Receiver Enable) → Ativa o receptor. Nível BAIXO = receber.
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/uart.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"

// UART defines
#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 8
#define UART_RX_PIN 9

//Canal para controle do servo
#define servo 28

//Escolhe sentido de transferência
#define DE_RE 4

//Top Wrap
#define top 25000

#define LED_G       11 //LED verde
#define LED_B       12 //LED verde
#define LED_R       13 //LED vermelho

/********** Prototipo de Funcoes **********/
void duty_cicle(float _percent, uint _slice, uint8_t _pin); //Valor percentual do duty cicle (natural)

int main()
{
    stdio_init_all();

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    //Configura o conversor para o modo escravo
    gpio_init(DE_RE); 
    gpio_set_dir(DE_RE, GPIO_OUT);
    gpio_put(DE_RE, 0);

    //Configuração do LED vermelho
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_put(LED_R, 0);

    //Configuração do LED verde
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_put(LED_G, 0);

    //Configuração do LED verde
    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);
    gpio_put(LED_B, 1);

    //Habilita a função PWM no pino do Servo
    gpio_set_function(servo, GPIO_FUNC_PWM);
    uint slice_pwm = pwm_gpio_to_slice_num(servo); //Obter o canal PWM Servo
    pwm_set_clkdiv(slice_pwm, 100.0); //Define o divisor de clock do PWM do Servo
    pwm_set_wrap(slice_pwm, top); //Definir valor do WRAP Servo

    sleep_ms(5000);

    while (true) 
    {
        if (uart_is_readable(UART_ID)) 
        {
            uint8_t c = uart_getc(UART_ID);
            printf("%c", c);

            switch (c)
            {
                case 'A':
                    duty_cicle(7.35, slice_pwm, servo); //Duty cicle 7.35% - Posição 90° do servo (1470 us)
                    gpio_put(LED_R, 0);
                    gpio_put(LED_G, 1);
                    gpio_put(LED_B, 0);
                    sleep_ms(3000);
                    duty_cicle(0.0, slice_pwm, servo); //Desabilita PWM
                    gpio_put(LED_R, 0);
                    gpio_put(LED_G, 0);
                    gpio_put(LED_B, 1);
                    c = 0;
                break;
                case 'B':
                    //duty_cicle(12.0, slice_pwm, servo); //Duty cicle 12% - Posição 180° do servo (2400us)
                    duty_cicle(2.5, slice_pwm, servo); //Duty cicle 2.5% - Posição 0° do servo (500 us)
                    gpio_put(LED_R, 1);
                    gpio_put(LED_G, 0);
                    gpio_put(LED_B, 0);
                    sleep_ms(3000);
                    duty_cicle(0.0, slice_pwm, servo); //Desabilita PWM
                    gpio_put(LED_R, 0);
                    gpio_put(LED_G, 0);
                    gpio_put(LED_B, 1);
                    c = 0;
                break;

                default:
                    gpio_put(LED_R, 0);
                    gpio_put(LED_G, 0);
                    gpio_put(LED_B, 1);
                    c = 0;
                break;
            }
        }
    }
}

void duty_cicle(float _percent, uint _slice, uint8_t _pin)
{
    pwm_set_enabled(_slice, false); //Desabilita PWM

    uint16_t valor_pwm = (_percent/100) * top;
    pwm_set_gpio_level(_pin, valor_pwm);

    pwm_set_enabled(_slice, true); //Habilitar PWM
}
