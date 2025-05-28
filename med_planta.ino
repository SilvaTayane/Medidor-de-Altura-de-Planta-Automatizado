#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NewPing.h>

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Sensor ultrassônico
#define TRIG_PIN 9
#define ECHO_PIN 10
#define MAX_DISTANCE 200

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

// Altura fixa do sensor ao solo (22 cm)
const int altura_solo_fixa_cm = 22;

// Array para armazenar leituras válidas (crescentes ou iguais)
const int NUM_LEITURAS = 100;
int leituras_validas[NUM_LEITURAS];
int indice = 0;
int total_leituras_validas = 0;

// Variáveis para exibição
int altura_planta = 0;
int delta = 0;

// Controle de tempo
unsigned long tempo_inicial = 0;

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Monitor Planta");
  delay(2000);
  lcd.clear();

  tempo_inicial = millis(); // início do monitoramento
}

void loop() {
  delay(2000);

  int distancia = sonar.ping_cm();
  if (distancia == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Erro no sensor   ");
    return;
  }

  // Calcula a altura da planta (solo fixo em 22 cm)
  int altura_atual = altura_solo_fixa_cm - distancia;
  bool leitura_valida = false;

  if (total_leituras_validas == 0) {
    leitura_valida = true; // primeira leitura
  } else {
    int ultima_valida = leituras_validas[(indice + NUM_LEITURAS - 1) % NUM_LEITURAS];
    if (altura_atual >= ultima_valida) {
      leitura_valida = true;
    }
  }

  // Atualiza somente se for leitura válida
  if (leitura_valida) {
    int ultima_valida = (total_leituras_validas > 0)
                        ? leituras_validas[(indice + NUM_LEITURAS - 1) % NUM_LEITURAS]
                        : altura_atual;

    delta = altura_atual - ultima_valida;
    altura_planta = altura_atual;

    // Armazena no array circular
    leituras_validas[indice] = altura_atual;
    indice = (indice + 1) % NUM_LEITURAS;
    if (total_leituras_validas < NUM_LEITURAS) total_leituras_validas++;
  }

  // Calcula taxa de crescimento
  float taxa_crescimento = 0;
  unsigned long tempo_atual = millis();
  float minutos_passados = (tempo_atual - tempo_inicial) / 60000.0;

  if (total_leituras_validas >= 2 && minutos_passados > 0) {
    int altura_inicio = leituras_validas[(indice + NUM_LEITURAS - total_leituras_validas) % NUM_LEITURAS];
    int altura_fim = leituras_validas[(indice + NUM_LEITURAS - 1) % NUM_LEITURAS];
    taxa_crescimento = (float)(altura_fim - altura_inicio) / minutos_passados;
  }

  // Exibição no LCD
  lcd.setCursor(0, 0);
  lcd.print("Monitor de Altura");

  lcd.setCursor(0, 1);
  lcd.print("Altura Solo: 22cm");

  lcd.setCursor(0, 2);
  lcd.print("Alt. Planta: ");
  lcd.print(altura_planta);
  lcd.print(" cm   ");

  lcd.setCursor(0, 3);
  lcd.print("Cresc: ");
  lcd.print(taxa_crescimento, 2);
  lcd.print(" cm/min");
}
