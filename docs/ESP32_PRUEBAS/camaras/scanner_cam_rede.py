import scapy.all as scapy
import socket
import netifaces
import requests
import tkinter as tk
from tkinter import ttk
from concurrent.futures import ThreadPoolExecutor

# Puertos comunes de cámaras IP
COMMON_PORTS = [80, 554, 8080, 443, 8000, 8554]

# Base de datos de marcas de cámaras según MAC (Ejemplo)
MAC_BRANDS = {
    "00:1A:79": "Hikvision",
    "00:1B:C1": "Dahua",
    "00:50:C2": "Axis",
    "00:0E:6D": "Samsung",
    "B0:C5:54": "Sony",
    "00:15:5D": "TP-Link",
}

def get_local_network():
    """Obtiene la dirección de la red local."""
    iface = netifaces.gateways()['default'][netifaces.AF_INET][1]
    ip_info = netifaces.ifaddresses(iface)[netifaces.AF_INET][0]
    ip = ip_info['addr']
    mask = ip_info['netmask']
    
    # Convertir máscara a prefijo (ejemplo: 255.255.255.0 -> /24)
    prefix = sum(bin(int(x)).count('1') for x in mask.split('.'))
    return f"{ip}/{prefix}"

def scan_network(network):
    """Escanea la red WiFi para encontrar dispositivos activos."""
    devices = []
    arp_request = scapy.ARP(pdst=network)
    broadcast = scapy.Ether(dst="ff:ff:ff:ff:ff:ff")
    arp_request_broadcast = broadcast / arp_request
    answered = scapy.srp(arp_request_broadcast, timeout=2, verbose=False)[0]
    
    for packet in answered:
        mac = packet[1].hwsrc
        brand = "Desconocido"
        
        # Identificación por MAC
        for prefix, name in MAC_BRANDS.items():
            if mac.upper().startswith(prefix):
                brand = name
                break

        devices.append({"ip": packet[1].psrc, "mac": mac, "brand": brand})
    
    return devices

def scan_ports(ip):
    """Escanea los puertos comunes de cámaras IP."""
    for port in COMMON_PORTS:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(1)
        result = sock.connect_ex((ip, port))
        if result == 0:
            return port
        sock.close()
    return None

def is_router_connected():
    """Verifica si el router está conectado a otro router (puerta de enlace)."""
    try:
        gateways = netifaces.gateways()
        default_gateway = gateways['default'][netifaces.AF_INET][0]
        if default_gateway:
            return f"El router está conectado a otro router: {default_gateway}"
    except Exception as e:
        return f"Error detectando router adicional: {e}"
    return "No se detectó conexión con otro router."

def scan_and_display():
    """Realiza el escaneo y muestra los resultados en la interfaz gráfica."""
    text_output.delete(1.0, tk.END)
    text_output.insert(tk.END, "🔍 Escaneando la red...\n")

    network = get_local_network()
    devices = scan_network(network)

    # Verificar si el router está conectado a otro router
    router_status = is_router_connected()
    text_output.insert(tk.END, f"🔌 {router_status}\n\n")
    
    text_output.insert(tk.END, f"🔎 Dispositivos encontrados: {len(devices)}\n")
    
    with ThreadPoolExecutor(max_workers=20) as executor:
        for device in devices:
            port = executor.submit(scan_ports, device["ip"]).result()
            if port:
                text_output.insert(tk.END, f"📷 Cámara detectada en {device['ip']}:{port} ({device['brand']})\n")

# ---------------------- INTERFAZ GRÁFICA ----------------------
root = tk.Tk()
root.title("Scanner de Cámaras IP")
root.geometry("500x400")

frame = ttk.Frame(root, padding=10)
frame.pack(fill="both", expand=True)

label = ttk.Label(frame, text="Escaneo de Cámaras IP en Red WiFi", font=("Arial", 12, "bold"))
label.pack(pady=10)

scan_button = ttk.Button(frame, text="Iniciar Escaneo", command=scan_and_display)
scan_button.pack(pady=5)

text_output = tk.Text(frame, height=15, width=60)
text_output.pack()

root.mainloop()
