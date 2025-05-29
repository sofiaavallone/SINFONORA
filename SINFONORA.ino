// Constantes de pinos
const byte BTN_PINS[7] = { 24, 26, 28, 30, 32, 34, 22 };  // botões de jogo
const byte BTN_BOTAO_TUDO = 36;                            // botão passar a vez
int COLOR_BTN_TUDO[3] = { 255, 0, 0 };                    // vermelho (botão TUDO)
const int LED_PINS_BTN_TUDO[3] = { 10, -1, -1 };          // led botão TUDO
const byte BUZZER_PIN = 50;                               // buzzer
int rodada = 0;
const byte POT_PIN = A0;                                  // potenciômetro
const int LED_PINS[7][3] = {
  // leds
  { 11, -1, -1 },  // vermelho
  { -1, 13, -1 },  // verde
  { 4, 5, -1 },    // amarelo
  { 9, 8, 7 },     // roxo
  { 2, 3, -1 },    // laranja
  { -1, -1, 12 },  // azul
  { 44, -1, 45 },  // rosa
};
// Cores
int COLORS[7][3] = {
  { 255, 0, 0 },    // vermelho
  { 0, 255, 0 },    // verde
  { 255, 70, 0 },   // amarelo
  { 160, 32, 240 },  // roxo
  { 255, 30, 0 },   // laranja
  { 0, 0, 255 },    // azul
  { 255, 0, 70 },   // rosa
};
// Parâmetros de jogo
const int TAM_MAX_SEQ = 50;    // limite fixo, evita alocação dinâmica
const int DEBOUNCE_MS = 350;    // debounce simples
const int FREQ_ACERTO = 1200;  // Hz
const int FREQ_PADRAO = 1000;  // Hz
const int FREQ_ERRO = 2000;    // Hz
const int DURACAO_TOM_MS = 400;
// Variáveis globais
unsigned long instanteDebounce = 0;
int array[50];
// Protótipos das Funções
int lerBotao();
void tocarSom(int freq);
void modo1(int valorPotInicial);
void modo2(int valorPotInicial);
void setup() {
  Serial.begin(9600);
  /* Configura botões como INPUT_PULLUP (ativo-BAIXO) */
  for (byte i = 0; i < 7; i++) {
    pinMode(BTN_PINS[i], INPUT_PULLUP);
  }
  pinMode(BTN_BOTAO_TUDO, INPUT_PULLUP);
  /* Leds */
  for (int i = 0; i < 7; i++) {
    for (int c = 0; c < 3; c++) {
      if (LED_PINS[i][c] != -1) {
        pinMode(LED_PINS[i][c], OUTPUT);
        analogWrite(LED_PINS[i][c], 0);
      }
    }
  }
  /* Buzzer */
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.println(F("=== Tudo certo, bora começar ==="));
  Serial.println(F("Gire o potenciômetro para escolher o modo."));
}
void loop() {
  int valorPot = analogRead(POT_PIN);
  static int modoAnterior = -1;              // o static faz com que essa variável não seja redefinida
  int modoAtual = (valorPot > 512) ? 1 : 2;  // if ternário
  if (modoAtual != modoAnterior) {
    Serial.print(F("\n>>> Entrando no modo "));
    Serial.println(modoAtual);
    if (modoAtual == 1) {
      modo1(valorPot);
    } else {
      modo2(valorPot);
    }
    modoAnterior = modoAtual;
  }
}
void setColor(int ledIndex, int red, int green, int blue) {
  if (LED_PINS[ledIndex][0] != -1) analogWrite(LED_PINS[ledIndex][0], red);
  if (LED_PINS[ledIndex][1] != -1) analogWrite(LED_PINS[ledIndex][1], green);
  if (LED_PINS[ledIndex][2] != -1) analogWrite(LED_PINS[ledIndex][2], blue);
}
void setColorTudo(int ledIndex, int red, int green, int blue) {
  if (LED_PINS_BTN_TUDO[0] != -1) analogWrite(LED_PINS_BTN_TUDO[0], red);
  if (LED_PINS_BTN_TUDO[1] != -1) analogWrite(LED_PINS_BTN_TUDO[1], green);
  if (LED_PINS_BTN_TUDO[2] != -1) analogWrite(LED_PINS_BTN_TUDO[2], blue);
}
/**
lerBotao
Retorna:
  0‒6: índice do botão de jogo pressionado
  7: botão PASSA pressionado
  -1: nenhum botão (ou ainda em debounce)
*/
int lerBotao() {
  if (millis() - instanteDebounce < DEBOUNCE_MS) return -1;  // ainda "debatendo"
  /* Verifica botões de jogo */
  for (byte i = 0; i < 7; i++) {
    if (digitalRead(BTN_PINS[i]) == LOW) {
      instanteDebounce = millis();
      setColor(i, COLORS[i][0], COLORS[i][1], COLORS[i][2]);
      if (i == 0) {
        tone(BUZZER_PIN, 262, 500);  // dó
      } else if (i == 1) {
        tone(BUZZER_PIN, 294, 500);  // ré
      } else if (i == 2) {
        tone(BUZZER_PIN, 330, 500);  // mi
      } else if (i == 3) {
        tone(BUZZER_PIN, 349, 500);  // fá
      } else if (i == 4) {
        tone(BUZZER_PIN, 392, 500);  // sol
      } else if (i == 5) {
        tone(BUZZER_PIN, 440, 500);  // lá
      } else if (i == 6) {
        tone(BUZZER_PIN, 494, 500);  // si
      }
      return i;  // botão de índice i
    } else {
      setColor(i, 0, 0, 0);
    }
  }
  /* Verifica botão PASSA */
  if (digitalRead(BTN_BOTAO_TUDO) == LOW) {
    instanteDebounce = millis();
    setColorTudo(LED_PINS_BTN_TUDO, COLOR_BTN_TUDO[0], COLOR_BTN_TUDO[1], COLOR_BTN_TUDO[2]);
    tone(BUZZER_PIN, FREQ_PADRAO, DURACAO_TOM_MS);
    return 7;  // código especial para PASSA
  } else {
    setColorTudo(LED_PINS_BTN_TUDO, 0, 0, 0);
  }
  return -1;  // nada pressionado
}
/**
tocarSom
Emite um tom curto no buzzer.
Parâmetro: freq (Hz)
*/
void tocarSom(int freq) {
  tone(BUZZER_PIN, freq, DURACAO_TOM_MS);
  delay(DURACAO_TOM_MS);  // garante que o som termine
  noTone(BUZZER_PIN);
}
/**
modo1
Implementa o modo 1 do jogo do projeto.
Parâmetro: valorPotInicial – leitura analógica no momento de entrada
Sai da função quando o potenciômetro variar >100 pts.
*/
void modo1(int valorPotInicial) {
  int rodada = 0;
  bool jogando = true;

  Serial.println("Modo 1: Sequência de memória.");
  Serial.println("Jogador 1 começa criando a sequência.");
  Serial.println("Pressione PASSA para passar a vez.");

  while (abs(analogRead(POT_PIN) - valorPotInicial) < 100 && jogando) {
    while (true) {
      int botao = lerBotao();
      if (botao == -1) continue;

      if (botao == 7) {  // PASSA
        Serial.println("Jogador passou a vez.");
        break;
      } else {
        array[rodada] = botao;
        rodada++;
        Serial.print("Botão adicionado: ");
        Serial.println(botao);
        delay(300);
      }
    }

    Serial.println("Agora o próximo jogador deve repetir a sequência:");
    bool acertou = true;
    for (int i = 0; i < rodada; i++) {
      int tentativa = -1;
      while (tentativa == -1) {
        tentativa = lerBotao();
      }

      if (tentativa != array[i]) {
        Serial.println("Errou a sequência!");
        gameOver();
        jogando = false;
        acertou = false;
        break;
      } else {
        Serial.print("Correto: ");
        Serial.println(tentativa);
        delay(300);
      }
    }

    if (acertou) {
      Serial.println("Acertou a sequência!");
      efeitoOnda();
      // Continua para próxima rodada (jogador seguinte adiciona mais um botão)
    }
  }

  Serial.println("<<< Potenciômetro alterado – saindo do Modo 1 >>>");
}

/**
modo2
Esqueleto do segundo modo; ainda não implementado.
Parâmetro: valorPotInicial – leitura analógica no momento de entrada
*/
void modo2(int valorPotInicial) {
  Serial.println("Modo 2 ainda será desenvolvido.");
  /* Simples "holding loop" até o potenciômetro mudar de posição */
  while (abs(analogRead(POT_PIN) - valorPotInicial) < 100) {
    // espaço reservado para lógica futura
    delay(100);
  }
  Serial.println(F("<<< Potenciômetro alterado – saindo do Modo 2 >>>"));
}
void efeitoOnda() {
  for (int i = 0; i < 7; i++) {
    setColor(i, COLORS[i][0], COLORS[i][1], COLORS[i][2]);
    tone(BUZZER_PIN, 700, 100);
    delay(100);
    setColor(LED_PINS, 0, 0, 0);
    delay(100);
  }
}
void gameOver() {
  for (byte i = 0; i < 4; i++) {
    setColorTudo(LED_PINS_BTN_TUDO, COLOR_BTN_TUDO[0], COLOR_BTN_TUDO[1], COLOR_BTN_TUDO[2]);
    tone(BUZZER_PIN, 150, 1000);
    delay(300);
    noTone(BUZZER_PIN);
    setColorTudo(LED_PINS_BTN_TUDO, 0, 0, 0);
  }
}
