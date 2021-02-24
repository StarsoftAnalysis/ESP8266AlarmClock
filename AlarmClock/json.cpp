// Simple/naive routines to encode as JSON

#include "json.h"

namespace json {

	// Homemade JSON helpers
	//
	// FIXME see https://github.com/me-no-dev/ESPAsyncWebServer/issues/487 etc.

	static String jcomma (bool comma) {
		return (comma ? ", " : "");
	}

	String jstring (String name, String value, bool comma) {
		return "\"" + name + "\":\"" + value + "\"" + jcomma(comma);
	}

	String jint (String name, int value, bool comma) {
		return "\"" + name + "\":" + String(value) + jcomma(comma);
	}

	String jinta (String name, int values[], size_t len, bool comma) {
		String result = "\"" + name + "\": [";
		for (size_t i = 0; i < len; i++) {
			result += String(values[i]) + jcomma(i < len-1);
		}
		return result + "]" + jcomma(comma);
	}

	String jlong (String name, long value, bool comma) {
		return "\"" + name + "\":" + String(value) + jcomma(comma);
	}

	String jlonga (String name, long values[], size_t len, bool comma) {
		String result = "\"" + name + "\": [";
		for (size_t i = 0; i < len; i++) {
			result += String(values[i]) + jcomma(i < len-1);
		}
		return result + "]" + jcomma(comma);
	}

	String jfloat (String name, float value, int dp, bool comma) {
		return "\"" + name + "\":" + String(value, dp) + jcomma(comma);
	}

	String jfloata (String name, float values[], size_t len, int dp, bool comma) {
		String result = "\"" + name + "\": [";
		for (size_t i = 0; i < len; i++) {
			result += String(values[i], dp) + jcomma(i < len-1);
		}
		return result + "]" + jcomma(comma);
	}

	String jbool (String name, bool value, bool comma) {
		return "\"" + name + "\":" + (value ? "true" : "false") + jcomma(comma);
	}

	String jboola (String name, bool values[], size_t len, bool comma) {
		String result = "\"" + name + "\": [";
		for (size_t i = 0; i < len; i++) {
			result += (values[i] ? "true" : "false") + jcomma(i < len-1);
		}
		return result + "]" + jcomma(comma);
	}

}

// vim: set ts=4 sw=4 tw=0 noet : 
