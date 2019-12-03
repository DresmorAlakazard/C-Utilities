#ifndef __SDA_H__
#define __SDA_H__

#include <string.h>
#include <stdlib.h>

typedef struct sda_header {
	size_t length;
	size_t capacity;
} sda_header;

sda_header* sda__query_header (void* target);
size_t sda__query_length (void* target);
size_t sda__query_capacity (void* target);
unsigned char* sda__query_data (void* target);

int sda__make (void** target, size_t size, size_t length, size_t capacity);
int sda__grow (void** target, size_t size, size_t length);
int sda__copy (void** target, size_t size, const void* source, size_t count);
int sda__insert (void** target, size_t size, size_t at, const void* source, size_t count);
int sda__push (void** target, size_t size, const void* source);

int sda__clear (void* target);
int sda__remove (void* target, size_t size, size_t at, size_t count);
int sda__pop (void* target);
int sda__removeSwap (void* target, size_t size, size_t at, size_t count);

int sda__free (void** target);

#define sda_length(target) (sda__query_length(target))
#define sda_capacity(target) (sda__query_capacity(target))

#define sda_copy(target, source, count) (sda__copy(&target, sizeof(*target), source, (size_t) count == (size_t) -1 ? sda__query_length(source) : count))
#define sda_insert(target, at, source, count) (sda__insert(&target, sizeof(*target), at, source, (size_t) count != (size_t) -1 ? count : sda_length(source)))
#define sda_push(target, elem) (sda__grow(&target, sizeof(*target), target ? sda__query_length(target) + 1 : 1) && (target[sda__query_length(target) - 1] = elem, 1))
#define sda_put(target, at, elem) (sda__grow(&target, sizeof(*target), target ? sda__query_length(target) + 1 : 1) && (target[at] = elem, 1))

#define sda_clear(target) (sda__clear(target))
#define sda_remove(target, at, count) (sda__remove(target, sizeof(*target), at, count))
#define sda_pop(target) (sda__pop(target))
#define sda_removeSwap(target, at, count) (sda__removeSwap(target, sizeof(*target), at, count))

#define sda_free(target) (sda__free(&target))

#endif

#if defined(__SDA_IMPLEMENTATION__) && !defined(__SDA_IMPLEMENTED__)
#define __SDA_IMPLEMENTED__

#define SDA_MIN_CAPACITY 4

sda_header* sda__query_header (void* target) {
	return (sda_header*) target - 1;
}

size_t sda__query_length (void* target) {
	return sda__query_header (target)->length;
}

size_t sda__query_capacity (void* target) {
	return sda__query_header (target)->capacity;
}

unsigned char* sda__query_data (void* target) {
	return (unsigned char*) target;
}

int sda__make (void** target, size_t size, size_t length, size_t capacity) {
	sda_header* header;

	if (*target) {
		header = (sda_header*) realloc (sda__query_header(*target), sizeof (sda_header) + size * capacity);
	} else {
		header = (sda_header*) malloc (sizeof (sda_header) + size * capacity);
	}

	if (header) {
		header->length = length;
		header->capacity = capacity;

		*target = header + 1;

		return 1;
	}

	return 0;
}

int sda__grow (void** target, size_t size, size_t length) {
	if (!*target || sda__query_capacity (*target) < length) {
		return sda__make (target, size, length, length < SDA_MIN_CAPACITY ? SDA_MIN_CAPACITY : length / 2 * 4);
	} else {
		sda__query_header (*target)->length = length;
	}

	return 1;
}

int sda__copy (void** target, size_t size, const void* source, size_t count) {
	sda__clear (*target);
	return sda__insert (target, size, 0, source, count);
}

int sda__insert (void** target, size_t size, size_t at, const void* source, size_t count) {
	if (!sda__grow (target, size, *target ? sda__query_length (*target) + count : count))
		return 0;

	sda_header* header = sda__query_header (*target);
	unsigned char* data = sda__query_data (*target);

	if (at + count <= header->length) {
		memmove (
			data + (at + count) * size,
			data + at * size,
			(header->length - at - count) * size
		);
	}

	memcpy (
		data + at * size,
		source,
		count * size
	);

	return 1;
}

int sda__push (void** target, size_t size, const void* source) {
	return sda__insert (target, size, *target ? sda__query_length (*target) - 1 : 0, source, 1);
}

int sda__clear (void* target) {
	if (!target)
		return 0;

	sda__query_header (target)->length = 0;

	return 1;
}

int sda__remove (void* target, size_t size, size_t at, size_t count) {
	if (!target)
		return 0;

	sda_header* header = sda__query_header (target);
	unsigned char* data = sda__query_data (target);

	if (at + count <= header->length) {
		memmove (
			data + at * size,
			data + (at + count) * size,
			(header->length - at - count) * size
		);

		header->length -= count;
	} else {
		return 0;
	}

	return 1;
}

int sda__pop (void* target) {
	if (!target)
		return 0;

	--sda__query_header (target)->length;

	return 1;
}

int sda__removeSwap (void* target, size_t size, size_t at, size_t count) {
	if (!target)
		return 0;

	sda_header* header = sda__query_header (target);
	unsigned char* data = sda__query_data (target);

	if (header->length >= at + count) {
		memcpy (
			data + at * size,
			data + (header->length - count) * size,
			count * size
		);

		header->length -= count;
	} else {
		return 0;
	}

	return 1;
}

int sda__free (void** target) {
	if (*target) {
		free (sda__query_header (*target));
		*target = NULL;
		return 1;
	}

	return 0;
}

#endif
