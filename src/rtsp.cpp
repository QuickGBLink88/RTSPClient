/**
** Blog: https://blog.csdn.net/quickgblink
** Email: quickgblink@126.com
**/

#include "stdafx.h"
#include "config_win32.h"
/*
#include <string.h>
#include <stdio.h>
*/
#include "rtsp.h"

#include "H264.h"

int WriteAdtsHeader(unsigned char* adts_headerbuf, unsigned int  framelen);

struct PACKET_LOST_INFO
{
	BYTE head;
	BYTE tail;
	int nLastNo;
	int nLostCount;
};

PACKET_LOST_INFO lostInfo1 = {0}; //视频丢包信息
PACKET_LOST_INFO lostInfo2 = {0}; //音频丢包信息

//#define _SAVE_STREAM_TO_FILE

FILE * poutfile =  NULL;	
char * outputfilename = "d:\\receive.aac";

// Forward function definitions:
const static ULONG n1970_1900_Seconds = 2208988800;  

unsigned const parseBufferSize = 100;

uint32_t fixed_vop_rate = 0;

char const* const UserAgentHeaderStr = "User-Agent: QuickGBLink V201911 \r\n";
	
static const u_char frame_sizes_nb[16] = {
    12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0
};

static const u_char frame_sizes_wb[16] = {
   17, 23, 32, 36, 40, 46, 50, 58, 60, 5, 5, 0, 0, 0, 0, 0
};

#ifdef	_WIN32
#define	snprintf			sprintf_s
#define	strncasecmp			_strnicmp
#define	strcasestr			StrStrI
#endif

char* strDup(char* str) 
{
	char* copy;
	unsigned int len;
	if (str == NULL) return NULL;
	len = strlen(str) + 1;
	copy = (char*)malloc(len*sizeof(char));

	if (copy != NULL) {
		memcpy(copy, str, len);
	}
	return copy;
}

char* strDupSize(char* str) 
{
	char* copy;
	unsigned int len;
	if (str == NULL) return NULL;
	len = strlen(str) + 1;
	copy = (char *)malloc(len*sizeof(char));

	return copy;
}

//将一个16进制数转成字符串的形式
int hex_a(unsigned char *hex, char *a,unsigned char length)
{
	unsigned char hLowbit,hHighbit;
	int i;

	for(i=0;i<length*2;i+=2)
	{
		hLowbit=hex[i/2]&0x0f;
		hHighbit=hex[i/2]/16;
		if (hHighbit>=10) a[i]=hHighbit+'7';
		else a[i]=hHighbit+'0';
		if (hLowbit>=10) a[i+1]=hLowbit+'7';
		else a[i+1]=hLowbit+'0';
	}
	a[length*2]='\0';
	return 0;
}

//将一个字符串转成16进制数字为元素的数组
int a_hex( char *a,unsigned char *hex,unsigned char len)
{
	 short i;
	 unsigned char aLowbit,aHighbit;
	 unsigned char hconval,lconval;

	 for(i=0;i<len;i+=2)
	 {
		aHighbit=toupper(a[i]);
		if ((aHighbit>='A')&&(aHighbit<='F')) 
			hconval='7';
		else
			 if ((aHighbit>='0')&&(aHighbit<='9')) 
			 	hconval='0';
			 else	
			 	return -1;
		aLowbit=toupper(a[i+1]);
		if ((aLowbit>='A')&&(aLowbit<='F')) 
			lconval='7';
		else
			 if ((aLowbit>='0')&&(aLowbit<='9')) 
			 	lconval='0';
			 else 
			 	return -1;
		hex[(i/2)]=((aHighbit-hconval)*16+(aLowbit-lconval));
	 }
	 hex[len/2]=0x00;
	 return 0;
}

unsigned char* skip(unsigned char * buffer,unsigned numBytes) 
{
	buffer = buffer + numBytes;
	return buffer;
} 

//////////////////////////////////////////////////////////////

#ifdef	_WIN32
#define	snprintf			sprintf_s
#endif



typedef struct AVMD5{
    unsigned long long	len;
    unsigned char		block[64];
    unsigned int		ABCD[4];
} AVMD5;

const int av_md5_size= sizeof(AVMD5);

static const unsigned char S[4][4] = {
    { 7, 12, 17, 22 },  /* round 1 */
    { 5,  9, 14, 20 },  /* round 2 */
    { 4, 11, 16, 23 },  /* round 3 */
    { 6, 10, 15, 21 }   /* round 4 */
};

static const unsigned int T[64] = { // T[i]= fabs(sin(i+1)<<32)
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,   /* round 1 */
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,

    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,   /* round 2 */
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,

    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,   /* round 3 */
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,

    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,   /* round 4 */
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};


#define CORE(i, a, b, c, d) \
        t = S[i>>4][i&3];\
        a += T[i];\
\
        if(i<32){\
            if(i<16) a += (d ^ (b&(c^d))) + X[      i &15 ];\
            else     a += (c ^ (d&(c^b))) + X[ (1+5*i)&15 ];\
        }else{\
            if(i<48) a += (b^c^d)         + X[ (5+3*i)&15 ];\
            else     a += (c^(b|~d))      + X[ (  7*i)&15 ];\
        }\
        a = b + (( a << t ) | ( a >> (32 - t) ));

///////////////////////////////////////////////////////////////////////////////////
#define AV_BSWAP16C(x) (((x) << 8 & 0xff00)  | ((x) >> 8 & 0x00ff))
#define AV_BSWAP32C(x) (AV_BSWAP16C(x) << 16 | AV_BSWAP16C((x) >> 16))
#define AV_BSWAP64C(x) (AV_BSWAP32C(x) << 32 | AV_BSWAP32C((x) >> 32))

#ifdef	_WIN32
#ifdef WORDS_BIGENDIAN
 #define be2me_16(x) (x)
 #define be2me_32(x) (x)
 #define be2me_64(x) (x)
 #define le2me_16(x) AV_BSWAP16C(x)
 #define le2me_32(x) AV_BSWAP32C(x)
 #define le2me_64(x) AV_BSWAP64C(x)
 #else
 #define be2me_16(x) AV_BSWAP16C(x)
 #define be2me_32(x) AV_BSWAP32C(x)
 #define be2me_64(x) AV_BSWAP64C(x)
 #define le2me_16(x) (x)
 #define le2me_32(x) (x)
 #define le2me_64(x) (x)
 #endif
#else
#ifndef WORDS_BIGENDIAN
 #define be2me_16(x) (x)
 #define be2me_32(x) (x)
 #define be2me_64(x) (x)
 #define le2me_16(x) AV_BSWAP16C(x)
 #define le2me_32(x) AV_BSWAP32C(x)
 #define le2me_64(x) AV_BSWAP64C(x)
 #else
 #define be2me_16(x) AV_BSWAP16C(x)
 #define be2me_32(x) AV_BSWAP32C(x)
 #define be2me_64(x) AV_BSWAP64C(x)
 #define le2me_16(x) (x)
 #define le2me_32(x) (x)
 #define le2me_64(x) (x)
 #endif
#endif
///////////////////////////////////////////////////////////////////////////////////

static void body(unsigned int ABCD[4], unsigned int X[16]){

    int t;
//    int i av_unused;
    unsigned int a= ABCD[3];
    unsigned int b= ABCD[2];
    unsigned int c= ABCD[1];
    unsigned int d= ABCD[0];

#ifdef	_WIN32
#else
	int	i;
#if 1 //HAVE_BIGENDIAN
    for(i=0; i<16; i++)
        X[i]= AV_BSWAP32C(X[i]);//bswap_32(X[i]);
#endif
#endif

#if CONFIG_SMALL
    for( i = 0; i < 64; i++ ){
        CORE(i,a,b,c,d)
        t=d; d=c; c=b; b=a; a=t;
    }
#else
#define CORE2(i) CORE(i,a,b,c,d) CORE((i+1),d,a,b,c) CORE((i+2),c,d,a,b) CORE((i+3),b,c,d,a)
#define CORE4(i) CORE2(i) CORE2((i+4)) CORE2((i+8)) CORE2((i+12))
CORE4(0) CORE4(16) CORE4(32) CORE4(48)
#endif

    ABCD[0] += d;
    ABCD[1] += c;
    ABCD[2] += b;
    ABCD[3] += a;
}

void av_md5_init(AVMD5 *ctx){
    ctx->len    = 0;

    ctx->ABCD[0] = 0x10325476;
    ctx->ABCD[1] = 0x98badcfe;
    ctx->ABCD[2] = 0xefcdab89;
    ctx->ABCD[3] = 0x67452301;
}


void av_md5_update(AVMD5 *ctx, unsigned char *src, int len)
{
    int i, j;

    j= ctx->len & 63;
    ctx->len += len;

    for( i = 0; i < len; i++ ){
        ctx->block[j++] = src[i];
        if( 64 == j ){
            body(ctx->ABCD, (unsigned int*) ctx->block);
            j = 0;
        }
    }
}

void av_md5_final(AVMD5 *ctx, unsigned char *dst){
    int i;
    unsigned long long finalcount= le2me_64(ctx->len<<3);
//    ULONGLONG finalcount= le2me_64(ctx->len<<3);

    av_md5_update(ctx, (unsigned char*)"\200", 1);
    while((ctx->len & 63)!=56)
        av_md5_update(ctx, (unsigned char*)"", 1);

    av_md5_update(ctx, (unsigned char*)&finalcount, 8);

    for(i=0; i<4; i++)
        ((unsigned int*)dst)[i]= le2me_32(ctx->ABCD[3-i]);
}

void av_md5_sum(unsigned char *dst, unsigned char *src, int len){
    AVMD5 ctx[1];

    av_md5_init(ctx);
    av_md5_update(ctx, src, len);
    av_md5_final(ctx, dst);
}

char *ff_data_to_hex(char *buff, const unsigned char *src, int s, int lowercase)
{
    int i;
    static const char hex_table_uc[16] = { '0', '1', '2', '3',
                                           '4', '5', '6', '7',
                                           '8', '9', 'A', 'B',
                                           'C', 'D', 'E', 'F' };
    static const char hex_table_lc[16] = { '0', '1', '2', '3',
                                           '4', '5', '6', '7',
                                           '8', '9', 'a', 'b',
                                           'c', 'd', 'e', 'f' };
    const char *hex_table = lowercase ? hex_table_lc : hex_table_uc;

    for(i = 0; i < s; i++) {
        buff[i * 2]     = hex_table[src[i] >> 4];
        buff[i * 2 + 1] = hex_table[src[i] & 0xF];
    }

    return buff;
}

static void update_md5_strings(struct AVMD5 *md5ctx, ...)
{
    va_list vl;

    va_start(vl, md5ctx);
    while (1) {
        const char* str = va_arg(vl, const char*);
        if (!str)
            break;
        av_md5_update(md5ctx, (unsigned char*)str, strlen(str));
    }
    va_end(vl);
}


/*****************************************************************************
* b64_encode: Stolen from VLC's http.c.
* Simplified by Michael.
* Fixed edge cases and made it work from data (vs. strings) by Ryan.
*****************************************************************************/

#define AV_BASE64_SIZE(x)  (((x)+2) / 3 * 4 + 1)

#ifndef	_WIN32
#define	UINT_MAX	0xFFFFFFFF
#endif

static char *av_base64_encode(char *out, int out_size, const unsigned char *in, int in_size)
{
    static const char b64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *ret, *dst;
    unsigned i_bits = 0;
    int i_shift = 0;
    int bytes_remaining = in_size;

    if (in_size >= (int)( UINT_MAX / 4 ) || out_size < (in_size+2) / 3 * 4 + 1)
        return NULL;
    ret = dst = out;
    while (bytes_remaining) {
        i_bits = (i_bits << 8) + *in++;
        bytes_remaining--;
        i_shift += 8;

        do {
            *dst++ = b64[(i_bits << 6 >> i_shift) & 0x3f];
            i_shift -= 6;
        } while (i_shift > 6 || (bytes_remaining == 0 && i_shift > 0));
    }
    while ((dst - ret) & 3)
        *dst++ = '=';
    *dst = '\0';

    return ret;
}

int GetAuthParam(char *pStr, char *realm, char *nonce)
{// 获取验证信息 Basic realm - Digest realm
	int		ret;
	char	*pChar,*pChar1;
	char	tmp[64];

	ret = 0;
	memset( realm, 0, 64 );
	memset( nonce, 0, 64 );

	pChar = strstr( pStr, "WWW-Authenticate:" );
	if( pChar )
	{
		pChar = strstr( pStr, "realm=\"" );
		if( pChar )
		{
			memcpy( tmp, pChar-6, 11 );
			tmp[11] = 0;
			if( strncasecmp( tmp, "Basic realm", 11 ) == 0 )
			{
				ret = 1;
			}
			memcpy( tmp, pChar-7, 12 );
			tmp[12] = 0;
			if( strncasecmp( tmp, "Digest realm", 12 ) == 0 )
			{
				ret = 2;
			}
			if( ret == 0 )
				return -1;
			//
			pChar += 7;
			pChar1 = pChar;
			while( *pChar1 != '"' && *pChar1 != '\r' && *pChar1 != 0 )
			{
				pChar1 ++;
			}
			if( *pChar1 == '"' )
			{
				memcpy( realm, pChar, pChar1-pChar );
			}
		}
		pChar = strstr( pStr, "nonce=\"" );
		if( pChar )
		{
			pChar += 7;
			pChar1 = pChar;
			while( *pChar1 != '"' && *pChar1 != '\r' && *pChar1 != 0 )
			{
				pChar1 ++;
			}
			if( *pChar1 == '"' )
			{
				memcpy( nonce, pChar, pChar1-pChar );
			}
		}
		
	}

	if( ret == 1 && realm[0] )
		return 1;
	if( ret == 2 && realm[0] && nonce[0] )
		return 2;

	return 0;
}



//pCommand 的内容可以是“OPTIONS”或者是“DESCRIBE”以及其他指令。
int GetAuthor(char *realm, char *nonce,int style, char *Name, char *Password, char *Uri, char *pAuth,char * pCommand)
{
	char			response[64],algorithm[64],opaque[4],qop[64],cnonce[64],nc[64];
	char			str[256];
	char			strbase[256];
    struct AVMD5	*md5ctx;
    unsigned char	hash[16];
	char			A1hash[33], A2hash[33];
	int				ret,len;

	*pAuth = 0;
	response[0]		= 0;
	algorithm[0]	= 0;
	opaque[0]		= 0;
	qop[0]			= 0;
	cnonce[0]		= 0;
	nc[0]			= 0;

	if (!pCommand)
	{
		return 0;
	}
	//
	ret = style;//GetAuthParam( pStr, realm, nonce );
	if( ret <= 0 )
		return 1;
	if( ret == 1 )
	{
		snprintf( cnonce, 64, "%s:%s", Name, Password );
		len = AV_BASE64_SIZE( strlen(cnonce) ) + 30;
		av_base64_encode( strbase, len, (unsigned char*)cnonce, strlen(cnonce) );
		snprintf( pAuth, 512, "Authorization: Basic %s\r\n", strbase );
	}
	else
	{
		md5ctx = (struct AVMD5*)malloc(av_md5_size);
		av_md5_init( md5ctx );
		update_md5_strings( md5ctx, Name, ":", realm, ":", Password, NULL);
		av_md5_final( md5ctx, hash );
		ff_data_to_hex( A1hash, hash, 16, 1 );
		A1hash[32] = 0;
		//
		//av_md5_init( md5ctx );
		//update_md5_strings( md5ctx, A1hash, ":", nonce, ":", cnonce, NULL);
		//av_md5_final( md5ctx, hash );
		//ff_data_to_hex( A1hash, hash, 16, 1 );
		//A1hash[32] = 0;
		//
		av_md5_init( md5ctx );
		update_md5_strings( md5ctx, pCommand, ":", Uri, NULL );
		av_md5_final( md5ctx, hash );
		ff_data_to_hex( A2hash, hash, 16, 1 );
		A2hash[32] = 0;
		//
		av_md5_init( md5ctx );
		update_md5_strings( md5ctx, A1hash, ":", nonce, NULL );
		update_md5_strings( md5ctx, ":", A2hash, NULL );
		av_md5_final( md5ctx, hash );
		ff_data_to_hex( response, hash, 16, 1 );
		response[32] = 0;
		//
		free(md5ctx);
		//
		snprintf( pAuth, 512, "Authorization: Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"\r\n", Name, realm, nonce, Uri, response );
		if( algorithm[0] )
		{
			snprintf( str, 256, ", algorithm=%s", algorithm );
#ifdef	_WIN32
			strcat_s( pAuth, 512, str );
#else
			strncat( pAuth, str, 512 );
#endif
		}
		if( opaque[0] )
		{
			snprintf( str, 256, ", opaque=%s", opaque );
#ifdef	_WIN32
			strcat_s( pAuth, 512, str );
#else
			strncat( pAuth, str, 512 );
#endif
		}
		if( qop[0] )
		{
			snprintf( str, 256, ", qop=%s", qop );
#ifdef	_WIN32
			strcat_s( pAuth, 512, str );
#else
			strncat( pAuth, str, 512 );
#endif
		}
		if( cnonce[0] )
		{
			snprintf( str, 256, ", cnonce=%s", cnonce );
#ifdef	_WIN32
			strcat_s( pAuth, 512, str );
#else
			strncat( pAuth, str, 512 );
#endif
		}
		if( nc[0] )
		{
			snprintf( str, 256, ", nc=%s", nc );
#ifdef	_WIN32
			strcat_s( pAuth, 512, str );
#else
			strncat( pAuth, str, 512 );
#endif
		}
	}

	return ret;
}

//base64解码
static const BYTE map2[] =
{
	0x3e, 0xff, 0xff, 0xff, 0x3f, 0x34, 0x35, 0x36,
	0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01,
	0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
	0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x1b,
	0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
	0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
	0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33
};

#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))


int av_base64_decode(BYTE *out, const char *in, int out_size)
{
	int i, v;
	BYTE *dst = out;

	v = 0;
	for (i = 0; in[i] && in[i] != '='; i++) {
		unsigned int index= in[i]-43;
		if (index>=FF_ARRAY_ELEMS(map2) || map2[index] == 0xff)
		{
			return dst - out;
		}
		v = (v << 6) + map2[index];
		if (i & 3) {
			if (dst - out < out_size) {
				*dst++ = v >> (6 - 2 * (i & 3));
			}
		}
	}

	return dst - out;
}

int GetSpropParameterSets_h264(char * source, BYTE * out,int & outlen)
{
	char head[] = "sprop-parameter-sets=";
	char str_tmp[4096] = {0};
	char sub_out[4096] = {0};
	int sub_len = 4096;
	const char * sub = NULL;
	int len = 0;
	int ret_len = 0;
	int index = 0;
	outlen = 0;
	if (source)
	{
		const char * str_sprop = strstr(source,head);
		if(str_sprop == NULL)
            str_sprop = strstr(source, "sprop_parameter-sets=");

		if (str_sprop)
		{
			strcpy(str_tmp,str_sprop+sizeof(head)-1);
			len = strlen(str_tmp);
			for (int i = 0;i<len;i++)
			{
				if (';' == str_tmp[i])
				{
					str_tmp[i] = '\0';
					break;
				}
			}
			char *out_ptr = NULL;
			sub = strtok_s(str_tmp,",", &out_ptr);
			while (sub)
			{
				ret_len = av_base64_decode((BYTE *)sub_out,sub,4096);
				if (ret_len>0)
				{
					out[index++] = 0;
					out[index++] = 0;
					out[index++] = 0;
					out[index++] = 1;
					memcpy(out+index,sub_out,ret_len);
					index += ret_len;
				}
				sub = strtok_s(NULL,",", &out_ptr);
			}
		}
	}
	outlen = index;
	return index;
}

int GetSpropParameterSets_h265(char * source, BYTE * out,int & outlen)
{
	
	char str_tmp[4096] = {0};
	char sub_out[4096] = {0};
	int sub_len = 4096;
	const char * sub = NULL;
	int len = 0;
	int ret_len = 0;
	int index = 0;
	outlen = 0;
	if (source)
	{
		//if (!strcmp(attr, "sprop-vps") || !strcmp(attr, "sprop-sps") ||
		//	!strcmp(attr, "sprop-pps") || !strcmp(attr, "sprop-sei")) 
		//{
		//}

		char head[3][32] = {"sprop-vps=", "sprop-sps=", "sprop-pps="};

		for(int i=0; i<3; i++)
		{
			const char * str_sprop = strstr(source, head[i]);
			if(str_sprop == NULL)
				return -1;

			if (str_sprop)
			{
				strcpy(str_tmp,str_sprop+strlen(head[i]));
				len = strlen(str_tmp);
				for (int i = 0;i<len;i++)
				{
					if (';' == str_tmp[i])
					{
						str_tmp[i] = '\0';
						break;
					}
					if ('\r' == str_tmp[i] || '\n' == str_tmp[i])
					{
						str_tmp[i] = '\0';
						break;
					}
				}
				char *out_ptr = NULL;
				sub = strtok_s(str_tmp,",", &out_ptr);
				while (sub)
				{
					ret_len = av_base64_decode((BYTE *)sub_out,sub,4096);
					if (ret_len>0)
					{
						out[index++] = 0;
						out[index++] = 0;
						out[index++] = 0;
						out[index++] = 1;
						memcpy(out+index,sub_out,ret_len);
						index += ret_len;
					}
					sub = strtok_s(NULL,",", &out_ptr);
				}
			}
		}

	}
	outlen = index;
	return index;
}






RtspClient::RtspClient()
{
	fCSeq = 0;

	memset(fBaseURL, 0, sizeof(fBaseURL));
	memset(fLastSessionId, 0, sizeof(fLastSessionId));

	fMaxPlayEndTime = 0;

	VTimestampFrequency = 0;
	ATimestampFrequency = 0;
	VPayloadType = 0;
	APayloadType = 0;
	PotPos = 0;
	VFrameRate = 0;
	vloopNum = 0;
	m_netbuf = NULL;
    m_rtpNumber = 0;
	m_rtcpNumber = 0;

	AllocNetBuffer(); //分配内存接收网络数据
		   
	m_RtspThread = NULL;
	m_SendThread = NULL;
    m_bStopThread = FALSE;
	m_bThreadActive = FALSE;

	LVrtpTimestamp = 0;
	LArtpTimestamp = 0;
	MaxFrameNum = 0;
	aloopNum = 0;
	vloopNum = 0;
	audioFirstTimestamp = 0;
	videoFirstTimestamp = 0;

	memset(&m_VFrame, 0, sizeof(m_VFrame));
	memset(&m_AFrame, 0, sizeof(m_AFrame));
	memset(&m_Attribute, 0, sizeof(MediaAttribute));

	m_lpUserContext = NULL;
	m_lpFunc = NULL;
    poutfile = NULL; //录制文件的文件句柄

	m_bUserAuth = FALSE;
	m_authType = 0;
	memset(m_realm, 0, sizeof(m_realm));
	memset(m_nonce, 0, sizeof(m_nonce));
    memset(m_username, 0, sizeof(m_username));
    memset(m_password, 0, sizeof(m_password));

	memset(m_sprop_parameter, 0, sizeof(m_sprop_parameter));
    m_sprop_len = 0;

	m_video_rtp_id  = m_audio_rtp_id = 0; 
	m_video_rtcp_id = m_audio_rtcp_id = 1; 

	m_vAudioFrameSlices.reserve(50); //先分配空间
	m_bFrameContainSPS = FALSE;

}
	
RtspClient::~RtspClient()
{
	m_bStopThread = TRUE;

	if (m_RtspThread != NULL) 
	{
        WaitForSingleObject(m_RtspThread, INFINITE);
		m_RtspThread = NULL;
	}
	if(m_SendThread != NULL)
	{
	    WaitForSingleObject(m_SendThread, INFINITE);
		m_SendThread = NULL;
	}

	ReleaseBuffer();

#ifdef _SAVE_STREAM_TO_FILE
	if(poutfile != NULL)
	{
		::fclose(poutfile);
		poutfile = NULL;
	}
#endif
}

//设置RTSP URL
void RtspClient::SetUrl(const char * szURL)
{
	strcpy(fBaseURL, szURL);
}

//设置登录用户名和密码
void RtspClient:: SetAuthInfo(const char * szUserName, const char * szPassword)
{
   if(szUserName)
      strcpy(m_username, szUserName);
   if(szPassword)
      strcpy(m_password, szPassword);
}



void RtspClient::clearup()
{
	//teardownMediaSession(m_socketNum);
	//memset(fBaseURL, 0, sizeof(fBaseURL));
	memset(fLastSessionId, 0, sizeof(fLastSessionId));

	if(m_socketNum > 0)
	{
		closesocket(m_socketNum);
		m_socketNum = 0;
	}
}

int RtspClient::startPlayingStreams(struct MediaSubsession *subsession,int subsessionNum)
{ 
	if(playMediaSession(0,-1) != 0)
	{
		fprintf(stderr,"Play MediaSession failed\n");
		return -1;
	}
	fprintf(stderr,"Play MediaSession successful\n");
	return 0;
}


int  RtspClient::setupStreams(struct MediaSubsession *subsessionHead,int subsessionNum) 
{
	struct MediaSubsession *mediasub;
	mediasub = subsessionHead;

	while(subsessionNum>0)
	 {     
		if(setupMediaSubsession(mediasub,subsessionNum) != 0) 
		{	
			fprintf(stderr,"Setup MediaSubsession Failed\n");
			return -1;
		} 
                   
		mediasub = mediasub->fNext;
		subsessionNum--;
	 }

	fprintf(stderr,"Setup Streams successful\n");
	return 0;
}

void RtspClient::FreeSubSessions(struct MediaSubsession *subsessionHead,int subsessionNum)
{
	struct MediaSubsession *mediasub;
	struct MediaSubsession *pLastSub;

	mediasub = subsessionHead;

	while(subsessionNum>0)
	 {    
       if(mediasub->fMediumName)  free(mediasub->fMediumName);       
	   if(mediasub->fCodecName)   free(mediasub->fCodecName); 
	   if(mediasub->fControlPath) free(mediasub->fControlPath); 
	   if(mediasub->fConfig)      free(mediasub->fConfig); 
	   if(mediasub->fSessionId)   free(mediasub->fSessionId);
	   if(mediasub->fFileName)    free(mediasub->fFileName);

	   pLastSub = mediasub;
	   mediasub = mediasub->fNext;

		free(pLastSub);

		subsessionNum--;
	 }
}


//  recv函数会阻塞，需要设置接收超时
//返回值： 成功返回0，失败返回-1
int RtspClient::handleRead(unsigned char* buffer,unsigned bufferMaxSize,unsigned *bytesRead,unsigned* NextTCPReadSize) 
{
	int readSuccess = -1;
	int totBytesToRead;
	int curBytesToRead;
	int curBytesRead;

	if (m_socketNum < 0) 
	{
		fprintf(stderr,"no socket active\n");
		return -1;
	}
	else 
	{
		// Read from the TCP connection:
		*bytesRead = 0;
		totBytesToRead = *NextTCPReadSize;
		*NextTCPReadSize = 0;
    
		if (totBytesToRead > bufferMaxSize) totBytesToRead = bufferMaxSize; 
    
		curBytesToRead = totBytesToRead;
		while ((curBytesRead = recv(m_socketNum, (char*)&buffer[*bytesRead], curBytesToRead,0)) > 0) 
		{
      			(*bytesRead) += curBytesRead;
			if ((*bytesRead) >= totBytesToRead) break;
			curBytesToRead -= curBytesRead;
			
		}
		if (curBytesRead <= 0) 
		{
			*bytesRead = 0;
			readSuccess = -1;
		} 
		else 
		{
			readSuccess = 0;
		}
	}

	return readSuccess;
}


//函数说明： 提取RTP包的头部信息和负载数据
//参数意义：
//        unsigned char * buffer -- RTP包内存地址
//        unsigned int bytesRead -- RTP包的长度
//        ResultData** data -- 返回指向视频或音频信息的结构体，其中data->buffer指向的是去掉RTP头部的负载数据的内存地址
//返回值：  接收到完整的一帧返回1，不是完整的一帧返回0， 失败返回-1
// 
int RtspClient::rtpHandler( unsigned char * buffer, unsigned int bytesRead, struct ResultData** data)
{
	//TRACE("---------------------\n");

	unsigned datasize;
	unsigned rtpHdr;
	int rtpMarkerBit;
	unsigned long rtpTimestamp;
	//static unsigned long LVrtpTimestamp = 0;
	//static unsigned long LArtpTimestamp = 0;
	unsigned rtpSSRC;
	unsigned cc;
	unsigned char payloadType = 0;
	unsigned char rtcpType =0;
	unsigned long subTime = 0;
	//static unsigned long MaxFrameNum = 0;
	//static unsigned long aloopNum = 0;
	//static unsigned long audioFirstTimestamp = 0;
	//static unsigned long videoFirstTimestamp = 0;

	unsigned extHdr;
	unsigned remExtSize;
	unsigned numPaddingBytes;
	unsigned short rtpSeqNo;
	unsigned long time_inc;
	//static unsigned subTimeFlag = 0;
    int nResult = 0;

	unsigned int h264_startcode = 0x01000000;

	memset(m_buffer2, 0, 1500);
	int datasize2 = 0;
      
	//注意： 以下循环总是执行一次
	do 
	{

		datasize = bytesRead;   

#if 0
		rtp_unpackage_H264_to_file(buffer, datasize);
#endif

		// Check for the 12-byte RTP header:
		if (datasize < 12) 
			break;
		rtpHdr = ntohl(*(unsigned*)(buffer)); 

		//marker
		rtpMarkerBit = (rtpHdr&0x00800000) >> 23;
		
		buffer = skip(buffer,4);
		datasize -=4;

		//timestamp
		rtpTimestamp = ntohl(*(unsigned*)(buffer));
	
		buffer = skip(buffer,4);
		datasize -=4;
		
		//ssrc
		rtpSSRC = ntohl(*(unsigned*)(buffer)); 

		buffer = skip(buffer,4);
		datasize -=4;
    
		//// ver
		//if ((rtpHdr&0xC0000000) != 0x80000000) 
		//	break;
    
		//cc
		cc = (rtpHdr>>24)&0xF;
		if (datasize < cc) 
			break;

		buffer = skip(buffer,cc);
		datasize-=cc;
		
		// header extension
		if (rtpHdr&0x10000000) 
		{
			if (datasize < 4) 
				break;
			extHdr = ntohl(*(unsigned*)(buffer)); 
			buffer = skip(buffer,4);
			datasize -=4;
			remExtSize = 4*(extHdr&0xFFFF);
			if (datasize < remExtSize) 
				break;
			buffer = skip(buffer,remExtSize);
			datasize -= remExtSize;
		}
    
		// padding bytes
		if (rtpHdr&0x20000000) 
		{
			if (datasize == 0) 
				break;
			numPaddingBytes	= (unsigned)(buffer)[datasize-1];
			if (datasize < numPaddingBytes) 
				break;
			if (numPaddingBytes > datasize) 
				numPaddingBytes = datasize;
			buffer = skip(buffer, numPaddingBytes);
			datasize -= numPaddingBytes;
		}    

		rtcpType = (unsigned char)((rtpHdr&0xFF0000)>>16);

		// seq
		rtpSeqNo = (unsigned short)(rtpHdr&0xFFFF);

		//payloadtype
		payloadType = (unsigned char)((rtpHdr&0x007F0000)>>16);
		
		if(payloadType == VPayloadType) //视频
		{
			struct ResultData * v_data = &m_VFrame;
			*data = v_data;

			if(v_data->buffer == NULL)
			{
				v_data->buffer = (unsigned char *)malloc(1024*500); //分配视频Buffer内存
			}

			if(v_data->cache_size > 0)  //将上一次缓存的数据移动到Buffer头部
			{
				memmove(v_data->buffer, v_data->buffer+v_data->len, v_data->cache_size);
				v_data->len = v_data->cache_size;
				v_data->cache_size  = 0;

				v_data->frtpTimestamp = LVrtpTimestamp; //上一次的时间戳
				v_data->cFrameType = LVFrameType; //上一次的帧类型
			}

			VideoFormat vFormat = GetVideoFormat();

			unsigned char cFrameType = '0'; //帧类型，目前只能区分I帧和非I帧
			int nIDR = 0;

			if(vFormat == VFMT_H264)
			{
#if 0
				int nWriteBytes = rtp_unpackage_H264(buffer, datasize, rtpMarkerBit, nIDR, m_buffer2, datasize2, rtpSeqNo); //重新组装H264的NALU单元, 该函数会检查RTP包有没有丢包
#else
				int nWriteBytes = h264_handle_packet(buffer, datasize,  nIDR, m_buffer2, datasize2);
#endif
				if(nWriteBytes <= 0)
				{
					//ASSERT(0);
					TRACE("Error: h264_handle_packet failed \n");
					break;
				}
			
				cFrameType = (nIDR == 5) ? 'I' : 'P';
			}
			else if(vFormat == VFMT_H265)
			{
				int nWriteBytes = hevc_handle_packet(buffer, datasize,  nIDR, m_buffer2, datasize2);
				if(nWriteBytes <= 0)
				{
					//ASSERT(0);
					TRACE("Error: h265_handle_packet failed \n");
					break;
				}
			
				cFrameType = (IS_IDR(nIDR) || IS_IRAP(nIDR)) ? 'I' : 'P';
			}
			else
			{
				cFrameType = 'P'; //  临时的做法，不支持所有格式获取帧类型，目前只支持H264和H265。
				memcpy(m_buffer2, buffer, datasize);
                datasize2 = datasize;
			}

			if(datasize2 <= 0)
			{
				TRACE("unpack wrong video data size! \n");
				break;
			}

			//if(poutfile != NULL)
			//{
			//  fwrite(buffer2,1,datasize2,poutfile);
			//}

			////////////////////////////////////////////////////////////////

			if(vloopNum == 0)
			{
				videoFirstTimestamp = rtpTimestamp;
				LVrtpTimestamp = rtpTimestamp; 
			}
			
			//if(rtpTimestamp<LVrtpTimestamp) //next packet's timestamp should not be lesser than the last one's
			//	break;

#if 1
			if(rtpTimestamp != LVrtpTimestamp) //不同PTS，新的一帧
			{
				m_bFrameContainSPS = FALSE; //新的一帧重置变量状态

				if(vFormat == VFMT_H264 || vFormat == VFMT_H265) //H264 or H265
				{
					if(vFormat == VFMT_H264)
					{
						if(nIDR == 7)
						  m_bFrameContainSPS = TRUE;
					}
					else if(vFormat == VFMT_H265)
					{
						if(nIDR == 33)
						  m_bFrameContainSPS = TRUE;
					}

					 if(!m_bFrameContainSPS && cFrameType == 'I') //如果当前帧没有包含SPS，则需要在I帧前插入SPS
					 {
						//I 帧前面插入SPS
						memcpy(v_data->buffer + v_data->len, m_sprop_parameter, m_sprop_len);
						memcpy(v_data->buffer + v_data->len + m_sprop_len, m_buffer2, datasize2);
						v_data->cache_size = m_sprop_len + datasize2;

						m_bFrameContainSPS = TRUE;
					 }
					 else
					 {
						memcpy(v_data->buffer + v_data->len, m_buffer2, datasize2);
						//v_data->len += datasize2;
						v_data->cache_size = datasize2;
					 }
				}
				else
				{
				   memcpy(v_data->buffer + v_data->len, m_buffer2, datasize2);
				   //v_data->len += datasize2;
					v_data->cache_size = datasize2;
				}


               //缓存新一帧的信息，新一帧的数据放在下次解码	        
				LVFrameType = cFrameType; // 帧类型
				LVrtpTimestamp = rtpTimestamp; //PTS
                
				nResult = 1; //返回完整的一帧
	
			}
			else //相同PTS表示两个包属于同一帧
			{ 

				if(vFormat == VFMT_H264 || vFormat == VFMT_H265) 
				{
					if(vFormat == VFMT_H264)
					{
						if(nIDR == 7)
						  m_bFrameContainSPS = TRUE;
					}
					else if(vFormat == VFMT_H265)
					{
						if(nIDR == 33)
						  m_bFrameContainSPS = TRUE;
					}

				   if(!m_bFrameContainSPS && cFrameType == 'I') //如果当前帧没有包含SPS，则需要在I帧前插入SPS
				   {
					   //I 帧前面插入SPS
						memcpy(v_data->buffer + v_data->len, m_sprop_parameter, m_sprop_len);
						v_data->len += m_sprop_len;
						memcpy(v_data->buffer + v_data->len, m_buffer2, datasize2);
						v_data->len += datasize2;

						m_bFrameContainSPS = TRUE;
				   }
				   else
				   {
				   	 memcpy(v_data->buffer + v_data->len, m_buffer2, datasize2); 
			         v_data->len += datasize2;
				   }
				}
				else
				{
				   memcpy(v_data->buffer + v_data->len, m_buffer2, datasize2); 
			       v_data->len += datasize2;
				}


				v_data->frtpTimestamp = rtpTimestamp;
				v_data->frtpMarkerBit = rtpMarkerBit;
				v_data->cFrameType = cFrameType;
			}
#else
			memcpy(v_data->buffer, buffer2, datasize2); //  拷贝视频数据
		    v_data->len = datasize2;
			nResult = 1; 
#endif
            v_data->fRTPPayloadFormat = payloadType;
			
			if(rtpTimestamp>videoFirstTimestamp&&!subTimeFlag)
			{
				if(!fixed_vop_rate)
				{
					subTime = rtpTimestamp-videoFirstTimestamp;
					time_inc = VTimestampFrequency/subTime;
					VFrameRate = VTimestampFrequency*1000/subTime;
					fprintf(stderr,"time_inc is %lu\n",time_inc);
					if(time_inc<24) 
					{
						time_inc = 24;
						VFrameRate = 24000*1000/1001;
					}
					fprintf(stderr,"subTime is %lu\n",subTime);
					fprintf(stderr,"time_inc is %lu\n",time_inc);
					MaxFrameNum = fMaxPlayEndTime*(time_inc*10-3)+90000;
				}
				else
				{
					MaxFrameNum = fMaxPlayEndTime*VFrameRate*10;	
				}
				subTimeFlag = 1;
                printf("MaxFrameNum=%lu\n",MaxFrameNum);
				fprintf(stderr,"VFrameRate is %lu\n",VFrameRate);
				fprintf(stderr,"fixed_vop_rate is %d\n",fixed_vop_rate);
			}
			if(rtpMarkerBit)
			{	
				vloopNum++;
			}

		}          
		else if(payloadType == APayloadType) //音频
		{
			struct ResultData * au_data = &m_AFrame;
            *data = au_data;

			if(au_data->buffer == NULL)
			{
				au_data->buffer = (unsigned char *)malloc(1024*192); //分配音频Buffer内存
			}

			if(aloopNum == 0)
			{
				audioFirstTimestamp = rtpTimestamp;
				LArtpTimestamp = rtpTimestamp;
			}
			//if(rtpTimestamp<LArtpTimestamp) 
			//	break;

			if(rtpTimestamp>=LArtpTimestamp)
			{
				LArtpTimestamp = rtpTimestamp;
			}

			if(lostInfo2.nLastNo > 0)
			{
				if(lostInfo2.nLastNo + 1 != rtpSeqNo)
				{
					lostInfo2.nLostCount++;
					TRACE("Audio Packets lost!\n");
				}
			}
			lostInfo2.nLastNo = rtpSeqNo;


			//ASSERT(rtpMarkerBit == 1);

			AudioFormat aFormat = GetAudioFormat();

			switch(aFormat)
			{
			case AFMT_PCM_RAW16:
				break;
			case AFMT_PCM_ULAW:
				break;
			case AFMT_AMR:
				nResult = rtp_unpackage_AMR(buffer,datasize,  m_buffer2, &datasize2);
				break;
			case AFMT_AAC:
				nResult = rtp_unpackage_AAC(buffer,datasize, rtpMarkerBit, m_Attribute.fAudioFrequency, m_buffer2, &datasize2);
				break;
			case AFMT_MP3:
				nResult = rtp_unpackage_MP3(buffer,datasize,  m_buffer2, &datasize2);
				break;
			default:
				break;
			}

			if(datasize2 <= 0)
			{
				TRACE("Error: unpack wrong audio packet size! \n");
				break;
			}

		//	datasize -=4;
		//	memcpy(buffer,buffer+4,datasize);
			memcpy(au_data->buffer,m_buffer2,datasize2); //  拷贝音频数据
			au_data->len = datasize2;
			au_data->fRTPPayloadFormat = payloadType;
			au_data->frtpTimestamp = rtpTimestamp;
			au_data->frtpMarkerBit = rtpMarkerBit;
			if(rtpMarkerBit) 
			{
				aloopNum++;
			}

			//nResult = 1;
		}
		else
		{
			TRACE("Not identified rtp format \n");
		}

		return nResult; //成功返回

	} while (0);
	
	return -1;

}



int RtspClient::parseResponseCode(char* line, unsigned int * responseCode) 
{
	if (sscanf(line, "%*s%u", responseCode) != 1) 
	{
		fprintf(stderr,"no response code in line\n");
		return -1;
	}

	return 0;
}


///////////////////////////////////////////

int RtspClient::getResponse(char* responseBuffer,unsigned responseBufferSize) 
{
	int fSocketNum;
	char *lastToCheck=NULL;
	char* p = NULL;//responseBuffer;
	int bytesRead = 0; // because we've already read the first byte
	int  bytesReadNow  = 0;
	char * pBufferHead = NULL;
	char * pBufferEnd = NULL;

	fSocketNum = m_socketNum;

	if (responseBufferSize == 0) return 0; // just in case...
	*(responseBuffer) = '\0';

	pBufferHead = responseBuffer;

	while (bytesRead < (int)responseBufferSize) 
	{
		lastToCheck = NULL;
		if(blockUntilReadable(fSocketNum)<=0)
		{
			fprintf(stderr,"socket is unreadable\n");
			TRACE("socket is unreadable\n");
			break;
		}
		bytesReadNow = recv(fSocketNum,(char*)(responseBuffer+bytesRead),1, 0);
		if (bytesReadNow != 1) 
		{
			fprintf(stderr,"RTSP response was truncated\n");
			break;
		}
		bytesRead++;
    
  
		lastToCheck = responseBuffer+bytesRead-4;
		if (lastToCheck < responseBuffer) continue;
		p = lastToCheck;
		if (*p == '\r' && *(p+1) == '\n' &&
						*(p+2) == '\r' && *(p+3) == '\n') 
		{
			*(responseBuffer+bytesRead)= '\0';

			pBufferEnd = responseBuffer+bytesRead-1;

			// Before returning, trim any \r or \n from the start:
      
			while(responseBuffer < pBufferEnd)
			{
				if (*responseBuffer == '\r' || *responseBuffer == '\n'||*responseBuffer!=0x52)  // 'R' = 0x52
				{
					++responseBuffer;
					--bytesRead;
					continue;
				}
				 //  查找RTSP应答的标志: 以字符串RTSP/1.0 开头
				if(*responseBuffer == 'R' && responseBuffer[1]=='T' && responseBuffer[2]=='S'&& responseBuffer[3]=='P')
				{
                     break;
				} 
				else
				{
					++responseBuffer;
					--bytesRead;
				}
			}

			if(pBufferHead != responseBuffer)
			{
			  memmove(pBufferHead, responseBuffer, bytesRead);
			}

			return bytesRead;
		}
	}
	return bytesRead;
}
/////////////////////
char* RtspClient::parseSDPLine(char* inputLine)
{
	char *ptr;
	for (ptr = inputLine; *ptr != '\0'; ++ptr) 
	{
		if (*ptr == '\r' || *ptr == '\n') 
		{
			++ptr;
			while (*ptr == '\r' || *ptr == '\n') ++ptr;
			if (ptr[0] == '\0') ptr = NULL; // special case for end
			break;
		}
	}

	if (inputLine[0] == '\r' || inputLine[0] == '\n') return ptr;
	if (strlen(inputLine) < 2 || inputLine[1] != '='
		    || inputLine[0] < 'a' || inputLine[0] > 'z') 
	{
		fprintf(stderr,"Invalid SDP line \n");
		return NULL;
	}

	return ptr;
}

char * RtspClient::parseSDPAttribute_rtpmap(char* sdpLine,unsigned* rtpTimestampFrequency,unsigned *fnumChannels) 
{
	unsigned rtpmapPayloadFormat;
	unsigned numChannels = 1;
	char* codecName = strDupSize(sdpLine); // ensures we have enough space
	if (sscanf(sdpLine, "a=rtpmap: %u %[^/]/%u/%u", &rtpmapPayloadFormat, codecName, rtpTimestampFrequency, &numChannels) == 4
		|| sscanf(sdpLine, "a=rtpmap: %u %[^/]/%u", &rtpmapPayloadFormat, codecName, rtpTimestampFrequency) == 3
		|| sscanf(sdpLine, "a=rtpmap: %u %s", &rtpmapPayloadFormat, codecName) == 2) 
	{
		*fnumChannels = numChannels;  
		return (codecName);

	}
	free(codecName);

	return NULL;
}

char * RtspClient::parseSDPAttribute_control(char* sdpLine) 
{
	char* controlPath = strDupSize(sdpLine); // ensures we have enough space
  
	if (sscanf(sdpLine, "a=control:%s", controlPath) == 1)   
	{
             //  printf("ControlPath\n"); 
		return (controlPath);         
	}
   
	free(controlPath);
         
	return NULL;
}

int RtspClient::parseSDPAttribute_range(char* sdpLine) 
{
	int parseSuccess = -1;
	char playEndTime[32];
	char temp[32];
	int index = 0;
	int j = 0;
	unsigned long endtime = 0;
	char *endstr;
	int k;
	int i,num;
	if (sscanf(sdpLine, "a=range:npt=0-  %s", playEndTime)==1) 
	{
		parseSuccess = 0;
		for(index = 0;index<strlen(playEndTime);index++)
		{
			if(playEndTime[index]!='.')
			{
				temp[j] = playEndTime[index];
				j++;
			}
			else
			{
				for(k=index+1;k<strlen(playEndTime);k++)
				{
					temp[j] = playEndTime[k];
					j++;
					PotPos++;
				}
				break;
			}

		}
		temp[j] = '\0';
		
		
		endtime = strtoul(temp,&endstr,10);

		if (endtime > fMaxPlayEndTime) 
		{
			if(PotPos>=3)
			{
				num = 1;
				for(i = 0;i<PotPos-3;i++)
				{
					num = 10*num;
				}
				fMaxPlayEndTime = endtime/num;
			}
			else
			{
				num = 1;
				for(i = 0;i<3-PotPos;i++)
				{
					num = 10*num;
				}
				fMaxPlayEndTime = endtime*num;
			}
			PotPos = 1000;
			
		}
		fprintf(stderr,"fMaxPlayEndTime is %lu\n",fMaxPlayEndTime);
	}

	return parseSuccess;
}


char * RtspClient::parseSDPAttribute_fmtp(char* sdpLine) 
{
	char* lineCopy;
	char* line;
	char* valueStr;
	char* c;
	do 
	{
		if (strncmp(sdpLine, "a=fmtp:", 7) != 0) 
			break; 
		sdpLine += 7;
		while (isdigit(*sdpLine)) ++sdpLine;
		
		lineCopy = strDup(sdpLine); 
		line = lineCopy;
		for (c = line; *c != '\0'; ++c) *c = tolower(*c);
		
		while (*line != '\0' && *line != '\r' && *line != '\n') 
		{
			valueStr = strDupSize(line);
			if (sscanf(line, " config = %[^; \t\r\n]", valueStr) == 1) 
			{
				free(lineCopy);
				
				return (valueStr);
			}
			free(valueStr);
			
			while (*line != '\0' && *line != '\r' && *line != '\n'&& *line != ';') ++line;
			while (*line == ';') ++line;
		}
		free(lineCopy);
	} while (0);
	return NULL;
}



 MediaAttribute * RtspClient::SetMediaAttrbute(struct MediaAttribute *Attribute,struct MediaSubsession * subsessionHead,int subsessionNum)
{
	int fd,size;
	//FILE *fd;
	struct MediaSubsession *mediasub;
	mediasub = subsessionHead;

	while(subsessionNum>0)
	{
		if(stricmp(mediasub->fCodecName,"H264") == 0)
		{
			Attribute->fVideoFormat = VFMT_H264;
		}
		else if(stricmp(mediasub->fCodecName,"MP4V-ES") == 0)
		{
			Attribute->fVideoFormat = VFMT_MPEG4;
		}
	    else if(stricmp(mediasub->fCodecName, "HEVC") == 0 ||
		      stricmp(mediasub->fCodecName, "H265") == 0)
		{
			Attribute->fVideoFormat = VFMT_H265;
		}

		if(Attribute->fVideoFormat == VFMT_H264 || 
		   Attribute->fVideoFormat == VFMT_MPEG4 || 
		   Attribute->fVideoFormat == VFMT_H265
			)
		{	

			VTimestampFrequency = mediasub->frtpTimestampFrequency;
			VPayloadType = mediasub->fRTPPayloadFormat;

			Attribute->fVideoFrequency = mediasub->frtpTimestampFrequency;
			Attribute->fVideoPayloadFormat = mediasub->fRTPPayloadFormat;
			Attribute->fVideoWidth = mediasub->fWidth; //获得分辨率
			Attribute->fVideoHeight = mediasub->fHeight;

			if( mediasub->fConfig !=NULL)
			{
			   int len = strlen(mediasub->fConfig);
				
				memcpy(Attribute->fConfigAsc,mediasub->fConfig,len);
				a_hex(mediasub->fConfig,Attribute->fConfigHex,len); //将字符串转成16进制数字
				Attribute->fConfigHexLen = len/2;

				len =strlen(mediasub->fConfig)/2;

				//if(Attribute->fVideoFormat == VFMT_MPEG4) //从SDP的Config字段中获取MPEG4的分辨率信息
				//{
				//	MPEG4_CONFIG_DATA mcd;
				//	memset(&mcd, 0, sizeof(MPEG4_CONFIG_DATA));

				//	mcd.bGetWH = TRUE;

				//	GetFrameWidthHeight(MPEG4_DEC, Attribute->fConfigHex,Attribute->fConfigHexLen, &mcd); //解析分辨率信息
				//	if(mcd.width > 0 && mcd.height > 0 && mcd.width < 2000 && mcd.height < 1200)
				//	{
				//		Attribute->fVideoWidth = mcd.width;
				//		Attribute->fVideoHeight = mcd.height;
				//	}
				//}

#if 0
				if(Attribute->fVideoFormat == VFMT_H264)//for H264 only
				{
					//提取帧率、分辨率信息
				   parse_vovod(Attribute->fConfigHex,Attribute->fConfigHexLen,&(Attribute->fVideoFrameRate),&(Attribute->fTimeIncBits),&(Attribute->fVideoWidth),&(Attribute->fVideoHeight));
				}
#endif

			  

			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			Attribute->fixed_vop_rate = fixed_vop_rate;
			VFrameRate = Attribute->fVideoFrameRate;

		}
		
		else if(stricmp(mediasub->fCodecName,"MPA") == 0||
			stricmp(mediasub->fCodecName,"mpeg4-generic") == 0||
			stricmp(mediasub->fCodecName,"L16") == 0 ||
			stricmp(mediasub->fCodecName, "AMR") == 0
			)
		{	
			ATimestampFrequency = mediasub->frtpTimestampFrequency;
			APayloadType = mediasub->fRTPPayloadFormat;

			if(stricmp(mediasub->fCodecName,"MPA") == 0)
			{
				Attribute->fAudioFormat = AFMT_MP3;
			}
			else if(stricmp(mediasub->fCodecName,"mpeg4-generic") == 0)
			{
				Attribute->fAudioFormat = AFMT_AAC;
			}
			else if(stricmp(mediasub->fCodecName, "AMR") == 0)
			{
				Attribute->fAudioFormat = AFMT_AMR;
				ASSERT(mediasub->fNumChannels == 1);
			}

			Attribute->fAudioFrequency = mediasub->frtpTimestampFrequency; //音频采样率
			Attribute->fAudioPayloadFormat = mediasub->fRTPPayloadFormat;
			Attribute->fTrackNum = mediasub->fNumChannels;
			Attribute->fAudioChannels = mediasub->fNumChannels; // 音频声道数

		}	
		mediasub = mediasub->fNext;
		subsessionNum--;
	}
	return Attribute;
}

MediaSubsession *  RtspClient::initializeWithSDP(char* sdpDescription,int *SubsessionNum)
{
	int Num = 0;
	char*  sdpLine = sdpDescription;
	char* nextSDPLine = NULL;
	struct MediaSubsession *fSubsessionsHead;
	struct MediaSubsession *fSubsessionsTail;
	struct MediaSubsession* subsession;
	char* mediumName;
	char * CodecName;
	char * ControlPath;
	char *Config;
	unsigned payloadFormat;
	if (sdpDescription == NULL) return NULL;
	fSubsessionsHead = fSubsessionsTail = NULL;
	while (1) 
	{
		if ((nextSDPLine = parseSDPLine(sdpLine)) == NULL)
		{
			return NULL;		
		}
		if (sdpLine[0] == 'm') 
		{
			Num++;
			break;
		}
		sdpLine = nextSDPLine;
		if (sdpLine == NULL) break; // there are no m= lines at all 
		if (!parseSDPAttribute_range(sdpLine)) continue;//check a=range:npt=
	}
  
	while (sdpLine != NULL) 
	{
		subsession = (struct MediaSubsession*)malloc(sizeof(struct MediaSubsession));
		memset(subsession, 0, sizeof(struct MediaSubsession)); //为结构体清零

		subsession->fMediumName =NULL;
		subsession->fCodecName =NULL;
		subsession->fControlPath = NULL;
		subsession->fConfig = NULL;
		subsession->fFileName =NULL;
		if (subsession == NULL) 
		{
			fprintf(stderr,"Unable to create new MediaSubsession\n");
			return NULL;
		}
		
		if (fSubsessionsTail == NULL) 
		{
			fSubsessionsHead = fSubsessionsTail = subsession;
		} 
		else 
		{
			fSubsessionsTail->fNext = subsession;
			subsession->fNext = NULL;
			fSubsessionsTail = subsession;
		}

		mediumName = strDupSize(sdpLine);

		if (sscanf(sdpLine, "m=%s %hu RTP/AVP %u",mediumName, &subsession->fClientPortNum, &payloadFormat) != 3
			|| payloadFormat > 127) 
		{
			fprintf(stderr,"Bad SDP line\n");
			free(mediumName);
			return NULL;
		}
		
		subsession->fMediumName = strDup(mediumName);
		free(mediumName);

		subsession->fRTPPayloadFormat = payloadFormat;
		
		while (1) 
		{
			sdpLine = nextSDPLine;//a=rtpmap...
			if (sdpLine == NULL) 
				break; // we've reached the end

			//if ((nextSDPLine = parseSDPLine(sdpLine)) == NULL)
			//{
			//	*SubsessionNum = Num;
			//	return fSubsessionsHead;			
			//}

			nextSDPLine = parseSDPLine(sdpLine);

			if (sdpLine[0] == 'm') 
			{
				Num ++;
				break; // we've reached the next subsession
			}
			// Check for various special SDP lines that we understand:
			//CodecName = strDupSize(sdpLine);
			if ((CodecName = parseSDPAttribute_rtpmap(sdpLine,&(subsession->frtpTimestampFrequency),&(subsession->fNumChannels)))!=NULL)
			{
				subsession->fCodecName = strDup(CodecName);
				
				//if(stricmp(CodecName,"H264") == 0||stricmp(CodecName,"MP4V-ES") == 0)
				//{	
				//	VTimestampFrequency = subsession->frtpTimestampFrequency;
				//	VPayloadType = subsession->fRTPPayloadFormat;
				//}
				//else if(stricmp(CodecName,"MPA") == 0||
				//	stricmp(CodecName,"mpeg4-generic") == 0||
				//	stricmp(CodecName,"L16") == 0 ||
				//	stricmp(CodecName, "AMR") == 0)
				//{	
				//	ATimestampFrequency = subsession->frtpTimestampFrequency;
				//	APayloadType = subsession->fRTPPayloadFormat;
				//}

				free(CodecName);
				continue;
			}
			
			//ControlPath = strDupSize(sdpLine);	
			if ((ControlPath = parseSDPAttribute_control(sdpLine))!=NULL)
			{	
				subsession->fControlPath = strDup(ControlPath);
				free(ControlPath);
				continue;
			}
			
			//Config = strDupSize(sdpLine);
			if ((Config = parseSDPAttribute_fmtp(sdpLine))!=NULL)
			{
				subsession->fConfig = strDup(Config);
				free(Config);
				continue;
			}

		   if(sdpLine != NULL && m_sprop_len <= 0)
		   {
			   //从sprop-parameter-sets=字段中提取SPS内容
	
				if(	GetSpropParameterSets_h264((char*)sdpLine, m_sprop_parameter, m_sprop_len) > 0||
					GetSpropParameterSets_h265((char*)sdpLine, m_sprop_parameter, m_sprop_len) > 0 )
				{
					TRACE("GetSpropParameterSets return len: %d \n", m_sprop_len);

					//FILE * file = fopen("d:\\sps.dat","ab" );
					//fwrite(m_sprop_parameter, 1, m_sprop_len, file);
					//fclose(file);


					if(m_sprop_parameter[0]==0 && m_sprop_parameter[1]==0 && m_sprop_parameter[2]==0  && m_sprop_parameter[3] == 0x01) //检查开头4个字节是不是H264开始码
					{

					}

					subsession->fWidth = subsession->fHeight = 0;

					SpsDecodeParser spsParser;
					spsParser.h264_decode_sps( m_sprop_parameter+4, m_sprop_len-4, subsession->fWidth, subsession->fHeight); //从SPS中拿到视频分辨率 

					TRACE("h264_decode_sps, fWidth = %d, fHeight = %d \n", subsession->fWidth, subsession->fHeight);
				}
		   }

		}	//while

		//  如果不知道CodeName，根据PayloadType查找对应的默认名称
		if (subsession->fCodecName == NULL) 
		{
		  subsession->fCodecName = lookupPayloadFormat(subsession->fRTPPayloadFormat,
					  subsession->frtpTimestampFrequency,
					  subsession->fNumChannels);

		  if (subsession->fCodecName == NULL) 
		  {
			fprintf(stderr, "Unknown codec name for RTP payload type: %d \n", subsession->fRTPPayloadFormat);
		  }
		}

		// PP: 以下指针变量值不能为NULL
		if(subsession->fMediumName == NULL) 
			subsession->fMediumName = strDup("");
		if(subsession->fCodecName == NULL)
			subsession->fCodecName = strDup("");
		if(subsession->fControlPath == NULL) 
			subsession->fControlPath = strDup("");

	
	}//while

	*SubsessionNum = Num;
	return fSubsessionsHead;
}

//  根据RTP协议的Playload类型获得对应的编码器属性
char* RtspClient::lookupPayloadFormat(unsigned char rtpPayloadType, unsigned& freq, unsigned& nCh) 
{
  // Look up the codec name and timestamp frequency for known (static)
  // RTP payload formats.
  char * temp = NULL;
  switch (rtpPayloadType) {
  case 0: {temp = "PCMU"; freq = 8000; nCh = 1; break;}
  case 2: {temp = "G726-32"; freq = 8000; nCh = 1; break;}
  case 3: {temp = "GSM"; freq = 8000; nCh = 1; break;}
  case 4: {temp = "G723"; freq = 8000; nCh = 1; break;}
  case 5: {temp = "DVI4"; freq = 8000; nCh = 1; break;}
  case 6: {temp = "DVI4"; freq = 16000; nCh = 1; break;}
  case 7: {temp = "LPC"; freq = 8000; nCh = 1; break;}
  case 8: {temp = "PCMA"; freq = 8000; nCh = 1; break;}
  case 9: {temp = "G722"; freq = 8000; nCh = 1; break;}
  case 10: {temp = "L16"; freq = 44100; nCh = 2; break;}
  case 11: {temp = "L16"; freq = 44100; nCh = 1; break;}
  case 12: {temp = "QCELP"; freq = 8000; nCh = 1; break;}
  case 14: {temp = "MPA"; freq = 90000; nCh = 1; break;} //  这些值可能有很多组合
    // 'number of channels' is actually encoded in the media stream
  case 15: {temp = "G728"; freq = 8000; nCh = 1; break;}
  case 16: {temp = "DVI4"; freq = 11025; nCh = 1; break;}
  case 17: {temp = "DVI4"; freq = 22050; nCh = 1; break;}
  case 18: {temp = "G729"; freq = 8000; nCh = 1; break;}
  case 25: {temp = "CELB"; freq = 90000; nCh = 1; break;}
  case 26: {temp = "JPEG"; freq = 90000; nCh = 1; break;}
  case 28: {temp = "NV"; freq = 90000; nCh = 1; break;}
  case 31: {temp = "H261"; freq = 90000; nCh = 1; break;}
  case 32: {temp = "MPV"; freq = 90000; nCh = 1; break;}
  case 33: {temp = "MP2T"; freq = 90000; nCh = 1; break;}
  case 34: {temp = "H263"; freq = 90000; nCh = 1; break;}
  };

  return strDup(temp);
}

int RtspClient::blockUntilwriteable(int socket)
{
	unsigned numFds;
	fd_set wd_set;
	int result = -1;

	struct timeval selectTimeout;

	selectTimeout.tv_sec  = 8;
	selectTimeout.tv_usec = 100*1000; //block 8100ms at maximum

	do 
	{
		FD_ZERO(&wd_set);
		if (socket < 0) break;
		FD_SET((unsigned) socket, &wd_set);
		numFds = socket+1;		
		result = select(numFds, NULL, &wd_set, NULL, &selectTimeout);
		if (result == 0)
		{
			TRACE("blockUntilwriteable Timeout \n");
			break; // this is OK - timeout occurred
		}
		else if(result <= 0) 
		{
			fprintf(stderr, "select() error\n ");
			break;
		}

		if (!FD_ISSET(socket, &wd_set)) 
		{
			fprintf(stderr, "select() error - !FD_ISSET\n");
			break;
		}
	} while (0);
	return result;
}


int RtspClient::blockUntilReadable(int socket)
{
	fd_set rd_set;
	int result = -1;
	unsigned numFds;
	
	struct timeval selectTimeout;

	selectTimeout.tv_sec  = 8;
	selectTimeout.tv_usec = 100*1000; //block 8100ms at maximum

	do 
	{
		FD_ZERO(&rd_set);
		if (socket < 0) break;
		FD_SET((unsigned) socket, &rd_set);
		numFds = socket+1;
		result = select(numFds, &rd_set, NULL, NULL, &selectTimeout);
		if (result == 0) 
		{
			TRACE("blockUntilReadable Timeout \n");
			break; // this is OK - timeout occurred
		} 
		else if (result <= 0) 
		{
			fprintf(stderr, "select() error\n ");
			break;
		}
    
		if (!FD_ISSET(socket, &rd_set)) 
		{
			fprintf(stderr, "select() error - !FD_ISSET\n");
			break;
		}	
	} while (0);

	return result;
}


int RtspClient::getSDPDescriptionFromURL(char* url,char* Description)
{
	int fSocketNum;
	char *readBuffer;//[MAX_READBUFSIZE+1]; 
	char* readBuf;

	int bytesRead;

	//unsigned cmdSize = 0;										
	char cmd[8*1024] = {0};
	char* firstLine;
	char* nextLineStart;
	unsigned responseCode;
	int contentLength = -1;
	char* lineStart;  
	unsigned numExtraBytesNeeded;
	char* ptr;
	int bytesRead2;
	
	readBuffer = (char *)malloc((MAX_READBUFSIZE+1)*sizeof(char));
	if(readBuffer == NULL)
	{
		fprintf(stderr,"failed to alloc the memory\n");
    		return -1;
	}
	memset(readBuffer,0,MAX_READBUFSIZE+1);
	readBuf =readBuffer;
	

	//fprintf(stderr,"alloc cmd successful\n");
	memset(cmd,0, sizeof(cmd));
	fSocketNum = m_socketNum;

	int nRequestTimes = 0;

	do 
	{  
 
		nRequestTimes++;

		// Send the DESCRIBE command:
		//fprintf(stderr,"Send the DESCRIBE command\n");

		if(!m_bUserAuth)
		{
			char* cmdFmt ="DESCRIBE %s RTSP/1.0\r\nCSeq: %d\r\nAccept: application/sdp\r\n%s\r\n";
		    sprintf(cmd, cmdFmt,url,++fCSeq,UserAgentHeaderStr);
		}
		else
		{
			char* const cmdFmt = "DESCRIBE %s RTSP/1.0\r\n"
				"CSeq: %d\r\n"
                "Accept: application/sdp\r\n"
				"%s"
				"%s\r\n";

			char StrAuth[1024] = {0};
            GetAuthor( m_realm, m_nonce, m_authType, m_username, m_password, fBaseURL, StrAuth ,"DESCRIBE");

			sprintf(cmd, cmdFmt,url,++fCSeq, UserAgentHeaderStr,StrAuth);
		}

  
		//fprintf(stderr,"start blockUntilwriteable\n");
  		if(blockUntilwriteable(fSocketNum)<=0)
  		{ 
  			fprintf(stderr,"socket is unwriteable\n");
  			break;
  		}
  		//fprintf(stderr,"start send\n");
  		if (send(fSocketNum, cmd, strlen(cmd), 0)<0) 
		{
			fprintf(stderr,"DESCRIBE send() failed\n ");
			break;
		}
		//fprintf(stderr,"DESCRIBE send() successful\n");
			// Get the response from the server:
		//fprintf(stderr,"Get the response from the server\n");
		bytesRead = getResponse(readBuf, MAX_READBUFSIZE);
		if (bytesRead <= 0) break;
		
		//fprintf(stderr,"DESCRIBE response-%d:\n%s\n",fCSeq,readBuf);
		//Received DESCRIBE response
    
		// Inspect the first line to check whether it's a result code that we can handle.

		firstLine = readBuf;
		nextLineStart = getLine(firstLine); //读服务器返回的应答信息的第一行内容，这一行包含状态码
		if (sscanf(firstLine, "%*s%u", &responseCode) != 1) 
		{
			fprintf(stderr,"no response code in line\n");
			break;
		}
		if (responseCode != 200 && responseCode != 401) 
		{
			fprintf(stderr,"cannot handle DESCRIBE response\n");
			break;
		}

		if(responseCode == 401 && nRequestTimes < 2) //服务器需要登录认证
		{
			while (1) 
			{
				char * pChar = NULL;

				lineStart = nextLineStart;
				if (lineStart == NULL||lineStart[0] == '\0') break;

				nextLineStart = getLine(lineStart);

				pChar = strstr( lineStart, "WWW-Authenticate:" );
				if(pChar != NULL)
				{
					m_authType = GetAuthParam((char*)lineStart, m_realm, m_nonce);
				}

			 }

			if(m_authType == 1 || m_authType == 2)
			{
				m_bUserAuth = TRUE;
			}

			continue;
		}

		// Skip every subsequent header line, until we see a blank line
		// The remaining data is assumed to be the SDP descriptor that we want.
		// We should really do some checking on the headers here - e.g., to
		// check for "Content-type: application/sdp", "Content-base",
		// "Content-location", "CSeq", etc. #####
		//fprintf(stderr,"start get Content-Length\n");
		while (1) 
		{
			lineStart = nextLineStart;
			if (lineStart == NULL||lineStart[0] == '\0') break;

			nextLineStart = getLine(lineStart);
			if (sscanf(lineStart, "Content-Length: %d", &contentLength) == 1
			 || sscanf(lineStart, "Content-length: %d", &contentLength) == 1) 
			if(contentLength>0)
			{
				if (contentLength < 0) 
				{
					fprintf(stderr,"Bad Content-length\n");
					break;
				}
				// Use the remaining data as the SDP descr, but first, check
				// the "Content-length:" header (if any) that we saw.  We may need to
				// read more data, or we may have extraneous data in the buffer.
				//bodyStart = nextLineStart;
				// We saw a "Content-length:" header
				
				//numBodyBytes = &readBuf[bytesRead] - bodyStart;
				//if (contentLength > (int)numBodyBytes) 
				//{
				// We need to read more data.  First, make sure we have enough
				// space for it:
				numExtraBytesNeeded = contentLength;// - numBodyBytes;
			
				// Keep reading more data until we have enough:
				bytesRead = 0;
				//fprintf(stderr,"start recv %d\n",contentLength);
				while (numExtraBytesNeeded > 0) 
				{
					if(blockUntilReadable(fSocketNum)<=0)
    					{
    						fprintf(stderr,"socket is unreadable\n");
    						break;
    					}
    					//fprintf(stderr,"start recv\n");
					ptr = Description+bytesRead;
					//ptr = &readBuf[bytesRead];
					bytesRead2 = recv(fSocketNum, (char*)ptr,numExtraBytesNeeded,0);
					if (bytesRead2 <0) break;
					ptr[bytesRead2] = '\0';
						

					bytesRead += bytesRead2;
					numExtraBytesNeeded -= bytesRead2;
				}
				if (numExtraBytesNeeded > 0) break; // one of the reads failed
			
				free(readBuffer);
	
				return 0; //正确应该在这里返回
			}
		}
	}while(nRequestTimes < 2);

	free(readBuffer);

	Description = NULL;
	return -1; //错误返回
}

char* RtspClient::getLine(char* startOfLine) 
{
	// returns the start of the next line, or NULL if none
	char* ptr;
	for (ptr = startOfLine; *ptr != '\0'; ++ptr) 
	{
		if (*ptr == '\r' || *ptr == '\n') 
		{
			// We found the end of the line
			*ptr++ = '\0';
			if (*ptr == '\n') ++ptr;
			return ptr;
		}
	}

	return NULL;
}

int RtspClient::openConnectionFromURL(char* url)
{
	unsigned short ver;
	WSADATA data;
	int nErr;
	struct hostent *hp;
	char address[100];
	int destPortNum;
	struct sockaddr_in server;
	int fSocketNum = -1;
	if (url == NULL) return -1;
	memset(address,0,100);
	if (parseRTSPURL(url, address, &destPortNum)) return -1;
  
 
	ver = MAKEWORD( 2, 0 );
	nErr = WSAStartup( ver, &data );
	if ( nErr < 0 )
	{
		return -1;
	}
	fSocketNum = socket(AF_INET, SOCK_STREAM, 0);
  
	if (fSocketNum < 0) 
	{
		fprintf(stderr,"Client: Error Opening socket\n");
		return -1;
	}
	hp = gethostbyname(address);
	if (hp == NULL ) 
	{
		fprintf(stderr,"Client: Cannot resolve address [%s]\n",address);
		return -1;
	}
	
	 //设置接收缓冲区大小
	int nRecvBuf = 1000*1024; //1M
    setsockopt( fSocketNum, SOL_SOCKET,SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));

	//设置接收超时时间
	int timeout = 15*1000; //15秒
	if (setsockopt(fSocketNum, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
	{
	   TRACE("Set Recv timeout Error\n");
	}

	//设置发送超时时间
	timeout = 10*1000; //10秒
	if (setsockopt(fSocketNum, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
	{
	   TRACE("Set Send timeout Error\n");
	}

#if 0
	memset(&server,0,sizeof(struct sockaddr_in));

	memcpy(&(server.sin_addr),hp->h_addr,hp->h_length);
	server.sin_family = AF_INET;
	server.sin_port = htons((unsigned short)destPortNum);

	if (connect(fSocketNum, (struct sockaddr*)&server, sizeof(struct sockaddr_in))!= 0) 
	{
		fprintf(stderr,"connect() failed\n");
		closesocket(fSocketNum);
		//close(fSocketNum);
		return -1;
	}
#else
   const int nMaxConnectTimes = 2;
   bool bConnected = false;
   for(int i=0; i<nMaxConnectTimes; i++) 
   {
	   if(m_bStopThread)
		   break;

		if( netRTSPConnect(fSocketNum, inet_addr(address), destPortNum, 1) != 0)  //连接失败
		{
			fprintf(stderr,"connect() failed\n");
			
		    Sleep(200);
			continue;
		}
		else //连接成功
		{
			bConnected = true;
			break;
		}
   }
	if(!bConnected)
	{
		closesocket(fSocketNum);
		TRACE("connect rtsp server failed. \n");
		return -1;
	}
#endif

	//fprintf(stderr,"connect() successful \n");
	TRACE("connect rtsp successful \n");

	return fSocketNum;
}

#define CFG_NONBLOCK_TIMEOUT //设置连接超时，避免连接服务器等待时间太长

//函数作用: 异步连接RTSP服务器
//ip --   服务器的IP
//port -- 服务器的端口，默认为554
//nTimeOut--连接的超时时间，单位：秒
//
int RtspClient::netRTSPConnect(SOCKET fd, uint32_t ip, uint32_t port, uint32_t nTimeOut)
{
	struct sockaddr_in to;
	int ret;
	BOOL bret = FALSE; //连接是否成功的标志
	int on = 1;
#ifdef CFG_NONBLOCK_TIMEOUT
	struct timeval tm;
	fd_set set;
	unsigned long ul = 1;
#endif
	
#ifdef CFG_NONBLOCK_TIMEOUT
	ioctlsocket(fd, FIONBIO, &ul); //设置Socket为非阻塞，连接了服务器后将Socket重置为阻塞
#endif

	//setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof (on));

	memset(&to, 0, sizeof(struct sockaddr_in));
	to.sin_family = AF_INET;
	to.sin_port = htons((uint16_t)port);
	to.sin_addr.s_addr = ip;
	ret = connect(fd, (struct sockaddr *)&to, sizeof(struct sockaddr_in));

	if(ret != 0)
	{
		tm.tv_sec = nTimeOut;
		tm.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(fd, &set);
		if(select(fd+1, NULL, &set, NULL, &tm) > 0)
		{
			int len = sizeof(BOOL);
			BOOL error = 0;
			getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
			if(error == 0)
			   bret = TRUE;
			 else
			   bret = FALSE;
		}
		else
		{
		  bret = FALSE;
		}
	}
	else
	{
		bret = TRUE;
	}

#ifdef CFG_NONBLOCK_TIMEOUT
	ul = 0;
	ioctlsocket(fd, FIONBIO, &ul); //设置为阻塞
#endif
	return bret ? 0 : -1;
}

int RtspClient::parseRTSPURL(char* url,char * address,int* portNum) 
{
	
	char const* prefix = "rtsp://";
	unsigned const prefixLength = 7;
	char* from = NULL ;
	char* to = NULL;
	unsigned i;
	char nextChar;
	if (strncmp(url, prefix, prefixLength) != 0) 
	{
		fprintf(stderr,"URL is not of the form\n");
		return -1;
	}
		
	from = &url[prefixLength];
	to = &address[0];
	for (i = 0; i < parseBufferSize; ++i) 
	{
		if (*from == '\0' || *from == ':' || *from == '/') 
		{
	// We've completed parsing the address
			*to = '\0';
			break;
		}
		*to++ = *from++;
	}
	if (i == parseBufferSize) 
	{
		fprintf(stderr,"URL is too long\n");
		return -1;
	}

 	*portNum = 554; // default value
	nextChar = *from;
	if (nextChar == ':') 
	{
		int portNumInt;
		if (sscanf(++from, "%d", &portNumInt) != 1) 
		{
			fprintf(stderr,"No port number follows :%d\n",portNumInt);
			return -1;
		}
		if (portNumInt < 1 || portNumInt > 65535) 
		{
			fprintf(stderr,"Bad port number\n");
			return -1;
		}
		*portNum = portNumInt;
	}
	//fprintf(stderr,"address is %s;portNum is %d \n",address,*portNum);
	return 0;
}

////////////////////////

int RtspClient::setupMediaSubsession(struct MediaSubsession* subsession,int subsessionNum) 
{

	int fSocketNum = m_socketNum;

	if(fSocketNum<0) return -1;

	char* lineStart;
	char cmd[2048] = {0};

	unsigned const readBufSize = 10000;
	char *readBuffer;//[readBufSize+1]; 
	char* readBuf ;

	char* firstLine;
	char* nextLineStart;
	unsigned responseCode;
	int bytesRead;
	char SessionId[64] = {0};
	char Sessionstr[128] = {0};
	char SessionTrackURL[1024] = {0};
	
	readBuffer = (char *)malloc((readBufSize+1)*sizeof(char));
 
	memset(readBuffer,0,readBufSize+1);
	readBuf = readBuffer;
	do 
	{
		if (strlen(fLastSessionId) > 0) 
		{
			sprintf(Sessionstr, "Session: %s\r\n", fLastSessionId);
		} 
		else 
		{
			strcpy(Sessionstr, "");
		}
		
		m_rtcpNumber = m_rtpNumber + 1;
		
		memset(cmd,0, sizeof(cmd));
		//   printf("subsession->fControlPath=%s\n",subsession->fControlPath);
		
		if(_strnicmp(subsession->fControlPath, "rtsp://", 7) == 0)
		{
			strcpy(SessionTrackURL, subsession->fControlPath);
		}
		else
		{
			sprintf(SessionTrackURL, "%s/%s", fBaseURL, subsession->fControlPath); 
		}

		if(!m_bUserAuth)
		{
			char cmdFmt[]= "SETUP %s RTSP/1.0\r\n"
			"CSeq: %d\r\n"
			"Transport: RTP/AVP/TCP;unicast;interleaved=%d-%d\r\n"
			"%s"
			"%s\r\n";

			sprintf(cmd, cmdFmt,
					SessionTrackURL,
					++fCSeq,
					m_rtpNumber, m_rtcpNumber,UserAgentHeaderStr,Sessionstr);
			
		}
		else 
		{   

			char cmdFmt[]= "SETUP %s RTSP/1.0\r\n"
				"CSeq: %d\r\n"
				"Transport: RTP/AVP/TCP;unicast;interleaved=%d-%d\r\n"
				"%s"
				"%s"
				"%s\r\n";

			char StrAuth[1024] = {0};
            GetAuthor( m_realm, m_nonce, m_authType, m_username, m_password, fBaseURL, StrAuth ,"SETUP");

			sprintf(cmd, cmdFmt,
					SessionTrackURL,
					++fCSeq,
					m_rtpNumber, m_rtcpNumber,UserAgentHeaderStr, Sessionstr, StrAuth);
		}


		fprintf(stderr,"SETUP command-%d:\n%s\n",fCSeq,cmd);
		
		if (send(fSocketNum,cmd,strlen(cmd),0)<0) 
		{
			fprintf(stderr,"SETUP send() failed\n");
			break;
		}
		
		// Get the response from the server:
		
		bytesRead = getResponse(readBuf, readBufSize);
		if (bytesRead <= 0) break;
		
		fprintf(stderr,"SETUP response-%d:\n%s\n",fCSeq,readBuf);
		
		// Inspect the first line to check whether it's a result code 200
		firstLine = readBuf;
		nextLineStart = getLine(firstLine);
		if (parseResponseCode(firstLine, &responseCode)) break;
		if (responseCode != 200) 
		{
			fprintf(stderr,"cannot handle SETUP response\n");
			break;
		}
		while (1) 
		{
			lineStart = nextLineStart;
			if (lineStart == NULL||lineStart[0] == '\0') break;
			nextLineStart = getLine(lineStart);
			if (sscanf(lineStart, "Session: %s",SessionId ) == 1)
			{
				char * p = NULL;
				if( (p  = strstr(SessionId, ";timeout")) != NULL)
				{
					*(p) = '\0';
				}
				subsession->fSessionId = strDup(SessionId);
	
				strcpy(fLastSessionId, SessionId);
				continue;
			}

			//获取视频流或音频流的RTP通道ID和RTCP通道ID.例如，服务器返回的Transport信息如下：
            //Transport: RTP/AVP/TCP;unicast;interleaved=0-1，则表示视频的RTP通道ID是0，RTCP通道ID是1
			int rtp_id, rtcp_id;
			if (sscanf(lineStart, "Transport: RTP/AVP/TCP;unicast;interleaved=%d-%d", &rtp_id, &rtcp_id)==2) 
			{
				subsession->rtpChannelID = rtp_id;
				subsession->rtcpChannelID = rtcp_id;

				if(subsession->fRTPPayloadFormat  == VPayloadType)
				{
					m_video_rtp_id = rtp_id;
					m_video_rtcp_id = rtcp_id;
				}
				if(subsession->fRTPPayloadFormat == APayloadType)
				{
					m_audio_rtp_id = rtp_id;
					m_audio_rtcp_id = rtcp_id;
				}

				continue;
			}
		}
		if (subsession->fSessionId == NULL) 
		{
			fprintf(stderr,"Session header is missing in the response\n");
			break;
		}

		free(readBuffer);
		return 0;
	} while (0);
	

	free(readBuffer);
	return -1;
}

///////////////////////////////
void RtspClient::resumeStreams()
{
	//double start;
	//start = (double)(vloopNum*1000)/(double)(VFrameRate);
	long start;
	start = vloopNum*1000/VFrameRate;

	//if(playMediaSession(m_socketNum,start,(double)fMaxPlayEndTime/(double)1000)) 
	if(playMediaSession(start,-1)) 
	{	
			fprintf(stderr,"Play MediaSubsession Failed\n");
			exit(0);
	}
	fprintf(stderr,"Play Streams successful\n");
}

int RtspClient::playMediaSession(int start,int end)//double start, double end)
{

	char startStr[30], endStr[30];
	//unsigned cmdSize;
	unsigned const readBufSize = 50000;
	char *readBuffer;
	char* readBuf;
	int bytesRead;
	char* firstLine;
	char* nextLineStart;
	unsigned responseCode;
	int nSendBytes = 0;
	int nCmdLen = 0;

	readBuffer = (char*)malloc(sizeof(char)*(readBufSize+1));
	if(readBuffer == NULL) return -1;
	memset(readBuffer,0,readBufSize+1);
	
	char cmd[2048] = {0};

	do 
	{
		// First, make sure that we have a RTSP session in progress
        //       printf("fLast=%s\n",fLastSessionId);
		if (strlen(fLastSessionId) == 0) 
		{
			fprintf(stderr,"No RTSP session is currently in progress\n");
			break;
		}
        
		
		// Send the PLAY command:
		
		// First, construct an authenticator string:
		//		sprintf(startStr, "%.3f", start); 
		//sprintf(endStr, "%.3f", end);
		sprintf(startStr,"%d",start);
		sprintf(endStr,"%d",end);
		
		if (start==-1) startStr[0]='\0';
		if (end == -1) endStr[0] = '\0';
		

	    memset(cmd,0,sizeof(cmd));

		if(!m_bUserAuth)
		{
			char cmdFmt[] =
			"PLAY %s/ RTSP/1.0\r\n"
			"CSeq: %d\r\n"
			"Session: %s\r\n"
			//"Range: \r\n"
			"Range: npt=%s-%s\r\n"
			"%s\r\n";

			sprintf(cmd, cmdFmt,
				fBaseURL,
				//subsession->fControlPath,
				++fCSeq,
				fLastSessionId,
				startStr, endStr,
				UserAgentHeaderStr);
		}
		else
		{
		   char cmdFmt[] =
			"PLAY %s/ RTSP/1.0\r\n"
			"CSeq: %d\r\n"
			"Session: %s\r\n"
			//"Range: \r\n"
			"Range: npt=%s-%s\r\n"
			"%s"
			"%s\r\n";

			char StrAuth[1024] = {0};
			GetAuthor( m_realm, m_nonce, m_authType, m_username, m_password, fBaseURL, StrAuth, "PLAY");

			sprintf(cmd, cmdFmt,
				fBaseURL,
				//subsession->fControlPath,
				++fCSeq,
				fLastSessionId,
				startStr, endStr,
				UserAgentHeaderStr,
				StrAuth);
		}
		
		fprintf(stderr,"PLAY command-%d:\n%s\n",fCSeq,cmd);

		nCmdLen = strlen(cmd);
		nSendBytes = send(m_socketNum,cmd, nCmdLen,0);
		if (nSendBytes <0) 
		{
			fprintf(stderr,"PLAY send() failed\n ");
			break;
		}
		readBuf = readBuffer;


		// Get the response from the server:
		bytesRead = getResponse(readBuf, readBufSize);
		
		if (bytesRead <= 0) break;
		
		fprintf(stderr,"PLAY response-%d:\n%s\n",fCSeq,readBuf);
		
		// Inspect the first line to check whether it's a result code 200
		firstLine = readBuf;
		nextLineStart = getLine(firstLine);
		if (parseResponseCode(firstLine,&responseCode)) break;
		if (responseCode != 200) 
		{
			fprintf(stderr,"cannot handle PLAY response\n ");
			break;
		}
		
		free(readBuffer);
		return 0;
	} while (0);
	
	free(readBuffer);
	return -1;
}


///////////////////////////////////////////////////////////////////////
//set parameters
///////////////////////////////////////////////////////////////////////
int RtspClient::SetParametersMediaSession(char *parameters) 
{
	char cmd[2048] = {0};
	char* const cmdFmt = "SET_PARAMETER %s RTSP/1.0\r\n"
				"CSeq: %d\r\n"
				"Content-length: 20"
				"Content-type: text/parameters\r\n"
				"barpara: %s\r\n"				
				"%s\r\n";
	//unsigned cmdSize;
	unsigned readBufSize = 10000;
	char* readBuffer; 
	char* readBuf;
	char* firstLine;
	unsigned responseCode;
	int bytesRead;

	readBuffer = (char *)malloc((readBufSize+1)*sizeof(char));
	if(readBuffer == NULL) return -1;
	memset(readBuffer,0,readBufSize+1);

	readBuf = readBuffer;
	do 
	{
		if (strlen(fLastSessionId) == 0) 
		{
			fprintf(stderr,"No RTSP session is currently in progress\n");
			break;
		}
		
		// Send the TEARDOWN command:

		// First, construct an authenticator string:
		
		memset(cmd,0,sizeof(cmd));

		if(!m_bUserAuth)
		{
		  char* const cmdFmt = "SET_PARAMETER %s RTSP/1.0\r\n"
				"CSeq: %d\r\n"
				"Content-length: 20"
				"Content-type: text/parameters\r\n"
				"barpara: %s\r\n"				
				"%s\r\n";

		  sprintf(cmd, cmdFmt,
			fBaseURL,
			++fCSeq,
			parameters,
			UserAgentHeaderStr);
		
		}
		else
		{
		   char* const cmdFmt = "SET_PARAMETER %s RTSP/1.0\r\n"
				"CSeq: %d\r\n"
				"Content-length: 20"
				"Content-type: text/parameters\r\n"
				"barpara: %s\r\n"
				"%s"
				"%s\r\n";

			char StrAuth[1024] = {0};
			GetAuthor( m_realm, m_nonce, m_authType, m_username, m_password, fBaseURL, StrAuth, "SET_PARAMETER");

			sprintf(cmd, cmdFmt,
				fBaseURL,
				++fCSeq,
				parameters,
				UserAgentHeaderStr,
				StrAuth);
		
		}
		
		fprintf(stderr," command-%d:\n%s\n",fCSeq,cmd);

		if (send(m_socketNum,cmd,strlen(cmd),0)<0) 
		{
			fprintf(stderr,"SET_PARAMETERS send() failed\n ");
			break;
		}

		// Get the response from the server:
		
		bytesRead = getResponse(readBuf, readBufSize);
		if (bytesRead <= 0) break;
		fprintf(stderr,"SET_PARAMETERS response-%d:\n%s\n",fCSeq,readBuf);
		// Inspect the first line to check whether it's a result code 200
		firstLine = readBuf;
		/*char* nextLineStart =*/getLine(firstLine);
		if (parseResponseCode(firstLine,&responseCode)) break;
		if (responseCode != 200) 
		{
			fprintf(stderr,"cannot handle SET_PARAMETERS response\n ");
			break;
		}
		
		free(readBuffer);
		return 0;
	} while (0);

	free(readBuffer);
	return -1;
}


////////////////////////////////////////////////////////////////////
//get_parameters
//////////////////////////////////////////////////////////////
int RtspClient::GetParametersMediaSession(char *parameters, bool bNotWaitForResponse) 
{
	char* nextLineStart =NULL;
	char cmd[2048] = {0};

	//unsigned cmdSize;
	unsigned readBufSize = 10000;
	char* readBuffer; 
	char* readBuf;
	char* firstLine;
	unsigned responseCode;
	int bytesRead;
	readBuffer = (char *)malloc((readBufSize+1)*sizeof(char));
	if(readBuffer == NULL) return -1;
	memset(readBuffer,0,readBufSize+1);

	readBuf = readBuffer;

	do 
	{
		if (strlen(fLastSessionId) == 0) 
		{
			fprintf(stderr,"No RTSP session is currently in progress\n");
			break;
		}
		
		// Send the command:

		// First, construct an authenticator string:
		
		memset(cmd,0,sizeof(cmd));

		if(!m_bUserAuth)
		{
			char* const cmdFmt = "GET_PARAMETER %s RTSP/1.0\r\n"
					"CSeq: %d\r\n"
					"Session: %s\r\n"
					//"Content-type: text/parameters\r\n"
					//"Content-length: %d\r\n\r\n"		
					"%s\r\n";

			sprintf(cmd, cmdFmt,
				fBaseURL,
				++fCSeq,
				fLastSessionId,
				UserAgentHeaderStr);
		}
		else
		{
			char* const cmdFmt = "GET_PARAMETER %s RTSP/1.0\r\n"
					"CSeq: %d\r\n"
					"Session: %s\r\n"
					//"Content-type: text/parameters\r\n"
					//"Content-length: %d\r\n\r\n"	
					"%s"
					"%s\r\n";

		   char StrAuth[1024] = {0};
		   GetAuthor( m_realm, m_nonce, m_authType, m_username, m_password, fBaseURL, StrAuth, "GET_PARAMETER");

			sprintf(cmd, cmdFmt,
				fBaseURL,
				++fCSeq,
				fLastSessionId,
				UserAgentHeaderStr,
				StrAuth);
		}


		fprintf(stderr," command-%d:\n%s\n",fCSeq,cmd);
		
		if (send(m_socketNum,cmd,strlen(cmd),0)<0) 
		{
			fprintf(stderr,"SET_PARAMETERS send() failed\n ");
			break;
		}

		if(!bNotWaitForResponse)
		{
			// Get the response from the server:
			bytesRead = getResponse(readBuf, readBufSize);
			if (bytesRead <= 0) 
				break;
			fprintf(stderr,"GET_PARAMETERS response-%d:\n%s\n",fCSeq,readBuf);
			// Inspect the first line to check whether it's a result code 200
			firstLine = readBuf;
			nextLineStart =getLine(firstLine);
			if (parseResponseCode(firstLine,&responseCode)) 
				break;
			if (responseCode != 200) 
			{
				fprintf(stderr,"cannot handle SET_PARAMETERS response\n ");
				break;
			}
		}
		
		free(readBuffer);
		return 0;
	} while (0);

	free(readBuffer);
	return -1;
}


int RtspClient::SendOptionsCmd(bool bNotWaitForResponse)
{
	char cmd[2048] = {0};
	
	//unsigned cmdSize;
	unsigned const readBufSize = 50000;
	char *readBuffer;
	char* readBuf;
	int bytesRead;
	char* firstLine;
	char* nextLineStart;
	unsigned responseCode;
	int nSendBytes = 0;
	int nCmdLen = 0;

	readBuffer = (char*)malloc(sizeof(char)*(readBufSize+1));
	if(readBuffer == NULL) return -1;
	memset(readBuffer,0,readBufSize+1);
	
	do 
	{
		memset(cmd,0,sizeof(cmd));

		if(!m_bUserAuth)
		{
			char cmdFmt[] =
			"OPTIONS %s RTSP/1.0\r\n"
			"CSeq: %d\r\n"
			"%s\r\n";

			sprintf(cmd, cmdFmt,
				fBaseURL,
				++fCSeq,
				UserAgentHeaderStr);
		}
		else
		{
			char cmdFmt[] =
			"OPTIONS %s RTSP/1.0\r\n"
			"CSeq: %d\r\n"
			"%s"
			"%s\r\n";

			char StrAuth[1024] = {0};
		    GetAuthor( m_realm, m_nonce, m_authType, m_username, m_password, fBaseURL, StrAuth, "OPTIONS");

			sprintf(cmd, cmdFmt,
				fBaseURL,
				++fCSeq,
				UserAgentHeaderStr,
				StrAuth);
		}

		
		fprintf(stderr,"Options command-%d:\n%s\n",fCSeq,cmd);

		if(blockUntilwriteable(m_socketNum) <= 0) //检查Socket是否可写
		{
			fprintf(stderr,"socket is unwriteable\n");
			TRACE("socket is unwriteable\n");
			break;
		}

		nCmdLen = strlen(cmd);
		nSendBytes = send(m_socketNum,cmd, nCmdLen,0);
		if (nSendBytes <0) 
		{
			fprintf(stderr,"Options send() failed\n ");
			break;
		}
		readBuf = readBuffer;

       if(!bNotWaitForResponse)
	   {
			// Get the response from the server:
			bytesRead = getResponse(readBuf, readBufSize);
			
			if (bytesRead <= 0) break;
			
			fprintf(stderr,"Options response-%d:\n%s\n",fCSeq,readBuf);
			
			// Inspect the first line to check whether it's a result code 200
			firstLine = readBuf;
			nextLineStart = getLine(firstLine);
			if (parseResponseCode(firstLine,&responseCode)) break;
			if (responseCode != 200) 
			{
				fprintf(stderr,"cannot handle Options response\n ");
				break;
			}
	   }
		
		free(readBuffer);
		return 0;
	} while (0);
	
	free(readBuffer);
	return -1;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
int RtspClient::pauseMediaSession()
{
	char cmd[2048] = {0};

	//unsigned cmdSize;
	unsigned readBufSize = 10000;
	char* readBuffer; 
	char* readBuf;
	int bytesRead;
	char* firstLine;
	unsigned responseCode;
	readBuffer = (char*)malloc(sizeof(char)*(readBufSize+1));
	if(readBuffer == NULL) return -1;
	memset(readBuffer,0,readBufSize+1);

	do 
	{
		// First, make sure that we have a RTSP session in progress
		if (strlen(fLastSessionId) == 0) 
		{
			fprintf(stderr,"No RTSP session is currently in progress\n");
			break;
		}
		
		// Send the PAUSE command:
		
		// First, construct an authenticator string:
		
		memset(cmd,0, sizeof(cmd));

		if(!m_bUserAuth)
		{
			char cmdFmt[] =
				"PAUSE %s/ RTSP/1.0\r\n"
				"CSeq: %d\r\n"
				"Session: %s\r\n"
				"%s\r\n";

			sprintf(cmd, cmdFmt,
				fBaseURL,
				//subsession->fControlPath,
				++fCSeq,
				fLastSessionId,
				UserAgentHeaderStr);
		}
		else
		{
			char cmdFmt[] =
			"PAUSE %s/ RTSP/1.0\r\n"
			"CSeq: %d\r\n"
			"Session: %s\r\n"
			"%s"
			"%s\r\n";

			char StrAuth[1024] = {0};
			GetAuthor( m_realm, m_nonce, m_authType, m_username, m_password, fBaseURL, StrAuth, "PAUSE");

			sprintf(cmd, cmdFmt,
				fBaseURL,
				//subsession->fControlPath,
				++fCSeq,
				fLastSessionId,
				UserAgentHeaderStr,
				StrAuth);
		}


		fprintf(stderr,"PAUSE command-%d:\n%s\n",fCSeq,cmd);
		
		if (send(m_socketNum,cmd,strlen(cmd),0)<0) 
		{
			fprintf(stderr,"PAUSE send() failed!\n ");
			break;
		}
		
		// Get the response from the server:
		readBuf = readBuffer;
		bytesRead = getResponse(readBuf, readBufSize);

		if (bytesRead <= 0) break;
		
		fprintf(stderr,"bytesRead is %d\n",bytesRead);
		fprintf(stderr,"PAUSE response-%d:\n%s\n",fCSeq,readBuf);
		
		 // Inspect the first line to check whether it's a result code 200
		firstLine = readBuf;
		/*char* nextLineStart =*/ getLine(firstLine);
		
		if (parseResponseCode(firstLine,&responseCode)) break;
		
		if (responseCode != 200) 
		{
			fprintf(stderr,"cannot handle PAUSE response\n ");
			break;
		}
		// (Later, check "CSeq" too #####)
		
		free(readBuffer);
		fprintf(stderr,"Pause Streams successful\n");
		return 0;
	} while (0);
	
	free(readBuffer);
	fprintf(stderr,"Pause Streams failed\n");
	return -1;
}


///////////////////////////////////////////////////////////////////

int RtspClient::teardownMediaSession() 
{
	char cmd[2048] = {0};

	//unsigned cmdSize;
	unsigned readBufSize = 10000;
	char* readBuffer; 
	char* readBuf;
	char* firstLine;
	unsigned responseCode;
	int bytesRead;

	readBuffer = (char *)malloc((readBufSize+1)*sizeof(char));
	if(readBuffer == NULL) return -1;
	memset(readBuffer,0,readBufSize+1);

	readBuf = readBuffer;
	do 
	{
		if (strlen(fLastSessionId) == 0) 
		{
			fprintf(stderr,"No RTSP session is currently in progress\n");
			break;
		}
		
		// Send the TEARDOWN command:

		// First, construct an authenticator string:
		
		memset(cmd,0, sizeof(cmd));

       if(!m_bUserAuth)
	   {
	   	char* const cmdFmt = "TEARDOWN %s RTSP/1.0\r\n"
				"CSeq: %d\r\n"
				"Session: %s\r\n"
				"%s\r\n";

		sprintf(cmd, cmdFmt,
			fBaseURL,
			++fCSeq,
			fLastSessionId,
			UserAgentHeaderStr);
	   }
	   else
	   {
	   	char* const cmdFmt = "TEARDOWN %s RTSP/1.0\r\n"
				"CSeq: %d\r\n"
				"Session: %s\r\n"
				"%s"
				"%s\r\n";

		char StrAuth[1024] = {0};
	    GetAuthor( m_realm, m_nonce, m_authType, m_username, m_password, fBaseURL, StrAuth, "TEARDOWN");

		sprintf(cmd, cmdFmt,
			fBaseURL,
			++fCSeq,
			fLastSessionId,
			UserAgentHeaderStr,
			StrAuth);

	   }

		fprintf(stderr,"TEARDOWN command-%d:\n%s\n",fCSeq,cmd);
		
		if (send(m_socketNum,cmd,strlen(cmd),0)<0) 
		{
			fprintf(stderr,"TEARDOWN send() failed\n ");
			break;
		}

		// Get the response from the server:
		
		bytesRead = getResponse(readBuf, readBufSize);
		if (bytesRead <= 0) break;
		fprintf(stderr,"TEARDOWN response-%d:\n%s\n",fCSeq,readBuf);
		// Inspect the first line to check whether it's a result code 200
		firstLine = readBuf;
		/*char* nextLineStart =*/ getLine(firstLine);
		if (parseResponseCode(firstLine,&responseCode)) break;
		if (responseCode != 200) 
		{
			fprintf(stderr,"cannot handle TEARDOWN response\n ");
			break;
		}
		
		free(readBuffer);

		return 0;
	} while (0);

	free(readBuffer);

	return -1;
}


unsigned char * RtspClient::AllocNetBuffer()
{
	m_netbuf = (unsigned char *)malloc(MAX_PACKET_SIZE*sizeof(unsigned char));
	if(m_netbuf == NULL)
	{
		fprintf(stderr,"alloc failed\n");
		return NULL;
	}
	memset(m_netbuf,0,MAX_PACKET_SIZE);
	return m_netbuf;
}

void RtspClient::ReleaseBuffer()
{
	if(m_netbuf != NULL)
	{
	  free(m_netbuf);
	  m_netbuf = NULL;
	}
	if(m_AFrame.buffer != NULL)
	{
		free(m_AFrame.buffer);
		m_AFrame.buffer = NULL;
	}
	if(m_VFrame.buffer != NULL)
	{
		free(m_VFrame.buffer);
		m_VFrame.buffer = NULL;
	}
}

BOOL RtspClient::OpenStream(const char * szURL)
{
	SetUrl(szURL);

	fCSeq = 0;
	PotPos = 0;
	m_socketNum = 0;
	m_rtpNumber = 0;
	m_rtcpNumber = 0;

	LVrtpTimestamp = 0;
	LArtpTimestamp = 0;

	VPayloadType = 0;
	APayloadType = 0;

	MaxFrameNum = 0;
	aloopNum = 0;
	vloopNum = 0;
	audioFirstTimestamp = 0;
	videoFirstTimestamp = 0;
	subTimeFlag = 0;

	memset(&m_VFrame, 0, sizeof(m_VFrame));
	memset(&m_AFrame, 0, sizeof(m_AFrame));
	memset(&m_Attribute, 0, sizeof(MediaAttribute));

	memset(m_realm, 0, sizeof(m_realm));
	memset(m_nonce, 0, sizeof(m_nonce));
	m_authType = 0;
	m_bUserAuth = FALSE; //是否需要用户认证,默认为False，除非服务器要求

	memset(m_sprop_parameter, 0, sizeof(m_sprop_parameter));
    m_sprop_len = 0;

	m_bFrameContainSPS = FALSE;
	memset(&lostInfo1, 0, sizeof(lostInfo1));
	memset(&lostInfo2, 0, sizeof(lostInfo2));

	memset(&m_video_rtcp_packet, 0, sizeof(m_video_rtcp_packet));
	memset(&m_audio_rtcp_packet, 0, sizeof(m_audio_rtcp_packet));

	/////////////////////////
	m_bStopThread = FALSE;

	//创建RTSP的连接线程
    DWORD threadID = 0;
	m_RtspThread = CreateThread(NULL, 0, RtspThrd,  this, 0, &threadID);

	return (m_RtspThread != NULL);
}

void RtspClient::CloseStream()
{
	m_bStopThread = TRUE; //设置停止线程的标志
	clearup(); //先断掉Socket

	memset(fBaseURL, 0, sizeof(fBaseURL)); //清掉URL

	//等待线程退出
	if (m_RtspThread != NULL) 
	{
		WaitForSingleObject(m_RtspThread, INFINITE);
		::CloseHandle(m_RtspThread);
		m_RtspThread = NULL;
	}
	if(m_SendThread != NULL)
	{
	    WaitForSingleObject(m_SendThread, INFINITE);
		::CloseHandle(m_SendThread);
		m_SendThread = NULL;
	}

	if(m_AFrame.buffer != NULL)
	{
		free(m_AFrame.buffer);
		m_AFrame.buffer = NULL;
	}
	if(m_VFrame.buffer != NULL)
	{
		free(m_VFrame.buffer);
		m_VFrame.buffer = NULL;
	}

	//将流类型复位
	VPayloadType = 0;
	APayloadType = 0;
}



void RtspClient::SetRtspCallback(RtspDataCallback lpFunc, LPVOID lpContext) 
{
	m_lpUserContext = lpContext;
	m_lpFunc = lpFunc;
}

//返回值： 如果视频格式，返回具体的视频格式（由VideoFormat定义）；如果是音频格式，返回具体的音频格式(AudioFormat)
int RtspClient::GetFormatType(unsigned char fRTPPayloadFormat)
{
	if(fRTPPayloadFormat  == VPayloadType)
		return GetVideoFormat();
	if(fRTPPayloadFormat == APayloadType)
		return GetAudioFormat();

	return 0;
}

VideoFormat RtspClient:: GetVideoFormat()
{
	return m_Attribute.fVideoFormat;
}

AudioFormat RtspClient:: GetAudioFormat()
{
	return m_Attribute.fAudioFormat;
}


const MediaAttribute * RtspClient::GetMediaAttribute()
{
	return &m_Attribute;
}

BOOL  RtspClient::GetVideoSize(int & nWidth, int & nHeight)
{
	nWidth  = m_Attribute.fVideoWidth;
	nHeight = m_Attribute.fVideoHeight;
	return (nWidth > 0 && nHeight > 0);
}



//函数作用：从RTP包中还原H264数据
//参数意义：
// recvbuf--输入的RTP数据体地址（已经去掉RTP头）
// len -- 输入数据长度
// marker -- 帧的结束标志，为1表示一帧的结尾
// nIDR -- IDR类型，5--I帧，6--SEI，7--SPS， 8--PPS，其他值：P/B帧
// outbuf -- 提取的H264数据
// total_bytes -- 提取的数据大小
//返回值： 返回total_bytes
//
int RtspClient::rtp_unpackage_H264(unsigned char *recvbuf, int len, BOOL & marker, int & nIDR, unsigned char * outbuf, int & total_bytes, int nSeqNo)
{

    NALU_HEADER * nalu_hdr = NULL;
	NALU_t  nalu_data = {0};
	NALU_t * n  = &nalu_data;
	FU_INDICATOR	*fu_ind = NULL;
	FU_HEADER		*fu_hdr= NULL;
	total_bytes = 0; 

	unsigned int h264_startcode = 0x01000000;

	nalu_hdr =(NALU_HEADER*)&recvbuf[0];                 
	//TRACE("forbidden_zero_bit: %d\n",nalu_hdr->F);          
	n->forbidden_bit= nalu_hdr->F << 7; 
	//TRACE("nal_reference_idc:  %d\n",nalu_hdr->NRI);
	n->nal_reference_idc = nalu_hdr->NRI << 5;               
	//TRACE("nal 负载类型:       %d\n",nalu_hdr->TYPE);
	n->nal_unit_type = nalu_hdr->TYPE;

	if(lostInfo1.nLastNo > 0 && lostInfo1.nLastNo + 1 != nSeqNo)
	{
	  lostInfo1.nLostCount++;
	}

	if ( nalu_hdr->TYPE  == 0)
	{
		TRACE("这个包有错误，0无定义\n");
	}
	else if ( nalu_hdr->TYPE >0 &&  nalu_hdr->TYPE < 24)  //单包
	{
		//TRACE("当前包为单包\n");

		marker = 1;
		//ASSERT(marker == 1); //单包的Marker为1，但少部分服务器会置为0

		memcpy(outbuf + total_bytes, &h264_startcode, 4);
		total_bytes +=4;

	 	memcpy(outbuf + total_bytes, nalu_hdr, 1);
		total_bytes += 1;

		memcpy(outbuf + total_bytes, &recvbuf[1],len-1);
		total_bytes += len-1;

		nIDR = (nalu_hdr->TYPE & 0x1f); //帧类型

		//TRACE("IDR: %d \n", nIDR);

		lostInfo1.tail = 1;
		lostInfo1.head = 0;
	}
	else if ( nalu_hdr->TYPE == 24)                    //STAP-A   单一时间的组合包
	{
		//TRACE("当前包为STAP-A\n");

		int BufLen = len;
		BYTE * pU8 = recvbuf;
		int nal_len = 0;

		BufLen --;
		pU8 ++;

		while( BufLen > 3 )
		{
			nal_len = *pU8*256;
			pU8 ++;
			nal_len += *pU8;
			pU8 ++;

			if( BufLen-2 < nal_len )
				break;

	
			nalu_hdr =(NALU_HEADER*)pU8; 

			//TRACE("forbidden_zero_bit: %d\n",nalu_hdr->F);     //网络传输中的方式为：F->NRI->TYPE.. 内存中存储方式为 TYPE->NRI->F (和nal头匹配)。
			n->forbidden_bit= nalu_hdr->F << 7;                   //内存中的字节序。
			//TRACE("nal_reference_idc:  %d\n",nalu_hdr->NRI);
			n->nal_reference_idc = nalu_hdr->NRI << 5;                      
			//TRACE("nal 负载类型:       %d\n",nalu_hdr->TYPE);
			n->nal_unit_type = nalu_hdr->TYPE;

			memcpy(outbuf + total_bytes, &h264_startcode, 4);
			total_bytes +=4;

			memcpy(outbuf + total_bytes, pU8, nal_len);
			total_bytes += nal_len;

			nIDR = (nalu_hdr->TYPE & 0x1f); //帧类型

			//TRACE("IDR: %d \n", nIDR);

			pU8 += nal_len;
			BufLen -= (nal_len+2);
		}
			
		lostInfo1.tail = 1;
		lostInfo1.head = 0;
	}
	else if ( nalu_hdr->TYPE == 25)                    //STAP-B   单一时间的组合包
	{
		TRACE("当前包为STAP-B\n");
	}
	else if (nalu_hdr->TYPE == 26)                     //MTAP16   多个时间的组合包
	{
		TRACE("当前包为MTAP16\n");
	}
	else if ( nalu_hdr->TYPE == 27)                    //MTAP24   多个时间的组合包
	{
		TRACE("当前包为MTAP24\n");
	}
    else if ( nalu_hdr->TYPE == 28)                    //FU-A分片包，解码顺序和传输顺序相同
	{

		fu_ind=(FU_INDICATOR*)&recvbuf[0];
		//TRACE("FU_INDICATOR->F     :%d\n",fu_ind->F);
		n->forbidden_bit = fu_ind->F << 7;
		//TRACE("FU_INDICATOR->NRI   :%d\n",fu_ind->NRI);
		n->nal_reference_idc = fu_ind->NRI << 5;                      
		//TRACE("FU_INDICATOR->TYPE  :%d\n",fu_ind->TYPE);
		n->nal_unit_type = fu_ind->TYPE;

		fu_hdr=(FU_HEADER*)&recvbuf[1];
		//TRACE("FU_HEADER->S        :%d\n",fu_hdr->S);
		//TRACE("FU_HEADER->E        :%d\n",fu_hdr->E);
		//TRACE("FU_HEADER->R        :%d\n",fu_hdr->R);
		//TRACE("FU_HEADER->TYPE     :%d\n",fu_hdr->TYPE);
		n->nal_unit_type = fu_hdr->TYPE;               //应用的是FU_HEADER的TYPE

		nIDR = (fu_hdr->TYPE & 0x1f); //帧类型

		//TRACE("IDR: %d \n", nIDR);

		if (marker == 1)      //分片包最后一个包
		{
			//TRACE("当前包为FU-A分片包最后一个包\n");

			memcpy(outbuf + total_bytes, &recvbuf[2], len - 2);
			total_bytes += len - 2;

			if(lostInfo1.head == 0)
			{
				TRACE("[E]lost First packet, IDR: %d\n", nIDR);
			}
			if(lostInfo1.nLastNo + 1 != nSeqNo)
			{
				TRACE("[E]lost middle packet, IDR: %d \n", nIDR);
			}
			lostInfo1.tail = 1;
			lostInfo1.head = 0;
			
		}
		else if (marker == 0)
		{
			if (fu_hdr->S == 1)    //分片的第一个包
			{
				//TRACE("当前包为FU-A分片包第一个包\n");

				unsigned char F;
				unsigned char NRI;
				unsigned char TYPE;
				unsigned char nh;
				
				memcpy(outbuf + total_bytes, &h264_startcode, 4);
		        total_bytes +=4;
  
				F = fu_ind->F << 7;
				NRI = fu_ind->NRI << 5;
				TYPE = fu_hdr->TYPE;                                            //应用的是FU_HEADER的TYPE
				//nh = n->forbidden_bit|n->nal_reference_idc|n->nal_unit_type;  //二进制文件也是按 大字节序存储
				nh = F | NRI | TYPE;

			    memcpy(outbuf + total_bytes, &nh, 1);
		        total_bytes +=1;

				memcpy(outbuf + total_bytes, &recvbuf[2], len - 2);
				total_bytes += len - 2;

				if(lostInfo1.tail != 1)
				{
					TRACE("[S]Lost last packet\n");
				}

				lostInfo1.head = 1;
				lostInfo1.tail = 0;
			}
			else   //如果是中间包
			{
				//TRACE("当前包为FU-A分片包\n");

				memcpy(outbuf + total_bytes, &recvbuf[2], len - 2);
				total_bytes += len - 2;

				if(lostInfo1.head == 0)
				{
					TRACE("[M]lost First packet, IDR: %d\n", nIDR);
				}
				if(lostInfo1.nLastNo + 1 != nSeqNo)
				{
				  TRACE("[M]lost middle packet, IDR: %d\n", nIDR);
				}
			
			}	
		}
	}
	else if ( nalu_hdr->TYPE == 29)                //FU-B分片包，解码顺序和传输顺序相同
	{
		if (marker == 1)                  //分片包最后一个包
		{
			TRACE("当前包为FU-B分片包最后一个包\n");

		}
		else if (marker == 0)             //分片包 但不是最后一个包
		{
			TRACE("当前包为FU-B分片包\n");
		}
	}
	else
	{
		TRACE("这个包有错误，30-31 没有定义\n");
	}

	lostInfo1.nLastNo = nSeqNo;

    return total_bytes;
}


//功能：解RTP AAC音频包，声道和采样频率必须知道。
//参数：1.数据缓冲地址 2.数据大小 3.输出数据地址 4.输出数据大小
//返回：1:表示一帧结束  0:帧未结束. 一般AAC音频包比较小，没有分片，所以应该每次都返回1。
////  一个RTP包里面可能包含有多个音频帧，但该函数没有返回每个帧的信息，所以需要一个中间变量记录每个帧的偏移量、长度信息
//
int RtspClient::rtp_unpackage_AAC(unsigned char * bufIn, int len, BOOL  marker, int audioSamprate, unsigned char* pBufOut,  int* pOutLen)
{
	//TRACE("AAC Header: %02x%02x%02x%02x \n", bufIn[0], bufIn[1], bufIn[2], bufIn[3]);

    unsigned char ADTS[] = {0xFF, 0xF9, 0x00, 0x00, 0x00, 0x00, 0xFC}; 
    int audioChannel = 2;//音频声道 1或2
    int audioBit = 16;//16位 固定
    unsigned int framelen;
	const int header = 7; // header = sizeof(ADTS);
   
	unsigned char * pBufSrc = bufIn;
    unsigned char * pBufDest = pBufOut;

	*pOutLen = 0;

    switch(audioSamprate) //音频采样率
    {
	case  8000:
	    ADTS[2] = 0x6C;
		break;
    case  16000:
        ADTS[2] = 0x60;
        break;
    case  32000:
        ADTS[2] = 0x54;//0xD4
        break;
    case  44100:
        ADTS[2] = 0x50;
        break;
    case  48000:
        ADTS[2] = 0x4C;
        break;
    case  96000:
        ADTS[2] = 0x40;
        break;
	case 11025:
		ADTS[2] = 0x68;
		break;
	case 12000:
		ADTS[2] = 0xE4;
		break;
	case 22050:
		ADTS[2] = 0xDC;
		break;
    default:
        break;
    }
    ADTS[3] = (audioChannel==2)?0x80:0x40;

#if 1
	{
		unsigned short au_sizes_array[32] = {0};
		unsigned int framelen = 0;
	
		int au_size = pBufSrc[1]>>3;
		int au_num = au_size>>1;
		ASSERT(au_num < 32);

		pBufSrc+=2;

		for(int i=0; i<au_num; i++)
		{
		  framelen =  ((unsigned int)pBufSrc[0] << 5) & 0x1FE0;
		  framelen |= ((pBufSrc[1] >> 3) & 0x1f);
          au_sizes_array[i] = framelen;

		  pBufSrc += 2;
		}

		//TRACE("Au_Num = %d \n", au_num);

		m_vAudioFrameSlices.clear(); //清掉之前的帧信息
        AudioFrameSlice audioSlice;

		for(int i=0; i<au_num; i++)
		{
			framelen = au_sizes_array[i];

			ADTS[3] |= ((framelen+header) & 0x1800) >> 11;
			ADTS[4] = ((framelen+header) & 0x1FF8) >> 3;
			ADTS[5] = ((framelen+header) & 0x0007) << 5;
			ADTS[5] |= 0x1F;

			memcpy(pBufDest, ADTS, sizeof(ADTS));
			memcpy(pBufDest+header, pBufSrc, framelen);

			//////////////////////////////////////////
         	audioSlice.pos = *pOutLen;
			audioSlice.framelen = framelen+header;

			m_vAudioFrameSlices.push_back(audioSlice); //m_vAudioFrameSlices作为中间变量记录所有音频帧的信息，外部调用者通过GetAudioFrameSliceInfo函数能获得这些信息

			/////////////////////////////////////////

			pBufDest += framelen+header;
			*pOutLen += framelen+header;
			pBufSrc += framelen;
		}

	}
#else
	{
		framelen =  ((unsigned int)pBufSrc[2] << 5) & 0x1FE0;
		framelen |= ((pBufSrc[3] >> 3) & 0x1f);

		ASSERT(framelen == len - 4);

		ADTS[3] |= ((framelen+header) & 0x1800) >> 11;
		ADTS[4] = ((framelen+header) & 0x1FF8) >> 3;
		ADTS[5] = ((framelen+header) & 0x0007) << 5;
		ADTS[5] |= 0x1F;

		memcpy(pBufDest, ADTS, sizeof(ADTS));
		memcpy(pBufDest+header, pBufSrc+4, framelen);
	
		pBufDest += framelen+header;
		*pOutLen += framelen+header;
	}
#endif

#ifdef _SAVE_STREAM_TO_FILE
	if(poutfile == NULL)
	{
		if (NULL == (poutfile = fopen(outputfilename, "wb")))
		{
			printf("Error: Open outputfilename file error\n");
		}
	}
	int fwrite_number = fwrite(pBufOut, 1, *pOutLen, poutfile);
#endif

    return 1;
}



int WriteAdtsHeader(unsigned char* adts_headerbuf, unsigned int  framelen)
{
	ADTS_HEADER adts; 

	adts_headerbuf[0] = 0xFF;
	adts_headerbuf[1] = 0xF9;
	adts_headerbuf[2] = 0x40; //0x40|0x10 = 0x50, 44100 Samplerate
	adts_headerbuf[2] |= 0x10;
	adts_headerbuf[3] = 0x80;
	adts_headerbuf[3] |= (framelen & 0x1800) >> 11;
	adts_headerbuf[4] = (framelen & 0x1FF8) >> 3;
	adts_headerbuf[5] = (framelen & 0x0007) << 5;
	adts_headerbuf[5] |= 0x1F;
	adts_headerbuf[6] = 0xFC;
	adts_headerbuf[6] |= 0x00;

	return 1;
}


//  参考FFmpeg的rtpdec_amr.c文件关于解析AMR RTP包的实现
int  RtspClient::rtp_unpackage_AMR(unsigned char * bufIn, int len,  unsigned char* pBufOut,  int* pOutLen)
{
	int total_bytes = 0;                 //当前包传出的数据
	int fwrite_number = 0;               //存入文件的数据长度
    int frames = 0;
    unsigned char * speech_data = NULL;
	unsigned char * ptr = pBufOut;

    AudioFrameSlice audioSlice;

	m_vAudioFrameSlices.clear(); //清掉之前的帧信息
	
    /* The AMR RTP packet consists of one header byte, followed
     * by one TOC byte for each AMR frame in the packet, followed
     * by the speech data for all the AMR frames.
    */

    /* Count the number of frames in the packet. The highest bit
     * is set in a TOC byte if there are more frames following.
     */
    for (frames = 1; frames < len && (bufIn[frames] & 0x80); frames++) ;

    if (1 + frames >= len) 
	{
        /* We hit the end of the packet while counting frames. */
        TRACE("No speech data found\n");
        return 0;
    }

    speech_data = bufIn + 1 + frames;

	int i=0; 
    for (i = 0; i < frames; i++)  
	{
        uint8_t toc = bufIn[1 + i];

		int frame_size = frame_sizes_nb[(toc>>3) & 0x0f];

		if((toc & 0x04) == 0) //坏帧
		{
			TRACE("Bad AMR frame! \n");
		}

        if (speech_data + frame_size > bufIn + len) 
		{
            /* Too little speech data */
            TRACE("Too little speech data in the RTP packet\n");
            *pOutLen = ptr - pBufOut;
            return 0;
        }

		//////////////////////////////////////
		audioSlice.framelen = 1+frame_size;//1个帧头+帧的长度
		audioSlice.pos = ptr - pBufOut;
		m_vAudioFrameSlices.push_back(audioSlice); //记录从这个RTP包提取到的所有帧的位移和大小信息
		//////////////////////////////////////

		*ptr++ = toc & 0x7C;
		memcpy(ptr, speech_data, frame_size);


		ptr += frame_size;
		speech_data += frame_size;
	
	}

	*pOutLen = ptr - pBufOut;

#ifdef _SAVE_STREAM_TO_FILE
	if(poutfile == NULL)
	{
		if (NULL == (poutfile = fopen(outputfilename, "wb")))
		{
			printf("Error: Open outputfilename file error\n");
		}

		const char AMRFileHeader[7] = "#!AMR\n"; //AMR-NB文件头为6个字节
        fwrite_number = fwrite(AMRFileHeader, 1, 6, poutfile);//写文件头
	}

	fwrite_number = fwrite(pBufOut, 1, *pOutLen, poutfile);
#endif

    return 1;
}

int  RtspClient::rtp_unpackage_MP3(unsigned char * bufIn, int len, unsigned char* pBufOut,  int* pOutLen)
{
	int fwrite_number = 0;   //存入文件的数据长度

	//fwrite_number = fwrite(bufIn + 4, 1, len - 4, poutfile);

	memcpy(pBufOut, bufIn+4, len-4);
	*pOutLen = len-4;
    return 1;
}

DWORD WINAPI RtspClient::RtspThrd(void * pParam)
{
	RtspClient * pClient = (RtspClient *) pParam;

	pClient->SetThreadActive(TRUE);
	int nRet = pClient->RtspThreadProc();
	pClient->SetThreadActive(FALSE);

	TRACE("RtspClient::RtspThrd exited \n");
	return nRet;
}

void  RtspClient::SetThreadActive(BOOL bActive)
{
    m_bThreadActive = bActive;
}

int RtspClient::RtspThreadProc()
{
  
	//int socketNum = -1;
	struct timeb startTime;
	struct timeb currentTime;
	char *timeline;
	struct ResultData * rtp_data = NULL;
	RTCP_HEAD  rtcp_data;
	struct MediaAttribute Attribute;
	int Finished = -1;
    int result  = -1;
	unsigned int video_count=0;
	unsigned int audio_count=0;
	FILE *file;
	BOOL bFirstFrameRead = 0;

	memset(&Attribute, 0, sizeof(Attribute));
	memset(Attribute.fConfigAsc,0, sizeof(Attribute.fConfigAsc));
	memset(Attribute.fConfigHex,0, sizeof(Attribute.fConfigHex));
	Attribute.fVideoFrameRate	= 0;
	Attribute.fTimeIncBits = 0;
	Attribute.fVideoWidth = 0;
	Attribute.fVideoHeight = 0;

	char* sdpDescription = NULL;
	struct MediaSubsession *subsession = NULL;
	int subsessionNum = 0;

	//char *setparameters="testparameteer22[ewqeq";
	char *setparameters;
	char parameter[10];

	char *getparameters=NULL;	

	parameter[0]='\0x8';
	parameter[1]='t';
	parameter[2]='t';
	parameter[3]='t';
	parameter[4]=']';
	parameter[5]='t';
	parameter[6]='6';
	parameter[7]='t';
	parameter[8]='b';
	parameter[9]='t';

	setparameters = parameter;
	//TRACE("parameter[0]=%x\n",parameter[0]);

	TRACE("start openConnectionFromURL %s\n",fBaseURL);
	m_socketNum = openConnectionFromURL(fBaseURL);//连接RTSP服务器的554端口

	TRACE("m_socketNum is %d\n",m_socketNum);
	if(m_socketNum<0)
	{
		TRACE("failed to open the URL\n");
		return (-1);
	}

	result = SendOptionsCmd(false); //发送一个Options请求
	if (result<0) 
	{
		TRACE("Failed to receive Option response \n");
		clearup();
		return (-1);
	}

	sdpDescription = (char*)malloc(MAX_READBUFSIZE*sizeof(char));
	if(sdpDescription == NULL)
	{
		TRACE("failed to alloc the memory\n");
		clearup();
		return (-1);
	}
	memset(sdpDescription,0,MAX_READBUFSIZE);

	TRACE("start getSDPDescriptionFromURL \n");
	result = getSDPDescriptionFromURL(fBaseURL,sdpDescription);  //获取SDP媒体信息
	if (result<0) 
	{
		TRACE("Failed to get a SDP description from URL\n");
		free(sdpDescription);
		clearup();
		return (-1);
	}

	TRACE("Opened URL %s   returning a SDP description:\n%s\n",fBaseURL,sdpDescription);
  
	TRACE("start initializeWithSDP\n");

	subsession = initializeWithSDP(sdpDescription,&subsessionNum); //从SDP提取媒体属性，赋值给SubSession的成员变量
 
	if(subsession == NULL)
	{
		TRACE("Failed to initialize a SDP description\n");
		free(sdpDescription);
		clearup();
		return (-1);
	}
 
	SetMediaAttrbute(&Attribute,subsession,subsessionNum);

	memcpy(&m_Attribute, &Attribute, sizeof(MediaAttribute));

	if(setupStreams(subsession,subsessionNum) < 0)
	{
		free(sdpDescription);
		clearup();
		return (-1);
	}

//	get_parametersMediaSession(getparameters);
//	printf("setparameter=%s\n",setparameters);
//	set_parametersMediaSession(setparameters);

	if(startPlayingStreams(subsession,subsessionNum) < 0)
	{
		free(sdpDescription);
		clearup();
		return (-1);
	}

	free(sdpDescription); sdpDescription = NULL;

	////////////////////////////////////////////////

	ftime(&startTime);
	timeline = ctime( & ( startTime.time ) );

	TRACE("Start receive streaming....\n");
	TRACE("Start time is: %.19s.%hu %s\n",timeline, startTime.millitm, &timeline[20]);

	memset(&rtcp_data, 0, sizeof(rtcp_data));

	DWORD dwStartRunTime = GetTickCount();

#if 0
	DWORD threadID = 0;
	m_SendThread = CreateThread(NULL, 0, SendThrd,  this, 0, &threadID); //发送心跳的线程
#endif

	PACKET_TYPE p_type; //收到的包类型（RTP、RTCP）

	while(!m_bStopThread)
	{
	    rtp_data = NULL;

		Finished = RTP_OR_RTCP_ReadHandler(&p_type, &rtp_data, &rtcp_data);

		if(Finished >= 0)
		{
			if(p_type == RTCP_PACKET) //RTCP
			{

			}
			else if(p_type == RTP_PACKET && rtp_data != NULL)
			{

				//nFormatType > 0 && nFormatType <= 10, 视频格式；
				//nFormatType > 10, 音频格式
				int nFormatType = GetFormatType(rtp_data->fRTPPayloadFormat);

				if( nFormatType > 0 && nFormatType <= 10) //video format
				{
					video_count += rtp_data->len;
					//TRACE("video: %d bytes,audio:%d bytes\r",video_count,audio_count); 

					//file = fopen("c:\\revdata.dat","ab" );
					//fseek(file,-1,2);
					//fwrite(rtp_data->buffer, 1, rtp_data->len, file);
					//fclose(file);

					if(Finished > 0) //获得完整一帧
					{
					   if(m_lpFunc) //回调视频帧
						   m_lpFunc(rtp_data->buffer, rtp_data->len, rtp_data->frtpTimestamp, nFormatType, rtp_data->cFrameType == 'I', m_lpUserContext);
					}
				}
				else if(nFormatType > 10) //Audio format
				{
					audio_count+= rtp_data->len;
					//TRACE("video: %d bytes,audio:%d bytes\r",video_count,audio_count); 

					//file = fopen("c:\\revdata.dat","ab" );
					//fseek(file,-1,2);
					//fwrite(rtp_data->buffer, 1, rtp_data->len, file);
					//fclose(file);

					//回调音频帧
					if(m_lpFunc)
						m_lpFunc(rtp_data->buffer, rtp_data->len, rtp_data->frtpTimestamp, nFormatType, 0, m_lpUserContext);
				}
			}
		}
		else
		{
			if(Finished < -1)//非Socket关闭错误可忽略
			{
				Sleep(1);
				continue;
			}

			break;
		}
	}	

#if 1
	{
		//Attribute.fVideoFrameRate = VFrameRate;
		TRACE("MediaAttribute fVideoFrequency is %u\n",Attribute.fVideoFrequency);
		TRACE("MediaAttribute fVideoPayloadFormat is %u\n",Attribute.fVideoPayloadFormat);
		TRACE("MediaAttribute fConfigAsc is %s\n",Attribute.fConfigAsc);
		TRACE("MediaAttribute fVideoFrameRate is %u\n",Attribute.fVideoFrameRate);
		TRACE("MediaAttribute fTimeIncBits is %u\n",Attribute.fTimeIncBits);
		TRACE("MediaAttribute fixed_vop_rate is %u\n",Attribute.fixed_vop_rate);
		TRACE("MediaAttribute fVideoWidth is %u\n",Attribute.fVideoWidth);
		TRACE("MediaAttribute fVideoHeight is %u\n",Attribute.fVideoHeight);
		TRACE("MediaAttribute fAudioFrequency is %u\n",Attribute.fAudioFrequency);
		TRACE("MediaAttribute fAudioPayloadFormat is %u\n",Attribute.fAudioPayloadFormat);
		TRACE("MediaAttribute fTrackNum is %u\n",Attribute.fTrackNum);

		TRACE("Video packets lost: %d, Audio packets lost: %d \n", lostInfo1.nLostCount, lostInfo2.nLostCount);

		ftime(&currentTime);
		timeline = ctime( & ( currentTime.time ) );

		TRACE("End time is: %.19s.%hu %s\n",timeline, currentTime.millitm, &timeline[20]);
		TRACE("Play total time: %ld sec.\n", (GetTickCount() - dwStartRunTime)/1000);
	}
#endif

	clearup();

	FreeSubSessions(subsession,subsessionNum);
	subsession = NULL;

	return 0;
}

//向服务器发送报文的线程
DWORD WINAPI RtspClient::SendThrd(void * pParam)
{
	RtspClient * pClient = (RtspClient *) pParam;

	int nRet = pClient->SendThreadProc();

	TRACE("RtspClient::SendThrd exited \n");
	return nRet;
}
	

//发送心跳的线程函数
int  RtspClient:: SendThreadProc()
{
	char szParameters[1024] = {0};
	DWORD dwLastTick = GetTickCount();
	DWORD dwCurrentTick = GetTickCount();
    int iErrorTimes = 0;

	while(!m_bStopThread)
	{
		dwCurrentTick = GetTickCount();

		if(dwCurrentTick < dwLastTick) //防止CPU时间重置
		{
           dwLastTick = dwCurrentTick;
		}
		if(dwCurrentTick - dwLastTick > 10000)//10秒发送一次心跳
		{
			//int nRet = get_parametersMediaSession(szParameters, true);
			int nRet = SendOptionsCmd(true);
			if(nRet != 0) //发生错误
			{
				TRACE("Error: SendOptionCmd failed! \n");
				if(iErrorTimes++ > 2 || !m_bThreadActive)
				{
					break;
				}
			}
			else
			{
				iErrorTimes = 0; //错误计数重新清零
			}

			dwLastTick = GetTickCount();
		}
		Sleep(50);
	}

	return 0;
}



//  分两步读： 1.读TCP头部4个字节,其中2个字节是数据包的长度， 2.读剩余的字节（RTP头部（12字节） + 负载数据，或者RTCP报文）
//
int RtspClient::RTP_OR_RTCP_ReadHandler(PACKET_TYPE * packet_type, struct ResultData** rtp_data, RTCP_HEAD * rtcp_data)
{
	unsigned char c;
	unsigned char streamChannelId;
	unsigned short size = 0;
	unsigned fNextTCPReadSize = 0;
	unsigned int bytesRead = 0;
	int result = 0;
	do 
	{
		do 
		{
			result = recv(m_socketNum, (char*)&c, 1, 0);
			//printf("result=%d\n",result);
			if (result <=0) 
			{ 	
				// error reading TCP socket
				//fprintf(stderr,"vloopNum is : %lu\n",vloopNum);
				printf("\n\n");
				fprintf(stderr,"error reading TCP socket\n");
				TRACE("error reading TCP socket\n");

			    if(WSAETIMEDOUT == ::WSAGetLastError()) //超时，这个错误可以忽略 
				{
					return -2;
				}

				return -1;
			}					
		} while (c != '$');

   
		if (recv(m_socketNum, (char*)&streamChannelId, 1, 0)!= 1) break;
		if (recv(m_socketNum, (char*)&size, 2,0) != 2) break;
		fNextTCPReadSize = ntohs(size);
		
		//TRACE("fNextTCPReadSize=%d\n",fNextTCPReadSize);

		result = handleRead(m_netbuf, MAX_PACKET_SIZE, &bytesRead,&fNextTCPReadSize); 
		if(result < 0)
		{
			break;
		}

		if(streamChannelId == m_video_rtp_id || streamChannelId == m_audio_rtp_id) //RTP
		{
		   *packet_type = RTP_PACKET;
		   result =  rtpHandler( m_netbuf, bytesRead, rtp_data);
		   if(result == -1)
		   {
			   result = -4;
			   break;
		   }

		   if(streamChannelId == m_video_rtp_id )
		   {
#ifdef _USE_RTCP_TIME_SYNC_
			   if(m_video_rtcp_packet.TimeRtp == 0)
			   {
				   result = -3;
				   break;
			   }
#endif
		   }
		   if(streamChannelId == m_audio_rtp_id)
		   {
#ifdef _USE_RTCP_TIME_SYNC_
		     if(m_audio_rtcp_packet.TimeRtp == 0)
			 {
				   result = -3;
				   break;
			 }
#endif
		   }
		}
		else if(streamChannelId == m_video_rtcp_id || streamChannelId == m_audio_rtcp_id) //RTCP
		{
			ASSERT(bytesRead >= sizeof(RTCP_HEAD));

			if(streamChannelId == m_video_rtcp_id)
			{
                UINT ntp_lsw_video, ntp_msw_video;
				RTCP_HEAD  header;
				memcpy(&header, m_netbuf, sizeof(RTCP_HEAD));

				HostOrderToNetOrder(&header);

				ntp_msw_video = (header.TimeNtpH) - n1970_1900_Seconds; 
				ntp_lsw_video = (header.TimeNtpL); 
       
				if( m_video_rtcp_packet.TimeNtpH == 0)
				{
				  memcpy(&m_video_rtcp_packet, &header, sizeof(header));
				  m_video_rtcp_packet.TimeNtpH = ntp_msw_video;
				  m_video_rtcp_packet.TimeNtpL = ntp_lsw_video;
				  //TRACE("video_ntp: %s \n", ctime(&ntp_msw_video));
				}
			
			}
			else if(streamChannelId == m_audio_rtcp_id)
			{
                UINT ntp_msw_audio, ntp_lsw_audio;
				RTCP_HEAD  header;
				memcpy(&header, m_netbuf, sizeof(RTCP_HEAD));

				HostOrderToNetOrder(&header);

				ntp_msw_audio = (header.TimeNtpH) - n1970_1900_Seconds;	
				ntp_lsw_audio = (header.TimeNtpL); 

				if(m_audio_rtcp_packet.TimeNtpH == 0)
				{
					memcpy(&m_audio_rtcp_packet, &header, sizeof(RTCP_HEAD));
					m_audio_rtcp_packet.TimeNtpH = ntp_msw_audio;
					m_audio_rtcp_packet.TimeNtpL = ntp_lsw_audio;
					//TRACE("audio_ntp: %s \n", ctime(&m_ntp_msw_audio));
				}
			
			}
		
			*packet_type = RTCP_PACKET;
            result = 0;
		}
		else
		{
			*packet_type = UNKNOWN_PACKET;
		}

		return  result;

	} while (0);

	if(WSAETIMEDOUT == ::WSAGetLastError()) //超时，这个错误可以忽略 
	{
		TRACE("socket read timeout! \n");
		return -2;
	}

	switch(result)
	{
	case -1:
	  TRACE("socket reading error! \n");
	  break;
	case -2:
	  TRACE("socket read timeout! \n");
	  break;
	case -3:
	  TRACE("No RTCP \n");
	  break;
	case -4:
	  TRACE("Unpack rtp packet failed \n");
	  break;
	}

	return result;
}

const std::vector<AudioFrameSlice> & RtspClient::GetAudioFrameSliceInfo()
{
	return m_vAudioFrameSlices;
}

void RtspClient::HostOrderToNetOrder(RTCP_HEAD * pRtcpHead)
{
	pRtcpHead->Len = htons(pRtcpHead->Len);
	pRtcpHead->SSRC = htonl(pRtcpHead->SSRC);
	pRtcpHead->TimeRtp = htonl(pRtcpHead->TimeRtp);
	pRtcpHead->TimeNtpH = htonl(pRtcpHead->TimeNtpH);
	pRtcpHead->TimeNtpL = htonl(pRtcpHead->TimeNtpL);
	pRtcpHead->RtpCounts = htonl(pRtcpHead->RtpCounts);
	pRtcpHead->RtpByteCounts = htonl(pRtcpHead->RtpByteCounts);
}

#ifdef _USE_RTCP_TIME_SYNC_
//convert the rtp time into ntp time
__int64  RtspClient:: GetVideoNtpTime(unsigned int rtpTimestamp)
{
	if(m_video_rtcp_packet.TimeRtp == 0)
		return -1;
	__int64 relativeNtp = (m_video_rtcp_packet.TimeNtpH - 0)*NTP_TIME_FREQUENCY;
	__int64 ntpTime = relativeNtp + ( (__int64)rtpTimestamp - (__int64)m_video_rtcp_packet.TimeRtp )*NTP_TIME_FREQUENCY/(__int64)VTimestampFrequency;
	//TRACE("Video rtp timestamp diff = %I64d \n", ( (__int64)rtpTimestamp - (__int64)m_video_rtcp_packet.TimeRtp )/(__int64)VTimestampFrequency);
	return ntpTime;
}
	
//convert the rtp time into ntp time
__int64 RtspClient:: GetAudioNtpTime(unsigned int rtpTimestamp)
{
	if(m_audio_rtcp_packet.TimeRtp == 0)
		return -1;
    __int64 relativeNtp = (m_audio_rtcp_packet.TimeNtpH - 0)*NTP_TIME_FREQUENCY;
	__int64 ntpTime = relativeNtp + ( (__int64)rtpTimestamp - (__int64)m_audio_rtcp_packet.TimeRtp )*NTP_TIME_FREQUENCY/(__int64)ATimestampFrequency;
	//TRACE("Audio rtp timestamp diff = %I64d \n", ( (__int64)rtpTimestamp - (__int64)m_audio_rtcp_packet.TimeRtp )/(__int64)ATimestampFrequency);
	return ntpTime;
}
#else
//convert the rtp time into ntp time
__int64  RtspClient:: GetVideoNtpTime(unsigned int rtpTimestamp)
{
	__int64 ntpTime = ( (__int64)rtpTimestamp)*NTP_TIME_FREQUENCY/(__int64)VTimestampFrequency;
	return ntpTime;
}
	
//convert the rtp time into ntp time
__int64 RtspClient:: GetAudioNtpTime(unsigned int rtpTimestamp)
{
	__int64 ntpTime = ( (__int64)rtpTimestamp)*NTP_TIME_FREQUENCY/(__int64)ATimestampFrequency;
	return ntpTime;
}
#endif