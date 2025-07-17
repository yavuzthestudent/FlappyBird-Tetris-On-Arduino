#include <U8g2lib.h>  // OLED ekran kütüphanesi
#include <Wire.h>    // I2C iletişimi için kütüphane

// OLED ekran için SSD1306 denetleyicisi kullanılıyor, I2C üzerinden bağlantı yapılmış
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// --- Genel Ayarlar ---
const int vrX = A0;       // Joystick X ekseni için analog pin
const int vrY = A1;       // Joystick Y ekseni için analog pin
const int buzzerPin = 8;  // Buzzer için dijital pin

// --- Menü Sistemi ---
int selectedItem = 0;             // Seçili menü öğesi (0: Flappy Bird, 1: Tetris)
unsigned long lastMoveTime = 0;   // Son joystick hareketi zamanı
unsigned long selectionStartTime = 0;  // Seçim başlangıç zamanı (3 saniye bekletip oyuna girmek için)
bool gameStarted = false;         // Oyun başladı mı?

// Oyun tiplerini tanımla
enum GameType { NONE, FLAPPY, TETRIS };
GameType currentGame = NONE;      // Şu an oynanan oyun

// --- Flappy Bird Değişkenleri ---
int birdY;          // Kuşun Y pozisyonu
int birdVelocity;   // Kuşun düşey hızı
const int gravity = 1;  // Yerçekimi etkisi
const int jumpStrength = -5;  // Zıplama gücü
int pipeX;          // Borunun X pozisyonu
int pipeGapY;       // Borudaki boşluğun Y başlangıcı
const int pipeWidth = 10;  // Boru genişliği
const int pipeGapHeight = 20;  // Borudaki boşluk yüksekliği
bool gameOverFlappy;  // Oyun bitti mi?
int scoreFlappy;      // Oyun skoru

// --- Tetris Değişkenleri ---
const int cols = 10;  // Oyun alanı sütun sayısı
const int rows = 8;   // Oyun alanı satır sayısı
int grid[rows][cols]; // Oyun alanı matrisi (1: dolu, 0: boş)
const int boxW = 6;   // Kutucuk genişliği
const int boxH = 6;   // Kutucuk yüksekliği
const int gridStartX = 60; // Oyun alanı başlangıç X koordinatı
const int gridStartY = 6;  // Oyun alanı başlangıç Y koordinatı 
unsigned long lastFallTime = 0; // Son düşme zamanı
bool gameOverTetris = false;    // Tetris oyunu bitti mi?
int scoreTetris = 0;            // Tetris skoru
byte currentShape[4][4];        // Şu anki aktif şekil
int shapeSize = 3;              // Şeklin büyüklüğü (3x3 veya 4x4)
int blockX = 3;                 // Şeklin X konumu
int blockY = 0;                 // Şeklin Y konumu

// Tetris şekilleri tanımlanıyor (T, I ve L şekilleri)
const byte SHAPE_T[4][4] = {
  {0,1,0,0},
  {1,1,1,0},
  {0,0,0,0},
  {0,0,0,0}
};
const byte SHAPE_I[4][4] = {
  {0,0,0,0},
  {1,1,1,1},
  {0,0,0,0},
  {0,0,0,0}
};
const byte SHAPE_L[4][4] = {
  {1,0,0,0},
  {1,1,1,0},
  {0,0,0,0},
  {0,0,0,0}
};

// Joystick durumunu takip etmek için değişken (çift tıklamaları önlemek için)
bool joystickReleased = true;

void setup() {
  // Pinleri yapılandır
  pinMode(vrX, INPUT);
  pinMode(vrY, INPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);  // Buzzer başlangıçta kapalı
  
  // OLED ekranı başlat
  u8g2.begin();
  
  // Başlangıç menüsünü göster
  drawMenu();
}

void loop() {
  if (!gameStarted) {
    // Eğer oyun başlamadıysa menüde gezinme işlemlerini yap
    int yVal = analogRead(vrY);  // Joystick Y değerini oku
    unsigned long now = millis();
    static int lastSelection = selectedItem;

    // Joystick yukarı itildiğinde, üst menü öğesine geç
    if (yVal < 400 && now - lastMoveTime > 300) {
      selectedItem = max(0, selectedItem - 1);
      lastMoveTime = now;
    }
    
    // Joystick aşağı itildiğinde, alt menü öğesine geç
    if (yVal > 600 && now - lastMoveTime > 300) {
      selectedItem = min(1, selectedItem + 1);
      lastMoveTime = now;
    }
    
    // Seçim değiştiyse menüyü güncelle
    if (selectedItem != lastSelection) {
      lastSelection = selectedItem;
      selectionStartTime = now;  // Seçim zamanını kaydet
      drawMenu();  // Menüyü yeniden çiz
    }
    
    // 3 saniye aynı seçimde beklenirse seçili oyunu başlat
    if (millis() - selectionStartTime >= 3000) {
      if (selectedItem == 0) startFlappyBird();
      else if (selectedItem == 1) startTetris();
    }
  } else {
    // Oyun başladıysa, ilgili oyun döngüsünü çalıştır
    if (currentGame == FLAPPY) flappyBirdLoop();
    else if (currentGame == TETRIS) tetrisLoop();
  }
}

// Menüyü çizen fonksiyon
void drawMenu() {
  u8g2.clearBuffer();  // Ekran tamponunu temizle
  u8g2.setFont(u8g2_font_ncenB08_tr);  // Font seç
  
  // Oyun isimlerini yaz
  u8g2.setCursor(20, 25);
  u8g2.print("Flappy Bird");
  u8g2.setCursor(20, 50);
  u8g2.print("Tetris");
  
  // Seçim okunu göster
  if (selectedItem == 0) u8g2.drawStr(5, 25, ">");
  else u8g2.drawStr(5, 50, ">");
  
  u8g2.sendBuffer();  // Ekranı güncelle
}

// Menüye dönüş fonksiyonu
void returnToMenu() {
  gameStarted = false;
  currentGame = NONE;
  gameOverFlappy = false;
  gameOverTetris = false;
  joystickReleased = false;  // Joystick bırakılana kadar bekle
  selectedItem = 0;  // Menüde ilk öğeyi seç
  selectionStartTime = 0;  // Seçim süresini sıfırla
  drawMenu();  // Menüyü göster
}

// --- Flappy Bird ---
// Flappy Bird oyunu başlatma fonksiyonu
void startFlappyBird() {
  // Oyun değişkenlerini başlangıç değerlerine ayarla
  birdY = 32;  // Kuş ekranın ortasında başlar
  birdVelocity = 0;
  pipeX = 128;  // Boru ekranın sağında başlar
  pipeGapY = random(10, 44);  // Borunun boşluk konumunu rastgele belirle
  scoreFlappy = 0;
  gameOverFlappy = false;
  gameStarted = true;
  currentGame = FLAPPY;
  joystickReleased = true;  // Joystick durumunu sıfırla
}

// Flappy Bird oyun döngüsü
void flappyBirdLoop() {
  int yVal = analogRead(vrY);  // Joystick Y değerini oku
  
  // Joystick merkez konumundaysa serbest bırakılmış sayılır
  if (yVal >= 400 && yVal <= 600) {
    joystickReleased = true;
  }
  
  if (!gameOverFlappy) {
    // Oyun devam ediyorsa
    
    // Zıplama kontrolü
    if (yVal < 400) birdVelocity = jumpStrength;
    
    // Fizik güncellemeleri
    birdVelocity += gravity;  // Yerçekimi etkisi
    birdY += birdVelocity;    // Kuşu hareket ettir
    pipeX -= 2;               // Boruyu sola doğru hareket ettir
    
    // Boru sağ taraftan çıktıysa yeni boru oluştur ve skoru artır
    if (pipeX + pipeWidth < 0) {
      pipeX = 128;
      pipeGapY = random(10, 44);
      scoreFlappy++;
      BuzzTheBuzzer();  // Skor artınca ses çıkar
    }
    
    // Çarpışma kontrolü
    if (birdY <= 0 || birdY >= 64 || (pipeX < 10 && (birdY < pipeGapY || birdY > pipeGapY + pipeGapHeight))) {
      gameOverFlappy = true;
    }
    
    // Ekranı çiz
    u8g2.clearBuffer();
    u8g2.drawBox(2, birdY, 6, 6);  // Kuşu çiz
    
    // Boruları çiz
    u8g2.drawBox(pipeX, 0, pipeWidth, pipeGapY);  // Üst boru
    u8g2.drawBox(pipeX, pipeGapY + pipeGapHeight, pipeWidth, 64 - pipeGapY - pipeGapHeight);  // Alt boru
    
    // Skoru göster
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setCursor(85 , 10);
    u8g2.print("Skor:");
    u8g2.print(scoreFlappy);
    
    u8g2.sendBuffer();  // Ekranı güncelle
    delay(40);  // Oyun hızını ayarla
  } else {
    // Oyun bittiyse
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(20, 30, "Oyun Bitti!");
    u8g2.setCursor(20, 50);
    u8g2.print("Skor: ");
    u8g2.print(scoreFlappy);
    u8g2.sendBuffer();
    
    // Yukarı itildiğinde oyunu yeniden başlat
    if (yVal < 400 && joystickReleased) {
      joystickReleased = false;
      delay(300);  // Debounce süresi
      startFlappyBird();
    }
    
    // Aşağı itildiğinde menüye dön
    if (yVal > 600 && joystickReleased) {
      joystickReleased = false;
      delay(300);  // Debounce süresi
      returnToMenu();
    }
  }
}

// --- Tetris ---

// Verilen şekli geçerli şekle kopyalar
void copyShape(const byte src[4][4]) {
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      currentShape[y][x] = src[y][x];
}

// Yeni bir tetris şekli oluşturur
void spawnNewShape() {
  int r = random(3);  // Rastgele şekil seç
  
  // Seçilen şekli kopyala ve boyutunu ayarla
  if (r == 0) { copyShape(SHAPE_T); shapeSize = 3; }
  else if (r == 1) { copyShape(SHAPE_I); shapeSize = 4; }
  else { copyShape(SHAPE_L); shapeSize = 3; }

  // Şekli oyun alanının üst ortasında konumlandır
  blockX = 3;
  blockY = 0;

  // Eğer yeni şekil mevcut blokların üzerine geliyorsa oyun biter
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      if (currentShape[y][x]) {
        int gx = blockX + x;
        int gy = blockY + y;
        if (gx >= 0 && gx < cols && gy >= 0 && gy < rows && grid[gy][gx])
          gameOverTetris = true;
      }
}

// Tetris oyununu başlatır
void startTetris() {
  // Oyun alanını temizle
  for (int y = 0; y < rows; y++)
    for (int x = 0; x < cols; x++)
      grid[y][x] = 0;
      
  scoreTetris = 0;
  gameOverTetris = false;
  blockX = 3;
  blockY = 0;
  lastFallTime = millis();
  gameStarted = true;
  currentGame = TETRIS;
  joystickReleased = true;  // Joystick durumunu sıfırla
  spawnNewShape();  // İlk şekli oluştur
}

// Tetris oyun döngüsü
void tetrisLoop() {
  int yVal = analogRead(vrY);
  
  // Joystick merkezdeyse serbest bırakılmış say
  if (yVal >= 400 && yVal <= 600) {
    joystickReleased = true;
  }
  
  // Oyun bitti mi kontrol et
  if (gameOverTetris) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(10, 30, "Oyun Bitti!");
    u8g2.setCursor(10, 50);
    u8g2.print("Skor: ");
    u8g2.print(scoreTetris);
    u8g2.sendBuffer();
    
    // Yukarı itildiğinde oyunu yeniden başlat
    if (yVal < 400 && joystickReleased) {
      joystickReleased = false;
      delay(300);  // Debounce süresi
      startTetris();
    }
    
    // Aşağı itildiğinde menüye dön
    if (yVal > 600 && joystickReleased) {
      joystickReleased = false;
      delay(300);  // Debounce süresi
      returnToMenu();
    }
    return;
  }

  unsigned long now = millis();
  int xVal = analogRead(vrX);  // Joystick X değerini oku

  // Joystick hareketlerini işle (200ms debounce süresinden sonra)
  static unsigned long lastMove = 0;
  if (now - lastMove > 200) {
    if (xVal < 400 && canMove(-1, 0)) {
      // Sola hareket
      blockX--;
      lastMove = now;
    } else if (xVal > 600 && canMove(1, 0)) {
      // Sağa hareket
      blockX++;
      lastMove = now;
    } else if (yVal < 400) {
      // Saat yönünde döndür
      rotateShapeCW();
      if (!canMove(0, 0)) rotateShapeCCW();  // Dönüş geçersizse geri al
      lastMove = now;
    } else if (yVal > 600) {
      // Saat yönünün tersine döndür
      rotateShapeCCW();
      if (!canMove(0, 0)) rotateShapeCW();  // Dönüş geçersizse geri al
      lastMove = now;
    }
  }

  // Blokları aşağı düşür (her 500ms'de bir)
  if (now - lastFallTime > 500) {
    if (canMove(0, 1)) {
      blockY++;  // Aşağı hareket mümkünse düşür
    } else {
      // Aşağı hareket mümkün değilse bloğu sabitle ve yeni blok oluştur
      mergeToGrid();
      checkAndClearLines();  // Dolmuş satırları kontrol et
      spawnNewShape();
    }
    lastFallTime = now;
  }

  // Oyun alanını çiz
  drawTetrisGame();
}

// Şeklin x,y yönünde hareket edip edemeyeceğini kontrol eder
bool canMove(int dx, int dy) {
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      if (currentShape[y][x]) {
        int nx = blockX + x + dx;
        int ny = blockY + y + dy;
        // Sınırları veya diğer blokları kontrol et
        if (nx < 0 || nx >= cols || ny >= rows) return false;
        if (ny >= 0 && grid[ny][nx]) return false;
      }
  return true;  // Hareket mümkünse true döndür
}

// Aktif şekli oyun alanına sabitleyen fonksiyon
void mergeToGrid() {
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      if (currentShape[y][x]) {
        int gx = blockX + x;
        int gy = blockY + y;
        if (gx >= 0 && gx < cols && gy >= 0 && gy < rows)
          grid[gy][gx] = 1;  // Bloğu oyun alanına yerleştir
      }
}

// Dolu satırları kontrol eden ve temizleyen fonksiyon
void checkAndClearLines() {
  for (int y = rows - 1; y >= 0; y--) {
    bool full = true;
    // Satırın tamamen dolu olup olmadığını kontrol et
    for (int x = 0; x < cols; x++) {
      if (!grid[y][x]) {
        full = false;
        break;
      }
    }
    if (full) {
      // Dolu satır bulundu, skoru artır ve ses çıkar
      scoreTetris++;
      BuzzTheBuzzer();
      
      // Satırları aşağı kaydır
      for (int i = y; i > 0; i--)
        for (int j = 0; j < cols; j++)
          grid[i][j] = grid[i - 1][j];
          
      // En üst satırı boşalt
      for (int j = 0; j < cols; j++)
        grid[0][j] = 0;
        
      // Aynı satırı tekrar kontrol et (kaydırma sonrası)
      y++;
    }
  }
}

// Şekli saat yönünde döndüren fonksiyon
void rotateShapeCW() {
  byte temp[4][4];
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      temp[y][x] = currentShape[3 - x][y];
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      currentShape[y][x] = temp[y][x];
}

// Şekli saat yönünün tersine döndüren fonksiyon
void rotateShapeCCW() {
  byte temp[4][4];
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      temp[y][x] = currentShape[x][3 - y];
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      currentShape[y][x] = temp[y][x];
}

// Tetris oyun alanını çizen fonksiyon
void drawTetrisGame() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  
  // Skoru göster
  u8g2.setCursor(2, 20);
  u8g2.print("Skor:");
  u8g2.setCursor(2, 40);
  u8g2.print(scoreTetris);
  
  // Sabitlenmiş blokları çiz
  for (int y = 0; y < rows; y++)
    for (int x = 0; x < cols; x++)
      if (grid[y][x])
        u8g2.drawBox(gridStartX + x * boxW, gridStartY + y * boxH, boxW, boxH);
  
  // Aktif şekli çiz
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      if (currentShape[y][x]) {
        int gx = blockX + x;
        int gy = blockY + y;
        if (gx >= 0 && gx < cols && gy >= 0 && gy < rows)
          u8g2.drawBox(gridStartX + gx * boxW, gridStartY + gy * boxH, boxW, boxH);
      }
  
  u8g2.sendBuffer();  // Ekranı güncelle
}

// Skor arttığında veya diğer önemli olaylarda ses çıkaran fonksiyon
void BuzzTheBuzzer(){
  tone(buzzerPin,1000);  // 1kHz frekansında ses üret
  delay(500);            // 0.5 saniye bekle
  noTone(buzzerPin);     // Sesi kapat
}
