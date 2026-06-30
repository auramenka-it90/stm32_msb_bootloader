import sys
import struct
import os

# Аппаратный алгоритм CRC32 контроллеров STM32
def stm32_crc32(data):
    crc = 0xFFFFFFFF
    for i in range(0, len(data), 4):
        word = struct.unpack('<I', data[i:i+4])[0]
        crc ^= word
        for _ in range(32):
            if crc & 0x80000000:
                crc = ((crc << 1) ^ 0x04C11DB7) & 0xFFFFFFFF
            else:
                crc = (crc << 1) & 0xFFFFFFFF
    return crc

def patch_bin(filepath):
    print(f"--- Starting CRC patch for: {os.path.basename(filepath)} ---")
    
    with open(filepath, 'rb') as f:
        data = bytearray(f.read())

    # Сигнатура: Magic(55AA55AA) + Version(1) + Size(FFFFFFFF) + CRC(FFFFFFFF)
    signature = struct.pack('<IIII', 0x55AA55AA, 1, 0xFFFFFFFF, 0xFFFFFFFF)
    
    header_offset = data.find(signature)
    if header_offset == -1:
        print("ERROR: fw_header signature not found in binary!")
        sys.exit(1)
        
    print(f"Header found at offset: 0x{header_offset:04X}")

    # Дополняем файл байтами 0xFF кратно 4 байтам
    padding_needed = (4 - (len(data) % 4)) % 4
    if padding_needed > 0:
        data.extend(b'\xFF' * padding_needed)

    total_size = len(data)
    
    # Данные для расчета CRC - это всё, что идет ПОСЛЕ заголовка (16 байт)
    payload_offset = header_offset + 16
    payload_data = data[payload_offset:]
    
    # Считаем CRC
    calculated_crc = stm32_crc32(payload_data)
    
    


    #####
    print(f"Total file size: {total_size} bytes")
    print(f"Calculated CRC32: 0x{calculated_crc:08X}")

    # Вписываем новые Size и CRC
    struct.pack_into('<I', data, header_offset + 8, total_size)
    struct.pack_into('<I', data, header_offset + 12, calculated_crc)

    # Перезаписываем файл
    with open(filepath, 'wb') as f:
        f.write(data)
        
    print("--- Patch successful! ---")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python patch_crc.py <path_to_bin_file>")
        sys.exit(1)
    
    patch_bin(sys.argv[1])