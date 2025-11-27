# Código-Fonte

Esta pasta contém o código-fonte para o microcontrolador do projeto Helmet-LiDAR.

## `main.c`

- **Microcontrolador Alvo:** Raspberry Pi Pico
- **Linguagem:** C

### Descrição

Este é o programa principal que orquestra todo o funcionamento do capacete. A lógica implementada realiza as seguintes tarefas:

1.  **Controle do Motor de Passo:** Utiliza um driver TMC2209 para girar o motor de passo em um ciclo contínuo: 360° no sentido horário, seguido por 360° no sentido anti-horário.

2.  **Varredura do Ambiente:** Realiza 12 medições de distância com o sensor ultrassônico a cada volta completa (uma medição a cada 30°).

3.  **Leitura do Sensor Ultrassônico:** Envia um pulso de trigger e mede a duração do pulso de echo para calcular a distância de obstáculos em centímetros.

4.  **Controle dos Motores de Vibração:**
    - A intensidade da vibração é inversamente proporcional à distância medida (quanto mais perto o obstáculo, mais forte a vibração).
    - A direção do obstáculo, determinada pelo ângulo do motor de passo, define qual dos quatro motores de vibração será acionado:
        - **Motor A (Frente):** 0° a 90°
        - **Motor B (Direita):** 90° a 180°
        - **Motor C (Atrás):** 180° a 270°
        - **Motor D (Esquerda):** 270° a 360°
    - O controle da intensidade é feito via PWM (Pulse Width Modulation).

### Pinos Utilizados (Raspberry Pi Pico)

| Função | Pino (GPIO) |
|---|---|
| **Sensor Ultrassônico** | |
| TRIGGER | 2 |
| ECHO | 3 |
| **Motores de Vibração** | |
| Motor A (Frente) | 6 |
| Motor B (Direita) | 8 |
| Motor C (Atrás) | 10 |
| Motor D (Esquerda) | 12 |
| **Motor de Passo (TMC2209)** | |
| STEP | 13 |
| DIR | 21 |
| EN | 5 |

### Compilação

Para compilar este projeto, é necessário ter o ambiente de desenvolvimento para o Raspberry Pi Pico configurado. Siga o guia oficial "Getting started with Raspberry Pi Pico" para mais detalhes.


## `gps_bluetooth.py`

- **Microcontrolador Alvo:** Raspberry Pi Pico W
- **Linguagem:** MicroPython

### Descrição

Este script é responsável por uma funcionalidade secundária do projeto: **rastreamento por GPS e comunicação via Bluetooth**.

1.  **Leitura do GPS:** O código utiliza uma comunicação serial (UART) para ler os dados brutos (sentenças NMEA) de um módulo GPS NEO-6M.

2.  **Parsing de Dados:** Ele processa especificamente as sentenças `$GPRMC` ou `$GNRMC`, que contêm as informações essenciais de geolocalização (latitude e longitude).

3.  **Conversão de Coordenadas:** Converte o formato de coordenadas NMEA para o formato de graus decimais, que é mais comum e fácil de usar em mapas.

4.  **Transmissão Bluetooth:** As coordenadas de latitude e longitude formatadas são então enviadas via Bluetooth (usando um módulo HC-05) para um dispositivo pareado, como um smartphone. Isso permite o monitoramento remoto da localização do usuário.

### Pinos Utilizados (Raspberry Pi Pico)

| Função | Pino (GPIO) do Pico | Módulo |
|---|---|---|
| **GPS (UART 1)** | |
| TX | 4 | RX do NEO-6M |
| RX | 5 | TX do NEO-6M |
| **Bluetooth (UART 0)** | |
| TX | 0 | RX do HC-05 |
| RX | 1 | TX do HC-05 |

**Nota:** Este script opera de forma independente do `main.c` e requer uma segunda Raspberry Pi Pico, conforme especificado na lista de materiais.
