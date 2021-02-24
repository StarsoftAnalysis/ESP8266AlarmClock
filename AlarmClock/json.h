// json.h

// Useful JSON routines

#ifndef HEADER_JSON
#define HEADER_JSON

#include <Arduino.h>    // needed for String

namespace json {

	// len = size of array
	// dp = decimal places for floats
	// comma = true to add trailing ','
	String jstring (String name, String value,                            bool comma = true);
	String jint    (String name, int    value,                            bool comma = true);
	String jinta   (String name, int    values[], size_t len,             bool comma = true);
	String jlong   (String name, long   value,                            bool comma = true);
	String jlonga  (String name, long   values[], size_t len,             bool comma = true);
	String jfloat  (String name, float  value,                int dp = 2, bool comma = true);
	String jfloata (String name, float  values[], size_t len, int dp = 2, bool comma = true);
	String jbool   (String name, bool   value,                            bool comma = true);
	String jboola  (String name, bool   values[], size_t len,             bool comma = true);

}

#endif

// vim: set ts=4 sw=4 tw=0 noet : 
