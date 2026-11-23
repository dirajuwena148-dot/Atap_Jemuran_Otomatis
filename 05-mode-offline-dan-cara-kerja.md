# 05 - Cara Kerja Program dan Mode Offline

Dokumen ini menjelaskan logika program dan bagaimana atap tetap berjalan **tanpa WiFi atau internet**.

## 1. Prinsip desain

> Keputusan buka/tutup dibuat oleh **NodeMCU sendiri** dari sensor. WiFi dan Blynk hanya tambahan untuk memantau dan mengontrol manual.

Konsekuensinya, program **tidak boleh menunggu** koneksi. Karena itu:

- WiFi dinyalakan di akhir `setup()` **tanpa menunggu** sampai tersambung.
- Penyambungan WiFi dan Blynk diurus fungsi `kelolaKoneksi()` yang dipanggil di `loop()` dan hanya mencoba sesekali.
- Semua pengiriman data ke Blynk dibungkus `if (!online()) return;` sehingga saat offline tidak ada yang tertahan.

## 2. Alur program

```
setup()
 ├─ WiFi dimatikan (agar sensor air stabil saat kalibrasi)
 ├─ Kalibrasi sensor air
 ├─ Ada kalibrasi tersimpan?
 │    ├─ Ya  : muat dari EEPROM → atap ke posisi BUKA
 │    └─ Tidak: kalibrasi limit & jarak (lihat 03-kalibrasi.md)
 └─ WiFi dinyalakan + Blynk.config()  (tidak ditunggu)

loop()  (berulang terus)
 ├─ Perintah 'k' di Serial? → hapus kalibrasi & restart
 ├─ Status gagal? → jaga koneksi, tampilkan pesan, berhenti menggerakkan motor
 ├─ kelolaKoneksi()  → jaga WiFi & Blynk
 ├─ Ada perintah manual dari Blynk? → buka/tutup
 └─ Setiap 2 detik:
      ├─ Baca sensor air, LDR, DHT11
      ├─ Tampilkan di Serial Monitor
      ├─ Kirim ke Blynk (jika online)
      └─ Jika tidak sedang mode manual: keputusan otomatis
```

## 3. Keputusan otomatis

```cpp
bool harusTutup = hujan || gelap || lembapTinggi;
```

| Kondisi | Dianggap terpenuhi jika |
|---|---|
| `hujan` | nilai sensor air > `ambangAir` (kering + `MARGIN_AIR`) |
| `gelap` | pin LDR = `LDR_GELAP` |
| `lembapTinggi` | kelembapan > `AMBANG_LEMBAP` (85%) |

**Konfirmasi berturut-turut.** Atap baru bergerak jika kondisi sama terbaca `JUMLAH_KONFIRMASI` (3) kali berturut-turut. Dengan interval 2 detik, itu sekitar 6 detik. Ini mencegah atap bergerak karena gangguan sesaat (misalnya bayangan lewat di depan LDR).

- `hitungTutup >= 3` dan atap terbuka → **tutup**.
- `hitungBuka >= 3` dan atap tertutup → **buka**.

Jika DHT11 gagal dibaca (`isnan`), kelembapan diabaikan dan dua sensor lain tetap bekerja.

## 4. Gerakan motor

**Membuka (`kembaliKeBuka`)**: motor berputar ke arah buka sampai limit switch BUKA tersentuh. Jika dalam 30 detik tidak tersentuh, arah dibalik dan dicoba lagi. Jika tetap gagal, status `gagal` aktif dan motor berhenti sampai NodeMCU di-reset.

**Menutup (`tutupAtap`)**: motor berputar ke arah sebaliknya sebanyak `jarakTutup` langkah (hasil kalibrasi). Setelah 2500 langkah, jika limit BUKA **masih tertekan**, berarti arah salah, sehingga arah dibalik dan dicoba lagi.

Selama motor berputar, program memakai `yield()` agar ESP8266 tidak restart sendiri (watchdog).

## 5. Mode offline

### Yang tetap berjalan tanpa WiFi/internet

| Fungsi | Status |
|---|---|
| Baca sensor air, LDR, DHT11 | Berjalan |
| Keputusan buka/tutup otomatis | Berjalan |
| Gerakan motor dan limit switch | Berjalan |
| Kalibrasi tersimpan (EEPROM) | Tetap tersimpan |
| Serial Monitor | Berjalan |

### Yang tidak tersedia saat offline

| Fungsi | Status |
|---|---|
| Dashboard Blynk | Tidak menerima data |
| Kontrol manual dari Blynk | Tidak bisa dan diabaikan |

### Penyambungan ulang otomatis

`kelolaKoneksi()` bekerja bertahap:

1. **WiFi belum tersambung** → setiap 30 detik (`JEDA_WIFI`) memanggil `WiFi.begin()` lagi.
2. **WiFi tersambung, Blynk belum** → setiap 20 detik (`JEDA_BLYNK`) mencoba `Blynk.connect(3000)`, menunggu maksimal 3 detik.
3. **Blynk tersambung** → memanggil `Blynk.run()` untuk memproses data.

Saat Blynk mencoba tersambung, program bisa tertahan sampai 3 detik, tetapi itu hanya terjadi sekali tiap 20 detik dan tidak mengganggu karena keputusan atap berselang 2 detik dengan konfirmasi 3 kali.

### Aturan mode manual

Mode manual hanya dianggap aktif jika `modeManual && online()`. Begitu koneksi putus, atap otomatis kembali memakai sensor. Dengan begitu atap tidak "terkunci" dalam mode manual saat internet mati, misalnya saat hujan deras.

## 6. Kenapa WiFi dimatikan saat kalibrasi?

Pada ESP8266, pembacaan ADC (pin A0) dipengaruhi aktivitas radio WiFi. Agar nilai "sensor air kering" akurat, kalibrasi sensor air dilakukan sebelum WiFi dinyalakan. Setelah WiFi aktif, nilai tetap dirata-rata 10 kali (`bacaAir()`) untuk mengurangi gangguan.

## 7. Keterbatasan yang perlu dipahami

- **Blynk bisa putus sebentar saat motor berputar.** Selama motor bergerak (sekitar 10 detik untuk 6000 langkah pada 600 langkah/detik), `Blynk.run()` tidak dipanggil agar langkah motor tidak meleset. Koneksi tersambung lagi sendiri setelah gerakan selesai.
- **Posisi atap tidak diketahui jika dipindah tangan** saat listrik mati. Karena itu, setiap boot atap selalu dikembalikan ke posisi BUKA sebagai acuan.
- **Hanya satu limit switch.** Posisi TUTUP diperkirakan dari jumlah langkah. Jika motor tergelincir atau ada hambatan mekanik, posisi tutup bisa bergeser. Ulangi kalibrasi bila perlu.
- **Nilai sensor air bisa sedikit bergeser saat WiFi aktif.** Sesuaikan `MARGIN_AIR` jika perlu.

## 8. Cara menguji mode offline

1. Matikan router (atau isi `WIFI_SSID` dengan nama yang salah), lalu nyalakan NodeMCU.
2. Buka Serial Monitor. Baris data harus tetap muncul tiap 2 detik dengan tulisan `Blynk: offline`.
3. Tutup LDR dengan tangan atau teteskan air pada sensor air. Atap harus menutup sendiri sekitar 6 detik kemudian.
4. Nyalakan kembali router. Dalam kurang lebih 30 sampai 50 detik, Serial Monitor harus menunjukkan `Blynk: online`.

Lanjut ke [06-pemecahan-masalah.md](06-pemecahan-masalah.md).
