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

## `gps_bluetooth.c` (Versão em C)

- **Microcontrolador Alvo:** Raspberry Pi Pico
- **Linguagem:** C (com Raspberry Pi Pico SDK)

### Descrição

Esta é a **versão em C** da funcionalidade de rastreamento GPS, convertida a partir do script MicroPython. As funcionalidades são idênticas:

1.  **Leitura do GPS:** Utiliza a UART1 para ler dados NMEA do módulo GPS NEO-6M.
2.  **Parsing de Dados:** Processa as sentenças `$GPRMC` ou `$GNRMC` para extrair status, latitude e longitude.
3.  **Conversão de Coordenadas:** Converte as coordenadas NMEA para o formato de graus decimais.
4.  **Transmissão Bluetooth:** Envia as coordenadas formatadas via UART0 para um módulo Bluetooth HC-05.

### Pinos Utilizados (Raspberry Pi Pico)

O mapeamento de pinos é o mesmo da versão em Python:

| Função | Pino (GPIO) do Pico | Módulo |
|---|---|---|
| **GPS (UART 1)** | |
| TX | 4 | RX do NEO-6M |
| RX | 5 | TX do NEO-6M |
| **Bluetooth (UART 0)** | |
| TX | 0 | RX do HC-05 |
| RX | 1 | TX do HC-05 |

### Compilação

Para compilar este código, é necessário ter o ambiente de desenvolvimento para o Raspberry Pi Pico SDK configurado. O código deve ser compilado junto com os arquivos de build do SDK (CMakeLists.txt, etc.).


## `main_enhanced.c` (Versão Aprimorada)

- **Microcontrolador Alvo:** Raspberry Pi Pico
- **Linguagem:** C

### Descrição

Esta é uma **versão aprimorada** do código `main.c`, com foco em melhorar a precisão e a estabilidade das medições do sensor ultrassônico.

### Melhorias Implementadas

1.  **Filtro de Média Móvel (`read_distance_filtered()`):**
    -   **Problema Resolvido:** Sensores ultrassônicos podem sofrer com leituras espúrias (picos de ruído) que geram dados incorretos e instáveis.
    -   **Solução:** Em vez de usar uma única leitura, esta função coleta **5 amostras** de distância em rápida sucessão.
    -   Ela então **descarta a menor e a maior** leitura (que são as mais prováveis de serem ruído) e calcula a **média das 3 leituras restantes**.
    -   **Resultado:** A distância retornada é muito mais estável e confiável, reduzindo falsos positivos e tornando a resposta do sistema de vibração mais consistente.

2.  **Código Mais Robusto:** A lógica de ordenação (bubble sort) garante que os valores mínimo e máximo sejam sempre descartados corretamente.

O restante da lógica (controle do motor de passo, PWM, etc.) permanece o mesmo, mas agora opera com dados de distância de maior qualidade.


## `gps_geofencing.py` (Versão Aprimorada com Geofencing)

- **Microcontrolador Alvo:** Raspberry Pi Pico W
- **Linguagem:** MicroPython

### Descrição

Esta é uma **versão avançada** do script de GPS, que adiciona a funcionalidade de **geofencing** (cercas virtuais) para reconhecer pontos de interesse pré-definidos.

### Melhorias e Funcionalidades

1.  **Filtro de Média Móvel (`get_filtered_position()`):**
    -   Assim como no `main_enhanced.c`, este código coleta múltiplas amostras de GPS para calcular uma posição média, resultando em uma localização muito mais estável e precisa.

2.  **Cálculo de Distância Haversine (`distance_m()`):**
    -   Implementa a fórmula de Haversine para calcular com precisão a distância em metros entre duas coordenadas geográficas, levando em conta a curvatura da Terra.

3.  **Geofencing com Waypoints (`check_location()`):**
    -   Você pode definir uma lista de **pontos de interesse** (waypoints) com nome, latitude, longitude e um raio em metros.
    -   O código verifica continuamente se a posição atual do usuário está dentro do raio de algum desses waypoints.

4.  **Notificações de Chegada (`main()`):**
    -   Quando o usuário entra na área de um waypoint, o sistema envia uma notificação via Bluetooth (ex: "Você chegou perto de: Casa").
    -   Ele evita notificações repetidas, enviando o aviso apenas uma vez ao entrar na área.

### Uso

-   **Personalize os Waypoints:** Edite a lista `WAYPOINTS` no código para adicionar os locais de interesse para o usuário.
-   **Ação Personalizada:** No laço principal, há um local marcado (`>>> AQUI você pode colocar a reação do sistema <<<`) onde você pode adicionar uma ação específica quando um waypoint é alcançado (ex: acender um LED, tocar um som, etc.).
