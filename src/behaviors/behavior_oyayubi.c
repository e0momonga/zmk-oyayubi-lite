/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * NICOLA Oyayubi (Thumb Shift) Input Behavior
 * Simplified implementation based on zmk-naginata
 */

#define DT_DRV_COMPAT zmk_behavior_oyayubi

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/keymap.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// ===============================
// 1. ビットマスク定義
// ===============================
#define B_1 (1UL << 0)
#define B_2 (1UL << 1)
#define B_3 (1UL << 2)
#define B_4 (1UL << 3)
#define B_5 (1UL << 4)
#define B_6 (1UL << 5)
#define B_7 (1UL << 6)
#define B_8 (1UL << 7)
#define B_9 (1UL << 8)
#define B_0 (1UL << 9)

#define B_Q (1UL << 10)
#define B_W (1UL << 11)
#define B_E (1UL << 12)
#define B_R (1UL << 13)
#define B_T (1UL << 14)
#define B_Y (1UL << 15)
#define B_U (1UL << 16)
#define B_I (1UL << 17)
#define B_O (1UL << 18)
#define B_P (1UL << 19)

#define B_A (1UL << 20)
#define B_S (1UL << 21)
#define B_D (1UL << 22)
#define B_F (1UL << 23)
#define B_G (1UL << 24)
#define B_H (1UL << 25)
#define B_J (1UL << 26)
#define B_K (1UL << 27)
#define B_L (1UL << 28)
#define B_SEMICOLON (1UL << 29)

#define NONE 0

// キーコードからビットマスクへの変換テーブル
static uint32_t ng_key[] = {
    B_A, B_A, B_A, B_A, B_A,  // 0-4
    B_1, B_2, B_3, B_4, B_5, B_6, B_7, B_8, B_9, B_0,  // 5-14: numbers
    B_A, B_A, B_A, B_A, B_A,  // 15-19
    B_Q, B_W, B_E, B_R, B_T, B_Y, B_U, B_I, B_O, B_P,  // 20-29: QWERTY
    B_A, B_S, B_D, B_F, B_G, B_H, B_J, B_K, B_L, B_SEMICOLON,  // 30-39: ASDFG...
};

static uint32_t pressed_keys = 0UL;
static int8_t n_pressed_keys = 0;

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
// ===============================
static const naginata_kanamap ngdickana_layer7[] = {
    // 数字行
    {.shift = NONE, .douji = B_2, .kana = {HID_USAGE_KEY_KEYBOARD_G, HID_USAGE_KEY_KEYBOARD_A, NONE, NONE, NONE, NONE}},  // が
    {.shift = NONE, .douji = B_3, .kana = {HID_USAGE_KEY_KEYBOARD_D, HID_USAGE_KEY_KEYBOARD_A, NONE, NONE, NONE, NONE}},  // だ
    {.shift = NONE, .douji = B_4, .kana = {HID_USAGE_KEY_KEYBOARD_G, HID_USAGE_KEY_KEYBOARD_O, NONE, NONE, NONE, NONE}},  // ご
    {.shift = NONE, .douji = B_5, .kana = {HID_USAGE_KEY_KEYBOARD_Z, HID_USAGE_KEY_KEYBOARD_A, NONE, NONE, NONE, NONE}},  // ざ
    {.shift = NONE, .douji = B_6, .kana = {HID_USAGE_KEY_KEYBOARD_Y, HID_USAGE_KEY_KEYBOARD_O, NONE, NONE, NONE, NONE}},  // よ
    {.shift = NONE, .douji = B_7, .kana = {HID_USAGE_KEY_KEYBOARD_N, HID_USAGE_KEY_KEYBOARD_I, NONE, NONE, NONE, NONE}},  // に
    {.shift = NONE, .douji = B_8, .kana = {HID_USAGE_KEY_KEYBOARD_R, HID_USAGE_KEY_KEYBOARD_U, NONE, NONE, NONE, NONE}},  // る
    {.shift = NONE, .douji = B_9, .kana = {HID_USAGE_KEY_KEYBOARD_M, HID_USAGE_KEY_KEYBOARD_A, NONE, NONE, NONE, NONE}},  // ま
    {.shift = NONE, .douji = B_0, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_E, NONE, NONE, NONE, NONE}},  // ぇ

    // QWERTY上段
    {.shift = NONE, .douji = B_Q, .kana = {HID_USAGE_KEY_KEYBOARD_V, HID_USAGE_KEY_KEYBOARD_U, NONE, NONE, NONE, NONE}},  // ゔ
    {.shift = NONE, .douji = B_W, .kana = {HID_USAGE_KEY_KEYBOARD_Z, HID_USAGE_KEY_KEYBOARD_I, NONE, NONE, NONE, NONE}},  // じ
    {.shift = NONE, .douji = B_E, .kana = {HID_USAGE_KEY_KEYBOARD_D, HID_USAGE_KEY_KEYBOARD_E, NONE, NONE, NONE, NONE}},  // で
    {.shift = NONE, .douji = B_R, .kana = {HID_USAGE_KEY_KEYBOARD_G, HID_USAGE_KEY_KEYBOARD_E, NONE, NONE, NONE, NONE}},  // げ
    {.shift = NONE, .douji = B_T, .kana = {HID_USAGE_KEY_KEYBOARD_Z, HID_USAGE_KEY_KEYBOARD_E, NONE, NONE, NONE, NONE}},  // ぜ
    {.shift = NONE, .douji = B_Y, .kana = {HID_USAGE_KEY_KEYBOARD_M, HID_USAGE_KEY_KEYBOARD_I, NONE, NONE, NONE, NONE}},  // み
    {.shift = NONE, .douji = B_U, .kana = {HID_USAGE_KEY_KEYBOARD_O, NONE, NONE, NONE, NONE, NONE}},  // お
    {.shift = NONE, .douji = B_I, .kana = {HID_USAGE_KEY_KEYBOARD_N, HID_USAGE_KEY_KEYBOARD_O, NONE, NONE, NONE, NONE}},  // の
    {.shift = NONE, .douji = B_O, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_Y, HID_USAGE_KEY_KEYBOARD_O, NONE, NONE, NONE}},  // ょ
    {.shift = NONE, .douji = B_P, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_T, HID_USAGE_KEY_KEYBOARD_U, NONE, NONE, NONE}},  // っ

    // ASDFGHJKL;中段
    {.shift = NONE, .douji = B_S, .kana = {HID_USAGE_KEY_KEYBOARD_B, HID_USAGE_KEY_KEYBOARD_I, NONE, NONE, NONE, NONE}},  // び
    {.shift = NONE, .douji = B_D, .kana = {HID_USAGE_KEY_KEYBOARD_Z, HID_USAGE_KEY_KEYBOARD_U, NONE, NONE, NONE, NONE}},  // ず
    {.shift = NONE, .douji = B_F, .kana = {HID_USAGE_KEY_KEYBOARD_B, HID_USAGE_KEY_KEYBOARD_U, NONE, NONE, NONE, NONE}},  // ぶ
    {.shift = NONE, .douji = B_G, .kana = {HID_USAGE_KEY_KEYBOARD_B, HID_USAGE_KEY_KEYBOARD_E, NONE, NONE, NONE, NONE}},  // べ
    {.shift = NONE, .douji = B_H, .kana = {HID_USAGE_KEY_KEYBOARD_N, HID_USAGE_KEY_KEYBOARD_U, NONE, NONE, NONE, NONE}},  // ぬ
    {.shift = NONE, .douji = B_J, .kana = {HID_USAGE_KEY_KEYBOARD_Y, HID_USAGE_KEY_KEYBOARD_U, NONE, NONE, NONE, NONE}},  // ゆ
    {.shift = NONE, .douji = B_K, .kana = {HID_USAGE_KEY_KEYBOARD_M, HID_USAGE_KEY_KEYBOARD_U, NONE, NONE, NONE, NONE}},  // む
    {.shift = NONE, .douji = B_L, .kana = {HID_USAGE_KEY_KEYBOARD_W, HID_USAGE_KEY_KEYBOARD_A, NONE, NONE, NONE, NONE}},  // わ
    {.shift = NONE, .douji = B_SEMICOLON, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_O, NONE, NONE, NONE, NONE}},  // ぉ
};

// ===============================
// 4. Layer 8用配列テーブル（右親指シフト）
// ===============================
static const naginata_kanamap ngdickana_layer8[] = {
    // 数字行
    {.shift = NONE, .douji = B_1, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_A, NONE, NONE, NONE, NONE}},  // ぁ
    {.shift = NONE, .douji = B_2, .kana = {HID_USAGE_KEY_KEYBOARD_E, NONE, NONE, NONE, NONE, NONE}},  // え
    {.shift = NONE, .douji = B_3, .kana = {HID_USAGE_KEY_KEYBOARD_R, HID_USAGE_KEY_KEYBOARD_I, NONE, NONE, NONE, NONE}},  // り
    {.shift = NONE, .douji = B_4, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_Y, HID_USAGE_KEY_KEYBOARD_A, NONE, NONE, NONE}},  // ゃ
    {.shift = NONE, .douji = B_5, .kana = {HID_USAGE_KEY_KEYBOARD_R, HID_USAGE_KEY_KEYBOARD_E, NONE, NONE, NONE, NONE}},  // れ
    {.shift = NONE, .douji = B_6, .kana = {HID_USAGE_KEY_KEYBOARD_P, HID_USAGE_KEY_KEYBOARD_A, NONE, NONE, NONE, NONE}},  // ぱ
    {.shift = NONE, .douji = B_7, .kana = {HID_USAGE_KEY_KEYBOARD_D, HID_USAGE_KEY_KEYBOARD_I, NONE, NONE, NONE, NONE}},  // ぢ
    {.shift = NONE, .douji = B_8, .kana = {HID_USAGE_KEY_KEYBOARD_G, HID_USAGE_KEY_KEYBOARD_U, NONE, NONE, NONE, NONE}},  // ぐ
    {.shift = NONE, .douji = B_9, .kana = {HID_USAGE_KEY_KEYBOARD_D, HID_USAGE_KEY_KEYBOARD_U, NONE, NONE, NONE, NONE}},  // づ
    {.shift = NONE, .douji = B_0, .kana = {HID_USAGE_KEY_KEYBOARD_P, HID_USAGE_KEY_KEYBOARD_I, NONE, NONE, NONE, NONE}},  // ぴ

    // QWERTY上段
    {.shift = NONE, .douji = B_Q, .kana = {HID_USAGE_KEY_KEYBOARD_W, HID_USAGE_KEY_KEYBOARD_O, NONE, NONE, NONE, NONE}},  // を
    {.shift = NONE, .douji = B_W, .kana = {HID_USAGE_KEY_KEYBOARD_A, NONE, NONE, NONE, NONE, NONE}},  // あ
    {.shift = NONE, .douji = B_E, .kana = {HID_USAGE_KEY_KEYBOARD_N, HID_USAGE_KEY_KEYBOARD_A, NONE, NONE, NONE, NONE}},  // な
    {.shift = NONE, .douji = B_R, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_Y, HID_USAGE_KEY_KEYBOARD_U, NONE, NONE, NONE}},  // ゅ
    {.shift = NONE, .douji = B_T, .kana = {HID_USAGE_KEY_KEYBOARD_M, HID_USAGE_KEY_KEYBOARD_O, NONE, NONE, NONE, NONE}},  // も
    {.shift = NONE, .douji = B_Y, .kana = {HID_USAGE_KEY_KEYBOARD_B, HID_USAGE_KEY_KEYBOARD_A, NONE, NONE, NONE, NONE}},  // ば
    {.shift = NONE, .douji = B_U, .kana = {HID_USAGE_KEY_KEYBOARD_D, HID_USAGE_KEY_KEYBOARD_O, NONE, NONE, NONE, NONE}},  // ど
    {.shift = NONE, .douji = B_I, .kana = {HID_USAGE_KEY_KEYBOARD_G, HID_USAGE_KEY_KEYBOARD_I, NONE, NONE, NONE, NONE}},  // ぎ
    {.shift = NONE, .douji = B_O, .kana = {HID_USAGE_KEY_KEYBOARD_P, HID_USAGE_KEY_KEYBOARD_O, NONE, NONE, NONE, NONE}},  // ぽ

    // ASDFGHJKL;中段
    {.shift = NONE, .douji = B_A, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_U, NONE, NONE, NONE, NONE}},  // ぅ
    {.shift = NONE, .douji = B_D, .kana = {HID_USAGE_KEY_KEYBOARD_R, HID_USAGE_KEY_KEYBOARD_O, NONE, NONE, NONE, NONE}},  // ろ
    {.shift = NONE, .douji = B_F, .kana = {HID_USAGE_KEY_KEYBOARD_Y, HID_USAGE_KEY_KEYBOARD_A, NONE, NONE, NONE, NONE}},  // や
    {.shift = NONE, .douji = B_G, .kana = {HID_USAGE_KEY_KEYBOARD_X, HID_USAGE_KEY_KEYBOARD_I, NONE, NONE, NONE, NONE}},  // ぃ
    {.shift = NONE, .douji = B_H, .kana = {HID_USAGE_KEY_KEYBOARD_P, HID_USAGE_KEY_KEYBOARD_U, NONE, NONE, NONE, NONE}},  // ぷ
    {.shift = NONE, .douji = B_J, .kana = {HID_USAGE_KEY_KEYBOARD_Z, HID_USAGE_KEY_KEYBOARD_O, NONE, NONE, NONE, NONE}},  // ぞ
    {.shift = NONE, .douji = B_K, .kana = {HID_USAGE_KEY_KEYBOARD_P, HID_USAGE_KEY_KEYBOARD_E, NONE, NONE, NONE, NONE}},  // ぺ
    {.shift = NONE, .douji = B_L, .kana = {HID_USAGE_KEY_KEYBOARD_B, HID_USAGE_KEY_KEYBOARD_O, NONE, NONE, NONE, NONE}},  // ぼ
};

#define LAYER7_SIZE (sizeof(ngdickana_layer7) / sizeof(ngdickana_layer7[0]))
#define LAYER8_SIZE (sizeof(ngdickana_layer8) / sizeof(ngdickana_layer8[0]))

// ===============================
// 5. 関数実装
// ===============================

// Layer番号を取得
static int get_current_layer(const struct zmk_behavior_binding *binding,
                             struct zmk_behavior_binding_event event) {
    // 現在のレイヤーを取得（ZMK API使用）
    return zmk_keymap_layer_active(event.layer);
}

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

    // キーコードからビットマスクに変換
    if (keycode < sizeof(ng_key) / sizeof(ng_key[0])) {
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

    // キーコードからビットマスクに変換
    if (keycode < sizeof(ng_key) / sizeof(ng_key[0])) {
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
    return 0;
}

#define OYAYUBI_INST(n)                                                                            \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_oyayubi_init, NULL, NULL, NULL, POST_KERNEL,              \
                           CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_oyayubi_driver_api);

DT_INST_FOREACH_STATUS_OKAY(OYAYUBI_INST)
