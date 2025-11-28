// main_enhanced.c
//
// Versão aprimorada do código principal com filtro de média móvel para o sensor ultrassônico.
//
// Varredura com motor de passo (TMC2209) + sensor ultrassônico,
// 4 motores vibratórios controlados por PWM.
// O motor gira 360° no sentido horário, depois 360° no sentido anti-horário,
// repetindo esse ciclo. São feitas 12 medições por volta (a cada 30°).
//
// Ajuste os parâmetros de microstepping conforme a configuração física do TMC2209.
//

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// ------------------------
// Configurações gerais do motor de passo
// ------------------------

// Motor de 1,8° => 200 passos cheios por volta
#define FULL_STEPS_PER_REV        200

// Fator de microstep configurado no TMC2209 (ajuste conforme jumpers / UART)
// Exemplos típicos: 1, 2, 4, 8, 16...
#define MICROSTEPS                8

// Total de pulsos STEP necessários para 1 volta mecânica
#define STEPS_PER_REV   (FULL_STEPS_PER_REV * MICROSTEPS)

// Velocidade do motor de passo (ajuste para mais rápido ou mais devagar)
#define STEP_PULSE_US            50    // largura do pulso STEP em us
#define STEP_MIN_DELAY_US        100   // espera mínima entre passos em us

// ------------------------
// Configurações das medições
// ------------------------

// Número de medições desejadas por volta (pedido: 12 -> 360° / 12 = 30°)
#define MEASUREMENTS_PER_REV     12

// Separação angular entre medições (em graus)
#define ANGLE_BETWEEN_MEASUREMENTS  (360.0f / MEASUREMENTS_PER_REV)

// Distância máxima considerada (cm) para o mapeamento de intensidade
// Aqui começamos a "considerar obstáculo" a partir de 2,5 m.
#define MAX_DISTANCE_CM          250.0f

// Fator de suavização da intensidade (1 = sem suavização, >1 = suaviza mais)
#define SMOOTHING_FACTOR         10

// Habilitar prints de depuração (0 = desliga, 1 = liga)
#define DEBUG                    0

// ------------------------
// Definições p/ ultrassom
// ------------------------
#define TRIGGER_PIN 2
#define ECHO_PIN    3

// ------------------------
// Definições p/ motores de vibração (PWM)
// ------------------------
#define MOTOR_A_PIN 6
#define MOTOR_B_PIN 8
#define MOTOR_C_PIN 10
#define MOTOR_D_PIN 12

// ------------------------
// Definições p/ motor de passo + TMC2209
// ------------------------
#define STEP_PIN    13
#define DIR_PIN     21
#define EN_PIN      5

// ------------------------
// Variável global de ângulo (0..360)
// ------------------------
float angulo = 0.0f;

// ------------------------
// Funções auxiliares
// ------------------------

float read_distance_cm() {
    // Disparo do pulso no TRIGGER
    gpio_put(TRIGGER_PIN, true);
    sleep_us(10);
    gpio_put(TRIGGER_PIN, false);

    // Espera o início do ECHO (subida) com timeout
    uint32_t start_time = time_us_32();
    while (!gpio_get(ECHO_PIN) && (time_us_32() - start_time < 25000)) {
        // espera até ~25 ms para subida (equivalente a ~4 m)
    }

    // Se não subiu, retorna distância máxima (sem obstáculo próximo)
    if (!gpio_get(ECHO_PIN)) {
        return MAX_DISTANCE_CM;
    }

    uint32_t echo_start = time_us_32();

    // Espera o fim do ECHO (descida) com timeout
    while (gpio_get(ECHO_PIN) && (time_us_32() - echo_start < 25000)) {
        // espera até ~25 ms de pulso
    }

    uint32_t echo_end = time_us_32();

    // Cálculo da distância em cm (velocidade do som ~ 343 m/s)
    float distance = (echo_end > echo_start)
                     ? (echo_end - echo_start) / 2.0f / 29.1f
                     : MAX_DISTANCE_CM;

    if (distance > MAX_DISTANCE_CM) distance = MAX_DISTANCE_CM;
    if (distance < 0.0f)            distance = 0.0f;

    return distance;
}

// *** FUNÇÃO DE MELHORIA ***
// Lê 5 amostras, descarta a maior e a menor, e retorna a média das 3 restantes.
float read_distance_filtered(void) {
    const int N = 5;
    float vals[N];

    for (int i = 0; i < N; i++) {
        vals[i] = read_distance_cm();
        sleep_ms(2); // Pequena pausa entre leituras
    }

    // Ordena o array para facilitar a remoção do min/max
    for (int i = 0; i < N - 1; i++) {
        for (int j = i + 1; j < N; j++) {
            if (vals[j] < vals[i]) {
                float temp = vals[i];
                vals[i] = vals[j];
                vals[j] = temp;
            }
        }
    }

    // Calcula a média dos valores centrais (descarta o primeiro e o último)
    float sum = 0;
    for (int i = 1; i < N - 1; i++) {
        sum += vals[i];
    }
    float media = sum / (N - 2);

    return media;
}

void set_motor_pwm(uint gpio, uint16_t level) {
    uint slice   = pwm_gpio_to_slice_num(gpio);
    uint channel = pwm_gpio_to_channel(gpio);
    pwm_set_chan_level(slice, channel, level);
}

void init_motor_pwm(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice, 1000);        // resolução do PWM (0..1000)
    pwm_set_enabled(slice, true);
}

// Dá 1 passo no motor de passo (1 micro-passo, conforme config do TMC2209)
void step_once(void) {
    gpio_put(STEP_PIN, 1);
    sleep_us(STEP_PULSE_US);
    gpio_put(STEP_PIN, 0);
    sleep_us(STEP_MIN_DELAY_US);
}

// Inicializa pinos do TMC2209 em modo STEP/DIR
void init_stepper_pins(void) {
    gpio_init(STEP_PIN);
    gpio_set_dir(STEP_PIN, GPIO_OUT);
    gpio_put(STEP_PIN, 0);

    gpio_init(DIR_PIN);
    gpio_set_dir(DIR_PIN, GPIO_OUT);
    // Vamos começar girando no sentido horário (valor lógico arbitrário)
    gpio_put(DIR_PIN, 1);

    gpio_init(EN_PIN);
    gpio_set_dir(EN_PIN, GPIO_OUT);
    // EN normalmente é ativo em nível baixo: 0 = habilita, 1 = desabilita
    gpio_put(EN_PIN, 0); // habilita o driver
}

int main() {
    stdio_init_all();

    // ------------------------
    // Inicializa ultrassom
    // ------------------------
    gpio_init(TRIGGER_PIN);
    gpio_set_dir(TRIGGER_PIN, GPIO_OUT);
    gpio_put(TRIGGER_PIN, false);

    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
    gpio_pull_down(ECHO_PIN);

    // ------------------------
    // Inicializa motores de vibração (PWM)
    // ------------------------
    init_motor_pwm(MOTOR_A_PIN);
    init_motor_pwm(MOTOR_B_PIN);
    init_motor_pwm(MOTOR_C_PIN);
    init_motor_pwm(MOTOR_D_PIN);

    // ------------------------
    // Inicializa motor de passo (TMC2209)
    // ------------------------
    init_stepper_pins();

    // Posição mecânica atual em passos (0..STEPS_PER_REV-1)
    int pos_step = 0;

    // Contador de passos dentro da volta atual
    int steps_in_rev = 0;

    // Direção atual: true = horário, false = anti-horário
    bool dir_cw = true;

    // Controle de medições por volta (baseado em passos)
    float steps_between_measures = (float)STEPS_PER_REV / (float)MEASUREMENTS_PER_REV;
    float next_measure_step = 0.0f;
    int measurements_this_rev = 0;

    // Intensidade suavizada
    uint16_t last_intensity = 0;

    // Distância atual (mantida entre medições para debug se quiser)
    float distancia = MAX_DISTANCE_CM;

    while (true) {
        // ------------------------
        // Salva ângulo anterior
        // ------------------------
        float angulo_anterior = angulo;

        // ------------------------
        // Dá 1 micro-passo no motor de passo
        // ------------------------
        step_once();

        // Atualiza posição mecânica em função da direção
        if (dir_cw) {
            pos_step++;
            if (pos_step >= STEPS_PER_REV) {
                pos_step = 0;
            }
        } else {
            if (pos_step == 0) {
                pos_step = STEPS_PER_REV - 1;
            } else {
                pos_step--;
            }
        }

        // Atualiza contagem de passos na volta atual
        steps_in_rev++;

        // Converte posição em ângulo (0..360)
        angulo = (pos_step * 360.0f) / (float)STEPS_PER_REV;
        if (angulo >= 360.0f) {
            angulo -= 360.0f;
        }

        // ------------------------
        // Verifica se é hora de medir (12 vezes por volta)
        // ------------------------
        bool do_measure = false;
        if ((float)steps_in_rev >= next_measure_step &&
            measurements_this_rev < MEASUREMENTS_PER_REV) {

            do_measure = true;
            measurements_this_rev++;
            next_measure_step += steps_between_measures;
        }

        if (do_measure) {
            // ------------------------
            // Mede distância (usando a função com filtro)
            // ------------------------
            distancia = read_distance_filtered();

            // Calcula intensidade de vibração (0..1000)
            float raw_intensity =
                1000.0f * (MAX_DISTANCE_CM - distancia) / MAX_DISTANCE_CM;

            if (raw_intensity < 0.0f)   raw_intensity = 0.0f;
            if (raw_intensity > 1000.0f) raw_intensity = 1000.0f;

            uint16_t intensidade = (uint16_t)raw_intensity;

            // Suaviza a intensidade
            intensidade = (uint16_t)(
                (last_intensity * (SMOOTHING_FACTOR - 1) + intensidade)
                / SMOOTHING_FACTOR
            );
            last_intensity = intensidade;

            // ------------------------
            // Lógica de ângulo -> motor A/B/C/D
            // ------------------------
            uint16_t A = 0, B = 0, C = 0, D = 0;

            if (angulo >= 0.0f && angulo < 90.0f) {
                A = intensidade;
            } else if (angulo >= 90.0f && angulo < 180.0f) {
                B = intensidade;
            } else if (angulo >= 180.0f && angulo < 270.0f) {
                C = intensidade;
            } else if (angulo >= 270.0f && angulo < 360.0f) {
                D = intensidade;
            }

            // Aplica PWM nos motores de vibração
            set_motor_pwm(MOTOR_A_PIN, A);
            set_motor_pwm(MOTOR_B_PIN, B);
            set_motor_pwm(MOTOR_C_PIN, C);
            set_motor_pwm(MOTOR_D_PIN, D);

            // Depuração opcional
        #if DEBUG
            printf("Dir: %s, passo=%d/%d, ang=%.2f, dist=%.2f cm, int=%u\n",
                   dir_cw ? "CW" : "CCW",
                   steps_in_rev, STEPS_PER_REV,
                   angulo, distancia, intensidade);
        #endif

            // Pequena pausa opcional após medição (se o sensor precisar)
            sleep_ms(2);
        }

        // ------------------------
        // Verifica fim da volta
        // ------------------------
        if (steps_in_rev >= STEPS_PER_REV) {
            // Reseta contadores da volta
            steps_in_rev = 0;
            measurements_this_rev = 0;
            next_measure_step = 0.0f;

            // Inverte a direção:
            // se estava horário (CW), passa para anti-horário (CCW) e vice-versa
            dir_cw = !dir_cw;
            gpio_put(DIR_PIN, dir_cw ? 1 : 0);

        #if DEBUG
            printf("Mudando direção para: %s\n", dir_cw ? "CW" : "CCW");
        #endif
        }
    }

    return 0;
}
