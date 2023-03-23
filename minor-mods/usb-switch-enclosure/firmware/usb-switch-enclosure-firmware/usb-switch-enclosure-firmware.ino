#define PIN_DCDCBUCKENABLE 6
#define PIN_INDICATORLED 0

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_DCDCBUCKENABLE, OUTPUT);
  digitalWrite(PIN_DCDCBUCKENABLE, LOW);
  pinMode(PIN_INDICATORLED, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int x = analogRead(PIN_INDICATORLED);
  Serial.print(millis(), DEC);
  Serial.print(": ");
  Serial.println(x);
  digitalWrite(PIN_DCDCBUCKENABLE, (x < 512) ? HIGH : LOW);
  delay(200);
}
