#pragma once

#include <Arduino.h>
#include <ModbusMaster.h>

struct DWIN_Key {
  uint16_t page;
  uint16_t addr;
  uint8_t  ID;
  uint8_t  Code;
  uint16_t State;
  bool     Enable;
  uint16_t val;

  DWIN_Key(
    uint16_t _page = 0,
    uint16_t _addr = 0,
    uint8_t  _ID   = 0,
    uint8_t  _Code = 0
  )
    : page(_page),
      addr(_addr),
      ID(_ID),
      Code(_Code),
      State(1),
      Enable(false),
      val(0) {}
};

struct DWIN_String {
  uint16_t page;
  uint16_t addr;
  String   val;
  uint16_t Length;

  DWIN_String(
    uint16_t _page = 0,
    uint16_t _addr = 0,
    uint16_t _len  = 0
  )
    : page(_page),
      addr(_addr),
      val(""),
      Length(_len) {}
};

struct DWIN_Value {
  uint16_t page;
  uint16_t addr;
  uint8_t  ID;
  uint8_t  Code;
  uint8_t  decimal;
  uint16_t State;
  uint16_t val;
  float    fval;

  DWIN_Value(
    uint16_t _page    = 0,
    uint16_t _addr    = 0,
    uint8_t  _ID      = 0,
    uint8_t  _Code    = 0,
    uint8_t  _decimal = 0
  )
    : page(_page),
      addr(_addr),
      ID(_ID),
      Code(_Code),
      decimal(_decimal),
      State(1),
      val(0),
      fval(0.0f) {}
};

class JW_DWIN_RS485 {
public:
  explicit JW_DWIN_RS485(ModbusMaster& node);

  void setPage(uint16_t page_id);

  template<typename T>
  void enableTouch(T& obj, uint16_t NewState, bool force = false) {
    if (NewState != obj.State || force) {
      uint16_t frame[4];
      frame[0] = 0x5AA5;
      frame[1] = obj.page;
      frame[2] = (static_cast<uint16_t>(obj.ID) << 8) | obj.Code;
      frame[3] = NewState;

      obj.State = NewState;

      _node.clearTransmitBuffer();
      for (uint8_t i = 0; i < 4; i++) {
        _node.setTransmitBuffer(i, frame[i]);
      }
      _node.writeMultipleRegisters(0xB0, 4);
      delay(25);
    }
  }

  void sendValue(DWIN_Value& v, uint16_t newValue, bool force = false, bool debug = false);
  void readValue(DWIN_Value& v, bool debug = false);
  void sendText(DWIN_String& s, const String& newText, bool force = false);
  void checkKey(DWIN_Key& key);
  void handleKeyEncoder(DWIN_Key& minus,
                        DWIN_Key& plus,
                        uint32_t& count,
                        uint32_t minv,
                        uint32_t maxv);

private:
  ModbusMaster& _node;

  inline uint16_t vpAddress(uint16_t page, uint16_t addr) const {
    return static_cast<uint16_t>((page << 8) | addr);
  }
};
