#include "LyricDecoder.h"
#include "zlib/zconf.h"
#include "zlib/zlib.h"
#include "stdio.h"
#include "QQMusicDES/des.h"
#include <sys/stat.h>
#include <malloc.h>
#include <stdlib.h>
#include <assert.h>

#include<string.h>

const unsigned char key[] = { 64, 71, 97, 119, 94, 50, 116, 71, 81, 54, 49, 45, 206, 210, 110, 105 };

//unsigned char QQKey[] = "!@#)(*$%123ZXC!@!@#)(NHL";
const unsigned char QQKey[] = { 0x21, 0x40, 0x23, 0x29, 0x28, 0x2A, 0x24, 0x25, 0x31, 0x32, 0x33, 0x5A, 0x58, 0x43, 0x21, 0x40, 0x21, 0x40, 0x23, 0x29, 0x28, 0x4E, 0x48, 0x4C };


// Deflate from the src buffer. The returned memory is allocated using malloc, and the caller is responsible for freeing it.
// A trailing '\0' is added to the decompress result
// Returns NULL on failure.
unsigned char* deflate_memory(Bytef* src, unsigned src_len) {
	int ret = Z_STREAM_ERROR;
	z_stream strm;
	size_t dest_len = 262144;  // 256k chunk size as suggested by zlib doc
	Bytef* dest = (Bytef*)malloc(dest_len);
	if (!dest) return NULL;

	// allocate inflate state
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = src;
	strm.avail_in = src_len;
	strm.next_out = dest;
	strm.avail_out = dest_len;
	if (inflateInit(&strm) != Z_OK) {
		free(dest);
		return NULL;
	}
	for (;;) {
		if (!strm.avail_in)
			break;
		ret = inflate(&strm, Z_NO_FLUSH);
		assert(ret != Z_STREAM_ERROR);  // state not clobbered
		switch (ret) {
		case Z_NEED_DICT:
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			inflateEnd(&strm);
			free(dest);
			return NULL;
		}
		if (strm.avail_out || ret == Z_STREAM_END)
			break;
		else {
			// double the size of output buffer
			Bytef* dest_new = (Bytef*)realloc(dest, 2 * dest_len);
			if (dest_new) {
				dest = dest_new;
				strm.next_out = dest + dest_len;
				strm.avail_out = dest_len;
				dest_len *= 2;
			}
			else {
				inflateEnd(&strm);
				free(dest);
				return NULL;
			}
		}
	}

	// clean up and return
	inflateEnd(&strm);
	if (ret != Z_STREAM_END) {
		free(dest);
		return NULL;
	}
	dest_len -= strm.avail_out;
	unsigned char* dest_new = (unsigned char*)realloc(dest, dest_len + 1);
	if (dest_new)
		dest = dest_new;
	else {
		free(dest);
		return NULL;
	}
	dest[dest_len] = 0;
	return dest;
}

char *krcdecode(char *src,int src_len){
	if (src_len < 4 || memcmp(src, "krc1", 4) != 0)
		return nullptr;

	for (int i = 4; i < src_len; i++) {
		src[i] = src[i] ^ key[(i-4) % 16];
	}

	return (char*)deflate_memory((Bytef*)src + 4, src_len - 4);
}

char *qrcdecode(char *src, int src_len) {
	if (src_len < 10 || memcmp(src, "[offset:0]", 10) != 0)
		return nullptr;

	BYTE schedule[3][16][6];
	three_des_key_setup(QQKey, schedule, DES_DECRYPT);
	for (int i = 11; i < src_len; i += 8)
		three_des_crypt(reinterpret_cast<BYTE*>(src) + i, reinterpret_cast<BYTE*>(src) + i, schedule);

	return (char*)deflate_memory((Bytef*)src + 11, src_len - 11);
}
