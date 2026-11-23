// ================== ISI DATA BLYNK & WIFI ==================
#define BLYNK_TEMPLATE_ID "TMPL6CKS4CWnT"
#define BLYNK_TEMPLATE_NAME "Atap Jemuran Otomatis"
#define BLYNK_AUTH_TOKEN "K387oeFQcbAu4FKP5Sz-P2v7Ue4uVes1"

const char* WIFI_SSID = "CCTV";
const char* WIFI_PASS = "matilampu";
// ===========================================================

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <EEPROM.h>
#include <AccelStepper.h>
#include <DHT.h>

// ================== PIN ==================
#define PIN_LDR_D0       D0
#define PIN_AIR          A0
#define PIN_DHT          D7
#define PIN_LIMIT_BUKA   D3

// ================== PENGATURAN BAWAAN ==================
const int   LDR_GELAP         = HIGH;
const float AMBANG_LEMBAP     = 85.0;
const int   MARGIN_AIR        = 100;
const int   KECEPATAN         = 600;
const long  JARAK_BAWAAN      = 6000;
const unsigned long BATAS_WAKTU_CARI = 30000;
const int   JUMLAH_KONFIRMASI = 3;
const uint32_t MAGIC          = 0xA7C0DE02;
const unsigned long JEDA_SENSOR   = 2000;    // ms, DHT11 min 2 detik
const unsigned long JEDA_WIFI     = 30000;   // ms, coba ulang WiFi
const unsigned long JEDA_BLYNK    = 20000;   // ms, coba ulang Blynk
// =======================================================

struct Kalibrasi {
  uint32_t magic;
  int32_t  idle;
  int32_t  dirBuka;
  int32_t  jarak;
};

DHT dht(PIN_DHT, DHT11);
AccelStepper stepper(AccelStepper::HALF4WIRE, D1, D5, D2, D6);

int  idleLevel   = HIGH;
int  dirBuka     = 1;
long jarakTutup  = JARAK_BAWAAN;
int  ambangAir   = 150;
bool roofOpen    = false;
bool gagal       = false;
bool bolehSimpan = false;
int  hitungTutup = 0;
int  hitungBuka  = 0;

// ---- Status Blynk / manual ----
bool modeManual     = false;
int  perintahManual = -1;      // -1 tidak ada, 0 tutup, 1 buka
unsigned long tWifi = 0, tBlynk = 0, tSensor = 0;

// Data sensor terakhir (untuk dikirim ke Blynk)
int   gAir = 0;
bool  gHujan = false, gGelap = false;
float gLembap = NAN, gSuhu = NAN;

// ================== BLYNK ==================
bool online() {
  return WiFi.status() == WL_CONNECTED && Blynk.connected();
}

void kirimData() {
  if (!online()) return;
  Blynk.virtualWrite(V0, gAir);
  Blynk.virtualWrite(V1, gHujan ? 1 : 0);
  Blynk.virtualWrite(V2, gGelap ? 1 : 0);
  if (!isnan(gLembap)) Blynk.virtualWrite(V3, gLembap);
  if (!isnan(gSuhu))   Blynk.virtualWrite(V4, gSuhu);
  Blynk.virtualWrite(V5, roofOpen ? "TERBUKA" : "TERTUTUP");
  Blynk.virtualWrite(V7, roofOpen ? 1 : 0);
}

BLYNK_CONNECTED() {
  Blynk.virtualWrite(V6, modeManual ? 1 : 0);
  Blynk.virtualWrite(V7, roofOpen ? 1 : 0);
  Blynk.virtualWrite(V5, roofOpen ? "TERBUKA" : "TERTUTUP");
}

BLYNK_WRITE(V6) {
  modeManual = (param.asInt() == 1);
  hitungTutup = 0;
  hitungBuka = 0;
  Serial.println(modeManual ? "[Blynk] Mode MANUAL" : "[Blynk] Mode OTOMATIS");
}

BLYNK_WRITE(V7) {
  perintahManual = param.asInt() ? 1 : 0;
}

// Menjaga WiFi & Blynk tanpa memblokir program lama-lama.
// Jika tidak ada internet, fungsi ini hanya mencoba sesekali dan atap tetap jalan.
void kelolaKoneksi() {
  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (now - tWifi > JEDA_WIFI) {
      tWifi = now;
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
    return;
  }

  if (Blynk.connected()) {
    Blynk.run();
  } else if (now - tBlynk > JEDA_BLYNK) {
    tBlynk = now;
    Blynk.connect(3000);   // maksimal 3 detik menunggu
  }
}

// ---------- Limit switch ----------
bool aktif() {
  return digitalRead(PIN_LIMIT_BUKA) != idleLevel;
}

bool aktifStabil() {
  if (!aktif()) return false;
  delay(5);
  return aktif();
}

// ---------- Penyimpanan kalibrasi ----------
void simpanKalibrasi() {
  if (!bolehSimpan) return;
  Kalibrasi k;
  k.magic   = MAGIC;
  k.idle    = idleLevel;
  k.dirBuka = dirBuka;
  k.jarak   = jarakTutup;
  EEPROM.put(0, k);
  EEPROM.commit();
}

void hapusKalibrasi() {
  Kalibrasi k;
  k.magic = 0;
  k.idle = 0;
  k.dirBuka = 0;
  k.jarak = 0;
  EEPROM.put(0, k);
  EEPROM.commit();
}

// ---------- Baca huruf dari Serial Monitor ----------
bool terimaHuruf(char c) {
  bool ketemu = false;
  while (Serial.available()) {
    char x = Serial.read();
    if (x == c || x == (c - 32)) ketemu = true;
  }
  return ketemu;
}

// ---------- Baca sensor air (rata-rata agar stabil saat WiFi aktif) ----------
int bacaAir() {
  long total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(PIN_AIR);
    delay(3);
  }
  return total / 10;
}

// ---------- Kalibrasi sensor air ----------
void kalibrasiAir() {
  long total = 0;
  for (int i = 0; i < 20; i++) {
    total += analogRead(PIN_AIR);
    delay(50);
  }
  int dasar = total / 20;
  ambangAir = constrain(dasar + MARGIN_AIR, 100, 600);
  Serial.print("Sensor air kering = ");
  Serial.print(dasar);
  Serial.print(" -> batas hujan = ");
  Serial.println(ambangAir);
  if (dasar > 300) {
    Serial.println("PERINGATAN: sensor air terbaca tinggi saat boot. Keringkan sensor lalu tekan RST.");
  }
}

// ---------- Kalibrasi arti sinyal limit switch ----------
bool kalibrasiLimit() {
  Serial.println();
  Serial.println("=== KALIBRASI AWAL (hanya sekali) ===");
  Serial.println("1) Pastikan atap TIDAK menyentuh limit switch BUKA.");
  Serial.println("   Jika menyentuh, geser atap dengan tangan menjauh lalu tekan tombol RST.");
  delay(4000);
  idleLevel = digitalRead(PIN_LIMIT_BUKA);

  Serial.println("2) Sekarang TEKAN limit switch BUKA dengan jari (waktu 30 detik)...");
  unsigned long mulai = millis();
  bool terdeteksi = false;
  while (millis() - mulai < 30000) {
    if (digitalRead(PIN_LIMIT_BUKA) != idleLevel) {
      delay(20);
      if (digitalRead(PIN_LIMIT_BUKA) != idleLevel) {
        terdeteksi = true;
        break;
      }
    }
    delay(10);
  }

  if (terdeteksi) {
    Serial.println("   Terdeteksi. Lepaskan limit switch...");
    unsigned long m2 = millis();
    while (millis() - m2 < 10000 && digitalRead(PIN_LIMIT_BUKA) != idleLevel) {
      delay(10);
    }
    delay(500);
  } else {
    Serial.println("   Tidak ada penekanan terdeteksi. Kalibrasi TIDAK disimpan,");
    Serial.println("   dipakai pengaturan sementara. Tekan RST untuk mengulang kalibrasi.");
  }
  return terdeteksi;
}

// ---------- Jika atap sedang menyentuh limit: cari arah menjauh ----------
void pelajariArahDariLimit() {
  for (int arah = 1; arah >= -1; arah -= 2) {
    unsigned long mulai = millis();
    stepper.enableOutputs();
    stepper.setSpeed(arah * KECEPATAN);
    while (millis() - mulai < 4000) {
      if (!aktif()) {
        delay(5);
        if (!aktif()) {
          stepper.disableOutputs();
          dirBuka = -arah;
          return;
        }
      }
      stepper.runSpeed();
      yield();
    }
    stepper.disableOutputs();
    delay(300);
  }
}

// ---------- Gerakkan atap sampai limit BUKA tersentuh ----------
bool kembaliKeBuka() {
  if (aktifStabil()) {
    return true;
  }

  Serial.println(">> Membuka atap...");

  for (int i = 0; i < 2; i++) {
    int arah = (i == 0) ? dirBuka : -dirBuka;
    unsigned long mulai = millis();
    stepper.enableOutputs();
    stepper.setSpeed(arah * KECEPATAN);

    while (millis() - mulai < BATAS_WAKTU_CARI) {
      if (aktifStabil()) {
        stepper.disableOutputs();
        if (arah != dirBuka) {
          dirBuka = arah;
          Serial.println(">> Arah motor dibalik otomatis.");
          simpanKalibrasi();
        }
        Serial.println(">> Selesai.");
        return true;
      }
      stepper.runSpeed();
      yield();
    }

    stepper.disableOutputs();
    delay(300);
  }

  Serial.println("!! GAGAL: limit BUKA tidak tersentuh. Periksa kabel limit switch dan motor.");
  return false;
}

// ---------- Tutup atap dengan menghitung langkah ----------
void tutupAtap() {
  Serial.println(">> Menutup atap...");

  for (int percobaan = 0; percobaan < 2; percobaan++) {
    long sisa = jarakTutup;
    long langkah = 0;
    bool salah = false;

    stepper.enableOutputs();
    stepper.setSpeed(-dirBuka * KECEPATAN);

    while (sisa > 0) {
      if (stepper.runSpeed()) {
        sisa--;
        langkah++;
      }
      if (langkah == 2500 && aktif()) {
        salah = true;
        break;
      }
      yield();
    }

    stepper.disableOutputs();

    if (salah) {
      dirBuka = -dirBuka;
      Serial.println(">> Arah motor dibalik otomatis, mencoba lagi...");
      simpanKalibrasi();
      delay(300);
      continue;
    }
    break;
  }
  Serial.println(">> Selesai.");
}

// ---------- Kalibrasi jarak tutup ----------
void kalibrasiJarak() {
  Serial.println("3) Kalibrasi jarak tutup: atap akan menutup pelan-pelan.");
  Serial.println("   Ketik huruf s lalu Enter di Serial Monitor saat atap sudah TERTUTUP PENUH.");
  Serial.println("   (Tidak ada respon dalam 30 detik: dipakai jarak bawaan.)");

  while (Serial.available()) Serial.read();

  long langkah = 0;
  bool dihentikan = false;
  unsigned long mulai = millis();

  stepper.enableOutputs();
  stepper.setSpeed(-dirBuka * KECEPATAN);

  while (millis() - mulai < 30000) {
    if (stepper.runSpeed()) langkah++;

    if (langkah == 2500 && aktif()) {
      dirBuka = -dirBuka;
      stepper.setSpeed(-dirBuka * KECEPATAN);
      langkah = 0;
      Serial.println("   Arah motor dibalik otomatis.");
    }

    if (terimaHuruf('s')) {
      dihentikan = true;
      break;
    }
    yield();
  }
  stepper.disableOutputs();

  if (dihentikan && langkah > 300) {
    jarakTutup = langkah;
    Serial.print("   Jarak tutup tersimpan: ");
    Serial.print(jarakTutup);
    Serial.println(" langkah.");
  } else {
    jarakTutup = JARAK_BAWAAN;
    Serial.println("   Tidak ada respon, dipakai jarak bawaan.");
  }
  simpanKalibrasi();
  Serial.println("=== KALIBRASI SELESAI ===");
}

// ---------- Aksi buka / tutup (dipakai otomatis & manual) ----------
void aksiTutup() {
  tutupAtap();
  roofOpen = false;
  hitungTutup = 0;
  kirimData();
}

void aksiBuka() {
  if (kembaliKeBuka()) {
    roofOpen = true;
  } else {
    gagal = true;
  }
  hitungBuka = 0;
  kirimData();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== Atap Jemuran Otomatis (1 limit switch + Blynk) ===");

  // WiFi dimatikan dulu selama kalibrasi supaya pembacaan sensor air stabil
  WiFi.mode(WIFI_OFF);

  pinMode(PIN_LDR_D0, INPUT);
  pinMode(PIN_LIMIT_BUKA, INPUT);

  dht.begin();
  stepper.setMaxSpeed(900);
  stepper.disableOutputs();

  EEPROM.begin(64);
  kalibrasiAir();

  Kalibrasi k;
  EEPROM.get(0, k);

  if (k.magic == MAGIC) {
    idleLevel  = k.idle;
    dirBuka    = k.dirBuka;
    jarakTutup = k.jarak;
    bolehSimpan = true;
    Serial.print("Kalibrasi tersimpan dimuat. Jarak tutup = ");
    Serial.print(jarakTutup);
    Serial.println(" langkah.");
    Serial.println("(Ketik k lalu Enter di Serial Monitor untuk kalibrasi ulang.)");

    if (kembaliKeBuka()) {
      roofOpen = true;
    } else {
      gagal = true;
    }
  } else {
    bool terdeteksi = kalibrasiLimit();
    bolehSimpan = terdeteksi;

    if (aktifStabil()) {
      pelajariArahDariLimit();
    }

    if (kembaliKeBuka()) {
      roofOpen = true;
      if (terdeteksi) {
        kalibrasiJarak();
        roofOpen = false;
      } else {
        Serial.println("Memakai jarak tutup bawaan (belum dikalibrasi).");
      }
    } else {
      gagal = true;
    }
  }

  // ---- Hidupkan WiFi SETELAH kalibrasi. Tidak menunggu: jika gagal, atap tetap jalan ----
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Blynk.config(BLYNK_AUTH_TOKEN);
  tWifi = millis();
  tBlynk = millis() - JEDA_BLYNK;   // boleh langsung coba Blynk begitu WiFi tersambung

  if (!gagal) {
    Serial.println("Siap. Atap bekerja otomatis (WiFi/Blynk tersambung di latar belakang).");
  }
}

void loop() {
  if (terimaHuruf('k')) {
    Serial.println("Kalibrasi dihapus. Memulai ulang...");
    hapusKalibrasi();
    delay(500);
    ESP.restart();
  }

  if (gagal) {
    Serial.println("Motor dihentikan karena gagal. Periksa kabel limit switch dan motor, lalu tekan tombol RST.");
    delay(5000);
    return;
  }

  // ---- Jaga koneksi (tidak mengganggu jika offline) ----
  kelolaKoneksi();

  // ---- Perintah manual dari Blynk (hanya saat online & mode manual) ----
  if (modeManual && online() && perintahManual != -1) {
    int p = perintahManual;
    perintahManual = -1;
    if (p == 0 && roofOpen)  { Serial.println("[Blynk] Tutup manual"); aksiTutup(); }
    if (p == 1 && !roofOpen) { Serial.println("[Blynk] Buka manual");  aksiBuka();  }
  } else if (!modeManual || !online()) {
    perintahManual = -1;   // abaikan perintah lama
  }

  // ---- Baca sensor & proses otomatis setiap 2 detik ----
  if (millis() - tSensor < JEDA_SENSOR) {
    delay(20);
    return;
  }
  tSensor = millis();

  gAir = bacaAir();
  gHujan = (gAir > ambangAir);
  gGelap = (digitalRead(PIN_LDR_D0) == LDR_GELAP);

  gLembap = dht.readHumidity();
  gSuhu   = dht.readTemperature();
  bool lembapTinggi = false;
  if (!isnan(gLembap)) {
    lembapTinggi = (gLembap > AMBANG_LEMBAP);
  }

  // ---- Tampilkan di Serial Monitor ----
  Serial.print("Air: ");
  Serial.print(gAir);
  Serial.print(gHujan ? " (HUJAN)" : " (kering)");
  Serial.print(" | Cahaya: ");
  Serial.print(gGelap ? "gelap" : "terang");
  Serial.print(" | Lembap: ");
  if (isnan(gLembap)) {
    Serial.print("gagal baca");
  } else {
    Serial.print(gLembap);
    Serial.print("%");
  }
  Serial.print(" | Atap: ");
  Serial.print(roofOpen ? "TERBUKA" : "TERTUTUP");
  Serial.print(" | Mode: ");
  Serial.print((modeManual && online()) ? "MANUAL" : "OTOMATIS");
  Serial.print(" | Blynk: ");
  Serial.println(online() ? "online" : "offline");

  // ---- Kirim ke dashboard (hanya jika online) ----
  kirimData();

  // ---- Keputusan otomatis (dilewati saat mode manual yang aktif) ----
  if (modeManual && online()) {
    hitungTutup = 0;
    hitungBuka = 0;
    return;
  }

  bool harusTutup = gHujan || gGelap || lembapTinggi;

  if (harusTutup) {
    hitungTutup++;
    hitungBuka = 0;
  } else {
    hitungBuka++;
    hitungTutup = 0;
  }

  if (hitungTutup >= JUMLAH_KONFIRMASI && roofOpen) {
    aksiTutup();
  } else if (hitungBuka >= JUMLAH_KONFIRMASI && !roofOpen) {
    aksiBuka();
  }
}
