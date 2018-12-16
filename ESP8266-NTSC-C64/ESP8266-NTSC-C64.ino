#include <Arduino.h>
const int PS2DataPin = D3;
const int PS2IRQpin  = D2;
#include "wifi_credentials.h"
#define FREQUENCY    160   
#include "ESP8266WiFi.h"
#include <ESP8266WiFiMulti.h>
#include <ArduinoOTA.h>
#include <PS2Keyboard.h>

#include "prg/hello-world.h"


extern "C" {
#include "user_interface.h"
#include "cpu.h"
}




void load_prg(const uint8_t *data, uint16_t len) {
  uint16_t ptr = 0;
  uint16_t addr = (uint16_t)pgm_read_byte(data + ptr++) + 0x100 * pgm_read_byte(data + ptr++);
  while (ptr < len) {
    write6502(addr + ptr - 2, pgm_read_byte(data + ptr));
    ++ptr;
  }
  // see mem_set_basic_text() at https://github.com/stuartcarnie/vice-emu/blob/238d23b8b7e4fe3b6077273d0c99b1d8c3fc0b8c/vice/src/autostart-prg.c
  uint16_t end = addr + ptr - 2;
  uint8_t lo = end & 0xff, hi = end >> 8;
  write6502(0x2d, lo); write6502(0x2f, lo); write6502(0x31, lo); write6502(0xae, lo);
  write6502(0x2e, hi); write6502(0x30, hi); write6502(0x32, hi); write6502(0xaf, hi);
}


//CBM Textset1 ROM
unsigned char charROM [] = { 0x1C , 0x22 , 0x4A , 0x56 , 0x4C , 0x20 , 0x1E , 0x0 , 0x18 , 0x24 , 0x42 , 0x7E , 0x42 , 0x42 , 0x42 , 0x0 , 0x7C , 0x22 , 0x22 , 0x3C , 0x22 , 0x22 , 0x7C , 0x0 , 0x1C , 0x22 , 0x40 , 0x40 , 0x40 , 0x22 , 0x1C , 0x0 , 0x78 , 0x24 , 0x22 , 0x22 , 0x22 , 0x24 , 0x78 , 0x0 , 0x7E , 0x40 , 0x40 , 0x78 , 0x40 , 0x40 , 0x7E , 0x0 , 0x7E , 0x40 , 0x40 , 0x78 , 0x40 , 0x40 , 0x40 , 0x0 , 0x1C , 0x22 , 0x40 , 0x4E , 0x42 , 0x22 , 0x1C , 0x0 , 0x42 , 0x42 , 0x42 , 0x7E , 0x42 , 0x42 , 0x42 , 0x0 , 0x1C , 0x8 , 0x8 , 0x8 , 0x8 , 0x8 , 0x1C , 0x0 , 0xE , 0x4 , 0x4 , 0x4 , 0x4 , 0x44 , 0x38 , 0x0 , 0x42 , 0x44 , 0x48 , 0x70 , 0x48 , 0x44 , 0x42 , 0x0 , 0x40 , 0x40 , 0x40 , 0x40 , 0x40 , 0x40 , 0x7E , 0x0 , 0x42 , 0x66 , 0x5A , 0x5A , 0x42 , 0x42 , 0x42 , 0x0 , 0x42 , 0x62 , 0x52 , 0x4A , 0x46 , 0x42 , 0x42 , 0x0 , 0x18 , 0x24 , 0x42 , 0x42 , 0x42 , 0x24 , 0x18 , 0x0 , 0x7C , 0x42 , 0x42 , 0x7C , 0x40 , 0x40 , 0x40 , 0x0 , 0x18 , 0x24 , 0x42 , 0x42 , 0x4A , 0x24 , 0x1A , 0x0 , 0x7C , 0x42 , 0x42 , 0x7C , 0x48 , 0x44 , 0x42 , 0x0 , 0x3C , 0x42 , 0x40 , 0x3C , 0x2 , 0x42 , 0x3C , 0x0 , 0x3E , 0x8 , 0x8 , 0x8 , 0x8 , 0x8 , 0x8 , 0x0 , 0x42 , 0x42 , 0x42 , 0x42 , 0x42 , 0x42 , 0x3C , 0x0 , 0x42 , 0x42 , 0x42 , 0x24 , 0x24 , 0x18 , 0x18 , 0x0 , 0x42 , 0x42 , 0x42 , 0x5A , 0x5A , 0x66 , 0x42 , 0x0 , 0x42 , 0x42 , 0x24 , 0x18 , 0x24 , 0x42 , 0x42 , 0x0 , 0x22 , 0x22 , 0x22 , 0x1C , 0x8 , 0x8 , 0x8 , 0x0 , 0x7E , 0x2 , 0x4 , 0x18 , 0x20 , 0x40 , 0x7E , 0x0 , 0x42 , 0x18 , 0x24 , 0x42 , 0x7E , 0x42 , 0x42 , 0x0 , 0x42 , 0x18 , 0x24 , 0x42 , 0x42 , 0x24 , 0x18 , 0x0 , 0x18 , 0x24 , 0x3C , 0x42 , 0x7E , 0x42 , 0x42 , 0x0 , 0x0 , 0x8 , 0x1C , 0x2A , 0x8 , 0x8 , 0x8 , 0x8 , 0x0 , 0x0 , 0x10 , 0x20 , 0x7F , 0x20 , 0x10 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x8 , 0x8 , 0x8 , 0x8 , 0x0 , 0x0 , 0x8 , 0x0 , 0x24 , 0x24 , 0x24 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x24 , 0x24 , 0x7E , 0x24 , 0x7E , 0x24 , 0x24 , 0x0 , 0x8 , 0x1E , 0x28 , 0x1C , 0xA , 0x3C , 0x8 , 0x0 , 0x0 , 0x62 , 0x64 , 0x8 , 0x10 , 0x26 , 0x46 , 0x0 , 0x30 , 0x48 , 0x48 , 0x30 , 0x4A , 0x44 , 0x3A , 0x0 , 0x4 , 0x8 , 0x10 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x4 , 0x8 , 0x10 , 0x10 , 0x10 , 0x8 , 0x4 , 0x0 , 0x20 , 0x10 , 0x8 , 0x8 , 0x8 , 0x10 , 0x20 , 0x0 , 0x8 , 0x2A , 0x1C , 0x3E , 0x1C , 0x2A , 0x8 , 0x0 , 0x0 , 0x8 , 0x8 , 0x3E , 0x8 , 0x8 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x8 , 0x8 , 0x10 , 0x0 , 0x0 , 0x0 , 0x7E , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x18 , 0x18 , 0x0 , 0x0 , 0x2 , 0x4 , 0x8 , 0x10 , 0x20 , 0x40 , 0x0 , 0x3C , 0x42 , 0x46 , 0x5A , 0x62 , 0x42 , 0x3C , 0x0 , 0x8 , 0x18 , 0x28 , 0x8 , 0x8 , 0x8 , 0x3E , 0x0 , 0x3C , 0x42 , 0x2 , 0xC , 0x30 , 0x40 , 0x7E , 0x0 , 0x3C , 0x42 , 0x2 , 0x1C , 0x2 , 0x42 , 0x3C , 0x0 , 0x4 , 0xC , 0x14 , 0x24 , 0x7E , 0x4 , 0x4 , 0x0 , 0x7E , 0x40 , 0x78 , 0x4 , 0x2 , 0x44 , 0x38 , 0x0 , 0x1C , 0x20 , 0x40 , 0x7C , 0x42 , 0x42 , 0x3C , 0x0 , 0x7E , 0x42 , 0x4 , 0x8 , 0x10 , 0x10 , 0x10 , 0x0 , 0x3C , 0x42 , 0x42 , 0x3C , 0x42 , 0x42 , 0x3C , 0x0 , 0x3C , 0x42 , 0x42 , 0x3E , 0x2 , 0x4 , 0x38 , 0x0 , 0x0 , 0x0 , 0x8 , 0x0 , 0x0 , 0x8 , 0x0 , 0x0 , 0x0 , 0x0 , 0x8 , 0x0 , 0x0 , 0x8 , 0x8 , 0x10 , 0xE , 0x18 , 0x30 , 0x60 , 0x30 , 0x18 , 0xE , 0x0 , 0x0 , 0x0 , 0x7E , 0x0 , 0x7E , 0x0 , 0x0 , 0x0 , 0x70 , 0x18 , 0xC , 0x6 , 0xC , 0x18 , 0x70 , 0x0 , 0x3C , 0x42 , 0x2 , 0xC , 0x10 , 0x0 , 0x10 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xFF , 0x0 , 0x0 , 0x0 , 0x8 , 0x1C , 0x3E , 0x7F , 0x7F , 0x1C , 0x3E , 0x0 , 0x10 , 0x10 , 0x10 , 0x10 , 0x10 , 0x10 , 0x10 , 0x10 , 0x0 , 0x0 , 0x0 , 0xFF , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xFF , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xFF , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xFF , 0x0 , 0x0 , 0x20 , 0x20 , 0x20 , 0x20 , 0x20 , 0x20 , 0x20 , 0x20 , 0x4 , 0x4 , 0x4 , 0x4 , 0x4 , 0x4 , 0x4 , 0x4 , 0x0 , 0x0 , 0x0 , 0x0 , 0xE0 , 0x10 , 0x8 , 0x8 , 0x8 , 0x8 , 0x8 , 0x4 , 0x3 , 0x0 , 0x0 , 0x0 , 0x8 , 0x8 , 0x8 , 0x10 , 0xE0 , 0x0 , 0x0 , 0x0 , 0x80 , 0x80 , 0x80 , 0x80 , 0x80 , 0x80 , 0x80 , 0xFF , 0x80 , 0x40 , 0x20 , 0x10 , 0x8 , 0x4 , 0x2 , 0x1 , 0x1 , 0x2 , 0x4 , 0x8 , 0x10 , 0x20 , 0x40 , 0x80 , 0xFF , 0x80 , 0x80 , 0x80 , 0x80 , 0x80 , 0x80 , 0x80 , 0xFF , 0x1 , 0x1 , 0x1 , 0x1 , 0x1 , 0x1 , 0x1 , 0x0 , 0x3C , 0x7E , 0x7E , 0x7E , 0x7E , 0x3C , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xFF , 0x0 , 0x36 , 0x7F , 0x7F , 0x7F , 0x3E , 0x1C , 0x8 , 0x0 , 0x40 , 0x40 , 0x40 , 0x40 , 0x40 , 0x40 , 0x40 , 0x40 , 0x0 , 0x0 , 0x0 , 0x0 , 0x3 , 0x4 , 0x8 , 0x8 , 0x81 , 0x42 , 0x24 , 0x18 , 0x18 , 0x24 , 0x42 , 0x81 , 0x0 , 0x3C , 0x42 , 0x42 , 0x42 , 0x42 , 0x3C , 0x0 , 0x8 , 0x1C , 0x2A , 0x77 , 0x2A , 0x8 , 0x8 , 0x0 , 0x2 , 0x2 , 0x2 , 0x2 , 0x2 , 0x2 , 0x2 , 0x2 , 0x8 , 0x1C , 0x3E , 0x7F , 0x3E , 0x1C , 0x8 , 0x0 , 0x8 , 0x8 , 0x8 , 0x8 , 0xFF , 0x8 , 0x8 , 0x8 , 0xA0 , 0x50 , 0xA0 , 0x50 , 0xA0 , 0x50 , 0xA0 , 0x50 , 0x8 , 0x8 , 0x8 , 0x8 , 0x8 , 0x8 , 0x8 , 0x8 , 0x0 , 0x0 , 0x1 , 0x3E , 0x54 , 0x14 , 0x14 , 0x0 , 0xFF , 0x7F , 0x3F , 0x1F , 0xF , 0x7 , 0x3 , 0x1 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xF0 , 0xF0 , 0xF0 , 0xF0 , 0xF0 , 0xF0 , 0xF0 , 0xF0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xFF , 0x80 , 0x80 , 0x80 , 0x80 , 0x80 , 0x80 , 0x80 , 0x80 , 0xAA , 0x55 , 0xAA , 0x55 , 0xAA , 0x55 , 0xAA , 0x55 , 0x1 , 0x1 , 0x1 , 0x1 , 0x1 , 0x1 , 0x1 , 0x1 , 0x0 , 0x0 , 0x0 , 0x0 , 0xAA , 0x55 , 0xAA , 0x55 , 0xFF , 0xFE , 0xFC , 0xF8 , 0xF0 , 0xE0 , 0xC0 , 0x80 , 0x3 , 0x3 , 0x3 , 0x3 , 0x3 , 0x3 , 0x3 , 0x3 , 0x8 , 0x8 , 0x8 , 0x8 , 0xF , 0x8 , 0x8 , 0x8 , 0x0 , 0x0 , 0x0 , 0x0 , 0xF , 0xF , 0xF , 0xF , 0x8 , 0x8 , 0x8 , 0x8 , 0xF , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xF8 , 0x8 , 0x8 , 0x8 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xFF , 0xFF , 0x0 , 0x0 , 0x0 , 0x0 , 0xF , 0x8 , 0x8 , 0x8 , 0x8 , 0x8 , 0x8 , 0x8 , 0xFF , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xFF , 0x8 , 0x8 , 0x8 , 0x8 , 0x8 , 0x8 , 0x8 , 0xF8 , 0x8 , 0x8 , 0x8 , 0xC0 , 0xC0 , 0xC0 , 0xC0 , 0xC0 , 0xC0 , 0xC0 , 0xC0 , 0xE0 , 0xE0 , 0xE0 , 0xE0 , 0xE0 , 0xE0 , 0xE0 , 0xE0 , 0x7 , 0x7 , 0x7 , 0x7 , 0x7 , 0x7 , 0x7 , 0x7 , 0xFF , 0xFF , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xFF , 0xFF , 0xFF , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xFF , 0xFF , 0xFF , 0x1 , 0x1 , 0x1 , 0x1 , 0x1 , 0x1 , 0x1 , 0xFF , 0x0 , 0x0 , 0x0 , 0x0 , 0xF0 , 0xF0 , 0xF0 , 0xF0 , 0xF , 0xF , 0xF , 0xF , 0x0 , 0x0 , 0x0 , 0x0 , 0x8 , 0x8 , 0x8 , 0x8 , 0xF8 , 0x0 , 0x0 , 0x0 , 0xF0 , 0xF0 , 0xF0 , 0xF0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xF0 , 0xF0 , 0xF0 , 0xF0 , 0xF , 0xF , 0xF , 0xF , 0xE3 , 0xDD , 0xB5 , 0xA9 , 0xB3 , 0xDF , 0xE1 , 0xFF , 0xE7 , 0xDB , 0xBD , 0x81 , 0xBD , 0xBD , 0xBD , 0xFF , 0x83 , 0xDD , 0xDD , 0xC3 , 0xDD , 0xDD , 0x83 , 0xFF , 0xE3 , 0xDD , 0xBF , 0xBF , 0xBF , 0xDD , 0xE3 , 0xFF , 0x87 , 0xDB , 0xDD , 0xDD , 0xDD , 0xDB , 0x87 , 0xFF , 0x81 , 0xBF , 0xBF , 0x87 , 0xBF , 0xBF , 0x81 , 0xFF , 0x81 , 0xBF , 0xBF , 0x87 , 0xBF , 0xBF , 0xBF , 0xFF , 0xE3 , 0xDD , 0xBF , 0xB1 , 0xBD , 0xDD , 0xE3 , 0xFF , 0xBD , 0xBD , 0xBD , 0x81 , 0xBD , 0xBD , 0xBD , 0xFF , 0xE3 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xE3 , 0xFF , 0xF1 , 0xFB , 0xFB , 0xFB , 0xFB , 0xBB , 0xC7 , 0xFF , 0xBD , 0xBB , 0xB7 , 0x8F , 0xB7 , 0xBB , 0xBD , 0xFF , 0xBF , 0xBF , 0xBF , 0xBF , 0xBF , 0xBF , 0x81 , 0xFF , 0xBD , 0x99 , 0xA5 , 0xA5 , 0xBD , 0xBD , 0xBD , 0xFF , 0xBD , 0x9D , 0xAD , 0xB5 , 0xB9 , 0xBD , 0xBD , 0xFF , 0xE7 , 0xDB , 0xBD , 0xBD , 0xBD , 0xDB , 0xE7 , 0xFF , 0x83 , 0xBD , 0xBD , 0x83 , 0xBF , 0xBF , 0xBF , 0xFF , 0xE7 , 0xDB , 0xBD , 0xBD , 0xB5 , 0xDB , 0xE5 , 0xFF , 0x83 , 0xBD , 0xBD , 0x83 , 0xB7 , 0xBB , 0xBD , 0xFF , 0xC3 , 0xBD , 0xBF , 0xC3 , 0xFD , 0xBD , 0xC3 , 0xFF , 0xC1 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xFF , 0xBD , 0xBD , 0xBD , 0xBD , 0xBD , 0xBD , 0xC3 , 0xFF , 0xBD , 0xBD , 0xBD , 0xDB , 0xDB , 0xE7 , 0xE7 , 0xFF , 0xBD , 0xBD , 0xBD , 0xA5 , 0xA5 , 0x99 , 0xBD , 0xFF , 0xBD , 0xBD , 0xDB , 0xE7 , 0xDB , 0xBD , 0xBD , 0xFF , 0xDD , 0xDD , 0xDD , 0xE3 , 0xF7 , 0xF7 , 0xF7 , 0xFF , 0x81 , 0xFD , 0xFB , 0xE7 , 0xDF , 0xBF , 0x81 , 0xFF , 0xBD , 0xE7 , 0xDB , 0xBD , 0x81 , 0xBD , 0xBD , 0xFF , 0xBD , 0xE7 , 0xDB , 0xBD , 0xBD , 0xDB , 0xE7 , 0xFF , 0xE7 , 0xDB , 0xC3 , 0xBD , 0x81 , 0xBD , 0xBD , 0xFF , 0xFF , 0xF7 , 0xE3 , 0xD5 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xFF , 0xFF , 0xEF , 0xDF , 0x80 , 0xDF , 0xEF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xFF , 0xFF , 0xF7 , 0xFF , 0xDB , 0xDB , 0xDB , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xDB , 0xDB , 0x81 , 0xDB , 0x81 , 0xDB , 0xDB , 0xFF , 0xF7 , 0xE1 , 0xD7 , 0xE3 , 0xF5 , 0xC3 , 0xF7 , 0xFF , 0xFF , 0x9D , 0x9B , 0xF7 , 0xEF , 0xD9 , 0xB9 , 0xFF , 0xCF , 0xB7 , 0xB7 , 0xCF , 0xB5 , 0xBB , 0xC5 , 0xFF , 0xFB , 0xF7 , 0xEF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFB , 0xF7 , 0xEF , 0xEF , 0xEF , 0xF7 , 0xFB , 0xFF , 0xDF , 0xEF , 0xF7 , 0xF7 , 0xF7 , 0xEF , 0xDF , 0xFF , 0xF7 , 0xD5 , 0xE3 , 0xC1 , 0xE3 , 0xD5 , 0xF7 , 0xFF , 0xFF , 0xF7 , 0xF7 , 0xC1 , 0xF7 , 0xF7 , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xF7 , 0xF7 , 0xEF , 0xFF , 0xFF , 0xFF , 0x81 , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xE7 , 0xE7 , 0xFF , 0xFF , 0xFD , 0xFB , 0xF7 , 0xEF , 0xDF , 0xBF , 0xFF , 0xC3 , 0xBD , 0xB9 , 0xA5 , 0x9D , 0xBD , 0xC3 , 0xFF , 0xF7 , 0xE7 , 0xD7 , 0xF7 , 0xF7 , 0xF7 , 0xC1 , 0xFF , 0xC3 , 0xBD , 0xFD , 0xF3 , 0xCF , 0xBF , 0x81 , 0xFF , 0xC3 , 0xBD , 0xFD , 0xE3 , 0xFD , 0xBD , 0xC3 , 0xFF , 0xFB , 0xF3 , 0xEB , 0xDB , 0x81 , 0xFB , 0xFB , 0xFF , 0x81 , 0xBF , 0x87 , 0xFB , 0xFD , 0xBB , 0xC7 , 0xFF , 0xE3 , 0xDF , 0xBF , 0x83 , 0xBD , 0xBD , 0xC3 , 0xFF , 0x81 , 0xBD , 0xFB , 0xF7 , 0xEF , 0xEF , 0xEF , 0xFF , 0xC3 , 0xBD , 0xBD , 0xC3 , 0xBD , 0xBD , 0xC3 , 0xFF , 0xC3 , 0xBD , 0xBD , 0xC1 , 0xFD , 0xFB , 0xC7 , 0xFF , 0xFF , 0xFF , 0xF7 , 0xFF , 0xFF , 0xF7 , 0xFF , 0xFF , 0xFF , 0xFF , 0xF7 , 0xFF , 0xFF , 0xF7 , 0xF7 , 0xEF , 0xF1 , 0xE7 , 0xCF , 0x9F , 0xCF , 0xE7 , 0xF1 , 0xFF , 0xFF , 0xFF , 0x81 , 0xFF , 0x81 , 0xFF , 0xFF , 0xFF , 0x8F , 0xE7 , 0xF3 , 0xF9 , 0xF3 , 0xE7 , 0x8F , 0xFF , 0xC3 , 0xBD , 0xFD , 0xF3 , 0xEF , 0xFF , 0xEF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0x0 , 0xFF , 0xFF , 0xFF , 0xF7 , 0xE3 , 0xC1 , 0x80 , 0x80 , 0xE3 , 0xC1 , 0xFF , 0xEF , 0xEF , 0xEF , 0xEF , 0xEF , 0xEF , 0xEF , 0xEF , 0xFF , 0xFF , 0xFF , 0x0 , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0x0 , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0x0 , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0x0 , 0xFF , 0xFF , 0xDF , 0xDF , 0xDF , 0xDF , 0xDF , 0xDF , 0xDF , 0xDF , 0xFB , 0xFB , 0xFB , 0xFB , 0xFB , 0xFB , 0xFB , 0xFB , 0xFF , 0xFF , 0xFF , 0xFF , 0x1F , 0xEF , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xFB , 0xFC , 0xFF , 0xFF , 0xFF , 0xF7 , 0xF7 , 0xF7 , 0xEF , 0x1F , 0xFF , 0xFF , 0xFF , 0x7F , 0x7F , 0x7F , 0x7F , 0x7F , 0x7F , 0x7F , 0x0 , 0x7F , 0xBF , 0xDF , 0xEF , 0xF7 , 0xFB , 0xFD , 0xFE , 0xFE , 0xFD , 0xFB , 0xF7 , 0xEF , 0xDF , 0xBF , 0x7F , 0x0 , 0x7F , 0x7F , 0x7F , 0x7F , 0x7F , 0x7F , 0x7F , 0x0 , 0xFE , 0xFE , 0xFE , 0xFE , 0xFE , 0xFE , 0xFE , 0xFF , 0xC3 , 0x81 , 0x81 , 0x81 , 0x81 , 0xC3 , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0x0 , 0xFF , 0xC9 , 0x80 , 0x80 , 0x80 , 0xC1 , 0xE3 , 0xF7 , 0xFF , 0xBF , 0xBF , 0xBF , 0xBF , 0xBF , 0xBF , 0xBF , 0xBF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFC , 0xFB , 0xF7 , 0xF7 , 0x7E , 0xBD , 0xDB , 0xE7 , 0xE7 , 0xDB , 0xBD , 0x7E , 0xFF , 0xC3 , 0xBD , 0xBD , 0xBD , 0xBD , 0xC3 , 0xFF , 0xF7 , 0xE3 , 0xD5 , 0x88 , 0xD5 , 0xF7 , 0xF7 , 0xFF , 0xFD , 0xFD , 0xFD , 0xFD , 0xFD , 0xFD , 0xFD , 0xFD , 0xF7 , 0xE3 , 0xC1 , 0x80 , 0xC1 , 0xE3 , 0xF7 , 0xFF , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0x0 , 0xF7 , 0xF7 , 0xF7 , 0x5F , 0xAF , 0x5F , 0xAF , 0x5F , 0xAF , 0x5F , 0xAF , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xFF , 0xFF , 0xFE , 0xC1 , 0xAB , 0xEB , 0xEB , 0xFF , 0x0 , 0x80 , 0xC0 , 0xE0 , 0xF0 , 0xF8 , 0xFC , 0xFE , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xF , 0xF , 0xF , 0xF , 0xF , 0xF , 0xF , 0xF , 0xFF , 0xFF , 0xFF , 0xFF , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0x0 , 0x7F , 0x7F , 0x7F , 0x7F , 0x7F , 0x7F , 0x7F , 0x7F , 0x55 , 0xAA , 0x55 , 0xAA , 0x55 , 0xAA , 0x55 , 0xAA , 0xFE , 0xFE , 0xFE , 0xFE , 0xFE , 0xFE , 0xFE , 0xFE , 0xFF , 0xFF , 0xFF , 0xFF , 0x55 , 0xAA , 0x55 , 0xAA , 0x0 , 0x1 , 0x3 , 0x7 , 0xF , 0x1F , 0x3F , 0x7F , 0xFC , 0xFC , 0xFC , 0xFC , 0xFC , 0xFC , 0xFC , 0xFC , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF0 , 0xF7 , 0xF7 , 0xF7 , 0xFF , 0xFF , 0xFF , 0xFF , 0xF0 , 0xF0 , 0xF0 , 0xF0 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF0 , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0x7 , 0xF7 , 0xF7 , 0xF7 , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0x0 , 0x0 , 0xFF , 0xFF , 0xFF , 0xFF , 0xF0 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0x0 , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0x0 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0x7 , 0xF7 , 0xF7 , 0xF7 , 0x3F , 0x3F , 0x3F , 0x3F , 0x3F , 0x3F , 0x3F , 0x3F , 0x1F , 0x1F , 0x1F , 0x1F , 0x1F , 0x1F , 0x1F , 0x1F , 0xF8 , 0xF8 , 0xF8 , 0xF8 , 0xF8 , 0xF8 , 0xF8 , 0xF8 , 0x0 , 0x0 , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0x0 , 0x0 , 0x0 , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0x0 , 0x0 , 0x0 , 0xFE , 0xFE , 0xFE , 0xFE , 0xFE , 0xFE , 0xFE , 0x0 , 0xFF , 0xFF , 0xFF , 0xFF , 0xF , 0xF , 0xF , 0xF , 0xF0 , 0xF0 , 0xF0 , 0xF0 , 0xFF , 0xFF , 0xFF , 0xFF , 0xF7 , 0xF7 , 0xF7 , 0xF7 , 0x7 , 0xFF , 0xFF , 0xFF , 0xF , 0xF , 0xF , 0xF , 0xFF , 0xFF , 0xFF , 0xFF , 0xF , 0xF , 0xF , 0xF , 0xF0 , 0xF0 , 0xF0 , 0xF0 };

uint8_t screenmem[1000];
uint8_t colormem[1000];
uint8_t RAM[16384];
uint8_t BOARDER;
uint8_t BGCOLOR;
uint8_t VIC_D020;
uint8_t VIC_D021;


extern "C" {
  void videoinit();
  void videostop();
  }


PS2Keyboard keyboard;

// see http://sta.c64.org/cbm64petkey.html and https://www.c64-wiki.com/wiki/Keyboard
#define C64_STOP           3
#define C64_RETURN        13
#define C64_DOWN          17
#define C64_HOME          19
#define C64_DELETE        20
#define C64_RIGHT         29
#define C64_RUN          131
#define C64_F1           133
#define C64_F3           134
#define C64_F5           135
#define C64_F7           136
#define C64_F2           137
#define C64_F4           138
#define C64_F6           139
#define C64_F8           140
#define C64_SHIFT_RETURN 141
#define C64_UP           145
#define C64_CLEAR        147
#define C64_INSERT       148
#define C64_LEFT         157

#define HOTKEY_F9        248
#define HOTKEY_F10       249
#define HOTKEY_F11       250
#define HOTKEY_F12       251
#define HOTKEY_SCROLL    252
#define HOTKEY_ESC       253
#define HOTKEY_SHIFT_ESC 254
#define HOTKEY_NUMLOCK   255

const PROGMEM PS2Keymap_t PS2Keymap_German_c64 = {
  // without shift
  {0, HOTKEY_F9, 0, C64_F5, C64_F3, C64_F1, C64_F2, HOTKEY_F12,
  0, HOTKEY_F10, C64_F8, C64_F6, C64_F4, C64_RUN /*PS2_TAB*/, '^', 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'Q', '1', 0,
  0, 0, 'Y', 'S', 'A', 'W', '2', 0,
  0, 'C', 'X', 'D', 'E', '4', '3', 0,
  0, ' ', 'V', 'F', 'T', 'R', '5', 0,
  0, 'N', 'B', 'H', 'G', 'Z', '6', 0,
  0, 0, 'M', 'J', 'U', '7', '8', 0,
  0, ',', 'K', 'I', 'O', '0', '9', 0,
  0, '.', '-', 'L', PS2_o_DIAERESIS, 'P', PS2_SHARP_S, 0,
  0, 0, PS2_a_DIAERESIS, 0, PS2_u_DIAERESIS, '\'', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, C64_RETURN /*PS2_ENTER*/ /*Enter*/, '+', 0, '#', 0, 0,
  0, '<', 0, 0, 0, 0, C64_DELETE /*PS2_BACKSPACE*/, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', HOTKEY_ESC, HOTKEY_NUMLOCK /*NumLock*/,
  HOTKEY_F11, '+', '3', '-', '*', '9', HOTKEY_SCROLL, 0,
  0, 0, 0, C64_F7 },
  // with shift
  {0, HOTKEY_F9, 0, C64_F5, C64_F3, C64_F1, C64_F2, HOTKEY_F12,
  0, HOTKEY_F10, C64_F8, C64_F6, C64_F4, C64_STOP /*PS2_TAB*/, PS2_DEGREE_SIGN, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'q', '!', 0,
  0, 0, 'y', 's', 'a', 'w', '"', 0,
  0, 'c', 'x', 'd', 'e', '$', PS2_SECTION_SIGN, 0,
  0, ' ', 'v', 'f', 't', 'r', '%', 0,
  0, 'n', 'b', 'h', 'g', 'z', '&', 0,
  0, 0, 'm', 'j', 'u', '/', '(', 0,
  0, ';', 'k', 'i', 'o', '=', ')', 0,
  0, ':', '_', 'l', PS2_O_DIAERESIS, 'p', '?', 0,
  0, 0, PS2_A_DIAERESIS, 0, PS2_U_DIAERESIS, '`', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, C64_SHIFT_RETURN /*PS2_ENTER*/ /*Enter*/, '*', 0, '\'', 0, 0,
  0, '>', 0, 0, 0, 0, C64_DELETE /*PS2_BACKSPACE*/, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', HOTKEY_SHIFT_ESC, HOTKEY_NUMLOCK /*NumLock*/,
  HOTKEY_F11, '+', '3', '-', '*', '9', HOTKEY_SCROLL, 0,
  0, 0, 0, C64_F7 },
  1,
  // with altgr
  {0, HOTKEY_F9, 0, C64_F5, C64_F3, C64_F1, C64_F2, HOTKEY_F12,
  0, HOTKEY_F10, C64_F8, C64_F6, C64_F4, C64_RUN /*PS2_TAB*/, 0, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, '@', 0, 0,
  0, 0, 0, 0, 0, 0, PS2_SUPERSCRIPT_TWO, 0,
  0, 0, 0, 0, PS2_CURRENCY_SIGN, 0, PS2_SUPERSCRIPT_THREE, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, PS2_MICRO_SIGN, 0, 0, '{', '[', 0,
  0, 0, 0, 0, 0, '}', ']', 0,
  0, 0, 0, 0, 0, 0, '\\', 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, C64_RETURN /*PS2_ENTER*/ /*Enter*/, '~', 0, '#', 0, 0,
  0, '|', 0, 0, 0, 0, C64_DELETE /*PS2_BACKSPACE*/, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', HOTKEY_ESC, HOTKEY_NUMLOCK /*NumLock*/,
  HOTKEY_F11, '+', '3', '-', '*', '9', HOTKEY_SCROLL, 0,
  0, 0, 0, C64_F7 }
};


bool debug=false;
bool wifi_connected=false;

ESP8266WiFiMulti WiFiMulti;

void setup() {
  system_update_cpu_freq(FREQUENCY);
  keyboard.begin(PS2DataPin, PS2IRQpin, PS2Keymap_German_c64);
  delay(500);
  debug = keyboard.available() && keyboard.readUnicode() == HOTKEY_ESC;
  Serial.begin(115200);
  Serial.println("\r\n\r\nDebug mode, ESP ID: " + String(ESP.getChipId(), HEX));
  if (debug) {
#if defined(WIFI_SSID) && defined(WIFI_PSK)
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(WIFI_SSID, WIFI_PSK);
    WiFi.hostname("ESP8266-" + String(ESP.getChipId(), HEX));
    ArduinoOTA.begin();
#endif
  } else {
    WiFi.forceSleepBegin();             
    delay(1);
    videoinit();
    reset6502();
  }
}

void loop() {  
  if (!debug) {
    exec6502(100);
  }
  if (keyboard.available()) {
    char c = keyboard.readUnicode();
    if (debug) {
      Serial.println("Key: " + String(c, DEC));
    }

    if (c == PS2_UPARROW) {
      c = C64_UP;
    } else if (c == PS2_DOWNARROW) {
      c = C64_DOWN;
    } else if (c == PS2_LEFTARROW) {
      c = C64_LEFT;
    } else if (c == PS2_RIGHTARROW) {
      c = C64_RIGHT;
    }
    
    if (c == HOTKEY_SCROLL) {
      videostop();
      ESP.restart();
    } else if (c == HOTKEY_ESC) {
      nmi6502();
    } else if (c == HOTKEY_SHIFT_ESC) {
      reset6502();
    } else if (c == HOTKEY_F9) {
        load_prg(hello_world_prg,hello_world_prg_len);
    } else if (c == HOTKEY_F10) {
    } else if (c == HOTKEY_F11) {
    } else if (c == HOTKEY_F12) {
    } else {
      write6502(addr_keybuf_len,1);
      write6502(addr_keybuf,c);
    }
  }
#if defined(WIFI_SSID) && defined(WIFI_PSK)
  if (debug) {
    if (!wifi_connected && WiFiMulti.run() == WL_CONNECTED) {
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      wifi_connected = true;
    }
    ArduinoOTA.handle();
    yield();
  }
#endif
}
