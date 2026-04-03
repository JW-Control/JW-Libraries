#include "JW_DWIN_RS485.h"

JW_DWIN_RS485::JW_DWIN_RS485(ModbusMaster& node)
  : _node(node) {
}

void JW_DWIN_RS485::setPage(uint16_t page_id) {
  _node.clearTransmitBuffer();
  _node.setTransmitBuffer(0, 0x5A01);
  _node.setTransmitBuffer(1, page_id);
  _node.writeMultipleRegisters(0x84, 2);
}

void JW_DWIN_RS485::sendValue(DWIN_Value& v, uint16_t newValue, bool force, bool debug) {
  if (v.val != newValue || force) {
    v.val = newValue;
    float divider = powf(10.0f, v.decimal);
    v.fval = v.val / divider;

    uint16_t address = vpAddress(v.page, v.addr);
    uint8_t result = _node.writeSingleRegister(address, newValue);

    if (debug) {
      Serial.printf("[DWIN] WRITE 0x%04X = %u (res=0x%02X)\n", address, newValue, result);
    }
  }
}

void JW_DWIN_RS485::readValue(DWIN_Value& v, bool debug) {
  uint16_t address = vpAddress(v.page, v.addr);
  uint8_t result = _node.readHoldingRegisters(address, 1);

  if (result == _node.ku8MBSuccess) {
    uint16_t raw = _node.getResponseBuffer(0);
    if (raw != v.val) {
      v.val = raw;
      float divider = powf(10.0f, v.decimal);
      v.fval = v.val / divider;

      if (debug) {
        Serial.printf("[DWIN] READ 0x%04X = ", address);
        if (v.decimal == 0) {
          Serial.println(v.val);
        } else {
          Serial.println(v.fval, v.decimal);
        }
      }
    }
  }
}

void JW_DWIN_RS485::sendText(DWIN_String& s, const String& newText, bool force) {
  if (s.val != newText || force) {
    s.val = newText;

    String full = newText;
    while (full.length() < s.Length) {
      full += ' ';
    }
    if (full.length() > s.Length) {
      full.remove(s.Length);
    }

    const char* txt = full.c_str();
    uint16_t numRegisters = (s.Length + 1) / 2;
    uint16_t baseAddr = vpAddress(s.page, s.addr);

    _node.clearTransmitBuffer();

    for (uint16_t i = 0; i < numRegisters; i++) {
      uint16_t val = 0;
      uint16_t idx = i * 2;
      char c1 = txt[idx];
      char c2 = (idx + 1 < s.Length) ? txt[idx + 1] : 0x00;

      val = (static_cast<uint16_t>(static_cast<uint8_t>(c1)) << 8) |
            static_cast<uint8_t>(c2);
      _node.setTransmitBuffer(i, val);
    }

    _node.writeMultipleRegisters(baseAddr, numRegisters);
  }
}

void JW_DWIN_RS485::checkKey(DWIN_Key& key) {
  uint16_t address = vpAddress(key.page, key.addr);
  uint8_t result = _node.readHoldingRegisters(address, 1);

  if (result == _node.ku8MBSuccess) {
    uint16_t readVal = _node.getResponseBuffer(0);
    if (readVal != 0) {
      key.Enable = true;
      _node.writeSingleRegister(address, 0);
    }
  }
}

void JW_DWIN_RS485::handleKeyEncoder(DWIN_Key& minus,
                                     DWIN_Key& plus,
                                     uint32_t& count,
                                     uint32_t minv,
                                     uint32_t maxv) {
  checkKey(minus);
  if (minus.Enable) {
    minus.Enable = false;
    if (count == minv) count = maxv;
    else count--;
    Serial.printf("[DWIN] count = %lu\n", static_cast<unsigned long>(count));
  }

  checkKey(plus);
  if (plus.Enable) {
    plus.Enable = false;
    if (count == maxv) count = minv;
    else count++;
    Serial.printf("[DWIN] count = %lu\n", static_cast<unsigned long>(count));
  }
}
