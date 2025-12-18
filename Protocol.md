# Protocol Specification: Zengge / LEDnetWF Bluetooth LE

This document describes the reverse-engineered BLE protocol for the **PhyplusMicro** (Zengge) LED controller (Model: `LEDnetWF020033435D5B`). The controller communicates via Service `0xFFFF`.

## 1. Connection & Handshake
The controller requires a notification handshake before it will accept or execute commands. Without this "activation" step, commands sent to the data characteristic are typically ignored.

* **Service UUID:** `FFFF`
* **Data Characteristic (TX/RX):** `FF01` (Handle: `0x0017`)
* **Config Characteristic (CCC):** `FF02` (Handle: `0x0015`)

**Connection Steps:**
1. Connect to the device MAC address.
2. Write `0x01 0x00` to the Client Characteristic Configuration Descriptor (UUID: `0x2902`) of characteristic `FF02`.
3. Wait approximately 500ms before sending the first command to allow the handshake to register.

## 2. Packet Structure
Commands must be sent as a **Write Command (0x52)** (no-response) to characteristic `FF01`. Standard packets are **16 bytes** long.

| Byte Index | Value | Description |
| :--- | :--- | :--- |
| 0 - 7 | `00 14 80 00 00 08 09 0b` | **Static Header:** Device-specific identifier for this controller type. |
| 8 | `0xXX` | **Command Byte:** Determines the action (Color, Power, etc.). |
| 9 | `0xXX` | **Parameter 1** |
| 10 | `0xXX` | **Parameter 2** |
| 11 | `0xXX` | **Parameter 3** |
| 12 - 14 | `00 00 0f` | **Padding/Mask:** Static trailer for the payload. |
| 15 | `0xXX` | **Checksum:** Sum of Bytes 8 through 14 (8-bit truncating). |

## 3. Command Sets

### 3.1 Power Control
Uses Command Byte `0x71`.
* **ON:** `00 14 80 00 00 08 09 0b 71 23 0f 00 00 00 0f a3`
* **OFF:** `00 14 80 00 00 08 09 0b 71 24 0f 00 00 00 0f a4`

### 3.2 Color Setting (RGB/Dimming)
Uses Command Byte `0x31`.
* **Structure:** `0x31 [Red] [Green] [Blue] 0x00 0x00 0x0f [CS]`
* **Example (Blue at Max):** `31 00 00 ff 00 00 0f 3f`

*Note: For single-color LED strips connected to specific pins, brightness usually maps to one of the RGB parameters. In this project's setup, the strip responds to the **Blue** parameter (Parameter 3).*

## 4. Checksum Calculation
The checksum is calculated by summing all bytes starting from the Command Byte (index 8) up to the end of the padding (index 14).

**Implementation in C++:**
```cpp
uint8_t checksum = 0;
for(int i = 8; i < 15; i++) {
    checksum += packet[i];
}
packet[15] = checksum;