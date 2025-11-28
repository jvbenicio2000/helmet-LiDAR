# gps_geofencing.py
#
# Versão aprimorada do código GPS com geofencing e filtro de média móvel.
#
from machine import UART, Pin
import time
import math

# ---------- Configuração das UARTs ----------
# GPS: NEO-6M na UART1 (GP4 = TX, GP5 = RX)
uart_gps = UART(1, baudrate=9600, tx=Pin(4), rx=Pin(5), timeout=1000)

# Bluetooth: HC-05 na UART0 (GP0 = TX, GP1 = RX) - opcional
uart_bt = UART(0, baudrate=9600, tx=Pin(0), rx=Pin(1))


# ---------- Tabela de pontos de interesse (waypoints) ----------
# Altere para os lugares que você quer reconhecer.
# lat / lon em graus decimais, radius_m em metros.
WAYPOINTS = [
    {"name": "Casa",      "lat": -23.555000, "lon": -46.630000, "radius_m": 30},
    {"name": "Loja 733",  "lat": -23.542000, "lon": -46.619000, "radius_m": 30},
    {"name": "Faculdade", "lat": -23.561000, "lon": -46.654000, "radius_m": 40},
    # Adicione mais pontos aqui, se quiser
]


# ---------- Função auxiliar: enviar texto via Bluetooth ----------
def bt_send(text):
    try:
        if isinstance(text, str):
            text = text.encode()
        uart_bt.write(text)
    except Exception as e:
        print("Erro BT:", e)


# ---------- Conversão NMEA -> graus decimais ----------
def nmea_to_decimal(coord, direction):
    """
    Converte coordenada NMEA (ddmm.mmmm) para graus decimais.
    direction: 'N', 'S', 'E', 'W'
    """
    if not coord:
        return None
    try:
        coord = float(coord)
    except:
        return None

    graus = int(coord // 100)
    minutos = coord - graus * 100
    decimal = graus + minutos / 60.0

    if direction in ("S", "W"):
        decimal = -decimal
    return decimal


# ---------- Parser de sentença GPRMC / GNRMC ----------
def parse_gprmc(sentence):
    """
    Recebe uma sentença NMEA GPRMC/GNRMC e retorna (lat, lon) em graus decimais,
    ou None se inválida.
    """
    parts = sentence.split(',')

    # Tamanho mínimo esperado
    if len(parts) < 7:
        return None

    # parts[2] = 'A' (ativo/válido), 'V' (inválido)
    if parts[2] != 'A':
        return None

    lat = nmea_to_decimal(parts[3], parts[4])
    lon = nmea_to_decimal(parts[5], parts[6])

    if lat is None or lon is None:
        return None

    return lat, lon


# ---------- Lê UMA posição válida do GPS (com timeout) ----------
def read_one_fix(uart, timeout_s=5):
    """
    Tenta ler uma posição válida (lat, lon) a partir do UART do GPS
    dentro de timeout_s segundos.
    Se conseguir, retorna (lat, lon). Caso contrário, retorna None.
    """
    buffer = b""
    t0 = time.ticks_ms()

    while time.ticks_diff(time.ticks_ms(), t0) < timeout_s * 1000:
        if uart.any():
            ch = uart.read(1)
            if not ch:
                continue

            if ch in (b'\n', b'\r'):
                # Fim de uma linha NMEA
                try:
                    line = buffer.decode().strip()
                except:
                    buffer = b""
                    continue

                buffer = b""

                if line.startswith("$GPRMC") or line.startswith("$GNRMC"):
                    result = parse_gprmc(line)
                    if result:
                        return result
                # Se não for RMC, simplesmente ignora e continua
            else:
                buffer += ch
                # Limita tamanho do buffer para evitar estouro
                if len(buffer) > 200:
                    buffer = b""
        else:
            # Sem dados disponíveis, espera um pouco
            time.sleep_ms(20)

    # Estourou o tempo sem fix válido
    return None


# ---------- Faz média de várias posições para reduzir ruído ----------
def get_filtered_position(uart, samples=5, timeout_total_s=20):
    """
    Tenta obter até 'samples' posições válidas dentro de 'timeout_total_s' segundos
    e retorna a média de latitude e longitude.
    Se não conseguir nenhuma posição válida, retorna None.
    """
    lats = []
    lons = []

    t0 = time.ticks_ms()
    while len(lats) < samples and time.ticks_diff(time.ticks_ms(), t0) < timeout_total_s * 1000:
        fix = read_one_fix(uart, timeout_s=3)
        if fix:
            lat, lon = fix
            lats.append(lat)
            lons.append(lon)
            print("Fix válido:", lat, lon)
        else:
            print("Sem fix válido ainda...")
        # Pequena pausa entre leituras
        time.sleep_ms(200)

    if not lats:
        # Nenhuma posição válida
        return None

    # Média simples das amostras (já reduz bastante o ruído)
    lat_avg = sum(lats) / len(lats)
    lon_avg = sum(lons) / len(lons)
    return lat_avg, lon_avg


# ---------- Distância entre dois pontos (Haversine) em metros ----------
def distance_m(lat1, lon1, lat2, lon2):
    """
    Calcula a distância aproximada entre duas coordenadas em metros
    usando a fórmula de Haversine.
    """
    R = 6371000.0  # Raio médio da Terra em metros
    phi1 = math.radians(lat1)
    phi2 = math.radians(lat2)
    dphi = math.radians(lat2 - lat1)
    dlambda = math.radians(lon2 - lon1)

    a = math.sin(dphi/2)**2 + math.cos(phi1)*math.cos(phi2)*math.sin(dlambda/2)**2
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))
    return R * c


# ---------- Verifica se a posição atual pertence a algum waypoint ----------
def check_location(lat, lon):
    """
    Verifica se (lat, lon) está dentro do raio de algum waypoint.
    Retorna:
        (name, dist_m) se estiver dentro de algum raio,
        (None, None)   caso contrário.
    """
    closest_name = None
    closest_dist = None

    for wp in WAYPOINTS:
        d = distance_m(lat, lon, wp["lat"], wp["lon"])
        if d <= wp["radius_m"]:
            # Entrou na área deste waypoint
            return wp["name"], d

        # Guarda o mais próximo (apenas para debug, se quiser usar)
        if closest_dist is None or d < closest_dist:
            closest_dist = d
            closest_name = wp["name"]

    # Não está em nenhum dos waypoints definidos
    return None, None


# ---------- Função de alto nível: posição + nome do lugar ----------
def get_position_and_place():
    """
    Faz leitura filtrada da posição e verifica se está em algum waypoint.
    Retorna (lat, lon, place_name).
    Se não houver posição confiável, retorna (None, None, None).
    """
    pos = get_filtered_position(uart_gps, samples=5, timeout_total_s=20)
    if not pos:
        return None, None, None

    lat, lon = pos
    place_name, dist = check_location(lat, lon)
    return lat, lon, place_name


# ---------- Exemplo de laço principal ----------
def main():
    """
    Laço principal de exemplo:
    - Lê posição filtrada
    - Envia latitude/longitude via BT
    - Se entrar em um waypoint, envia o nome do lugar e pode disparar alguma ação.
    """
    last_place = None

    print("Iniciando GPS, vá para perto de uma janela...")
    bt_send("GPS iniciado.\n")

    while True:
        lat, lon, place = get_position_and_place()

        if lat is None:
            print("Não consegui posição confiável.")
            bt_send("Sem fix GPS confiável.\n")
        else:
            msg = f"Lat: {lat:.6f}, Lon: {lon:.6f}\n"
            print(msg.strip())
            bt_send(msg)

            if place:
                # Entrou na área de algum ponto conhecido
                if place != last_place:
                    aviso = f"Você chegou perto de: {place}\n"
                    print(aviso.strip())
                    bt_send(aviso)

                    # >>> AQUI você pode colocar a reação do sistema <<<
                    # Exemplo: acender um LED na GPIO 15:
                    # led = Pin(15, Pin.OUT)
                    # led.value(1)

                last_place = place
            else:
                # Não está em nenhum ponto conhecido
                last_place = None

        # Ajuste o intervalo conforme sua necessidade
        time.sleep(2)


# Descomente a linha abaixo se quiser que o programa rode automaticamente ao ligar o Pico:
# main()
