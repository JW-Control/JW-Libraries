#include <HardwareSerial.h>
#include <ModbusMaster.h>
#include <JW_DWIN_RS485.h>

HardwareSerial Serial485(2);
ModbusMaster node;
JW_DWIN_RS485 dwin(node);

const int RXD2 = 16;
const int TXD2 = 17;
const int DE_RE_PIN = 4;

void preTransmission() {
  digitalWrite(DE_RE_PIN, HIGH);
}

void postTransmission() {
  digitalWrite(DE_RE_PIN, LOW);
}

DWIN_Value V_Speed(0x10, 0x00, 0x01, 0x00, 0);
DWIN_Key   K_Minus(0x10, 0x10, 0x01, 0x05);
DWIN_Key   K_Plus (0x10, 0x12, 0x02, 0x05);

uint32_t speedCount = 0;

void setup() {
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, LOW);

  Serial.begin(115200);
  Serial485.begin(115200, SERIAL_8N1, RXD2, TXD2);

  node.begin(1, Serial485);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  dwin.setPage(0x10);
  dwin.enableTouch(K_Minus, 1, true);
  dwin.enableTouch(K_Plus, 1, true);
  dwin.sendValue(V_Speed, speedCount, true);
}

void loop() {
  dwin.handleKeyEncoder(K_Minus, K_Plus, speedCount, 0, 999);
  dwin.sendValue(V_Speed, speedCount);
  delay(50);
}
