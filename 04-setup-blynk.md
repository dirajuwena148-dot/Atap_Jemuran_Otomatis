# 04 - Setup Dashboard Blynk

Panduan ini membuat Template, Datastream, Dashboard, dan Device di **Blynk Console** (<https://blynk.cloud>), lalu menghubungkannya ke program.

Istilah singkat:

- **Template**: cetakan yang berisi daftar data (datastream) dan tampilan dashboard.
- **Datastream**: saluran data antara NodeMCU dan dashboard. Dalam proyek ini memakai **Virtual Pin** (V0, V1, dst.).
- **Device**: satu perangkat nyata (NodeMCU Anda) yang dibuat dari template dan punya **Auth Token** sendiri.

## 1. Buat akun

Daftar atau login di <https://blynk.cloud>, lalu verifikasi email.

## 2. Buat Template

1. Menu kiri: **Developer Zone > My Templates > + New Template**.
2. Isi:
   - **Name:** `Atap Jemuran Otomatis` (harus sama dengan `BLYNK_TEMPLATE_NAME` di kode)
   - **Hardware:** `ESP8266`
   - **Connection Type:** `WiFi`
3. Klik **Done**.

## 3. Buat Datastream

Di dalam template, buka tab **Datastreams > + New Datastream > Virtual Pin**. Buat 8 datastream berikut. **Nomor pin harus persis sama** karena dipakai oleh kode.

| Nama | Pin | Tipe data | Unit | Min | Max | Arah data |
|---|---|---|---|---|---|---|
| Sensor Air | V0 | Integer | - | 0 | 1024 | NodeMCU → dashboard |
| Hujan | V1 | Integer | - | 0 | 1 | NodeMCU → dashboard |
| Gelap | V2 | Integer | - | 0 | 1 | NodeMCU → dashboard |
| Kelembapan | V3 | Double | % | 0 | 100 | NodeMCU → dashboard |
| Suhu | V4 | Double | °C | 0 | 60 | NodeMCU → dashboard |
| Status Atap | V5 | String | - | - | - | NodeMCU → dashboard |
| Mode Manual | V6 | Integer | - | 0 | 1 | Dashboard → NodeMCU |
| Perintah Atap | V7 | Integer | - | 0 | 1 | Dashboard → NodeMCU |

Klik **Save** di kanan atas setelah selesai.

> **Batas datastream akun gratis.** Informasi yang beredar tidak seragam: halaman harga resmi Blynk menyebut 20 datastream per template untuk paket gratis, sementara sebagian pengguna di forum melaporkan batas 5 atau 10. Jika Anda terkena batas, buat hanya **V0, V3, V5, V6, V7**, lalu hapus tiga baris ini di fungsi `kirimData()` pada file `.ino`:
> ```cpp
> Blynk.virtualWrite(V1, gHujan ? 1 : 0);
> Blynk.virtualWrite(V2, gGelap ? 1 : 0);
> if (!isnan(gSuhu))   Blynk.virtualWrite(V4, gSuhu);
> ```

### Arti tiap datastream

| Pin | Isi |
|---|---|
| V0 | Nilai mentah sensor air (naik saat basah) |
| V1 | 1 = hujan, 0 = kering |
| V2 | 1 = gelap, 0 = terang |
| V3 | Kelembapan udara dari DHT11 |
| V4 | Suhu dari DHT11 |
| V5 | Teks `TERBUKA` atau `TERTUTUP` |
| V6 | 0 = mode otomatis, 1 = mode manual |
| V7 | Perintah manual: 1 = buka, 0 = tutup. Juga menampilkan status atap saat ini |

## 4. Buat tampilan Web Dashboard

1. Di template, buka tab **Web Dashboard**. Pastikan mode edit aktif (tombol **Save** dan **Cancel** terlihat).
2. Klik tombol **+** hijau untuk menambah widget, lalu seret ke area dashboard.
3. Arahkan kursor ke widget, klik ikon **gerigi (⚙)** untuk membuka pengaturan, dan pilih **Datastream** pada dropdown.

Susunan yang disarankan:

| Widget | Datastream | Fungsi |
|---|---|---|
| Gauge | Kelembapan (V3) | Kelembapan udara |
| Gauge | Suhu (V4) | Suhu |
| Label | Status Atap (V5) | Terbuka/tertutup |
| Chart atau Gauge | Sensor Air (V0) | Nilai sensor air |
| LED | Hujan (V1) | Menyala saat hujan |
| LED | Gelap (V2) | Menyala saat gelap |
| Switch | Mode Manual (V6) | Otomatis/manual |
| Switch | Perintah Atap (V7) | Buka/tutup |

4. Klik **Save** di kanan atas.

**Jika widget tidak punya pilihan datastream:** pastikan datastream-nya sudah dibuat dan sudah di-Save di tab Datastreams, lalu muat ulang halaman (F5). Jika ikon gerigi tidak terlihat pada widget kecil, lebarkan widget dengan menarik pojok kanan bawahnya.

**Pengaturan Switch:** untuk Switch V6 dan V7, pastikan nilai ON = 1 dan OFF = 0.

## 5. Buat Device dan ambil Auth Token

1. Menu kiri: **Devices > + New Device > From template**.
2. Pilih template **Atap Jemuran Otomatis**, beri nama device, klik **Create**.
3. Buka device tersebut, lalu buka **Device Info** (atau Firmware Configuration). Anda akan melihat:

```cpp
#define BLYNK_TEMPLATE_ID   "TMPLxxxxxxxx"
#define BLYNK_TEMPLATE_NAME "Atap Jemuran Otomatis"
#define BLYNK_AUTH_TOKEN    "xxxxxxxxxxxxxxxxxxxxxxxx"
```

4. **Salin ketiganya** dan tempel di bagian atas file `.ino`, menggantikan nilai `ISI_...`. Cara paling aman adalah menyalin langsung dari Blynk agar huruf besar/kecilnya tepat.

> **Auth Token itu rahasia.** Jangan dibagikan atau diunggah ke repository publik. Jika bocor, buat token baru di Device Info.

## 6. Hubungkan dan uji

1. Isi `WIFI_SSID` dan `WIFI_PASS` (WiFi **2,4 GHz**), upload program, lalu selesaikan kalibrasi ([03-kalibrasi.md](03-kalibrasi.md)).
2. Di Serial Monitor harus muncul `Blynk: online`.
3. Di Blynk, status device berubah menjadi **Online** dan nilai di dashboard bergerak tiap 2 detik.
4. Tes kontrol manual:
   - Nyalakan **Switch Mode Manual (V6)**.
   - Tekan **Switch Perintah Atap (V7)** untuk membuka atau menutup atap.
   - Matikan Switch V6 untuk kembali ke mode otomatis.

## 7. (Opsional) Pantau dari HP

1. Install aplikasi **Blynk IoT** (Android/iOS), lalu login dengan akun yang sama.
2. Buat tampilan HP di tab **App Dashboard** (atau Mobile Dashboard) pada template dengan cara yang sama seperti web dashboard.

## Aturan kontrol manual

| Kondisi | Perilaku |
|---|---|
| Mode manual ON dan **online** | Atap mengikuti Switch V7. Sensor tetap dibaca dan ditampilkan, tetapi tidak menggerakkan atap |
| Mode manual ON tetapi **offline** | Perintah diabaikan, atap kembali **otomatis** memakai sensor |
| Mode manual OFF | Atap sepenuhnya otomatis |

Lanjut ke [05-mode-offline-dan-cara-kerja.md](05-mode-offline-dan-cara-kerja.md).
