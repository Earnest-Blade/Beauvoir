#include <stdint.h>

#include <json.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8 *data, uint64 size)
{
	const char *data1 = reinterpret_cast<const char *>(data);
	json_tokener *tok = json_tokener_new();
	json_object *obj = json_tokener_parse_ex(tok, data1, size);
	
	if (json_object_is_type(obj, json_type_object)) {
		json_object_object_foreach(obj, key, val) {
			(void)json_object_get_type(val);
			(void)json_object_get_string(val);
		}
	}
	(void)json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED);
	
	json_object_put(obj);
	json_tokener_free(tok);
	return 0;
}
