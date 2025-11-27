from machine import UART, Pin
import time

# UART do GPS (NEO-6M)
uart_gps = UART(1, baudrate=9600, tx=Pin(4), rx=Pin(5), timeout=1000)

# UART do Bluetooth (HC-05)
uart_bt = UART(0, baudrate=9600, tx=Pin(0), rx=Pin(1))


def nmea_to_decimal(coord, direction):
    if not coord:
        return None

    try:
        coord = float(coord)
    except ValueError:
        return None

    degrees = int(coord // 100)
    minutes = coord - (degrees * 100)
    decimal = degrees + minutes / 60

    if direction in ("S", "W"):
        decimal = -decimal

    return decimal


def parse_gprmc(sentence):
    parts = sentence.split(',')

    if len(parts) < 7:
        return None

    if parts[2] != 'A':  # A = válido, V = inválido
        return None

    lat = nmea_to_decimal(parts[3], parts[4])
    lon = nmea_to_decimal(parts[5], parts[6])

    if lat is None or lon is None:
        return None

    return lat, lon


def bt_send(text):
    try:
        if isinstance(text, str):
            text = text.encode()
        uart_bt.write(text)
    except Exception as e:
        print("Erro ao enviar BT:", e)


print("Iniciando GPS... Vá para perto de uma janela.")
buffer = b""

while True:

    if uart_gps.any():
        ch = uart_gps.read(1)

        if not ch:
            continue

        if ch in (b'\n', b'\r'):
            # Concluiu uma linha NMEA
            try:
                line = buffer.decode().strip()
            except:
                buffer = b""
                continue

            buffer = b""  # limpa buffer

            if line.startswith("$GPRMC") or line.startswith("$GNRMC"):
                result = parse_gprmc(line)

                if result:
                    lat, lon = result
                    msg = f"Lat: {lat:.6f}, Lon: {lon:.6f}\n"
                    print("GPS:", msg.strip())
                    bt_send(msg)
                else:
                    print("Sem fix GPS")
                    bt_send("Sem sinal GPS\n")  # <--- envia texto escolhido

        else:
            buffer += ch

            # Evita buffer infinito
            if len(buffer) > 200:
                buffer = b""

    time.sleep_ms(5)
