#include <Arduino.h>
#include <NimBLEDevice.h>

// UUID's van jouw LEDnetWF strip
static const NimBLEUUID serviceUUID("FFFF");
static const NimBLEUUID char01UUID("FF01"); // Data poort (Handle 0x0017)
static const NimBLEUUID char02UUID("FF02"); // Config poort (Handle 0x0015)

const char* TARGET_MAC_ADDRESS = "E4:98:BB:43:5D:5B"; 

static bool doConnect = false;
static const NimBLEAdvertisedDevice* advDevice = nullptr; 
static NimBLERemoteCharacteristic* pDataChar;

// --- FUNCTIES VOOR DE STRIP ---

void stuurCommando(uint8_t cmd, uint8_t p1, uint8_t p2, uint8_t p3) {
    if (!pDataChar) return;

    // We gebruiken de 16-byte structuur uit jouw Wireshark log (Frame 13876)
    uint8_t pakket[16] = {
        0x00, 0x14, 0x80, 0x00, 0x00, 0x08, 0x09, 0x0b, 
        cmd, p1, p2, p3, 0x00, 0x00, 0x0f, 0x00
    };

    // Bereken Checksum (Som van byte 9 t/m 15)
    uint8_t cs = 0;
    for(int i = 8; i < 15; i++) cs += pakket[i];
    pakket[15] = cs;

    // Verstuur als Write Command (false)
    pDataChar->writeValue(pakket, 16, false);
}

void aan() { stuurCommando(0x71, 0x23, 0x0f, 0x00); Serial.println("Strip AAN"); }
void uit() { stuurCommando(0x71, 0x24, 0x0f, 0x00); Serial.println("Strip UIT"); }
void blauw(uint8_t helderheid) { stuurCommando(0x31, 0x00, 0x00, helderheid); Serial.println("Kleur BLAUW ingesteld"); }

// --- BLE AFHANDELING ---

class MyScanCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice* device) override {
        if (device->getAddress().equals(NimBLEAddress(TARGET_MAC_ADDRESS, BLE_ADDR_PUBLIC))) {
            advDevice = device;
            doConnect = true;
            NimBLEDevice::getScan()->stop();
        }
    }
};

void setup() {
    Serial.begin(115200);
    NimBLEDevice::init("ESP32-Licht-Controller");
    
    NimBLEScan* pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(new MyScanCallbacks(), false);
    pScan->start(10000, false);
}

void loop() {
    if (doConnect) {
        doConnect = false;
        NimBLEClient* pClient = NimBLEDevice::createClient();
        
        if (pClient->connect(advDevice)) {
            Serial.println("Verbonden!");
            NimBLERemoteService* pSvc = pClient->getService(serviceUUID);
            if (pSvc) {
                pDataChar = pSvc->getCharacteristic(char01UUID);
                NimBLERemoteCharacteristic* pConfChar = pSvc->getCharacteristic(char02UUID);

                // Handshake (Notificaties aan op FF02)
                if (pConfChar) {
                    uint8_t ccc[] = {0x01, 0x00};
                    pConfChar->getDescriptor(NimBLEUUID((uint16_t)0x2902))->writeValue(ccc, 2, true);
                    delay(500);
                }

                // --- TEST SEQUENCE ---
                aan();
                delay(2000);
                blauw(255); // Volledig aan (Blauwe klem)
                delay(5000);
                uit();
                // ---------------------
            }
            delay(1000);
            pClient->disconnect();
            Serial.println("Klaar!");
        }
    }
}