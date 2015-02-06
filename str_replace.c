#include <string.h>
// For the "bool" datatype
#include <stdbool.h>

#include "str_replace.h"

// Credits: Brandin of http://stackoverflow.com/questions/4408170/for-string-find-and-replace

// ------------------------------------------------------------------
// Implementation of function
// ------------------------------------------------------------------
#define SUCCESS (char *)haystack
#define FAILURE (void *)NULL

// locate_forward: compare needle_ptr and read_ptr to see if a match occured
// needle_ptr is updated as appropriate for the next call
// return true if match occured, false otherwise
static inline bool locate_forward(const char **needle_ptr, char *read_ptr, const char *needle, const char *needle_last)
{
	if (**needle_ptr == *read_ptr) {
		(*needle_ptr)++;
		if (*needle_ptr > needle_last) {
			*needle_ptr = (const char *)needle;
			return true;
		}
	} else
		*needle_ptr = (const char *)needle;
	return false;
}

// locate_backward: compare needle_ptr and read_ptr to see if a match occured
// needle_ptr is updated as appropriate for the next call
// return true if match occured, false otherwise
static inline bool locate_backward(const char **needle_ptr, char *read_ptr, const char *needle, const char *needle_last)
{
	if (**needle_ptr == *read_ptr) {
		(*needle_ptr)--;
		if (*needle_ptr < needle) {
			*needle_ptr = (const char *)needle_last;
			return true;
		}
	} else
		*needle_ptr = (const char *)needle_last;
	return false;
}

/*
 * Replace oldneedle by newneedle in hackstack.
 *
 * NOTE: hackstacksize needs to be big enough to contain newneedle instead of oldneedle.
 * A good practice is to pass in the total buffer size of haystack.
 */
char *str_replace(char *haystack, size_t haystacksize, const char *oldneedle, const char *newneedle)
{
	size_t oldneedle_len = strlen(oldneedle);
	size_t newneedle_len = strlen(newneedle);
	const char *oldneedle_ptr;	// locates occurences of oldneedle
	char *read_ptr;		// where to read in the haystack
	char *write_ptr;	// where to write in the haystack
	const char *oldneedle_last =	// the last character in oldneedle
	    oldneedle + oldneedle_len - 1;

	// Case 0: oldneedle is empty
	if (oldneedle_len == 0)
		return SUCCESS;	// nothing to do; define as success

	// Case 1: newneedle is not longer than oldneedle
	if (newneedle_len <= oldneedle_len) {
		// Pass 1: Perform copy/replace using read_ptr and write_ptr
		for (oldneedle_ptr = (const char *)oldneedle, read_ptr = haystack, write_ptr = haystack; *read_ptr != '\0'; read_ptr++, write_ptr++) {
			*write_ptr = *read_ptr;
			bool found = locate_forward(&oldneedle_ptr, read_ptr,
						    oldneedle, oldneedle_last);
			if (found) {
				// then perform update
				write_ptr -= oldneedle_len;
				memcpy(write_ptr + 1, newneedle, newneedle_len);
				write_ptr += newneedle_len;
			}
		}
		*write_ptr = '\0';
		return SUCCESS;
	}
	// Case 2: newneedle is longer than oldneedle
	else {
		size_t diff_len =	// the amount of extra space needed 
		    newneedle_len -	// to replace oldneedle with newneedle
		    oldneedle_len;	// in the expanded haystack

		// Pass 1: Perform forward scan, updating write_ptr along the way
		for (oldneedle_ptr = (const char *)oldneedle, read_ptr = haystack, write_ptr = haystack; *read_ptr != '\0'; read_ptr++, write_ptr++) {
			bool found = locate_forward(&oldneedle_ptr, read_ptr,
						    oldneedle, oldneedle_last);
			if (found) {
				// then advance write_ptr
				write_ptr += diff_len;
			}
			if (write_ptr >= haystack + haystacksize)
				return FAILURE;	// no more room in haystack
		}

		// Pass 2: Walk backwards through haystack, performing copy/replace
		for (oldneedle_ptr = (const char *)oldneedle_last; write_ptr >= haystack; write_ptr--, read_ptr--) {
			*write_ptr = *read_ptr;
			bool found = locate_backward(&oldneedle_ptr, read_ptr,
						     oldneedle, oldneedle_last);
			if (found) {
				// then perform replacement
				write_ptr -= diff_len;
				memcpy(write_ptr, newneedle, newneedle_len);
			}
		}
		return SUCCESS;
	}
}
