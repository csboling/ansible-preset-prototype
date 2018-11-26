#include "encoding_tests.h"
#include "deserialize_jsmn.h"

#include "greatest/greatest.h"

TEST decimal_signed_round_trip(const char* s, size_t len, int32_t i) {
	int32_t got = decode_decimal(s, len);
	ASSERT_EQ(i, got);
	char* dec = encode_decimal_signed(i);
	ASSERT_STRN_EQ(s, dec, len);
	PASS();
}

TEST decimal_unsigned_round_trip(const char* s, size_t len, uint32_t i) {
	uint32_t got = decode_decimal(s, len);
	ASSERT_EQ(i, got);
	char* dec = encode_decimal_unsigned(i);
	ASSERT_STRN_EQ(s, dec, len);
	PASS();
}

TEST hexbuf_decode(const char* s, size_t len, const char* hex) {
	char hex_out[8];
	ASSERT_EQ(0, decode_hexbuf(hex_out, s, len));
	ASSERT_STRN_EQ(hex, hex_out, len / 2);
	PASS();
}

SUITE(encoding_suite) {
  RUN_TESTp(decimal_signed_round_trip, "123", 3, 123);
  RUN_TESTp(decimal_signed_round_trip, "2147483647", 10, 2147483647);
  RUN_TESTp(decimal_signed_round_trip, "-123", 4, -123);
  RUN_TESTp(decimal_signed_round_trip, "-2147483648", 11, -2147483647 - 1);
  RUN_TESTp(decimal_unsigned_round_trip, "4294967295", 10, 4294967295);

  RUN_TESTp(hexbuf_decode, "00112233", 8, "\x00\x11\x22\x33");
  RUN_TESTp(hexbuf_decode, "AABBCCDD", 8, "\xAA\xBB\xCC\xDD");
}
