// N1 de JW_DWIN en host con un transporte simulado.
#include "JW_DWIN.h"
#include <map>
#include <vector>

uint32_t g_mockMs = 1000;
Stream Serial;

struct Op { char type; uint16_t vp; std::vector<uint16_t> data; };

class MockTransport : public JW_DWIN_Transport
{
public:
    std::map<uint16_t, uint16_t> vp;
    std::vector<Op> ops;
    bool online = true;
    bool beginOk = true;

    bool begin() override { return beginOk; }
    bool readRegisters(uint16_t a, uint16_t n, uint16_t *d) override
    {
        ops.push_back({'R', a, {}});
        if (!online) return false;
        for (uint16_t i = 0; i < n; i++) d[i] = vp[a + i];
        return true;
    }
    bool writeRegister(uint16_t a, uint16_t v) override
    {
        ops.push_back({'W', a, {v}});
        if (!online) return false;
        vp[a] = v;
        return true;
    }
    bool writeRegisters(uint16_t a, uint16_t n, const uint16_t *s) override
    {
        ops.push_back({'M', a, std::vector<uint16_t>(s, s + n)});
        if (!online) return false;
        for (uint16_t i = 0; i < n; i++) vp[a + i] = s[i];
        return true;
    }
    const char *lastErrorString() const override { return online ? "OK" : "Timeout"; }
};

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

int main()
{
    // Antes de begin(): no se usa el transporte.
    {
        MockTransport t;
        JW_DWIN dwin(t);
        DWIN_Value v = {0x10, 0x00, 0, 0, 0};
        dwin.sendValue(v, 5);
        CHECK(t.ops.empty());
        t.beginOk = false;
        CHECK(!dwin.begin());
    }

    MockTransport t;
    JW_DWIN dwin(t);
    CHECK(dwin.begin());

    // sendValue: primera llamada escribe aunque el valor sea 0; luego cache.
    DWIN_Value prog = {0x10, 0x00, 0, 0, 0};
    dwin.sendValue(prog, 0);
    CHECK(t.ops.size() == 1 && t.ops[0].type == 'W' && t.ops[0].vp == 0x1000);
    dwin.sendValue(prog, 0);
    CHECK(t.ops.size() == 1);
    dwin.sendValue(prog, 3);
    CHECK(t.ops.size() == 2 && t.vp[0x1000] == 3);
    dwin.sendValue(prog, 3, true);
    CHECK(t.ops.size() == 3);

    // Escritura fallida: val refleja lo pedido (lógica de la aplicación) y
    // se reintenta en la siguiente llamada con el mismo valor.
    t.online = false;
    dwin.sendValue(prog, 7);
    CHECK(prog.val == 7 && t.vp[0x1000] == 3);
    t.online = true;
    t.ops.clear();
    dwin.sendValue(prog, 7);
    CHECK(t.ops.size() == 1 && t.vp[0x1000] == 7);

    // readValue
    DWIN_Value temp = {0x10, 0x01, 22, 0, 1};
    t.vp[0x1001] = 555;
    CHECK(dwin.readValue(temp) && temp.val == 555 && temp.fval > 55.4f && temp.fval < 55.6f);
    CHECK(!dwin.readValue(temp));
    t.online = false;
    t.vp[0x1001] = 1;
    CHECK(!dwin.readValue(temp) && temp.val == 555);
    t.online = true;

    // sendText: relleno con espacios, empaquetado big-endian y recorte.
    DWIN_String name = {0x10, 0xA0, "", 5};
    t.ops.clear();
    CHECK(dwin.sendText(name, "AB"));
    CHECK(t.ops.size() == 1 && t.ops[0].type == 'M' && t.ops[0].vp == 0x10A0);
    CHECK(t.ops[0].data.size() == 3 && t.ops[0].data[0] == 0x4142 && t.ops[0].data[1] == 0x2020 && t.ops[0].data[2] == 0x2000);
    CHECK(dwin.sendText(name, "AB") && t.ops.size() == 1);
    CHECK(dwin.sendText(name, "ABCDEFG") && t.ops.back().data[2] == 0x4500);
    DWIN_String tooLong = {0x10, 0xB0, "", JW_DWIN_MAX_TEXT + 1};
    CHECK(!dwin.sendText(tooLong, "x"));

    // checkKey: VP distinto de 0 activa Enable y se limpia en la DWIN.
    DWIN_Key start = {0x10, 0x31, 4, 5};
    t.vp[0x1031] = 0;
    dwin.checkKey(start);
    CHECK(!start.Enable);
    t.vp[0x1031] = 1;
    dwin.checkKey(start);
    CHECK(start.Enable && t.vp[0x1031] == 0);

    // keyEncoder con vuelta circular.
    DWIN_Key up = {0x12, 0x20, 5, 5};
    DWIN_Key down = {0x12, 0x21, 6, 5};
    uint32_t count = 5;
    t.vp[0x1220] = 1;
    dwin.keyEncoder(up, down, count, 5, 9);
    CHECK(count == 9);
    t.vp[0x1221] = 1;
    dwin.keyEncoder(up, down, count, 5, 9);
    CHECK(count == 5);

    // enableTouch: trama 0x5AA5 en 0x00B0, cache por estado, espera 25 ms.
    t.ops.clear();
    uint32_t before = millis();
    dwin.enableTouch(start, 0);
    CHECK(t.ops.size() == 1 && t.ops[0].vp == 0x00B0);
    CHECK(t.ops[0].data[0] == 0x5AA5 && t.ops[0].data[1] == 0x10 && t.ops[0].data[2] == 0x0405 && t.ops[0].data[3] == 0);
    CHECK(millis() - before == 25);
    dwin.enableTouch(start, 0);
    CHECK(t.ops.size() == 1);

    // setPage
    t.ops.clear();
    CHECK(dwin.setPage(0x12) && t.ops[0].vp == 0x0084 && t.ops[0].data[0] == 0x5A01 && t.ops[0].data[1] == 0x12);
    CHECK(dwin.currentPage() == 0x12);

    // Sin enlace: tras 3 fallos solo un intento cada 500 ms.
    t.online = false;
    for (int i = 0; i < 3; i++) dwin.sendValue(prog, 100 + i);
    CHECK(!dwin.isOnline());
    t.ops.clear();
    for (int i = 0; i < 10; i++) dwin.sendValue(prog, 200 + i);
    CHECK(t.ops.empty());
    g_mockMs += 500;
    dwin.sendValue(prog, 300);
    CHECK(t.ops.size() == 1);

    // Reconexión: invalida la cache y avisa una sola vez.
    t.online = true;
    g_mockMs += 500;
    dwin.sendValue(prog, 300);
    CHECK(dwin.isOnline() && dwin.reconnected() && !dwin.reconnected());
    t.ops.clear();
    dwin.sendText(name, "ABCDEFG");  // mismo texto: se reenvía tras reconexión
    dwin.enableTouch(start, 0);      // mismo estado: se reenvía tras reconexión
    CHECK(t.ops.size() == 2);
    dwin.sendText(name, "ABCDEFG");
    dwin.enableTouch(start, 0);
    CHECK(t.ops.size() == 2);

    // Sin debug configurado no se imprime nada.
    CHECK(Serial.out.empty());

    if (failures == 0) printf("JW_DWIN N1 PASS\n");
    return failures == 0 ? 0 : 1;
}
