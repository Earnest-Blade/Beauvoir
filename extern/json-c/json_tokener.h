/*
 * $Id: json_tokener.h,v 1.10 2006/07/25 03:24:50 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

/**
 * @file
 * @brief Methods to parse an input string into a tree of json_object objects.
 */
#ifndef _json_tokener_h_
#define _json_tokener_h_

#include "json_object.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum json_tokener_error
{
	json_tokener_success,
	json_tokener_continue,
	json_tokener_error_depth,
	json_tokener_error_parse_eof,
	json_tokener_error_parse_unexpected,
	json_tokener_error_parse_null,
	json_tokener_error_parse_boolean,
	json_tokener_error_parse_number,
	json_tokener_error_parse_array,
	json_tokener_error_parse_object_key_name,
	json_tokener_error_parse_object_key_sep,
	json_tokener_error_parse_object_value_sep,
	json_tokener_error_parse_string,
	json_tokener_error_parse_comment,
	json_tokener_error_parse_utf8_string,
	json_tokener_error_size,   /* A string longer than INT32_MAX was passed as input */
	json_tokener_error_memory  /* Failed to allocate memory */
};

/**
 * @deprecated Don't use this outside of json_tokener.c, it will be made private in a future release.
 */
enum json_tokener_state
{
	json_tokener_state_eatws,
	json_tokener_state_start,
	json_tokener_state_finish,
	json_tokener_state_null,
	json_tokener_state_comment_start,
	json_tokener_state_comment,
	json_tokener_state_comment_eol,
	json_tokener_state_comment_end,
	json_tokener_state_string,
	json_tokener_state_string_escape,
	json_tokener_state_escape_unicode,
	json_tokener_state_escape_unicode_need_escape,
	json_tokener_state_escape_unicode_need_u,
	json_tokener_state_boolean,
	json_tokener_state_number,
	json_tokener_state_array,
	json_tokener_state_array_add,
	json_tokener_state_array_sep,
	json_tokener_state_object_field_start,
	json_tokener_state_object_field,
	json_tokener_state_object_field_end,
	json_tokener_state_object_value,
	json_tokener_state_object_value_add,
	json_tokener_state_object_sep,
	json_tokener_state_array_after_sep,
	json_tokener_state_object_field_start_after_sep,
	json_tokener_state_inf
};

/**
 * @deprecated Don't use this outside of json_tokener.c, it will be made private in a future release.
 */
struct json_tokener_srec
{
	enum json_tokener_state state, saved_state;
	struct json_object *obj;
	struct json_object *current;
	char *obj_field_name;
};

#define JSON_TOKENER_DEFAULT_DEPTH 32

/**
 * Internal state of the json parser.
 * Do not access any fields of this structure directly.
 * Its definition is published due to historical limitations
 * in the json tokener API, and will be changed to be an opaque
 * type in the future.
 */
struct json_tokener
{
	/**
	 * @deprecated Do not access any of these fields outside of json_tokener.c
	 */
	char *str;
	struct printbuf *pb;
	int max_depth, depth, is_double, st_pos;
	/**
	 * @deprecated See json_tokener_get_parse_end() instead.
	 */
	int char_offset;
	/**
	 * @deprecated See json_tokener_get_error() instead.
	 */
	enum json_tokener_error err;
	unsigned int ucs_char, high_surrogate;
	char quote_char;
	struct json_tokener_srec *stack;
	int flags;
};

/**
 * Return the offset of the byte after the last byte parsed
 * relative to the start of the most recent string passed in
 * to json_tokener_parse_ex().  i.e. this is where parsing
 * would start again if the input contains another JSON object
 * after the currently parsed one.
 *
 * Note that when multiple parse calls are issued, this is *not* the
 * total number of characters parsed.
 *
 * In the past this would have been accessed as tok->char_offset.
 *
 * See json_tokener_parse_ex() for an example of how to use this.
 */
JSON_EXPORT uint64 json_tokener_get_parse_end(struct json_tokener *tok);

/**
 * @deprecated Unused in json-c code
 */
typedef struct json_tokener json_tokener;

/**
 * Be strict when parsing JSON input.  Use caution with
 * this flag as what is considered valid may become more
 * restrictive from one release to the next, causing your
 * code to fail on previously working input.
 *
 * Note that setting this will also effectively disable parsing
 * of multiple json objects in a single character stream
 * (e.g. {"foo":123}{"bar":234}); if you want to allow that
 * also set JSON_TOKENER_ALLOW_TRAILING_CHARS
 *
 * This flag is not set by default.
 *
 * @see json_tokener_set_flags()
 */
#define JSON_TOKENER_STRICT 0x01

/**
 * Use with JSON_TOKENER_STRICT to allow trailing characters after the
 * first parsed object.
 *
 * @see json_tokener_set_flags()
 */
#define JSON_TOKENER_ALLOW_TRAILING_CHARS 0x02

/**
 * Cause json_tokener_parse_ex() to validate that input is UTF8.
 * If this flag is specified and validation fails, then
 * json_tokener_get_error(tok) will return
 * json_tokener_error_parse_utf8_string
 *
 * This flag is not set by default.
 *
 * @see json_tokener_set_flags()
 */
#define JSON_TOKENER_VALIDATE_UTF8 0x10

/**
 * Given an error previously returned by json_tokener_get_error(),
 * return a human readable description of the error.
 *
 * @return a generic error message is returned if an invalid error value is provided.
 */
JSON_EXPORT const char *json_tokener_error_desc(enum json_tokener_error jerr);

/**
 * Retrieve the error caused by the last call to json_tokener_parse_ex(),
 * or json_tokener_success if there is no error.
 *
 * When parsing a JSON string in pieces, if the tokener is in the middle
 * of parsing this will return json_tokener_continue.
 *
 * @see json_tokener_error_desc().
 */
JSON_EXPORT enum json_tokener_error json_tokener_get_error(struct json_tokener *tok);

/**
 * Allocate a new json_tokener.
 * When done using that to parse objects, free it with json_tokener_free().
 * See json_tokener_parse_ex() for usage details.
 */
JSON_EXPORT struct json_tokener *json_tokener_new(void);

/**
 * Allocate a new json_tokener with a custom max nesting depth.
 * The depth must be at least 1.
 * @see JSON_TOKENER_DEFAULT_DEPTH
 */
JSON_EXPORT struct json_tokener *json_tokener_new_ex(int depth);

/**
 * Free a json_tokener previously allocated with json_tokener_new().
 */
JSON_EXPORT void json_tokener_free(struct json_tokener *tok);

/**
 * Reset the state of a json_tokener, to prepare to parse a 
 * brand new JSON object.
 */
JSON_EXPORT void json_tokener_reset(struct json_tokener *tok);

/**
 * Parse a json_object out of the string `str`.
 *
 * If you need more control over how the parsing occurs,
 * see json_tokener_parse_ex().
 */
JSON_EXPORT struct json_object *json_tokener_parse(const char *str);

/**
 * Parse a json_object out of the string `str`, but if it fails
 * return the error in `*error`.
 * @see json_tokener_parse()
 * @see json_tokener_parse_ex()
 */
JSON_EXPORT struct json_object *json_tokener_parse_verbose(const char *str,
                                                           enum json_tokener_error *error);

/**
 * Set flags that control how parsing will be done.
 */
JSON_EXPORT void json_tokener_set_flags(struct json_tokener *tok, int flags);

/**
 * Parse a string and return a non-NULL json_object if a valid JSON value
 * is found.  The string does not need to be a JSON object or array;
 * it can also be a string, number or boolean value.
 *
 * A partial JSON string can be parsed.  If the parsing is incomplete,
 * NULL will be returned and json_tokener_get_error() will return
 * json_tokener_continue.
 * json_tokener_parse_ex() can then be called with additional bytes in str
 * to continue the parsing.
 *
 * If json_tokener_parse_ex() returns NULL and the error is anything other than
 * json_tokener_continue, a fatal error has occurred and parsing must be
 * halted.  Then, the tok object must not be reused until json_tokener_reset()
 * is called.
 *
 * When a valid JSON value is parsed, a non-NULL json_object will be
 * returned, with a reference count of one which belongs to the caller.  Also,
 * json_tokener_get_error() will return json_tokener_success. Be sure to check
 * the type with json_object_is_type() or json_object_get_type() before using
 * the object.
 *
 * Trailing characters after the parsed value do not automatically cause an
 * error.  It is up to the caller to decide whether to treat this as an
 * error or to handle the additional characters, perhaps by parsing another
 * json value starting from that point.
 *
 * If the caller knows that they are at the end of their input, the length
 * passed MUST include the final '\0' character, so values with no inherent
 * end (i.e. numbers) can be properly parsed, rather than just returning
 * json_tokener_continue.
 *
 * Extra characters can be detected by comparing the value returned by
 * json_tokener_get_parse_end() against
 * the length of the last len parameter passed in.
 *
 * The tokener does \b not maintain an internal buffer so the caller is
 * responsible for a subsequent call to json_tokener_parse_ex with an 
 * appropriate str parameter starting with the extra characters.
 *
 * This interface is presently not 64-bit clean due to the int len argument
 * so the function limits the maximum string size to INT32_MAX (2GB).
 * If the function is called with len == -1 then strlen is called to check
 * the string length is less than INT32_MAX (2GB)
 *
 * Example:
 * @code
json_object *jobj = NULL;
const char *mystring = NULL;
int stringlen = 0;
enum json_tokener_error jerr;
do {
	mystring = ...  // get JSON string, e.g. read from file, etc...
	stringlen = strlen(mystring);
	if (end_of_input)
		stringlen++;  // Include the '\0' if we know we're at the end of input
	jobj = json_tokener_parse_ex(tok, mystring, stringlen);
} while ((jerr = json_tokener_get_error(tok)) == json_tokener_continue);
if (jerr != json_tokener_success)
{
	fprintf(stderr, "Error: %s\n", json_tokener_error_desc(jerr));
	// Handle errors, as appropriate for your application.
}
if (json_tokener_get_parse_end(tok) < stringlen)
{
	// Handle extra characters after parsed object as desired.
	// e.g. issue an error, parse another object from that point, etc...
}
// Success, use jobj here.

@endcode
 *
 * @param tok a json_tokener previously allocated with json_tokener_new()
 * @param str an string with any valid JSON expression, or portion of.  This does not need to be null terminated.
 * @param len the length of str
 */
JSON_EXPORT struct json_object *json_tokener_parse_ex(struct json_tokener *tok, const char *str,
                                                      int len);

#ifdef __cplusplus
}
#endif

#endif
