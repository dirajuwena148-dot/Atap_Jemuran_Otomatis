# 03 - Kalibrasi

Kalibrasi dilakukan **sekali** saat pertama kali program dijalankan. Hasilnya disimpan di memori EEPROM NodeMCU, sehingga **tidak hilang saat listrik mati** dan tidak butuh WiFi.

## Apa yang dikalibrasi?

| Yang dikalibrasi | Cara | Kapan |
|---|---|---|
| Sensor air (batas hujan) | Otomatis | Setiap kali NodeMCU menyala |
| Arti sinyal limit switch (HIGH atau LOW saat ditekan) | Anda menekan limit switch | Sekali, saat kalibrasi awal |
| Arah putaran motor untuk membuka | Otomatis (dikoreksi sendiri jika terbalik) | Sekali dan saat diperlukan |
| Jarak menutup (jumlah langkah) | Anda mengetik `s` saat atap tertutup penuh | Sekali, saat kalibrasi awal |

## Persiapan

- NodeMCU terhubung ke komputer lewat USB.
- Serial Monitor terbuka dengan baud **115200**.
- Atap **tidak menyentuh** limit switch BUKA.
- Sensor air dalam keadaan **kering**.
- Ruang gerak atap bebas dari hambatan.

## Langkah kalibrasi pertama

Setelah upload, tekan tombol **RST** pada NodeMCU. Ikuti pesan di Serial Monitor.

**1. Kalibrasi sensor air (otomatis)**

```
Sensor air kering = 12 -> batas hujan = 112
```

Nilai bisa berbeda. Jika muncul peringatan sensor terbaca tinggi, keringkan sensor lalu tekan RST.

**2. Deteksi limit switch**

```
=== KALIBRASI AWAL (hanya sekali) ===
1) Pastikan atap TIDAK menyentuh limit switch BUKA.
2) Sekarang TEKAN limit switch BUKA dengan jari (waktu 30 detik)...
```

Tekan limit switch dengan jari saat diminta, lalu lepas ketika ada perintah. Program mencatat nilai pin saat **tidak ditekan** sebagai kondisi normal.

**3. Mencari posisi BUKA**

Atap bergerak sampai limit switch tersentuh. Jika arah awal salah, program membaliknya sendiri (`>> Arah motor dibalik otomatis.`). Batas waktu pencarian 30 detik. Jika gagal, periksa kabel limit switch dan motor.

**4. Kalibrasi jarak tutup**

```
3) Kalibrasi jarak tutup: atap akan menutup pelan-pelan.
   Ketik huruf s lalu Enter di Serial Monitor saat atap sudah TERTUTUP PENUH.
```

Atap bergerak menutup. Saat atap sudah tertutup penuh, ketik **`s`** lalu tekan **Enter** di kolom input Serial Monitor. Contoh hasil:

```
   Jarak tutup tersimpan: 5840 langkah.
=== KALIBRASI SELESAI ===
```

Jika Anda tidak mengetik apa pun dalam 30 detik, program memakai jarak bawaan (`JARAK_BAWAAN` = 6000 langkah). Jarak ini hanya perkiraan, jadi sebaiknya kalibrasi ulang.

Setelah itu program berjalan otomatis. WiFi dan Blynk baru disambungkan setelah kalibrasi selesai.

## Mengulang kalibrasi

Ketik **`k`** lalu Enter di Serial Monitor kapan saja. Kalibrasi lama dihapus dan NodeMCU restart, lalu kalibrasi dimulai dari awal.

Ulangi kalibrasi jika:
- Posisi atau panjang jalur atap berubah.
- Limit switch dipindah.
- Atap berhenti tidak tepat di posisi tertutup.
- Motor atau mekanik diganti.

## Boot berikutnya (sudah terkalibrasi)

```
Kalibrasi tersimpan dimuat. Jarak tutup = 5840 langkah.
>> Membuka atap...
>> Selesai.
```

Pada setiap boot, atap bergerak ke posisi BUKA sebagai titik acuan ("home"), lalu bekerja otomatis. Jadi setelah listrik mati dan menyala lagi, atap akan membuka dulu sebelum menentukan keputusan berikutnya.

## Menyetel sensor air

Batas hujan dihitung otomatis: `nilai kering + MARGIN_AIR`, dibatasi antara 100 dan 600.

- Atap menutup padahal tidak hujan: naikkan `MARGIN_AIR` (misalnya 150 atau 200).
- Atap lambat menutup saat gerimis: turunkan `MARGIN_AIR` (misalnya 60).

Nilai sensor air bisa sedikit bergeser saat WiFi aktif. Amati nilai "Air:" di Serial Monitor setelah WiFi tersambung, lalu sesuaikan `MARGIN_AIR`.

## Menyetel sensor lain

- **LDR terbalik:** ubah `LDR_GELAP` menjadi `LOW`.
- **Kelembapan:** ubah `AMBANG_LEMBAP` (bawaan 85%). Di daerah yang lembap sepanjang hari, naikkan nilainya agar atap tidak sering menutup.

Lanjut ke [04-setup-blynk.md](04-setup-blynk.md).
