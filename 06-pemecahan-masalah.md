# 06 - Pemecahan Masalah (Troubleshooting)

Langkah pertama untuk semua masalah: buka **Serial Monitor (115200 baud)** dan baca pesannya. Setiap baris data menampilkan nilai sensor, status atap, mode, status WiFi, dan status Blynk, contohnya:

```
Air: 12 (kering) | Cahaya: terang | Lembap: 62.00% | Atap: TERBUKA | Mode: OTOMATIS | WiFi: tersambung | Blynk: online
```

## A. Masalah koneksi WiFi dan Blynk

| Gejala | Kemungkinan penyebab | Solusi |
|---|---|---|
| `WiFi: SSID tidak ditemukan` | WiFi 5 GHz atau nama salah | Pakai WiFi **2,4 GHz** (nama tanpa "5G"). Periksa huruf besar/kecil dan spasi |
| `WiFi: password salah` | Password keliru | Periksa `WIFI_PASS` |
| `WiFi: tersambung` tetapi `Blynk: offline` | Token atau template tidak cocok, atau internet router mati | Salin ulang 3 `#define` dari **Device Info**. Pastikan router punya internet |
| Device di Blynk tetap **Offline** | Device belum dibuat dari template, atau token milik device lain | Buat device lewat **Devices > New Device > From template**, pakai token device itu |
| Dashboard kosong padahal `Blynk: online` | Widget belum dihubungkan ke datastream, atau lupa Save | Atur datastream tiap widget, klik **Save**, muat ulang halaman |
| Data V1, V2, atau V4 tidak muncul | Datastream belum dibuat (mis. terkena batas akun gratis) | Lihat catatan batas datastream di [04-setup-blynk.md](04-setup-blynk.md) |
| Dashboard sempat putus saat atap bergerak | Normal | Tersambung lagi sendiri setelah gerakan selesai |
| Baru boot, Blynk belum online | WiFi baru dinyalakan setelah kalibrasi, lalu butuh beberapa detik | Tunggu 30-60 detik |

## B. Masalah kompilasi dan upload

| Pesan atau gejala | Solusi |
|---|---|
| `BlynkSimpleEsp8266.h: No such file` | Install library **Blynk** lewat Library Manager |
| `DHT.h: No such file` | Install **DHT sensor library** dan **Adafruit Unified Sensor** |
| `AccelStepper.h: No such file` | Install **AccelStepper** |
| Error terkait `BLYNK_TEMPLATE_ID` | Tiga `#define` Blynk harus berada **sebelum** `#include <BlynkSimpleEsp8266.h>` |
| Port tidak muncul | Ganti kabel USB (harus kabel data), install driver CH340/CP2102 |
| Upload gagal "Connecting..." | Tahan tombol **FLASH** saat upload dimulai |
| Board tidak menjalankan program setelah dinyalakan | Pin D3 (GPIO0) tertarik LOW saat boot, lihat catatan di [01-perangkat-keras.md](01-perangkat-keras.md) |

## C. Masalah sensor

| Gejala | Solusi |
|---|---|
| Selalu "HUJAN" | Sensor air basah atau kotor saat boot, keringkan lalu tekan **RST**. Naikkan `MARGIN_AIR` (150-200) jika nilai bergeser saat WiFi aktif |
| Tidak mendeteksi hujan | Turunkan `MARGIN_AIR` (misalnya 60), cek kabel S ke A0 dan tegangan 3,3V |
| "gelap" saat terang (terbalik) | Ubah `LDR_GELAP` menjadi `LOW` |
| LDR tidak peka | Putar potensiometer pada modul LDR |
| Lembap "gagal baca" | Cek kabel DATA DHT11 ke D7 dan tegangan. Program tetap berjalan dengan sensor lain |
| Atap sering menutup karena lembap | Naikkan `AMBANG_LEMBAP` (mis. 90) |

## D. Masalah motor dan limit switch

| Gejala | Solusi |
|---|---|
| Motor bergetar tetapi tidak berputar | Urutan kabel IN1-IN4 salah. Cek tabel di [01-perangkat-keras.md](01-perangkat-keras.md) |
| Motor tidak bergerak sama sekali | Driver ULN2003 belum dapat daya 5V, atau **GND driver belum disambung ke GND NodeMCU** |
| NodeMCU restart saat motor jalan | Daya kurang kuat. Pakai adaptor/powerbank 5V terpisah untuk motor, jangan dari pin NodeMCU |
| `!! GAGAL: limit BUKA tidak tersentuh` | Cek kabel limit switch, posisikan limit agar benar-benar tersentuh atap, cek apakah atap macet |
| "Tidak ada penekanan terdeteksi" saat kalibrasi | Limit switch tidak terbaca. Cek kabel OUT ke D3 dan tegangan modul |
| Atap berhenti tidak tepat di posisi tertutup | Ulangi kalibrasi: ketik `k` lalu Enter, kemudian ketik `s` tepat saat atap tertutup penuh |
| Atap membuka saat seharusnya menutup (arah terbalik) | Biasanya dikoreksi otomatis (`Arah motor dibalik otomatis`). Jika tidak, ulangi kalibrasi |
| Motor berhenti dan Serial Monitor mencetak pesan "Motor dihentikan karena gagal" | Perbaiki penyebabnya (kabel/limit/mekanik), lalu tekan **RST** |

## E. Masalah kontrol manual

| Gejala | Solusi |
|---|---|
| Switch V7 tidak menggerakkan atap | Nyalakan dulu **Switch Mode Manual (V6)**. Pastikan `Blynk: online` |
| Atap bergerak sendiri padahal mode manual | Mungkin NodeMCU sedang offline. Saat offline, atap kembali otomatis |
| Perintah buka tidak berefek | Atap sudah terbuka. Perintah hanya dijalankan jika berbeda dengan status saat ini |

## F. Pesan Serial Monitor dan artinya

| Pesan | Arti |
|---|---|
| `Sensor air kering = X -> batas hujan = Y` | Hasil kalibrasi otomatis sensor air saat boot |
| `Kalibrasi tersimpan dimuat` | Kalibrasi dari EEPROM dipakai |
| `>> Membuka atap...` / `>> Menutup atap...` | Motor sedang bergerak |
| `>> Arah motor dibalik otomatis.` | Arah motor awal salah dan sudah dikoreksi |
| `[Blynk] Mode MANUAL` / `OTOMATIS` | Switch V6 diubah dari dashboard |
| `[Blynk] Buka manual` / `Tutup manual` | Perintah dari Switch V7 dijalankan |
| `Kalibrasi dihapus. Memulai ulang...` | Anda mengetik `k` |

## G. Jika masih bermasalah

Siapkan informasi berikut sebelum meminta bantuan:

1. Salinan teks Serial Monitor sejak boot (**hapus token dan password sebelum dibagikan**).
2. Jenis modul LDR, sensor air, limit switch, dan motor yang dipakai.
3. Foto sambungan kabel.
4. Versi library dan board package (lihat Boards Manager dan Library Manager).

## Keamanan

Jangan membagikan **Auth Token Blynk** dan **password WiFi**. Jika sudah terlanjur terlihat orang lain, buat token baru di Device Info Blynk dan ganti password WiFi.
