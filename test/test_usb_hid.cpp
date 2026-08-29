#define private public   // 测试需要访问私有构造与报告编码方法
#include "key_scan.h"
#include "usb_hid.h"
#undef private

#include "tusb_stub.h"

#include "harness.h"

#include <vector>

namespace {

class SimMatrixIO : public MatrixIO {
public:
    SimMatrixIO(const KeyMatrix& m)
        : matrix_(m), cells_(m.rows() * m.cols(), false) {}
    void init() override {}
    bool read_cell(uint8_t row, uint8_t col) const override {
        return cells_[(size_t)row * matrix_.cols() + col];
    }
    void set(uint8_t row, uint8_t col, bool pressed) {
        cells_[(size_t)row * matrix_.cols() + col] = pressed;
    }

private:
    const KeyMatrix& matrix_;
    std::vector<bool> cells_;
};

const uint8_t kRowsP[] = { 1, 2 };
const uint8_t kColsP[] = { 3, 4 };
const KeyCodes kMap[2][2] = {
    { KEY_A, KEY_B },
    { KEY_C, KEY_D },
};
const KeyMatrix::Config kCfg = {
    .rows       = 2,
    .cols       = 2,
    .row_pins   = kRowsP,
    .col_pins   = kColsP,
    .keymap     = &kMap[0][0],
    .direction  = KeyMatrix::Direction::RowToCol,
};

// 组装一套可测试的 UsbHid（KeyMatrix + SimMatrixIO + KeyScanner + UsbHid）
struct Fixture {
    KeyMatrix   matrix;
    SimMatrixIO io;
    KeyScanner  scan;
    UsbHid      hid;
    Fixture() : matrix(kCfg), io(matrix), scan(matrix, io), hid(scan) {}
};

} // namespace

UT_CASE(key_to_usage_media) {
    Fixture f;
    uint16_t u = 0;
    CHECK(f.hid.key_to_usage(KEY_E_PLAY_PAUSE, u) && u == 0x00CD);
    CHECK(f.hid.key_to_usage(KEY_E_STOP,       u) && u == 0x00B7);
    CHECK(f.hid.key_to_usage(KEY_E_NEXT_TRACK, u) && u == 0x00B5);
    CHECK(f.hid.key_to_usage(KEY_E_PREV_TRACK, u) && u == 0x00B6);
    CHECK(f.hid.key_to_usage(KEY_E_MUTE,       u) && u == 0x00E2);
    CHECK(f.hid.key_to_usage(KEY_E_VOL_INC,    u) && u == 0x00E9);
    CHECK(f.hid.key_to_usage(KEY_E_VOL_DEC,    u) && u == 0x00EA);
}

UT_CASE(key_to_usage_system) {
    Fixture f;
    uint16_t u = 0;
    CHECK(f.hid.key_to_usage(KEY_E_POWER, u) && u == 0x0030);
    CHECK(f.hid.key_to_usage(KEY_E_RESET, u) && u == 0x0031);
    CHECK(f.hid.key_to_usage(KEY_E_SLEEP, u) && u == 0x0032);
}

UT_CASE(key_to_usage_app) {
    Fixture f;
    uint16_t u = 0;
    CHECK(f.hid.key_to_usage(KEY_E_AL_EMAIL,        u) && u == 0x018A);
    CHECK(f.hid.key_to_usage(KEY_E_AL_CALCULATOR,   u) && u == 0x0192);
    CHECK(f.hid.key_to_usage(KEY_E_AL_AUDIO_PLAYER, u) && u == 0x01C6);
}

UT_CASE(key_to_usage_non_extended) {
    Fixture f;
    uint16_t u = 0;
    CHECK(!f.hid.key_to_usage(KEY_A, u));                // 普通键不是 usage
    CHECK(!f.hid.key_to_usage(KEY_P_NKRO_ON_OFF, u));    // 私有键不是 usage
}

UT_CASE(build_report_6kro) {
    Fixture f;
    std::vector<KeyCodes> pressed = { KEY_LEFT_CTRL, KEY_A, KEY_B };
    hid_keyboard_report_t r = {};
    f.hid.build_report_6kro(pressed, r);
    CHECK_EQ(r.modifier, 1u);            // L-Ctrl
    CHECK_EQ(r.keycode[0], (uint8_t)KEY_A);
    CHECK_EQ(r.keycode[1], (uint8_t)KEY_B);
    CHECK_EQ(r.keycode[2], 0u);
}

UT_CASE(build_report_nkro) {
    Fixture f;
    std::vector<KeyCodes> pressed = { KEY_A };
    uint8_t out[2 + NKRO_BITMAP_SIZE] = {0};
    f.hid.build_report_nkro(pressed, out);
    CHECK(out[2] & (uint8_t)(1u << (KEY_A & 7)));   // KEY_A=4 → bit4
    CHECK_EQ(out[0], 0u);                            // 无修饰键
    CHECK_EQ(out[1], 0u);                            // 保留字节
}

UT_CASE(build_report_consumer) {
    Fixture f;
    std::vector<KeyCodes> pressed = { KEY_E_VOL_INC };
    uint8_t out[CONSUMER_BITMAP_SIZE] = {0};
    f.hid.build_report_consumer(pressed, out);
    // 0xE9 - 0xB0 = 57 → byte 7, bit 1
    CHECK(out[7] & (uint8_t)(1u << (57 & 7)));
}

UT_CASE(build_report_system) {
    Fixture f;
    std::vector<KeyCodes> pressed = { KEY_E_POWER };
    uint8_t out[CONSUMER_SYS_BITMAP_SIZE] = {0};
    f.hid.build_report_system(pressed, out);
    CHECK(out[0] & 0x01u);   // Power → bit0
}

UT_CASE(build_report_app) {
    Fixture f;
    std::vector<KeyCodes> pressed = { KEY_E_AL_EMAIL };
    uint8_t out[CONSUMER_APP_BITMAP_SIZE] = {0};
    f.hid.build_report_app(pressed, out);
    // 0x18A - 0x184 = 6 → byte 0, bit 6
    CHECK(out[0] & (uint8_t)(1u << 6));
}

UT_CASE(nkro_mode_switch) {
    Fixture f;
    CHECK(!f.hid.nkro_enabled());
    f.hid.set_nkro(true);
    CHECK(f.hid.nkro_enabled());
    f.hid.toggle_nkro();
    CHECK(!f.hid.nkro_enabled());
}

UT_CASE(task_sends_6kro_report) {
    tusb_stub_reset();
    Fixture f;
    f.io.set(0, 0, true);                 // KEY_A 按下
    for (int i = 0; i < 5; i++) f.scan.scan();
    f.hid.task();
    CHECK_EQ(g_last_report_id, REPORT_ID_KEYBOARD);
    CHECK_EQ(g_last_report[2], (uint8_t)KEY_A);  // 6KRO: byte0=mod, byte1=res, byte2=kc0
    CHECK(g_report_calls >= 1u);
}

UT_CASE(send_consumer_report_spy) {
    tusb_stub_reset();
    Fixture f;
    f.hid.send_consumer_report({ KEY_E_VOL_INC });
    CHECK_EQ(g_last_report_id, REPORT_ID_CONSUMER);
    CHECK_EQ(g_last_report_len, (uint16_t)CONSUMER_BITMAP_SIZE);
    CHECK(g_last_report[7] & (uint8_t)(1u << 1));
}
