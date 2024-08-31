// detect file/mime types
// - rlyeh, public domain.

const char *os_mime(const char *filename);
const char *os_mime_buf(const char *buf, int len);

#ifdef MIME_C
#pragma once
#include <string.h>

const char *os_mime_buf(const char *buf, int len) {
    const struct type {
        int len; const char *ext; const char *buf; int off;
    } types[] = { //// Descending priority order
       21, "deb", "!<arch>\x0a""debian-binary", 0,
       20, "xpi", "META-INF/mozilla.rsa", 30,
       14, "mxf", "\x06\x0e+4\x02\x05\x01\x01\x0d\x01\x02\x01\x01\x02", 0,
       11, "hdr", "#?RADIANCE\x0a", 0,
       11, "m4v", "\x00\x00\x00\x1c""ftypM4V", 0, // before mp4
       10, "wmv", "0&\xb""2u""\x8e""f""\xcf\x11\xa6\xd9", 0,
        8, "mkv", "matroska", 31,
        8, "mov", "\x00\x00\x00\x14""ftyp", 0,
        8, "msi", "\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1", 0,
        8, "opus", "OpusHead", 28, // before ogg
        8, "wav", "RIFFWAVE", 0,
        8, "woff", "wOFF\x00\x01\x00\x00", 0,
        8, "woff", "wOFFOTTO", 0,
        8, "woff2", "wOF2\x00\x01\x00\x00", 0,
        8, "woff2", "wOF2OTTO", 0,
        7, "ar", "!<arch>", 0,
        7, "avi", "RIFFAVI", 0,
        7, "hdr", "#?RGBE\x0a", 0,
        7, "m4a", "ftypM4A", 4,
        7, "rar", "Rar!\x1a\x07\x00", 0,
        7, "rar", "Rar!\x1a\x07\x01", 0,
        6, "7z", "7z\xbc\xaf'\x1c", 0,
        6, "amr", "#!AMR\x0a", 0,
        6, "xz", "\xfd""7zXZ\x00", 0,
        5, "amr", "#!AMR", 0,
        5, "otf", "OTTO\x00", 0,
        5, "rtf", "{\rtf", 0,
        5, "rtf", "{\x0dtf\x00", 0,
        5, "tar", "ustar", 257,
        5, "ttf", "\x00\x01\x00\x00\x00", 0,
        4, "cab", "ISc(", 0,
        4, "cab", "MSCF", 0,
        4, "crx", "Cr24", 0,
        4, "exr", "v/1\x01", 0,
        4, "flac", "fLaC", 0,
        4, "flif", "FLIF", 0,
        4, "flv", "FLV\x01", 0,
        4, "ico", "\x00\x00\x01\x00", 0,
        4, "lz", "LZIP", 0,
        4, "m4a", "M4A ", 0,
        4, "mid", "MThd", 0,
        4, "mkv", "\x1a""E""\xdf\xa3", 0,
        4, "mp4", "\x00\x00\x00\x1c", 0,
        4, "mp4", "3gp5", 0,
        4, "mp4", "ftyp", 4,
        4, "mpg", "\x00\x00\x01b", 0,
        4, "nes", "NES\x1a", 0,
        4, "ogg", "OggS", 0,
        4, "otf", "OTTO", 0,
        4, "pdf", "%PDF", 0,
        4, "png", "\x89PNG", 0,
        4, "psd", "8BPS", 0,
        4, "rar", "Rar!", 0,
        4, "rpm", "\xed\xab\xee\xdb", 0,
        4, "sqlite", "SQLi", 0,
        4, "svg", "<svg", 0,
        4, "tif", "II*\x00", 0,
        4, "tif", "MM\x00*", 0,
        4, "tiff", "II*\x00", 0,
        4, "tiff", "MM\x00*", 0,
        4, "wav", "WAVE", 8,
        4, "webm", "\x1a""E""\xdf\xa3", 0,
        4, "webp", "WEBP", 8,
        4, "woff", "wOFF", 0,
        4, "woff2", "wOF2", 0,
        3, "avi", "AVI", 8,
        3, "bz2", "BZh", 0,
        3, "eot", "\x00\x00\x01", 8,
        3, "eot", "\x01\x00\x02", 8,
        3, "eot", "\x02\x00\x02", 8,
        3, "gif", "GIF", 0,
        3, "gz", "\x1f\x8b\x08", 0,
        3, "jpg", "\xff\xd8\xff", 0,
        3, "jxr", "II\xbc", 0,
        3, "mp3", "ID3", 0,
        3, "mpg", "\x00\x00\x01", 0,
        3, "swf", "CWS", 0,
        3, "swf", "FWS", 0,
        2, "bmp", "BM", 0,
        2, "dmg", "x\x01", 0,
        2, "exe", "MZ", 0,
        2, "mp3", "\xff\xfb", 0,
        2, "ps", "%!", 0,
        2, "sh", "#/-", 0,
        2, "swf", "WS", 1,
        2, "z", "\x1f\x9d", 0,
        2, "z", "\x1f\xa0", 0,
        2, "zip", "PK", 0,
        1, "json", "[", 0, // @todo: improve this weak detection someday.
        1, "json", "{", 0, // @todo: improve this weak detection someday.
        1, "xml", "<", 0,  // @todo: improve this weak detection someday.
        0
    };
    for( int i = 0; types[i].ext; ++i ) {
        if( (types[i].off + types[i].len) < len ) {
            if( 0 == memcmp( buf + types[i].off, types[i].buf, types[i].len ) ) {
                return types[i].ext;
            }
        }
    }
    return "";
}

const char *os_mime(const char *fname) {
    const char *ftype = "";
    // examine contents...
    for( FILE *fp = fopen(fname, "rb" ); fp; fclose(fp), fp = 0) {
        char buf[512];
        int len = fread(buf, 1, 512, fp);
        ftype = os_mime_buf(buf, len);
    }
    // else use extension as detection
    if( !ftype[0] ) {
        ftype = strrchr(fname, '.');
        ftype += !!ftype;
    }
    return ftype;
}

#endif

#ifdef MIME_DEMO
#include <stdio.h>
int main( int argc, const char **argv ) {
    if( argc == 2 ) {
        puts( os_mime(argv[1]) );
        exit(0);
    }
    // samples taken from https://github.com/mathiasbynens/small
    const unsigned char unknown1[] = {
        0x3C,0x3F,0x78,0x6D,0x6C,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x22,0x31,
        0x2E,0x31,0x22,0x3F,0x3E,0x3C,0x5F,0x2F,0x3E
    };
    const unsigned char unknown2[] = {
        0x47,0x49,0x46,0x38,0x39,0x61,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x3B
    };
    const unsigned char unknown3[] = {
        0x42,0x4D,0x1E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1A,0x00,0x00,0x00,0x0C,0x00,
        0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x18,0x00,0x00,0x00,0xFF,0x00
    };
    const unsigned char unknown4[] = {
        0x50,0x4B,0x05,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00
    };
    const unsigned char unknown5[] = {
        0x7B,0x5C,0x72,0x74,0x66,0x31,0x7D
    };
    const unsigned char unknown6[] = {
        0x52,0x49,0x46,0x46,0x12,0x00,0x00,0x00,0x57,0x45,0x42,0x50,0x56,0x50,0x38,0x4C,
        0x06,0x00,0x00,0x00,0x2F,0x41,0x6C,0x6F,0x00,0x6B
    };
    puts( os_mime_buf( unknown1, sizeof(unknown1) ) );
    puts( os_mime_buf( unknown2, sizeof(unknown2) ) );
    puts( os_mime_buf( unknown3, sizeof(unknown3) ) );
    puts( os_mime_buf( unknown4, sizeof(unknown4) ) );
    puts( os_mime_buf( unknown5, sizeof(unknown5) ) );
    puts( os_mime_buf( unknown6, sizeof(unknown6) ) );
}
#endif
