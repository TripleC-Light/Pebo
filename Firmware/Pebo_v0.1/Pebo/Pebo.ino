// Pebo LED Test
// STM32L031G6U6TR

const uint32_t LED_BLUE_1 = PA5;
const uint32_t LED_BLUE_2 = PA6;
const uint32_t LED_BLUE_3 = PA7;
const uint32_t LED_BLUE_4 = PA8;
const uint32_t LED_RED    = PA15;

void setup()
{
  pinMode(LED_BLUE_1, OUTPUT);
  pinMode(LED_BLUE_2, OUTPUT);
  pinMode(LED_BLUE_3, OUTPUT);
  pinMode(LED_BLUE_4, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // 全部先關閉
  digitalWrite(LED_BLUE_1, LOW);
  digitalWrite(LED_BLUE_2, LOW);
  digitalWrite(LED_BLUE_3, LOW);
  digitalWrite(LED_BLUE_4, LOW);
  digitalWrite(LED_RED, LOW);
}

void loop()
{
  // Blue 1
  digitalWrite(LED_BLUE_1, HIGH);
  delay(500);
  digitalWrite(LED_BLUE_1, LOW);

  // Blue 2
  digitalWrite(LED_BLUE_2, HIGH);
  delay(500);
  digitalWrite(LED_BLUE_2, LOW);

  // Blue 3
  digitalWrite(LED_BLUE_3, HIGH);
  delay(500);
  digitalWrite(LED_BLUE_3, LOW);

  // Blue 4
  digitalWrite(LED_BLUE_4, HIGH);
  delay(500);
  digitalWrite(LED_BLUE_4, LOW);

  // Red
  digitalWrite(LED_RED, HIGH);
  delay(500);
  digitalWrite(LED_RED, LOW);

  delay(1000);
}