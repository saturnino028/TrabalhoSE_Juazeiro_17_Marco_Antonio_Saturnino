/**
 * TX → DI (Data In do MAX485)
 * RX ← RO (Receiver Output do MAX485)
 * DE (Driver Enable) → Ativa o transmissor. Nível ALTO = transmitir.
 * RE (Receiver Enable) → Ativa o receptor. Nível BAIXO = receber.
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"


// UART defines
#define UART_ID uart1
#define BAUD_RATE 9600

#define UART_TX_PIN 8
#define UART_RX_PIN 9

//Escolhe sentido de transferência
#define DE_RE 4

//LED verde
#define LED_G       11

//Botões
#define bot_A       5 //Botão do pânico
#define bot_B       6 //Campainha



/***************** Variaveis **************/
volatile uint8_t flag_botoes = 0; //Usada para sinalizar eventos de interrupção
volatile uint32_t passado = 0; //Usada para implementar o debouncing

/********** Prototipo de Funcoes **********/
void botoes_callback(uint gpio, uint32_t events); //Callback interrupção
void gpio_init_pins(void); //Função para inicialização dos pinos GPIO

int main()
{
    stdio_init_all();

    printf("System Clock Frequency is %d Hz\n", clock_get_hz(clk_sys));
    printf("USB Clock Frequency is %d Hz\n", clock_get_hz(clk_usb));

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    //Configura a comunicação RS485 para o modo mestre
    gpio_init(DE_RE);
    gpio_set_dir(DE_RE, GPIO_OUT);
    gpio_put(DE_RE, 1);

    //Configuração do botao A
    gpio_init(bot_A);
    gpio_pull_up(bot_A);
    gpio_set_dir(bot_A, GPIO_IN);

    //Configuração do botao B
    gpio_init(bot_B);
    gpio_pull_up(bot_B);
    gpio_set_dir(bot_B, GPIO_IN);

    //Configuração do LED verde
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_put(LED_G, 1);
    
    //Ativação das interrupções
    gpio_set_irq_enabled_with_callback(bot_A, GPIO_IRQ_EDGE_FALL, true, botoes_callback); //Interrupção botão A
    gpio_set_irq_enabled_with_callback(bot_B, GPIO_IRQ_EDGE_FALL, true, botoes_callback); //Interrupção botão B

    while (true) 
    {
        if(flag_botoes == 1)
        {
            uart_puts(UART_ID, "A");
            flag_botoes = 0;
        }
        else if(flag_botoes == 2)
        {
            uart_puts(UART_ID, "B");
            flag_botoes = 0;
        }
        sleep_ms(10);
    }
}


/**
 * @brief trata a interrupção gerada pelos botões A e B da BitDog
 * @param gpio recebe o pino que gerou a interrupção
 * @param events recebe o evento que causou a interrupção
 */
void botoes_callback(uint gpio, uint32_t events)
{
    // Obtém o tempo atual em microssegundos
    uint32_t agora = to_us_since_boot(get_absolute_time());
    // Verifica se passou tempo suficiente desde o último evento
    if (agora - passado > 500000) // 500 ms de debouncing
    {
        passado  = agora;
        if(gpio == bot_A)
            flag_botoes = 1;
        else if(gpio == bot_B)
            flag_botoes = 2;
    }
}