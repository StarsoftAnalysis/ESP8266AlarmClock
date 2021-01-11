//   A 
// F   B
//   G
// E   C
//   D   DP

const uint8_t LETTER_A = SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G;
const uint8_t LETTER_b = SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
const uint8_t LETTER_C = SEG_A | SEG_D | SEG_E | SEG_F;
const uint8_t LETTER_d = SEG_B | SEG_C | SEG_D | SEG_E | SEG_G;
const uint8_t LETTER_E = SEG_A | SEG_D | SEG_E | SEG_F | SEG_G;
const uint8_t LETTER_F = SEG_A | SEG_E | SEG_F | SEG_G;

const uint8_t LETTER_o = SEG_C | SEG_D | SEG_E | SEG_G;
const uint8_t LETTER_I = SEG_E | SEG_F;
const uint8_t LETTER_L = SEG_D | SEG_E | SEG_F;
const uint8_t LETTER_n = SEG_C | SEG_E | SEG_G;
const uint8_t LETTER_r = SEG_E | SEG_G;
const uint8_t LETTER_t = SEG_D | SEG_E | SEG_F | SEG_G;
const uint8_t LETTER_W = SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;

const uint8_t SEG_CONF[] = {
    LETTER_C,
    LETTER_o,
    LETTER_n,
    LETTER_F
};

const uint8_t SEG_BOOT[] = {
    LETTER_b,
    LETTER_o,
    LETTER_o,
    LETTER_t
};

const uint8_t SEG_ERR[] = {
  LETTER_E,
  LETTER_r,
  LETTER_r,
  0
  };

const uint8_t SEG_FILE[] = {
  LETTER_F,
  LETTER_I,
  LETTER_L,
  LETTER_E
};

const uint8_t SEG_WIFI[] = {
  LETTER_W,
  LETTER_I,
  LETTER_F,
  LETTER_I 
};
