// - rlyeh, public domain

#ifdef BUF_C
#define ARC4_C
#define BASE64_C
#define BASE92_C
#define COBS_C
#define CRC_C
#define ENDIAN_C
#define INTERLEAVE_C
#define NETSTRING_C
#define ZIGZAG_C
#define PACK_C
#define HALF_C
#endif

#include "os.c"
#include "buf_arc4.c"        // buffer_crypt_arc4
#include "buf_base64.c"      // buffer_rebase_base64
#include "buf_base92.c"      // buffer_rebase_base92
#include "buf_cobs.c"        // buffer_rebase_cobs
#include "buf_crc.c"         // buffer_checksum_crc
#include "buf_endian.c"      // buffer_utils_endianness
#include "buf_interleave.c"  // buffer_utils_interleave
#include "buf_netstring.c"   // buffer_escape_netstring
#include "buf_zigzag.c"      // buffer_utils_zigzag

#include "buf_packint.h"     // integer packing
#include "buf_packvli.h"     // variable int packing
#include "buf_pack754.h"     // float754 packing
#include "buf_packhalf.h"    // floathalf packing
#include "buf_pack.c"        // data packer
