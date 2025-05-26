// Constantes de pinos
const byte BTN_PINS[7] = { 24, 26, 28, 30, 32, 34, 22 };  // botões de jogo
const byte BTN_TUDO = 36;                                 // botão TUDO
const LED_PINS_BNT_TUDO[3] = { 10, -1, -1 };            // led botão TUDO
int COLOR_BTN_TUDO[3] = { 255, 0, 0 };                       // vermelho (botão TUDO)
const byte BUZZER_PIN = 50;                               // buzzer
const byte POT_PIN = A0;                                  // potenciômetro
const int LED_PINS[7][3] = {                              // leds
  {11, -1, -1},     // vermelho
  {-1, 13, -1},     // verde
  {4, 5, -1},       // amarelo
  {9, 8, 7},        // roxo
  {2, 3, -1},       // laranja
  {-1, -1, 12},     // azul
  {44, -1, 45},     // rosa
};

// Cores
int COLORS[7][3] = {
  {255, 0, 0},      // vermelho
  {0, 255, 0},      // verde
  {255, 70, 0},     // amarelo
  {128, 0, 110},    // roxo
  {255, 30, 0},     // laranja
  {0, 0, 255},      // azul
  {255, 0, 70},     // rosa
};

// Parâmetros de jogo
const int TAM_MAX_SEQ = 50;    // limite fixo, evita alocação dinâmica
const int DEBOUNCE_MS = 50;    // debounce simples
const int FREQ_ACERTO = 1200;  // Hz
const int FREQ_PADRAO = 1000;  // Hz
const int FREQ_ERRO = 2000;    // Hz
const int DURACAO_TOM_MS = 400;

// Variáveis globais
unsigned long instanteDebounce = 0;

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

  pinMode(BTN_TUDO, INPUT_PULLUP);

  /* Leds */
  for (int i = 0; i < 8; i++) {
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

/**
* lerBotao
* Retorna:
*   0‒6: índice do botão de jogo pressionado
*   7: botão PASSA pressionado
*   -1: nenhum botão (ou ainda em debounce)
*/
int lerBotao() {
  if (millis() - instanteDebounce < DEBOUNCE_MS) return -1;  // ainda "debatendo"

  /* Verifica botões de jogo */
  for (byte i = 0; i < 7; i++) {
    if (digitalRead(BTN_PINS[i]) == LOW) {
      instanteDebounce = millis();
      return i;  // botão de índice i
    }
  }

  /* Verifica botão PASSA */
  if (digitalRead(BTN_TUDO) == LOW) {
    instanteDebounce = millis();
    return 7;  // código especial para PASSA
  }

  return -1;  // nada pressionado
}

/**
* tocarSom
* Emite um tom curto no buzzer.
* Parâmetro: freq (Hz)
*/
void tocarSom(int freq) {
  tone(BUZZER_PIN, freq, DURACAO_TOM_MS);
  delay(DURACAO_TOM_MS);  // garante que o som termine
  noTone(BUZZER_PIN);
}

/* Led e som do botão TUDO */
void botaoTUDO_padrao(int freq) {
  // Acende o LED
  if (LED_PINS_BNT_TUDO[0] != -1) analogWrite(LED_PINS_BNT_TUDO[0], COLOR_BTN_TUDO[0]);
  if (LED_PINS_BNT_TUDO[1] != -1) analogWrite(LED_PINS_BNT_TUDO[1], COLOR_BTN_TUDO[1]);
  if (LED_PINS_BNT_TUDO[2] != -1) analogWrite(LED_PINS_BNT_TUDO[2], COLOR_BTN_TUDO[2]);

  // Som padrão
  tone(BUZZER_PIN, freq, DURACAO_TOM_MS);
  delay(DURACAO_TOM_MS);
  noTone(BUZZER_PIN);

  // Apaga o LED
  if (LED_PINS_BNT_TUDO[0] != -1) analogWrite(LED_PINS_BNT_TUDO[0], 0);
  if (LED_PINS_BNT_TUDO[1] != -1) analogWrite(LED_PINS_BNT_TUDO[1], 0);
  if (LED_PINS_BNT_TUDO[2] != -1) analogWrite(LED_PINS_BNT_TUDO[2], 0);
}


/**
* modo1
* Implementa o modo 1 do jogo do projeto.
* Parâmetro: valorPotInicial – leitura analógica no momento de entrada
* Sai da função quando o potenciômetro variar >100 pts.
*/
void modo1(int valorPotInicial) {
  int sequencia[TAM_MAX_SEQ];
  int tamanhoSeq = 0;
  bool aguardandoJogador1 = true;

  Serial.println(F("Modo 1: Jogador 1 cria sequência, Jogador 2 repete."));
  Serial.println(F("Pressione PASSA para alternar a vez."));

  /* Loop do modo: roda até o potenciômetro mudar bastante de valor */
  while (abs(analogRead(POT_PIN) - valorPotInicial) < 100) {
    int botao = lerBotao();
    if (botao == -1) continue;  // nada novo

    /* ------------------- Fase de gravação (Jogador 1) ------------------- */
    if (aguardandoJogador1) {
      if (botao == 7) {  // PASSA: transição para Jogador 2
        if (tamanhoSeq == 0) {
          Serial.println(F("Nenhum botão registrado! Comece de novo."));
          tocarSom(FREQ_ERRO);
        } else {
          Serial.print(F("Sequência gravada com "));
          Serial.print(tamanhoSeq);
          Serial.println(F(" passos. Jogador 2, sua vez, faz teu nome!"));
          aguardandoJogador1 = false;
        }
      } else if (tamanhoSeq < TAM_MAX_SEQ) {  // grava botão
        sequencia[tamanhoSeq++] = botao;
        Serial.print(F("Gravou passo "));
        Serial.print(tamanhoSeq);
        Serial.print(F(": botão "));
        Serial.println(botao);
      } else {
        Serial.println(F("Limite de passos atingido! Pressione PASSA."));
      }
      continue;  // basicamente, não deixa passar daqui a não ser que seja a vez do jogador 2
    }

    /* ------------------- Fase de repetição (Jogador 2) ------------------ */
    static int tentativa[TAM_MAX_SEQ];
    static int tamanhoTent = 0;

    if (botao == 7) {                             // PASSA: hora de verificar
      bool acerto = (tamanhoTent == tamanhoSeq);  // acertou o tamanho da sequência
      if (acerto) {
        for (int i = 0; i < tamanhoSeq; i++) {  // verifica a sequÊncia de fato
          if (tentativa[i] != sequencia[i]) {
            acerto = false;
            break;
          }
        }
      }

      if (acerto) {
        Serial.println(F("Sequência correta! Jogador 2 venceu."));
        tocarSom(FREQ_ACERTO);
      } else {
        Serial.println(F("Sequência incorreta! Reiniciando jogo."));
        tocarSom(FREQ_ERRO);
      }

      /* Reinicia para nova partida */
      aguardandoJogador1 = true;
      tamanhoSeq = 0;
      tamanhoTent = 0;
      Serial.println(F("\nJogador 1, crie uma nova sequência."));
      delay(600);
    } else if (tamanhoTent < TAM_MAX_SEQ) {
      tentativa[tamanhoTent++] = botao;
      Serial.print(F("Jogador 2 registrou passo "));
      Serial.print(tamanhoTent);
      Serial.print(F(": botão "));
      Serial.println(botao);
    }
  }

  Serial.println(F("<<< Potenciômetro alterado – saindo do Modo 1 >>>"));
}

/**
* modo2
* Esqueleto do segundo modo; ainda não implementado.
* Parâmetro: valorPotInicial – leitura analógica no momento de entrada
*/
void modo2(int valorPotInicial) {
  Serial.println(F("Modo 2 ainda será desenvolvido."));

  /* Simples "holding loop" até o potenciômetro mudar de posição */
  while (abs(analogRead(POT_PIN) - valorPotInicial) < 100) {
    // espaço reservado para lógica futura
    delay(100);
  }

  Serial.println(F("<<< Potenciômetro alterado – saindo do Modo 2 >>>"));
}

/* Leds e buzzer */
void ledSom() {
  for (int i = 0; i < 7; i++) {
    int BUTTON_STATE = digitalRead(BTN_PINS[i]);
    if (BUTTON_STATE == LOW) {
      setColor(i, COLORS[i][0], COLORS[i][1], COLORS[i][2]);

      switch(i) {
        case 0:
        tone(BUZZER_PIN, 262, 500); // dó
        break;
        case 1:
        tone(BUZZER_PIN, 294, 500); // ré
        break;
        case 2:
        tone(BUZZER_PIN, 330, 500); // mi
        break;
        case 3:
        tone(BUZZER_PIN, 349, 500); // fá
        break;
        case 4:
        tone(BUZZER_PIN, 392, 500); // sol
        break;
        case 5:
        tone(BUZZER_PIN, 440, 500); // lá
        break;
        case 6:
        tone(BUZZER_PIN, 494, 500); // si
        break;
      }

      delay(500);
      noTone(BUZZER_PIN);

    } else {
      setColor(i, 0, 0, 0);
    }
  if (digitalRead(BTN_TUDO) == LOW) {
    botaoTUDO_padrao(FREQ_PADRAO);
  }
  }
}

void setColor(int ledIndex, int red, int green, int blue) {
  if (LED_PINS[ledIndex][0] != -1) analogWrite(LED_PINS[ledIndex][0], red);
  if (LED_PINS[ledIndex][1] != -1) analogWrite(LED_PINS[ledIndex][1], green);
  if (LED_PINS[ledIndex][2] != -1) analogWrite(LED_PINS[ledIndex][2], blue);
}
