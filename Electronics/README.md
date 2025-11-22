# Eletrônica

Esta pasta contém os arquivos relacionados ao hardware eletrônico do projeto, incluindo o esquema elétrico e, futuramente, o layout da Placa de Circuito Impresso (PCB).

## Esquema Elétrico (`schematic.jpeg`)

O arquivo `schematic.jpeg` apresenta o diagrama de conexões entre o microcontrolador Raspberry Pi Pico e os principais periféricos do sistema.

### Visão Geral dos Componentes

- **Microcontrolador (RP):** O cérebro do projeto é um Raspberry Pi Pico.
- **Driver do Motor de Passo (TMC2209):** Controla o `MOTOR DE PASSO` com alta precisão, responsável por girar o sensor ultrassônico.
- **Regulador de Tensão (VI):** Converte a `FONTE 9V` (conector J1) para uma tensão adequada para alimentar os componentes.
- **Conectores:**
  - `PH1` e `PH2`: Conexão para o sensor ultrassônico (Trigger e Echo).
  - `DIGITAIS PONTE H`: Conector de 8 pinos para os drivers L298N que acionam os motores de vibração.
  - `PWM'S`: Conector de 4 pinos para os sinais PWM que controlam a intensidade dos motores de vibração.
  - `J4`: Conector de 4 pinos, provavelmente para o módulo GPS ou outro periférico.

### Mapeamento de Pinos (Conforme o Esquema)

A tabela abaixo detalha as conexões entre o Raspberry Pi Pico e os demais componentes, conforme ilustrado no esquema elétrico.

| Pino (GPIO) do Pico | Componente Conectado | Função |
|---|---|---|
| GP0 - GP7 | Conector `DIGITAIS PONTE H` | Sinais de controle para os drivers L298N. |
| GP8 - GP11 | Conector `PWM'S` | Sinais PWM para os 4 motores de vibração. |
| GP12 | TMC2209 | Sinal de `STEP` para o motor de passo. |
| GP13 | TMC2209 | Sinal de `DIR` (Direção) para o motor de passo. |
| GP14 | Conector `PH1` | Sinal de `TRIGGER` para o sensor ultrassônico. |
| GP15 | Conector `PH2` | Sinal de `ECHO` do sensor ultrassônico. |
| GP21 | TMC2209 | Sinal de `EN` (Enable) para o driver do motor. |

**Observação Importante:** O mapeamento de pinos neste esquema elétrico **difere** do que está definido no arquivo `Coding/main.c`. Será necessário ajustar o código-fonte para refletir as conexões do hardware final ou vice-versa. O esquema representa a documentação de hardware mais recente.
