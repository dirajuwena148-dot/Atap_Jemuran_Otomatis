# 02 - Instalasi Software (Arduino IDE, Board, Library)

## 1. Install Arduino IDE

Unduh dan install **Arduino IDE** dari <https://www.arduino.cc/en/software>.

## 2. Tambahkan board ESP8266

1. Buka **File > Preferences**.
2. Pada kolom **Additional Boards Manager URLs**, tambahkan:
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
3. Buka **Tools > Board > Boards Manager**, cari **esp8266**, lalu klik **Install**.
4. Pilih board: **Tools > Board > esp8266 > NodeMCU 1.0 (ESP-12E Module)**.
5. Pilih port: **Tools > Port > COMx** (Windows) atau `/dev/ttyUSBx` (Linux).

Jika port tidak muncul, install driver USB-Serial sesuai chip di NodeMCU Anda (CH340 atau CP2102).

## 3. Install library

Buka **Sketch > Include Library > Manage Libraries**, cari dan install satu per satu:

| Library | Pembuat | Fungsi |
|---|---|---|
| **Blynk** | Volodymyr Shymanskyy | Koneksi ke Blynk Cloud |
| **DHT sensor library** | Adafruit | Membaca DHT11 |
| **Adafruit Unified Sensor** | Adafruit | Dibutuhkan oleh library DHT |
| **AccelStepper** | Mike McCauley | Menggerakkan motor stepper |

Library `ESP8266WiFi` dan `EEPROM` sudah ikut dalam board package esp8266.

## 4. Buka dan isi source code

1. Buka file `atap_jemuran_blynk/atap_jemuran_blynk.ino`. Nama folder dan nama file `.ino` harus sama, jangan diubah salah satunya.
2. Isi bagian atas file:

```cpp
#define BLYNK_TEMPLATE_ID   "ISI_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Atap Jemuran Otomatis"
#define BLYNK_AUTH_TOKEN    "ISI_AUTH_TOKEN"

const char* WIFI_SSID = "NAMA_WIFI";
const char* WIFI_PASS = "PASSWORD_WIFI";
```

- Tiga nilai Blynk didapat dari **Device Info** di Blynk Console. Lihat [04-setup-blynk.md](04-setup-blynk.md).
- `WIFI_SSID` harus jaringan **2,4 GHz**. ESP8266 tidak bisa tersambung ke WiFi 5 GHz (biasanya bernama dengan akhiran "5G").
- Ketiga `#define` Blynk **harus berada sebelum** baris `#include <BlynkSimpleEsp8266.h>`.

## 5. Upload

1. Hubungkan NodeMCU ke komputer lewat kabel USB **data** (bukan kabel khusus pengisi daya).
2. Klik **Upload** (ikon panah).
3. Jika gagal tersambung, tahan tombol **FLASH** di NodeMCU saat tulisan "Connecting..." muncul, lalu lepas.
4. Setelah selesai, buka **Tools > Serial Monitor** dan atur baud ke **115200**.

## 6. Pengaturan opsional di source code

Semua ada di bagian "PENGATURAN BAWAAN":

| Konstanta | Bawaan | Fungsi |
|---|---|---|
| `LDR_GELAP` | `HIGH` | Nilai pin LDR saat gelap. Ubah ke `LOW` jika terbalik |
| `AMBANG_LEMBAP` | `85.0` | Atap menutup jika kelembapan di atas nilai ini (%) |
| `MARGIN_AIR` | `100` | Kenaikan nilai sensor air dari kondisi kering untuk dianggap hujan |
| `KECEPATAN` | `600` | Kecepatan motor (langkah per detik) |
| `JARAK_BAWAAN` | `6000` | Jumlah langkah menutup bila belum dikalibrasi |
| `BATAS_WAKTU_CARI` | `30000` | Batas waktu mencari limit switch BUKA (ms) |
| `JUMLAH_KONFIRMASI` | `3` | Berapa kali kondisi harus sama berturut-turut sebelum atap bergerak |
| `JEDA_SENSOR` | `2000` | Interval pembacaan sensor (ms). Jangan kurang dari 2000 karena batas DHT11 |
| `JEDA_WIFI` | `30000` | Jeda mencoba sambung ulang WiFi (ms) |
| `JEDA_BLYNK` | `20000` | Jeda mencoba sambung ulang Blynk (ms) |

Lanjut ke [03-kalibrasi.md](03-kalibrasi.md).
