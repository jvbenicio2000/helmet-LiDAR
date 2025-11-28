// gps_bluetooth.c
//
// Sistema de rastreamento GPS com transmissão Bluetooth
// Lê coordenadas do módulo GPS NEO-6M e transmite via Bluetooth HC-05
// Raspberry Pi Pico SDK
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

// Configuração das UARTs
#define UART_GPS_ID uart1
#define UART_GPS_TX_PIN 4
#define UART_GPS_RX_PIN 5
#define UART_GPS_BAUD 9600

#define UART_BT_ID uart0
#define UART_BT_TX_PIN 0
#define UART_BT_RX_PIN 1
#define UART_BT_BAUD 9600

// Buffer para leitura de sentenças NMEA
#define NMEA_BUFFER_SIZE 256

// Estrutura para armazenar coordenadas GPS
typedef struct {
    double latitude;
    double longitude;
    bool valid;
} GPSCoordinates;

// Função para converter coordenadas NMEA para decimal
double nmea_to_decimal(const char* coord_str, char direction) {
    if (!coord_str || strlen(coord_str) == 0) {
        return 0.0;
    }

    double coord = atof(coord_str);
    
    // Extrai graus e minutos
    int degrees = (int)(coord / 100);
    double minutes = coord - (degrees * 100);
    
    // Converte para decimal
    double decimal = degrees + (minutes / 60.0);
    
    // Aplica sinal baseado na direção
    if (direction == 'S' || direction == 'W') {
        decimal = -decimal;
    }
    
    return decimal;
}

// Função para fazer parsing da sentença GPRMC/GNRMC
GPSCoordinates parse_gprmc(const char* sentence) {
    GPSCoordinates coords = {0.0, 0.0, false};
    
    // Cria uma cópia da sentença para tokenização
    char buffer[NMEA_BUFFER_SIZE];
    strncpy(buffer, sentence, NMEA_BUFFER_SIZE - 1);
    buffer[NMEA_BUFFER_SIZE - 1] = '\0';
    
    // Tokeniza a sentença usando vírgula como delimitador
    char* token = strtok(buffer, ",");
    int field_count = 0;
    
    char lat_str[16] = {0};
    char lat_dir = 0;
    char lon_str[16] = {0};
    char lon_dir = 0;
    char status = 0;
    
    while (token != NULL && field_count < 10) {
        switch (field_count) {
            case 2: // Status (A = válido, V = inválido)
                status = token[0];
                break;
            case 3: // Latitude
                strncpy(lat_str, token, sizeof(lat_str) - 1);
                break;
            case 4: // Direção da latitude (N/S)
                lat_dir = token[0];
                break;
            case 5: // Longitude
                strncpy(lon_str, token, sizeof(lon_str) - 1);
                break;
            case 6: // Direção da longitude (E/W)
                lon_dir = token[0];
                break;
        }
        
        token = strtok(NULL, ",");
        field_count++;
    }
    
    // Verifica se o status é válido
    if (status == 'A' && strlen(lat_str) > 0 && strlen(lon_str) > 0) {
        coords.latitude = nmea_to_decimal(lat_str, lat_dir);
        coords.longitude = nmea_to_decimal(lon_str, lon_dir);
        coords.valid = true;
    }
    
    return coords;
}

// Função para enviar dados via Bluetooth
void bt_send(const char* text) {
    if (text) {
        uart_puts(UART_BT_ID, text);
    }
}

// Inicializa as UARTs
void init_uarts(void) {
    // Inicializa UART do GPS
    uart_init(UART_GPS_ID, UART_GPS_BAUD);
    gpio_set_function(UART_GPS_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_GPS_RX_PIN, GPIO_FUNC_UART);
    
    // Inicializa UART do Bluetooth
    uart_init(UART_BT_ID, UART_BT_BAUD);
    gpio_set_function(UART_BT_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_BT_RX_PIN, GPIO_FUNC_UART);
}

int main() {
    // Inicializa stdio
    stdio_init_all();
    
    // Inicializa as UARTs
    init_uarts();
    
    printf("Iniciando GPS... Vá para perto de uma janela.\n");
    bt_send("Sistema GPS iniciado\n");
    
    char nmea_buffer[NMEA_BUFFER_SIZE];
    int buffer_index = 0;
    
    while (true) {
        // Verifica se há dados disponíveis na UART do GPS
        if (uart_is_readable(UART_GPS_ID)) {
            char ch = uart_getc(UART_GPS_ID);
            
            // Verifica fim de linha
            if (ch == '\n' || ch == '\r') {
                if (buffer_index > 0) {
                    // Termina a string
                    nmea_buffer[buffer_index] = '\0';
                    
                    // Verifica se é uma sentença GPRMC ou GNRMC
                    if (strncmp(nmea_buffer, "$GPRMC", 6) == 0 || 
                        strncmp(nmea_buffer, "$GNRMC", 6) == 0) {
                        
                        GPSCoordinates coords = parse_gprmc(nmea_buffer);
                        
                        if (coords.valid) {
                            // Formata e envia as coordenadas
                            char output[128];
                            snprintf(output, sizeof(output), 
                                    "Lat: %.6f, Lon: %.6f\n", 
                                    coords.latitude, coords.longitude);
                            
                            printf("GPS: %s", output);
                            bt_send(output);
                        } else {
                            printf("Sem fix GPS\n");
                            bt_send("Sem sinal GPS\n");
                        }
                    }
                    
                    // Reseta o buffer
                    buffer_index = 0;
                }
            } else {
                // Adiciona caractere ao buffer
                if (buffer_index < NMEA_BUFFER_SIZE - 1) {
                    nmea_buffer[buffer_index++] = ch;
                } else {
                    // Buffer cheio, reseta
                    buffer_index = 0;
                }
            }
        }
        
        // Pequeno delay para não sobrecarregar a CPU
        sleep_ms(5);
    }
    
    return 0;
}
