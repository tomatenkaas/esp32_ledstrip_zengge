# Protocol Beschrijving: Zengge / LEDnetWF Bluetooth LE

Dit document beschrijft het onderschepte protocol voor de **PhyplusMicro** (Zengge) LED-controller (Model: `LEDnetWF020033435D5B`). De controller werkt via Bluetooth Low Energy (BLE) op Service `0xFFFF`.

## 1. Verbinding & Handshake
Voordat de controller commando's accepteert, moet er een notificatie-handshake plaatsvinden op de configuratie-karakteristiek.

* **Service UUID:** `FFFF`
* **Data Karakteristiek (TX/RX):** `FF01` (Handle: `0x0017`)
* **Config Karakteristiek (CCC):** `FF02` (Handle: `0x0015`)

**Stappen:**
1. Verbind met het MAC-adres.
2. Schrijf `0x01 0x00` naar de Descriptor (`0x2902`) van karakteristiek `FF02` om notificaties te activeren.
3. Wacht ~500ms voordat het eerste commando wordt gestuurd.

## 2. Pakket Structuur
Alle commando's worden verzonden als een **Write Command (0x52)** (geen Request) naar karakteristiek `FF01`. Een pakket is doorgaans **16 bytes** lang.

| Byte | Waarde | Omschrijving |
| :--- | :--- | :--- |
| 0-7 | `00 14 80 00 00 08 09 0b` | **Vaste Header:** Identificatie voor dit type controller. |
| 8 | `0xXX` | **Command Byte:** Bepaalt het type actie (Kleur, Power, etc). |
| 9 | `0xXX` | **Parameter 1** |
| 10 | `0xXX` | **Parameter 2** |
| 11 | `0xXX` | **Parameter 3** |
| 12-14| `00 00 0f` | **Padding/Mask:** Vaste afsluiting van de payload. |
| 15 | `0xXX` | **Checksum:** Som van Byte 8 t/m 14 (8-bit truncating). |

## 3. Commando's

### 3.1 Power (Aan/Uit)
Gebruikt Command Byte `0x71`.
* **AAN:** `00 14 80 00 00 08 09 0b 71 23 0f 00 00 00 0f a3`
* **UIT:** `00 14 80 00 00 08 09 0b 71 24 0f 00 00 00 0f a4`

### 3.2 Kleur Instellen (RGB)
Gebruikt Command Byte `0x31`.
* **Structuur:** `0x31 [R] [G] [B] 0x00 0x00 0x0f [CS]`
* **Voorbeeld (Blauw op max):** `31 00 00 ff 00 00 0f 3f`

*Let op: In de specifieke opstelling van dit project (gele LED-strip) reageert de helderheid op het **Blauwe** kanaal (Parameter 3).*

## 4. Checksum Berekening
De checksum is de som van alle bytes in de payload (vanaf de Command Byte tot en met de laatste padding byte).

**Voorbeeld in C++:**
```cpp
uint8_t cs = 0;
for(int i = 8; i < 15; i++) {
    cs += pakket[i];
}
pakket[15] = cs;