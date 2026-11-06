#!/usr/bin/env python3
"""
AbioticTrackIR: OpenTrack & FreeTrack Diagnostic Tool
Tests connection to OpenTrack FreeTrack 2.0 Shared Memory and UDP port 4242.
"""
import ctypes
import struct
import time
import socket
from ctypes import wintypes

def test_shared_memory():
    print("--------------------------------------------------")
    print("[1] Testing FreeTrack Shared Memory ('FT_SharedMem')...")
    FILE_MAP_READ = 4
    kernel32 = ctypes.windll.kernel32
    kernel32.OpenFileMappingA.restype = wintypes.HANDLE
    kernel32.OpenFileMappingA.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.LPCSTR]
    kernel32.MapViewOfFile.restype = ctypes.c_void_p
    kernel32.MapViewOfFile.argtypes = [wintypes.HANDLE, wintypes.DWORD, wintypes.DWORD, wintypes.DWORD, ctypes.c_size_t]

    hMap = kernel32.OpenFileMappingA(FILE_MAP_READ, False, b"FT_SharedMem")
    if not hMap:
        print("[-] 'FT_SharedMem' NOT found. Make sure OpenTrack is running with Output: 'freetrack 2.0 Enhanced'.")
        return False

    print("[+] Successfully opened 'FT_SharedMem' handle:", hMap)
    pView = kernel32.MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 1024)
    if not pView:
        print("[-] Failed to map view of file.")
        kernel32.CloseHandle(hMap)
        return False

    buf = ctypes.string_at(pView, 36)
    data_id, w, h, yaw, pitch, roll, x, y, z = struct.unpack('<3i6f', buf)
    RAD_TO_DEG = 180.0 / 3.141592653589793
    print(f"[+] Active Frame ID: {data_id}")
    print(f"    Yaw:   {yaw * RAD_TO_DEG:7.2f} deg")
    print(f"    Pitch: {pitch * RAD_TO_DEG:7.2f} deg")
    print(f"    Roll:  {roll * RAD_TO_DEG:7.2f} deg")
    print(f"    X:     {x * 0.1:7.2f} cm (Lateral)")
    print(f"    Y:     {y * 0.1:7.2f} cm (Depth)")
    print(f"    Z:     {z * 0.1:7.2f} cm (Vertical)")

    kernel32.UnmapViewOfFile(ctypes.c_void_p(pView))
    kernel32.CloseHandle(hMap)
    return True

def test_udp_socket(port=4242):
    print("--------------------------------------------------")
    print(f"[2] Testing OpenTrack UDP Listener on port {port}...")
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.settimeout(0.5)
        sock.bind(('127.0.0.1', port))
        print(f"[+] Listening on 127.0.0.1:{port} for 1 second...")
        data, addr = sock.recvfrom(1024)
        if len(data) == 48:
            vals = struct.unpack('<6d', data)
            print(f"[+] Received 48-byte OpenTrack UDP packet from {addr}:")
            print(f"    X: {vals[0]:.2f} cm, Y: {vals[1]:.2f} cm, Z: {vals[2]:.2f} cm")
            print(f"    Yaw: {vals[3]:.2f}°, Pitch: {vals[4]:.2f}°, Roll: {vals[5]:.2f}°")
            sock.close()
            return True
        else:
            print(f"[?] Received packet of unexpected length: {len(data)} bytes")
            sock.close()
            return True
    except socket.timeout:
        print("[-] UDP packet timeout (no UDP packets received). OpenTrack is likely using Shared Memory output (Recommended!).")
        return False
    except Exception as e:
        print(f"[-] Socket test info: {e}")
        return False

if __name__ == '__main__':
    print("==================================================")
    print("  AbioticTrackIR: OpenTrack Diagnostics")
    print("==================================================")
    sm_ok = test_shared_memory()
    udp_ok = test_udp_socket()
    print("==================================================")
    if sm_ok or udp_ok:
        print(">>> SUCCESS: OpenTrack tracking data is active and ready for Abiotic Factor! <<<")
    else:
        print(">>> NOTICE: Launch OpenTrack and click 'Start' to begin streaming. <<<")
