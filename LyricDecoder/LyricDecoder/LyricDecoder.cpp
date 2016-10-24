#include "LyricDecoder.h"
#include "zlib/zconf.h"
#include "zlib/zlib.h"
#include "stdio.h"
#include <sys/stat.h>
#include <malloc.h>
#include <stdlib.h>

#include<string.h>
#include<windows.h>

const char key[] = {64, 71, 97, 119, 94, 50, 116, 71, 81, 54, 49, 45, 206, 210,110, 105};

//unsigned char QQKey1[] = "!@#)(NHLiuy*$%^&";
//unsigned char QQKey2[] = "123ZXC!@#)(*$%^&";
//unsigned char QQKey3[] = "!@#)(*$%^&abcDEF";
unsigned char QQKey1[] = { 0x21, 0x40, 0x23, 0x29, 0x28, 0x4E, 0x48, 0x4C, 0x69, 0x75, 0x79, 0x2A, 0x24, 0x25, 0x5E, 0x26 };
unsigned char QQKey2[] = { 0x31, 0x32, 0x33, 0x5A, 0x58, 0x43, 0x21, 0x40, 0x23, 0x29, 0x28, 0x2A, 0x24, 0x25, 0x5E, 0x26 };
unsigned char QQKey3[] = { 0x21, 0x40, 0x23, 0x29, 0x28, 0x2A, 0x24, 0x25, 0x5E, 0x26, 0x61, 0x62, 0x63, 0x44, 0x45, 0x46 };
typedef int(*QQdes)(unsigned char *, unsigned char *, int);
typedef int(*QQDdes)(unsigned char *, unsigned char *, int);
typedef int(*QQUncompressCommon)(unsigned char *, unsigned long *, unsigned char const *, unsigned long);



char *krcdecode(char *src,int src_len){

	
	if (!src) return nullptr;

	if (memcmp(src, "krc1", 4) != 0) {
		return nullptr;
	}
	

	for (int i = 4; i < src_len; i++) {
		src[i] = src[i] ^ key[(i-4) % 16];
	}
	uLongf dstsize = src_len * 100;
	uLongf *dstsizep = &dstsize;
	unsigned char *result = (unsigned char *)malloc(dstsize);
	if (Z_OK != uncompress(result, dstsizep, (const Bytef*)(src + 4), src_len-4)) {
		return nullptr;
	}
		
	result[dstsize] = '\0';

	return (char *)result;
}

char *qrcdecode(char *src, int src_len) {

	char qrc_hearder[] = "[offset:0]";
	HMODULE hDLL = LoadLibrary(L"QQMusicCommon.dll");
	if (!hDLL) {
		return nullptr;
	}
	
	
	if (memcmp(src, qrc_hearder, 10) != 0) {
		return nullptr;
	}
	

	unsigned long out_len = src_len * 16;
	unsigned long *out_lenp = &out_len;

	QQdes des = QQdes(GetProcAddress(hDLL, "?des@qqmusic@@YAHPAE0H@Z"));
	QQDdes Ddes = QQDdes(GetProcAddress(hDLL, "?Ddes@qqmusic@@YAHPAE0H@Z"));
	//QQUncompressCommon UncompressCommon = QQUncompressCommon(GetProcAddress(hDLL, "?UncompressCommon@qqmusic@@YAHPAEPAKPBEK@Z"));
	Ddes((unsigned char *)(src + 11), QQKey1, src_len - 11);
	des( (unsigned char *)(src + 11), QQKey2, src_len - 11);
	Ddes((unsigned char *)(src + 11), QQKey3, src_len - 11);

	unsigned char *result = (unsigned char *)malloc(out_len);
	
	//if (!result) return nullptr;
	
	//UncompressCommon(result, out_lenp, (unsigned char *)(src + 11), src_len - 11);
	if (Z_OK != uncompress(result, out_lenp, (const Bytef*)(src + 11), src_len - 11)) {
		return nullptr;
	}
	result[out_len] = '\0';
	FreeLibrary(hDLL);
	return (char *)result;
}
