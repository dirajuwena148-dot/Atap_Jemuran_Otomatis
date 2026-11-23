# 01 - Perangkat Keras dan Wiring

Panduan ini menjelaskan komponen yang dipakai dan cara menyambungkannya ke NodeMCU ESP8266.

## 1. Tabel sambungan pin

| Komponen | Pin komponen | Pin NodeMCU | Keterangan |
|---|---|---|---|
| Modul LDR | D0 | **D0** | Output digital: HIGH = gelap (modul standar) |
| Sensor air | S | **A0** | Output analog |
| DHT11 | DATA | **D7** | Kelembapan dan suhu |
| Limit switch BUKA | OUT | **D3** | Penanda posisi atap terbuka |
| Driver ULN2003 | IN1 | **D1** | Kontrol motor |
| Driver ULN2003 | IN2 | **D2** | Kontrol motor |
| Driver ULN2003 | IN3 | **D5** | Kontrol motor |
| Driver ULN2003 | IN4 | **D6** | Kontrol motor |

Pin **D4 tidak dipakai**.

> Urutan pin motor pada kode adalah `AccelStepper(HALF4WIRE, D1, D5, D2, D6)`. Ini sengaja berbeda dari urutan IN1-IN4 agar kumparan motor 28BYJ-48 teraliri dalam urutan yang benar. Jangan diubah kecuali motor Anda berbeda.

## 2. Tegangan dan catu daya

| Bagian | Tegangan | Catatan |
|---|---|---|
| NodeMCU | 5V via USB | Dari adaptor atau powerbank |
| Driver ULN2003 + motor | **5V dari adaptor/powerbank terpisah** | Jangan ambil dari pin NodeMCU, arusnya tidak cukup |
| LDR, DHT11, limit switch | 3,3V dari NodeMCU | |
| Sensor air | 3,3V dari NodeMCU | Pin A0 NodeMCU maksimal 3,3V |

**Wajib:** hubungkan **GND driver ULN2003 ke GND NodeMCU**. Tanpa GND bersama, motor tidak bergerak atau bergerak acak.

## 3. Penjelasan tiap komponen

### Modul LDR
Modul standar mengeluarkan **HIGH saat gelap**. Jika di Serial Monitor tulisan "gelap" muncul saat ruangan terang, ubah di kode:

```cpp
const int LDR_GELAP = LOW;   // default: HIGH
```

Putar potensiometer kecil di modul untuk mengatur kepekaan.

### Sensor air
Dipakai untuk mendeteksi hujan. Program mengukur nilai saat kondisi **kering** ketika boot, lalu menetapkan batas hujan = nilai kering + `MARGIN_AIR` (100). Karena itu, **sensor harus kering saat NodeMCU dinyalakan**.

### DHT11
Hanya bisa dibaca setiap 2 detik. Atap menutup jika kelembapan > 85% (`AMBANG_LEMBAP`). Jika pembacaan gagal, program mengabaikan kelembapan dan tetap memakai sensor lain.

### Limit switch BUKA
Hanya ada satu limit switch, di posisi **BUKA**. Posisi TUTUP dihitung dari jumlah langkah motor, yang diukur saat kalibrasi.

> **Perhatian pin D3 (GPIO0):** pin ini juga dipakai NodeMCU untuk mode flash. Jika limit switch menarik D3 ke LOW **saat NodeMCU baru dinyalakan**, board bisa masuk mode flash dan tidak menjalankan program. Cara aman: gunakan modul limit switch yang outputnya **HIGH saat tidak ditekan atau saat ditekan** (tidak menarik D3 ke LOW saat boot). Jika modul Anda mengeluarkan LOW saat ditekan, hindari menyalakan/me-reset NodeMCU ketika atap sedang berada di posisi BUKA. Jika board tidak mau menjalankan program setelah dinyalakan, ini kemungkinan penyebabnya.

### Motor stepper dan ULN2003
Kode memakai mode `HALF4WIRE` dengan kecepatan 600 langkah/detik (`KECEPATAN`) dan batas maksimum 900. Untuk motor 28BYJ-48 ini kecepatan yang aman.

## 4. Diagram sambungan (teks)

```
                 NodeMCU
              ┌───────────┐
 LDR D0 ─────►│ D0        │
 Sensor air S►│ A0        │
 DHT11 DATA ─►│ D7        │
 Limit OUT ──►│ D3        │
 ULN2003 IN1 ◄│ D1        │
 ULN2003 IN2 ◄│ D2        │
 ULN2003 IN3 ◄│ D5        │
 ULN2003 IN4 ◄│ D6        │
              │ 3V3 ──────┼──► VCC LDR, DHT11, sensor air, limit switch
              │ GND ──────┼──► GND semua modul + GND ULN2003
              └───────────┘

 Adaptor / powerbank 5V ──► VCC (+) dan GND (-) driver ULN2003
```

## 5. Pemasangan mekanik

1. Pasang limit switch di titik di mana atap **menyentuhnya saat terbuka penuh**.
2. Pastikan motor bisa menggerakkan atap sepanjang jalur tanpa tersangkut.
3. Lindungi NodeMCU, driver, dan sambungan kabel dari air (kotak kedap air atau posisi terlindung).
4. Letakkan sensor air di tempat yang terkena hujan, tetapi **kering saat kalibrasi/boot**.
5. Letakkan LDR menghadap cahaya langit, jangan terkena lampu rumah pada malam hari.

## 6. Uji sebelum dirakit permanen

- Dengan kabel USB, lihat apakah Serial Monitor menampilkan nilai sensor yang masuk akal.
- Tutup LDR dengan tangan: harus muncul "gelap".
- Teteskan air pada sensor air: nilai A0 harus naik.
- Tekan limit switch: pada kalibrasi harus terdeteksi.

Lanjut ke [02-instalasi-software.md](02-instalasi-software.md).
