#ifndef RYANJSON_RFC8259_EMBEDDED_H
#define RYANJSON_RFC8259_EMBEDDED_H

#include <stdint.h>

typedef struct
{	const char *name;
	const unsigned char *data;
	uint32_t len;
} rfc8259EmbeddedFile_t;

extern const rfc8259EmbeddedFile_t gRfc8259EmbeddedFiles[];
extern const uint32_t gRfc8259EmbeddedFileCount;

#endif // RYANJSON_RFC8259_EMBEDDED_H
