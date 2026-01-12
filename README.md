# ESP32 Lap Timer Project

## Overview
Two ESP32 MCUs communicating via ESP-NOW for lap timing applications.

## Project Structure
- `transmitter/` - Broadcasts packets every 100ms with sequence numbers
- `receiver/` - Receives packets and monitors connection status
- `components/esp_now_protocol/` - Shared protocol definitions
- `build/` - Compiled binaries (created after build)

## Features
- **Transmitter**: Broadcasts sequence-numbered packets every 100ms
- **Receiver**: Logs received packets and warns if no data for >500ms
- **Range Control**: Channel 1, no Wi-Fi AP, broadcast mode for short range

## Building and Flashing

### Quick Build (from project root)
```bash
# Build both applications
./build_all.sh

# Clean all build artifacts
./clean_all.sh

# Clean everything including parent build directory
./clean_all.sh --all
```

### Individual Build
```bash
cd transmitter
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor

cd ../receiver
idf.py build
idf.py -p /dev/ttyUSB1 flash monitor
```

### Flash from Parent Build
```bash
# After running ./build_all.sh, binaries are in build/
idf.py -p /dev/ttyUSB0 write_flash 0x1000 build/transmitter_*.bin
idf.py -p /dev/ttyUSB1 write_flash 0x1000 build/receiver_*.bin
```

## Expected Behavior
1. Receiver shows "Connection established" when first packet arrives
2. Receiver logs each sequence number received
3. Receiver shows warning if no packets for >500ms
4. Effective range: few meters (broadcast + low power)
