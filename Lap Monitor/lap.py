import psutil
import requests
import time

ESP32_IP = "192.168.4.1"

# Previous network stats
prev_sent = psutil.net_io_counters().bytes_sent
prev_recv = psutil.net_io_counters().bytes_recv

while True:
    time.sleep(1)

    # CPU and RAM
    cpu = int(psutil.cpu_percent())
    ram = int(psutil.virtual_memory().percent)

    # Network TX/RX
    new_sent = psutil.net_io_counters().bytes_sent
    new_recv = psutil.net_io_counters().bytes_recv
    tx_speed = (new_sent - prev_sent) // 1024
    rx_speed = (new_recv - prev_recv) // 1024
    prev_sent = new_sent
    prev_recv = new_recv

    # Uptime in seconds
    uptime_sec = int(time.time() - psutil.boot_time())

    # Send to ESP32
    try:
        url = f"http://{ESP32_IP}/update?cpu={cpu}&ram={ram}&tx={tx_speed}&rx={rx_speed}&uptime={uptime_sec}"
        requests.get(url, timeout=0.5)
    except:
        pass

    print(f"CPU:{cpu}% RAM:{ram}% TX:{tx_speed}KB/s RX:{rx_speed}KB/s Uptime:{uptime_sec}s")