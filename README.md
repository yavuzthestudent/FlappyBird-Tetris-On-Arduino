# 🎮 Flappy Bird & Tetris on Arduino

Bu proje, **Arduino UNO**, **8x8 LED Matrix** ve temel donanım bileşenleri ile **Flappy Bird** ve **Tetris** oyunlarının minimalist versiyonlarını oynamanıza olanak tanır. Hem oyun mantığını hem de donanım tasarımını içerir. Elektronik ve gömülü sistemlere ilgi duyanlar için eğlenceli bir uygulama örneğidir.

---

## 🚀 Özellikler

- 🕹️ 2 farklı oyun: **Flappy Bird** ve **Tetris**
- 💡 8x8 LED Matrix ile görselleştirme
- 🧠 Oyun mantıkları Arduino C++ dili ile yazılmıştır
- 🔘 Buton ile kontrol sistemi
- 🔄 Döngüsel oyun deneyimi (game over sonrası yeniden başlatma)

---

## 🛠 Gerekli Donanımlar

| Bileşen              | Miktar |
|----------------------|--------|
| Arduino UNO          | 1      |
| 8x8 LED Dot Matrix   | 1      |
| MAX7219 Modülü       | 1      |
| Push Button (Tuş)    | 1      |
| Breadboard & Jumper  | Birkaç |

---

## 🧩 Kurulum ve Kullanım

### 1. Donanımı Bağlayın

📌 **MAX7219 LED Matrix bağlantısı**:

| MAX7219 Pin | Arduino Pin |
|-------------|-------------|
| VCC         | 5V          |
| GND         | GND         |
| DIN         | D11         |
| CS          | D10         |
| CLK         | D13         |

📌 **Buton bağlantısı**:

- Bir ucu: D2 pinine
- Diğer ucu: GND’ye
- 10K pull-down direnç önerilir

### 2. Arduino IDE ile Kodu Yükleyin

- Arduino IDE’yi açın
- Gerekli kütüphaneleri yükleyin:
  - `LedControl.h`
- `flappy.ino` veya `tetris.ino` dosyasını açıp Arduino’ya yükleyin

### 3. Oyna!

- **Flappy Bird** için: `flappy.ino` dosyasını yükle
- **Tetris** için: `tetris.ino` dosyasını yükle
- Başlamak için butona basın ve keyfini çıkarın 🎉
