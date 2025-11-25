// ESP32 + 74HC595 + LDR -> controle de brilho via OE PWM (active LOW)
#include <Arduino.h>

#define PIN_DATA   23   // SER / DS
#define PIN_CLOCK  18   // SRCLK
#define PIN_LATCH  5    // RCLK
#define PIN_OE     14   // OE (active LOW) -> PWM here

#define ADC_PIN    34   // LDR node -> ADC1_CH6 (GPIO34)

#define NUM_595    1    // quantidade de 74HC595 encadeados

// PWM settings (ESP32 ledc)
const int PWM_CH = 0;
const int PWM_FREQ = 5000;    // 5 kHz, tá bom pra LED
const int PWM_RES = 8;        // 8-bit resolution -> duty 0..255
const int MAX_DUTY = (1<<PWM_RES) - 1; // 255

// smoothing
const int SAMPLES = 8;
int sampleBuf[SAMPLES];
int sampleIdx = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_OE, OUTPUT);
  digitalWrite(PIN_DATA, LOW);
  digitalWrite(PIN_CLOCK, LOW);
  digitalWrite(PIN_LATCH, LOW);
  // setup PWM on OE pin (remember OE is active low)
  ledcSetup(PWM_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(PIN_OE, PWM_CH);
  // start with outputs enabled (OE low) -> max brightness
  ledcWrite(PWM_CH, 0); // 0 means OE low -> enabled (but we'll invert below properly)
  // zero samples
  for(int i=0;i<SAMPLES;i++) sampleBuf[i]=0;
}

void shiftOutByte(uint8_t data) {
  for (int i = 7; i >= 0; i--) {
    uint8_t bitv = (data >> i) & 1;
    digitalWrite(PIN_DATA, bitv ? HIGH : LOW);
    digitalWrite(PIN_CLOCK, HIGH);
    digitalWrite(PIN_CLOCK, LOW);
  }
}

void shiftOutBytes(uint8_t *dados, int n) {
  digitalWrite(PIN_LATCH, LOW);
  for (int i = 0; i < n; i++) {
    shiftOutByte(dados[i]);
  }
  digitalWrite(PIN_LATCH, HIGH);
  digitalWrite(PIN_LATCH, LOW);
}

int readLDRsmooth() {
  int v = analogRead(ADC_PIN); // 0..4095
  sampleBuf[sampleIdx] = v;
  sampleIdx = (sampleIdx+1) % SAMPLES;
  long sum=0;
  for(int i=0;i<SAMPLES;i++) sum += sampleBuf[i];
  return sum / SAMPLES; // média
}

void loop() {
  // EXEMPLO: pattern simples nos leds (pode substituir)
  static uint8_t data[NUM_595];
  for (int i=0;i<NUM_595;i++) data[i]=0xFF; // por enquanto todos acesos (controlamos brilho via OE)

  shiftOutBytes(data, NUM_595);

  // lê LDR
  int raw = readLDRsmooth(); // 0..4095
  // mapa invertido: raw grande (muita luz) -> duty baixo (LED off)
  // vamos mapear raw [min..max] para duty [MAX_DUTY..0]
  // primeiro calibra min/max:
  const int MIN_RAW = 5;    //medir em ambiente escuro
  const int MAX_RAW = 1000; // ajustar medindo em muita luz; 1000 pode saturar
  int clipped = raw;
  if (clipped < MIN_RAW) clipped = MIN_RAW;
  if (clipped > MAX_RAW) clipped = MAX_RAW;

  // inverte e mapeia
  float norm = float(clipped - MIN_RAW) / float(MAX_RAW - MIN_RAW); // 0..1
  // norm=0 -> escuro ; norm=1 -> muita luz
  int duty = (int)((1.0 - norm) * MAX_DUTY + 0.5); // inverso
  int oeDuty = MAX_DUTY - duty;
  ledcWrite(PWM_CH, oeDuty);

  // debug
  Serial.printf("raw=%d norm=%.2f duty=%d oeDuty=%d\n", raw, norm, duty, oeDuty);
  Serial.println(raw);
  delay(200);

}
