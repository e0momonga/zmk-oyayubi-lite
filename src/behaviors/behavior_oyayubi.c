/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * NICOLA Oyayubi (Thumb Shift) Input Behavior
 * Based on official NICOLA layout from https://forum.pc5bai.com/work/oya/layout/
 */

#define DT_DRV_COMPAT zmk_behavior_oyayubi

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/keymap.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <dt-bindings/zmk/keys.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// ===============================
// 1. ビットマスク定義
// ===============================
#define B_Q (1UL << 0)
#define B_W (1UL << 1)
#define B_E (1UL << 2)
#define B_R (1UL << 3)
#define B_T (1UL << 4)
#define B_Y (1UL << 5)
#define B_U (1UL << 6)
#define B_I (1UL << 7)
#define B_O (1UL << 8)
#define B_P (1UL << 9)

#define B_A (1UL << 10)
#define B_S (1UL << 11)
#define B_D (1UL << 12)
#define B_F (1UL << 13)
#define B_G (1UL << 14)
#define B_H (1UL << 15)
#define B_J (1UL << 16)
#define B_K (1UL << 17)
#define B_L (1UL << 18)
#define B_SEMICOLON (1UL << 19)

#define B_Z (1UL << 20)
#define B_X (1UL << 21)
#define B_C (1UL << 22)
#define B_V (1UL << 23)
#define B_B (1UL << 24)
#define B_N (1UL << 25)
#define B_M (1UL << 26)
#define B_COMMA (1UL << 27)
#define B_DOT (1UL << 28)
#define B_SLASH (1UL << 29)

#define NONE 0

// キーコードからビットマスクへの変換テーブル
static uint32_t ng_key[256] = {0};

static uint32_t pressed_keys = 0UL;
static int8_t n_pressed_keys = 0;

static void init_key_table(void) {
    ng_key[HID_USAGE_KEY_KEYBOARD_Q] = B_Q;
    ng_key[HID_USAGE_KEY_KEYBOARD_W] = B_W;
    ng_key[HID_USAGE_KEY_KEYBOARD_E] = B_E;
    ng_key[HID_USAGE_KEY_KEYBOARD_R] = B_R;
    ng_key[HID_USAGE_KEY_KEYBOARD_T] = B_T;
    ng_key[HID_USAGE_KEY_KEYBOARD_Y] = B_Y;
    ng_key[HID_USAGE_KEY_KEYBOARD_U] = B_U;
    ng_key[HID_USAGE_KEY_KEYBOARD_I] = B_I;
    ng_key[HID_USAGE_KEY_KEYBOARD_O] = B_O;
    ng_key[HID_USAGE_KEY_KEYBOARD_P] = B_P;

    ng_key[HID_USAGE_KEY_KEYBOARD_A] = B_A;
    ng_key[HID_USAGE_KEY_KEYBOARD_S] = B_S;
    ng_key[HID_USAGE_KEY_KEYBOARD_D] = B_D;
    ng_key[HID_USAGE_KEY_KEYBOARD_F] = B_F;
    ng_key[HID_USAGE_KEY_KEYBOARD_G] = B_G;
    ng_key[HID_USAGE_KEY_KEYBOARD_H] = B_H;
    ng_key[HID_USAGE_KEY_KEYBOARD_J] = B_J;
    ng_key[HID_USAGE_KEY_KEYBOARD_K] = B_K;
    ng_key[HID_USAGE_KEY_KEYBOARD_L] = B_L;
    ng_key[HID_USAGE_KEY_KEYBOARD_SEMICOLON] = B_SEMICOLON;

    ng_key[HID_USAGE_KEY_KEYBOARD_Z] = B_Z;
    ng_key[HID_USAGE_KEY_KEYBOARD_X] = B_X;
    ng_key[HID_USAGE_KEY_KEYBOARD_C] = B_C;
    ng_key[HID_USAGE_KEY_KEYBOARD_V] = B_V;
    ng_key[HID_USAGE_KEY_KEYBOARD_B] = B_B;
    ng_key[HID_USAGE_KEY_KEYBOARD_N] = B_N;
    ng_key[HID_USAGE_KEY_KEYBOARD_M] = B_M;
    ng_key[HID_USAGE_KEY_KEYBOARD_COMMA] = B_COMMA;
    ng_key[HID_USAGE_KEY_KEYBOARD_DOT] = B_DOT;
    ng_key[HID_USAGE_KEY_KEYBOARD_SLASH] = B_SLASH;
}

// ===============================
// 2. データ構造
// ===============================
typedef struct {
    uint32_t shift;      // NICOLA実装では使用しない
    uint32_t douji;      // キービットマスク
    uint32_t kana[6];    // ローマ字出力（最大6文字）
} naginata_kanamap;

// ===============================
// 3. Layer 7用配列テーブル（左親指シフト）
// Based on https://forum.pc5bai.com/work/oya/layout/
// 公式NICOLA: ぁえりゃれ/をあなゅも/ぅーろやぃ/ぱぢぐづぴ/ばどぎぽ/ぷぞぺぼ
// ===============================
static const naginata_kanamap ngdickana_layer7[] = {
    // QWERTY上段 - 左親指シフト: ぁえりゃれ/ぱぢぐづぴ
    {.shift = NONE, .douji = B_Q, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_A, NONE}},  // Q:ぁ
    {.shift = NONE, .douji = B_W, .kana = {HID_USAGE_KEY_KEYBOARD_E, NONE}},  // W:え
    {.shift = NONE, .douji = B_E, .kana = {HID_USAGE_KEY_KEYBOARD_R, HID_USAGE_KEY_KEYBOARD_I, NONE}},  // E:り
    {.shift = NONE, .douji = B_R, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_Y, HID_USAGE_KEY_KEYBOARD_A, NONE}},  // R:ゃ
    {.shift = NONE, .douji = B_T, .kana = {HID_USAGE_KEY_KEYBOARD_R, HID_USAGE_KEY_KEYBOARD_E, NONE}},  // T:れ
    {.shift = NONE, .douji = B_Y, .kana = {HID_USAGE_KEY_KEYBOARD_P, HID_USAGE_KEY_KEYBOARD_A, NONE}},  // Y:ぱ
    {.shift = NONE, .douji = B_U, .kana = {HID_USAGE_KEY_KEYBOARD_D, HID_USAGE_KEY_KEYBOARD_I, NONE}},  // U:ぢ
    {.shift = NONE, .douji = B_I, .kana = {HID_USAGE_KEY_KEYBOARD_G, HID_USAGE_KEY_KEYBOARD_U, NONE}},  // I:ぐ
    {.shift = NONE, .douji = B_O, .kana = {HID_USAGE_KEY_KEYBOARD_D, HID_USAGE_KEY_KEYBOARD_U, NONE}},  // O:づ
    {.shift = NONE, .douji = B_P, .kana = {HID_USAGE_KEY_KEYBOARD_P, HID_USAGE_KEY_KEYBOARD_I, NONE}},  // P:ぴ

    // ASDFG中段 - 左親指シフト: をあなゅも/ばどぎぽ
    {.shift = NONE, .douji = B_A, .kana = {HID_USAGE_KEY_KEYBOARD_W, HID_USAGE_KEY_KEYBOARD_O, NONE}},  // A:を
    {.shift = NONE, .douji = B_S, .kana = {HID_USAGE_KEY_KEYBOARD_A, NONE}},  // S:あ
    {.shift = NONE, .douji = B_D, .kana = {HID_USAGE_KEY_KEYBOARD_N, HID_USAGE_KEY_KEYBOARD_A, NONE}},  // D:な
    {.shift = NONE, .douji = B_F, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_Y, HID_USAGE_KEY_KEYBOARD_U, NONE}},  // F:ゅ
    {.shift = NONE, .douji = B_G, .kana = {HID_USAGE_KEY_KEYBOARD_M, HID_USAGE_KEY_KEYBOARD_O, NONE}},  // G:も
    {.shift = NONE, .douji = B_H, .kana = {HID_USAGE_KEY_KEYBOARD_B, HID_USAGE_KEY_KEYBOARD_A, NONE}},  // H:ば
    {.shift = NONE, .douji = B_J, .kana = {HID_USAGE_KEY_KEYBOARD_D, HID_USAGE_KEY_KEYBOARD_O, NONE}},  // J:ど
    {.shift = NONE, .douji = B_K, .kana = {HID_USAGE_KEY_KEYBOARD_G, HID_USAGE_KEY_KEYBOARD_I, NONE}},  // K:ぎ
    {.shift = NONE, .douji = B_L, .kana = {HID_USAGE_KEY_KEYBOARD_P, HID_USAGE_KEY_KEYBOARD_O, NONE}},  // L:ぽ
    {.shift = NONE, .douji = B_SEMICOLON, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_T, HID_USAGE_KEY_KEYBOARD_U, NONE}},  // ;:っ

    // ZXCVB下段 - 左親指シフト: ぅーろやぃ/ぷぞぺぼ
    {.shift = NONE, .douji = B_Z, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_U, NONE}},  // Z:ぅ
    {.shift = NONE, .douji = B_X, .kana = {HID_USAGE_KEY_KEYBOARD_MINUS, NONE}},  // X:ー
    {.shift = NONE, .douji = B_C, .kana = {HID_USAGE_KEY_KEYBOARD_R, HID_USAGE_KEY_KEYBOARD_O, NONE}},  // C:ろ
    {.shift = NONE, .douji = B_V, .kana = {HID_USAGE_KEY_KEYBOARD_Y, HID_USAGE_KEY_KEYBOARD_A, NONE}},  // V:や
    {.shift = NONE, .douji = B_B, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_I, NONE}},  // B:ぃ
    {.shift = NONE, .douji = B_N, .kana = {HID_USAGE_KEY_KEYBOARD_P, HID_USAGE_KEY_KEYBOARD_U, NONE}},  // N:ぷ
    {.shift = NONE, .douji = B_M, .kana = {HID_USAGE_KEY_KEYBOARD_Z, HID_USAGE_KEY_KEYBOARD_O, NONE}},  // M:ぞ
    {.shift = NONE, .douji = B_COMMA, .kana = {HID_USAGE_KEY_KEYBOARD_P, HID_USAGE_KEY_KEYBOARD_E, NONE}},  // ,:ぺ
    {.shift = NONE, .douji = B_DOT, .kana = {HID_USAGE_KEY_KEYBOARD_B, HID_USAGE_KEY_KEYBOARD_O, NONE}},  // .:ぼ
    // /:゛ - 濁点記号（実装省略）
};

// ===============================
// 4. Layer 8用配列テーブル（右親指シフト）
// Based on https://forum.pc5bai.com/work/oya/layout/
// 公式NICOLA: がだござ/ゔじでげぜ/びずぶべ/よにるまぇ/みおのょっ/ぬゆむわぉ
// ===============================
static const naginata_kanamap ngdickana_layer8[] = {
    // QWERTY上段 - 右親指シフト: ゜がだござ/よにるまぇ (゜は省略)
    {.shift = NONE, .douji = B_W, .kana = {HID_USAGE_KEY_KEYBOARD_G, HID_USAGE_KEY_KEYBOARD_A, NONE}},  // W:が
    {.shift = NONE, .douji = B_E, .kana = {HID_USAGE_KEY_KEYBOARD_D, HID_USAGE_KEY_KEYBOARD_A, NONE}},  // E:だ
    {.shift = NONE, .douji = B_R, .kana = {HID_USAGE_KEY_KEYBOARD_G, HID_USAGE_KEY_KEYBOARD_O, NONE}},  // R:ご
    {.shift = NONE, .douji = B_T, .kana = {HID_USAGE_KEY_KEYBOARD_Z, HID_USAGE_KEY_KEYBOARD_A, NONE}},  // T:ざ
    {.shift = NONE, .douji = B_Y, .kana = {HID_USAGE_KEY_KEYBOARD_Y, HID_USAGE_KEY_KEYBOARD_O, NONE}},  // Y:よ
    {.shift = NONE, .douji = B_U, .kana = {HID_USAGE_KEY_KEYBOARD_N, HID_USAGE_KEY_KEYBOARD_I, NONE}},  // U:に
    {.shift = NONE, .douji = B_I, .kana = {HID_USAGE_KEY_KEYBOARD_R, HID_USAGE_KEY_KEYBOARD_U, NONE}},  // I:る
    {.shift = NONE, .douji = B_O, .kana = {HID_USAGE_KEY_KEYBOARD_M, HID_USAGE_KEY_KEYBOARD_A, NONE}},  // O:ま
    {.shift = NONE, .douji = B_P, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_E, NONE}},  // P:ぇ

    // ASDFG中段 - 右親指シフト: ゔじでげぜ/みおのょっ
    {.shift = NONE, .douji = B_A, .kana = {HID_USAGE_KEY_KEYBOARD_V, HID_USAGE_KEY_KEYBOARD_U, NONE}},  // A:ゔ
    {.shift = NONE, .douji = B_S, .kana = {HID_USAGE_KEY_KEYBOARD_Z, HID_USAGE_KEY_KEYBOARD_I, NONE}},  // S:じ
    {.shift = NONE, .douji = B_D, .kana = {HID_USAGE_KEY_KEYBOARD_D, HID_USAGE_KEY_KEYBOARD_E, NONE}},  // D:で
    {.shift = NONE, .douji = B_F, .kana = {HID_USAGE_KEY_KEYBOARD_G, HID_USAGE_KEY_KEYBOARD_E, NONE}},  // F:げ
    {.shift = NONE, .douji = B_G, .kana = {HID_USAGE_KEY_KEYBOARD_Z, HID_USAGE_KEY_KEYBOARD_E, NONE}},  // G:ぜ
    {.shift = NONE, .douji = B_H, .kana = {HID_USAGE_KEY_KEYBOARD_M, HID_USAGE_KEY_KEYBOARD_I, NONE}},  // H:み
    {.shift = NONE, .douji = B_J, .kana = {HID_USAGE_KEY_KEYBOARD_O, NONE}},  // J:お
    {.shift = NONE, .douji = B_K, .kana = {HID_USAGE_KEY_KEYBOARD_N, HID_USAGE_KEY_KEYBOARD_O, NONE}},  // K:の
    {.shift = NONE, .douji = B_L, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_Y, HID_USAGE_KEY_KEYBOARD_O, NONE}},  // L:ょ
    // ;:空白 - 実装省略

    // ZXCVB下段 - 右親指シフト: びずぶべ（空）/ぬゆむわぉ
    {.shift = NONE, .douji = B_Z, .kana = {HID_USAGE_KEY_KEYBOARD_B, HID_USAGE_KEY_KEYBOARD_I, NONE}},  // Z:び
    {.shift = NONE, .douji = B_X, .kana = {HID_USAGE_KEY_KEYBOARD_Z, HID_USAGE_KEY_KEYBOARD_U, NONE}},  // X:ず
    {.shift = NONE, .douji = B_C, .kana = {HID_USAGE_KEY_KEYBOARD_B, HID_USAGE_KEY_KEYBOARD_U, NONE}},  // C:ぶ
    {.shift = NONE, .douji = B_V, .kana = {HID_USAGE_KEY_KEYBOARD_B, HID_USAGE_KEY_KEYBOARD_E, NONE}},  // V:べ
    // B:空白 - 実装省略
    {.shift = NONE, .douji = B_N, .kana = {HID_USAGE_KEY_KEYBOARD_N, HID_USAGE_KEY_KEYBOARD_U, NONE}},  // N:ぬ
    {.shift = NONE, .douji = B_M, .kana = {HID_USAGE_KEY_KEYBOARD_Y, HID_USAGE_KEY_KEYBOARD_U, NONE}},  // M:ゆ
    {.shift = NONE, .douji = B_COMMA, .kana = {HID_USAGE_KEY_KEYBOARD_M, HID_USAGE_KEY_KEYBOARD_U, NONE}},  // ,:む
    {.shift = NONE, .douji = B_DOT, .kana = {HID_USAGE_KEY_KEYBOARD_W, HID_USAGE_KEY_KEYBOARD_A, NONE}},  // .:わ
    {.shift = NONE, .douji = B_SLASH, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_O, NONE}},  // /:ぉ
};

#define LAYER7_SIZE (sizeof(ngdickana_layer7) / sizeof(ngdickana_layer7[0]))
#define LAYER8_SIZE (sizeof(ngdickana_layer8) / sizeof(ngdickana_layer8[0]))

// ===============================
// 5. 関数実装
// ===============================

// キー入力が何個のエントリにマッチするかカウント
static int count_kana_entries(uint32_t keyset, int layer) {
    const naginata_kanamap *table;
    int table_size;

    if (layer == 7) {
        table = ngdickana_layer7;
        table_size = LAYER7_SIZE;
    } else {
        table = ngdickana_layer8;
        table_size = LAYER8_SIZE;
    }

    int count = 0;
    for (int i = 0; i < table_size; i++) {
        if ((table[i].douji & keyset) == table[i].douji) {
            count++;
        }
    }
    return count;
}

// マッチしたエントリの文字を出力
static void oy_type(uint32_t keyset, int layer, int64_t timestamp) {
    const naginata_kanamap *table;
    int table_size;

    if (layer == 7) {
        table = ngdickana_layer7;
        table_size = LAYER7_SIZE;
    } else {
        table = ngdickana_layer8;
        table_size = LAYER8_SIZE;
    }

    for (int i = 0; i < table_size; i++) {
        if (table[i].douji == keyset) {
            // ローマ字出力
            for (int k = 0; k < 6; k++) {
                if (table[i].kana[k] == NONE) break;

                // キープレス
                raise_zmk_keycode_state_changed_from_encoded(
                    table[i].kana[k], true, timestamp);
                // キーリリース
                raise_zmk_keycode_state_changed_from_encoded(
                    table[i].kana[k], false, timestamp);
            }
            return;
        }
    }
}

// Behavior: キー押下処理
static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    uint32_t keycode = binding->param1;
    int layer = event.layer;

    if (keycode < 256 && ng_key[keycode] != 0) {
        pressed_keys |= ng_key[keycode];
        n_pressed_keys++;

        int count = count_kana_entries(pressed_keys, layer);
        if (count == 1) {
            // 1つに確定したら即座に出力
            oy_type(pressed_keys, layer, event.timestamp);
            pressed_keys = 0UL;
            n_pressed_keys = 0;
        }
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

// Behavior: キー解放処理
static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    uint32_t keycode = binding->param1;
    int layer = event.layer;

    if (keycode < 256 && ng_key[keycode] != 0) {
        pressed_keys &= ~ng_key[keycode];
        n_pressed_keys--;

        // 全てのキーが離されたら残りを出力
        if (n_pressed_keys == 0 && pressed_keys != 0UL) {
            oy_type(pressed_keys, layer, event.timestamp);
            pressed_keys = 0UL;
        }
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

// ===============================
// 6. ZMK Behavior登録
// ===============================
static const struct behavior_driver_api behavior_oyayubi_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

static int behavior_oyayubi_init(const struct device *dev) {
    init_key_table();
    return 0;
}

#define OYAYUBI_INST(n)                                                                            \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_oyayubi_init, NULL, NULL, NULL, POST_KERNEL,              \
                           CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_oyayubi_driver_api);

DT_INST_FOREACH_STATUS_OKAY(OYAYUBI_INST)
