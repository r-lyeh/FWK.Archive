// KISSDB written by Adam Ierymenko <adam.ierymenko@zerotier.com>
// KISSDB is in the public domain and is distributed with NO WARRANTY.
// http://creativecommons.org/publicdomain/zero/1.0/

#ifndef DBKV_H
#define DBKV_H
#include <stdbool.h>

#if 0
#if !defined(DBKV_LEN_KEY) && !defined(DBKV_LEN_VALUE)
// UDP packet size  576 (IPv4) = UDP payload  508 + UDP header 8 + IP header 60
// UDP packet size 1280 (IPv6) = UDP payload 1212 + UDP header 8 + IP header 60
enum {
    DBKV_HASH_BUCKET = 508 /*1024*/,
    DBKV_LEN_KEY = 31+1, /*8*/
    DBKV_LEN_VALUE = 508 - DBKV_LEN_KEY /*64*/
};
#endif
#else
enum {
    DBKV_HASH_BUCKET = 1024,
    DBKV_LEN_KEY = 8,
    DBKV_LEN_VALUE = 64
};
#endif

typedef char dbkv_key[DBKV_LEN_KEY];
typedef char dbkv_val[DBKV_LEN_VALUE];

bool dbkv_read( const char *dbfile, const char *keystr, char *val );
bool dbkv_write( const char *dbfile, const char *keystr, const char *val );

#endif

#ifdef DBKV_C
#pragma once
#include <stdint.h>
//#include "detect/detect_memory.c" // realloc

#if 1 //ndef $
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#include <stdio.h>
#  if !defined fseeko && defined _MSC_VER // _MSC_VER
#   define fseeko _fseeki64
#   define ftello _ftelli64
#  elif !defined fseeko
#   define fseeko fseek
#   define ftello ftell
#  endif
#  if !defined fopen_s && !defined _MSC_VER
#   define fopen_s(fp,path,mode) ( (*(fp) = fopen( path, mode ) ) )
#endif
#endif

/* (Keep It) Simple Stupid Database
 *
 * Written by Adam Ierymenko <adam.ierymenko@zerotier.com>
 * KISSDB is in the public domain and is distributed with NO WARRANTY.
 * http://creativecommons.org/publicdomain/zero/1.0/
 *
 * Note: big-endian systems will need changes to implement byte swapping
 * on hash table file I/O. Or you could just use it as-is if you don't care
 * that your database files will be unreadable on little-endian systems. 
 *
 * -----
 *
 * KISSDB file format (version 2)
 * Author: Adam Ierymenko <adam.ierymenko@zerotier.com>
 * http://creativecommons.org/publicdomain/zero/1.0/
 *
 * In keeping with the goal of minimalism the file format is very simple, the
 * sort of thing that would be given as an example in an introductory course in
 * data structures. It's a basic hash table that adds additional pages of hash
 * table entries on collision.
 *
 * It consists of a 28 byte header followed by a series of hash tables and data.
 * All integer values are stored in the native word order of the target
 * architecture (in the future the code might be fixed to make everything
 * little-endian if anyone cares about that).
 *
 * The header consists of the following fields:
 *
 * [0-3]   magic numbers: (ASCII) 'K', 'd', 'B', KISSDB_VERSION (currently 2)
 * [4-11]  64-bit hash table size in entries
 * [12-19] 64-bit key size in bytes
 * [20-27] 64-bit value size in bytes
 *
 * Hash tables are arrays of [hash table size + 1] 64-bit integers. The extra
 * entry, if nonzero, is the offset in the file of the next hash table, forming
 * a linked list of hash tables across the file.
 *
 * Immediately following the header, the first hash table will be written when
 * the first key/value is added. The algorithm for adding new entries is as
 * follows:
 *
 * (1) The key is hashed using a 64-bit variant of the DJB2 hash function, and
 *     this is taken modulo hash table size to get a bucket number.
 * (2) Hash tables are checked in order, starting with the first hash table,
 *     until a zero (empty) bucket is found. If one is found, skip to step (4).
 * (3) If no empty buckets are found in any hash table, a new table is appended
 *     to the file and the final pointer in the previous hash table is set to
 *     its offset. (In the code the update of the next hash table pointer in
 *     the previous hash table happens last, after the whole write is complete,
 *     to avoid corruption on power loss.)
 * (4) The key and value are appended, in order with no additional meta-data,
 *     to the database file. Before appending the offset in the file stream
 *     where they will be stored is saved. After appending, this offset is
 *     written to the empty hash table bucket we chose in steps 2/3. Hash table
 *     updates happen last to avoid corruption if the write does not complete.
 *
 * Lookup of a key/value pair occurs as follows:
 *
 * (1) The key is hashed and taken modulo hash table size to get a bucket
 *     number.
 * (2) If this bucket's entry in the hash table is nonzero, the key at the
 *     offset specified by this bucket is compared to the key being looked up.
 *     If they are equal, the value is read and returned.
 * (3) If the keys are not equal, the next hash table is checked and step (2)
 *     is repeated. If an empty bucket is encountered or if we run out of hash
 *     tables, the key was not found.
 *
 * To update an existing value, its location is looked up and the value portion
 * of the entry is rewritten.
 */

// --- kissdb.h ---

/* (Keep It) Simple Stupid Database
 *
 * Written by Adam Ierymenko <adam.ierymenko@zerotier.com>
 * KISSDB is in the public domain and is distributed with NO WARRANTY.
 *
 * http://creativecommons.org/publicdomain/zero/1.0/ */

/**
 * Version: 2
 *
 * This is the file format identifier, and changes any time the file
 * format changes. The code version will be this dot something, and can
 * be seen in tags in the git repository.
 */
#define KISSDB_VERSION 2

/**
 * KISSDB database state
 *
 * These fields can be read by a user, e.g. to look up key_size and
 * value_size, but should never be changed.
 */
typedef struct {
    unsigned long hash_table_size;
    unsigned long key_size;
    unsigned long value_size;
    unsigned long hash_table_size_bytes;
    unsigned long num_hash_tables;
    uint64_t *hash_tables;
    FILE *f;
} KISSDB;

enum { KISSDB_ERROR_IO = -1 };                   // I/O error or file not found
enum { KISSDB_ERROR_MALLOC = -2 };               // Out of memory
enum { KISSDB_ERROR_INVALID_PARAMETERS = -3 };   // Invalid paramters (e.g. missing _size paramters on init to create database)
enum { KISSDB_ERROR_CORRUPT_DBFILE = -4 };       // Database file appears corrupt
enum { KISSDB_OPEN_MODE_RDONLY = 1 };            // Open mode: read only
enum { KISSDB_OPEN_MODE_RDWR = 2 };              // Open mode: read/write
enum { KISSDB_OPEN_MODE_RWCREAT = 3 };           // Open mode: read/write, create if doesn't exist
enum { KISSDB_OPEN_MODE_RWREPLACE = 4 };         // Open mode: truncate database, open for reading and writing

/**
 * Open database
 *
 * The three _size parameters must be specified if the database could
 * be created or re-created. Otherwise an error will occur. If the
 * database already exists, these parameters are ignored and are read
 * from the database. You can check the struture afterwords to see what
 * they were.
 *
 * @param db Database struct
 * @param path Path to file
 * @param mode One of the KISSDB_OPEN_MODE constants
 * @param hash_table_size Size of hash table in 64-bit entries (must be >0)
 * @param key_size Size of keys in bytes
 * @param value_size Size of values in bytes
 * @return 0 on success, nonzero on error
 */
static int KISSDB_open(
    KISSDB *db,
    const char *path,
    int mode,
    unsigned long hash_table_size,
    unsigned long key_size,
    unsigned long value_size);

/**
 * Close database
 *
 * @param db Database struct
 */
static void KISSDB_close(KISSDB *db);

/**
 * Get an entry
 *
 * @param db Database struct
 * @param key Key (key_size bytes)
 * @param vbuf Value buffer (value_size bytes capacity)
 * @return -1 on I/O error, 0 on success, 1 on not found
 */
static int KISSDB_get(KISSDB *db,const void *key,void *vbuf);

/**
 * Put an entry (overwriting it if it already exists)
 *
 * In the already-exists case the size of the database file does not
 * change.
 *
 * @param db Database struct
 * @param key Key (key_size bytes)
 * @param value Value (value_size bytes)
 * @return -1 on I/O error, 0 on success
 */
static int KISSDB_put(KISSDB *db,const void *key,const void *value);

/**
 * Cursor used for iterating over all entries in database
 */
typedef struct {
    KISSDB *db;
    unsigned long h_no;
    unsigned long h_idx;
} KISSDB_Iterator;

/**
 * Initialize an iterator
 *
 * @param db Database struct
 * @param i Iterator to initialize
 */
static void KISSDB_Iterator_init(KISSDB *db,KISSDB_Iterator *dbi);

/**
 * Get the next entry
 *
 * The order of entries returned by iterator is undefined. It depends on
 * how keys hash.
 *
 * @param Database iterator
 * @param kbuf Buffer to fill with next key (key_size bytes)
 * @param vbuf Buffer to fill with next value (value_size bytes)
 * @return 0 if there are no more entries, negative on error, positive if an kbuf/vbuf have been filled
 */
static int KISSDB_Iterator_next(KISSDB_Iterator *dbi,void *kbuf,void *vbuf);

// --- kissdb.c ---

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define KISSDB_HEADER_SIZE ((sizeof(uint64_t) * 3) + 4)

/* djb2 hash function */
static uint64_t KISSDB_hash(const void *b,unsigned long len)
{
    unsigned long i;
    uint64_t hash = 5381;
    for(i=0;i<len;++i)
        hash = ((hash << 5) + hash) + (uint64_t)(((const uint8_t *)b)[i]);
    return hash;
}

static int KISSDB_open(
    KISSDB *db,
    const char *path,
    int mode,
    unsigned long hash_table_size,
    unsigned long key_size,
    unsigned long value_size)
{
    uint64_t tmp;
    uint8_t tmp2[4];
    uint64_t *httmp;
    uint64_t *hash_tables_rea;

    db->f = (FILE *)0;
    fopen_s(&db->f,path,((mode == KISSDB_OPEN_MODE_RWREPLACE) ? "w+b" : (((mode == KISSDB_OPEN_MODE_RDWR)||(mode == KISSDB_OPEN_MODE_RWCREAT)) ? "r+b" : "rb")));
    if (!db->f) {
        if (mode == KISSDB_OPEN_MODE_RWCREAT) {
            db->f = (FILE *)0;
            fopen_s(&db->f,path,"w+b");
        }
        if (!db->f)
            return KISSDB_ERROR_IO;
    }

    if (fseeko(db->f,0,SEEK_END)) {
        fclose(db->f);
        return KISSDB_ERROR_IO;
    }
    if (ftello(db->f) < KISSDB_HEADER_SIZE) {
        /* write header if not already present */
        if ((hash_table_size)&&(key_size)&&(value_size)) {
            if (fseeko(db->f,0,SEEK_SET)) { fclose(db->f); return KISSDB_ERROR_IO; }
            tmp2[0] = 'K'; tmp2[1] = 'd'; tmp2[2] = 'B'; tmp2[3] = KISSDB_VERSION;
            if (fwrite(tmp2,4,1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
            tmp = hash_table_size;
            if (fwrite(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
            tmp = key_size;
            if (fwrite(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
            tmp = value_size;
            if (fwrite(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
            fflush(db->f);
        } else {
            fclose(db->f);
            return KISSDB_ERROR_INVALID_PARAMETERS;
        }
    } else {
        if (fseeko(db->f,0,SEEK_SET)) { fclose(db->f); return KISSDB_ERROR_IO; }
        if (fread(tmp2,4,1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
        if ((tmp2[0] != 'K')||(tmp2[1] != 'd')||(tmp2[2] != 'B')||(tmp2[3] != KISSDB_VERSION)) {
            fclose(db->f);
            return KISSDB_ERROR_CORRUPT_DBFILE;
        }
        if (fread(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
        if (!tmp) {
            fclose(db->f);
            return KISSDB_ERROR_CORRUPT_DBFILE;
        }
        hash_table_size = (unsigned long)tmp;
        if (fread(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
        if (!tmp) {
            fclose(db->f);
            return KISSDB_ERROR_CORRUPT_DBFILE;
        }
        key_size = (unsigned long)tmp;
        if (fread(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
        if (!tmp) {
            fclose(db->f);
            return KISSDB_ERROR_CORRUPT_DBFILE;
        }
        value_size = (unsigned long)tmp;
    }

    db->hash_table_size = hash_table_size;
    db->key_size = key_size;
    db->value_size = value_size;
    db->hash_table_size_bytes = sizeof(uint64_t) * (hash_table_size + 1); /* [hash_table_size] == next table */

    httmp = MALLOC(db->hash_table_size_bytes);
    if (!httmp) {
        fclose(db->f);
        return KISSDB_ERROR_MALLOC;
    }
    db->num_hash_tables = 0;
    db->hash_tables = (uint64_t *)0;
    while (fread(httmp,db->hash_table_size_bytes,1,db->f) == 1) {
        hash_tables_rea = REALLOC(db->hash_tables,db->hash_table_size_bytes * (db->num_hash_tables + 1));
        if (!hash_tables_rea) {
            KISSDB_close(db);
            FREE(httmp);
            return KISSDB_ERROR_MALLOC;
        }
        db->hash_tables = hash_tables_rea;

        memcpy(((uint8_t *)db->hash_tables) + (db->hash_table_size_bytes * db->num_hash_tables),httmp,db->hash_table_size_bytes);
        ++db->num_hash_tables;
        if (httmp[db->hash_table_size]) {
            if (fseeko(db->f,httmp[db->hash_table_size],SEEK_SET)) {
                KISSDB_close(db);
                FREE(httmp);
                return KISSDB_ERROR_IO;
            }
        } else break;
    }
    FREE(httmp);

    return 0;
}

static void KISSDB_close(KISSDB *db)
{
    if (db->hash_tables)
        FREE(db->hash_tables);
    if (db->f)
        fclose(db->f);
    memset(db,0,sizeof(KISSDB));
}

static int KISSDB_get(KISSDB *db,const void *key,void *vbuf)
{
    uint8_t tmp[4096];
    const uint8_t *kptr;
    unsigned long klen,i;
    uint64_t hash = KISSDB_hash(key,db->key_size) % (uint64_t)db->hash_table_size;
    uint64_t offset;
    uint64_t *cur_hash_table;
    long n;

    cur_hash_table = db->hash_tables;
    for(i=0;i<db->num_hash_tables;++i) {
        offset = cur_hash_table[hash];
        if (offset) {
            if (fseeko(db->f,offset,SEEK_SET))
                return KISSDB_ERROR_IO;

            kptr = (const uint8_t *)key;
            klen = db->key_size;
            while (klen) {
                n = (long)fread(tmp,1,(klen > sizeof(tmp)) ? sizeof(tmp) : klen,db->f);
                if (n > 0) {
                    if (memcmp(kptr,tmp,n))
                        goto get_no_match_next_hash_table;
                    kptr += n;
                    klen -= (unsigned long)n;
                } else return 1; /* not found */
            }

            if (fread(vbuf,db->value_size,1,db->f) == 1)
                return 0; /* success */
            else return KISSDB_ERROR_IO;
        } else return 1; /* not found */
get_no_match_next_hash_table:
        cur_hash_table += db->hash_table_size + 1;
    }

    return 1; /* not found */
}

static int KISSDB_put(KISSDB *db,const void *key,const void *value)
{
    uint8_t tmp[4096];
    const uint8_t *kptr;
    unsigned long klen,i;
    uint64_t hash = KISSDB_hash(key,db->key_size) % (uint64_t)db->hash_table_size;
    uint64_t offset;
    uint64_t htoffset,lasthtoffset;
    uint64_t endoffset;
    uint64_t *cur_hash_table;
    uint64_t *hash_tables_rea;
    long n;

    lasthtoffset = htoffset = KISSDB_HEADER_SIZE;
    cur_hash_table = db->hash_tables;
    for(i=0;i<db->num_hash_tables;++i) {
        offset = cur_hash_table[hash];
        if (offset) {
            /* rewrite if already exists */
            if (fseeko(db->f,offset,SEEK_SET))
                return KISSDB_ERROR_IO;

            kptr = (const uint8_t *)key;
            klen = db->key_size;
            while (klen) {
                n = (long)fread(tmp,1,(klen > sizeof(tmp)) ? sizeof(tmp) : klen,db->f);
                if (n > 0) {
                    if (memcmp(kptr,tmp,n))
                        goto put_no_match_next_hash_table;
                    kptr += n;
                    klen -= (unsigned long)n;
                }
            }

            /* C99 spec demands seek after fread(), required for Windows */
            fseeko(db->f,0,SEEK_CUR);
 
            if (fwrite(value,db->value_size,1,db->f) == 1) {
                fflush(db->f);
                return 0; /* success */
            } else return KISSDB_ERROR_IO;
        } else {
            /* add if an empty hash table slot is discovered */
            if (fseeko(db->f,0,SEEK_END))
                return KISSDB_ERROR_IO;
            endoffset = ftello(db->f);

            if (fwrite(key,db->key_size,1,db->f) != 1)
                return KISSDB_ERROR_IO;
            if (fwrite(value,db->value_size,1,db->f) != 1)
                return KISSDB_ERROR_IO;

            if (fseeko(db->f,htoffset + (sizeof(uint64_t) * hash),SEEK_SET))
                return KISSDB_ERROR_IO;
            if (fwrite(&endoffset,sizeof(uint64_t),1,db->f) != 1)
                return KISSDB_ERROR_IO;
            cur_hash_table[hash] = endoffset;

            fflush(db->f);

            return 0; /* success */
        }
put_no_match_next_hash_table:
        lasthtoffset = htoffset;
        htoffset = cur_hash_table[db->hash_table_size];
        cur_hash_table += (db->hash_table_size + 1);
    }

    /* if no existing slots, add a new page of hash table entries */
    if (fseeko(db->f,0,SEEK_END))
        return KISSDB_ERROR_IO;
    endoffset = ftello(db->f);

    hash_tables_rea = REALLOC(db->hash_tables,db->hash_table_size_bytes * (db->num_hash_tables + 1));
    if (!hash_tables_rea)
        return KISSDB_ERROR_MALLOC;
    db->hash_tables = hash_tables_rea;
    cur_hash_table = &(db->hash_tables[(db->hash_table_size + 1) * db->num_hash_tables]);
    memset(cur_hash_table,0,db->hash_table_size_bytes);

    cur_hash_table[hash] = endoffset + db->hash_table_size_bytes; /* where new entry will go */

    if (fwrite(cur_hash_table,db->hash_table_size_bytes,1,db->f) != 1)
        return KISSDB_ERROR_IO;

    if (fwrite(key,db->key_size,1,db->f) != 1)
        return KISSDB_ERROR_IO;
    if (fwrite(value,db->value_size,1,db->f) != 1)
        return KISSDB_ERROR_IO;

    if (db->num_hash_tables) {
        if (fseeko(db->f,lasthtoffset + (sizeof(uint64_t) * db->hash_table_size),SEEK_SET))
            return KISSDB_ERROR_IO;
        if (fwrite(&endoffset,sizeof(uint64_t),1,db->f) != 1)
            return KISSDB_ERROR_IO;
        db->hash_tables[((db->hash_table_size + 1) * (db->num_hash_tables - 1)) + db->hash_table_size] = endoffset;
    }

    ++db->num_hash_tables;

    fflush(db->f);

    return 0; /* success */
}

static void KISSDB_Iterator_init(KISSDB *db,KISSDB_Iterator *dbi)
{
    dbi->db = db;
    dbi->h_no = 0;
    dbi->h_idx = 0;
}

static int KISSDB_Iterator_next(KISSDB_Iterator *dbi,void *kbuf,void *vbuf)
{
    uint64_t offset;

    if ((dbi->h_no < dbi->db->num_hash_tables)&&(dbi->h_idx < dbi->db->hash_table_size)) {
        while (!(offset = dbi->db->hash_tables[((dbi->db->hash_table_size + 1) * dbi->h_no) + dbi->h_idx])) {
            if (++dbi->h_idx >= dbi->db->hash_table_size) {
                dbi->h_idx = 0;
                if (++dbi->h_no >= dbi->db->num_hash_tables)
                    return 0;
            }
        }
        if (fseeko(dbi->db->f,offset,SEEK_SET))
            return KISSDB_ERROR_IO;
        if (fread(kbuf,dbi->db->key_size,1,dbi->db->f) != 1)
            return KISSDB_ERROR_IO;
        if (fread(vbuf,dbi->db->value_size,1,dbi->db->f) != 1)
            return KISSDB_ERROR_IO;
        if (++dbi->h_idx >= dbi->db->hash_table_size) {
            dbi->h_idx = 0;
            ++dbi->h_no;
        }
        return 1;
    }

    return 0;
}

#endif

#ifdef DBKV_BENCH
#include <time.h>
#include <inttypes.h>

#ifdef _MSC_VER
#include <omp.h>
#define clock_t double
#define clock omp_get_wtime
#undef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1.0
#endif

int main(int argc,char **argv)
{
    #ifndef N
    enum { N = 100000 };
    #endif
    uint64_t i,j;
    uint64_t v[8];
    KISSDB db;
    KISSDB_Iterator dbi;
    char *got_all_values = malloc(N);
    int q;

    clock_t begin, end;

    #ifdef STDIO2
    if( argc < 2 ) {
        printf("%s [file.db]\n", argv[0]);
        printf("%s [mem://file.db]\n", argv[0]);
        exit(0);
    }
    const char *dbfile = argv[1];
    #else
    const char *dbfile = "test.db";
    #endif

    begin = clock();

    printf("Opening new empty database %s...\n", dbfile);
    if (KISSDB_open(&db,dbfile,KISSDB_OPEN_MODE_RWREPLACE,1024,8,sizeof(v))) {
        printf("KISSDB_open failed\n");
        return 1;
    }

    end = clock();
    {
        double sec = (end - begin)/(double)CLOCKS_PER_SEC;
        printf("OK! %5.2fs\n", sec);
    }
    begin = clock();

    printf("Putting %d 64-byte values...\n", N);
    for(i=0;i<N;++i) {
        for(j=0;j<8;++j)
            v[j] = i;
        if (KISSDB_put(&db,&i,v)) {
            printf("KISSDB_put failed (%"PRIu64")\n",i);
            return 1;
        }
    }

    end = clock();
    {
        int ops = N;
        double sec = (end - begin)/(double)CLOCKS_PER_SEC;
        printf("OK! %d ops in %5.2fs, %5.2fops/s, %5.2fops/frame\n", ops, sec, ops/sec, ops/sec/60);
    }
    begin = clock();

    printf("Getting %d 64-byte values...\n", N);
    for(i=0;i<N;++i) {
        if ((q = KISSDB_get(&db,&i,v))) {
            printf("KISSDB_get (2) failed (%"PRIu64") (%d)\n",i,q);
            return 1;
        }
        for(j=0;j<8;++j) {
            if (v[j] != i) {
                printf("KISSDB_get (2) failed, bad data (%"PRIu64")\n",i);
                return 1;
            }
        }
    }

    end = clock();
    {
        int ops = N;
        double sec = (end - begin)/(double)CLOCKS_PER_SEC;
        printf("OK! %d ops in %5.2fs, %5.2fops/s, %5.2fops/frame\n", ops, sec, ops/sec, ops/sec/60);
    }
    begin = clock();

    printf("Closing and re-opening database in read-only mode...\n");
    KISSDB_close(&db);
    if (KISSDB_open(&db,dbfile,KISSDB_OPEN_MODE_RDONLY,1024,8,sizeof(v))) {
        printf("KISSDB_open failed\n");
        return 1;
    }

    end = clock();
    {
        double sec = (end - begin)/(double)CLOCKS_PER_SEC;
        printf("OK! %5.2fs\n", sec);
    }
    begin = clock();

    printf("Getting %d 64-byte values...\n", N);
    for(i=0;i<N;++i) {
        if ((q = KISSDB_get(&db,&i,v))) {
            printf("KISSDB_get (3) failed (%"PRIu64") (%d)\n",i,q);
            return 1;
        }
        for(j=0;j<8;++j) {
            if (v[j] != i) {
                printf("KISSDB_get (3) failed, bad data (%"PRIu64")\n",i);
                return 1;
            }
        }
    }

    end = clock();
    {
        int ops = N;
        double sec = (end - begin)/(double)CLOCKS_PER_SEC;
        printf("OK! %d ops in %5.2fs, %5.2fops/s, %5.2fops/frame\n", ops, sec, ops/sec, ops/sec/60);
    }
    begin = clock();

    printf("Iterating %d 64-byte values...\n", N);
    KISSDB_Iterator_init(&db,&dbi);
    i = 0xdeadbeef;
    memset(got_all_values,0,sizeof(N));
    while (KISSDB_Iterator_next(&dbi,&i,&v) > 0) {
        if (i < N)
            got_all_values[i] = 1;
        else {
            printf("KISSDB_Iterator_next failed, bad data (%"PRIu64")\n",i);
            return 1;
        }
    }
    for(i=0;i<N;++i) {
        if (!got_all_values[i]) {
            printf("KISSDB_Iterator failed, missing value index %"PRIu64"\n",i);
            return 1;
        }
    }

    KISSDB_close(&db);

    end = clock();
    {
        int ops = N;
        double sec = (end - begin)/(double)CLOCKS_PER_SEC;
        printf("OK! %d ops in %5.2fs, %5.2fops/s, %5.2fops/frame\n", ops, sec, ops/sec, ops/sec/60);
    }
    begin = clock();

    return 0;
}
#define main main__
#endif

#ifdef DBKV_C
#pragma once
bool dbkv_read( const char *dbfile, const char *keystr, char *val ) {
    KISSDB kdb = {0};
    int ok = 0 == KISSDB_open( &kdb, dbfile, KISSDB_OPEN_MODE_RWCREAT, DBKV_HASH_BUCKET, DBKV_LEN_KEY, DBKV_LEN_VALUE);
    if( ok ) {
        dbkv_key key = {0};
        strcpy( key, keystr ); //sprintf(key, "%.*s", strlen(keystr) > DBKV_LEN_KEY ? DBKV_LEN_KEY : strlen(keystr), keystr);

        int result = KISSDB_get( &kdb, key, val ), not_found = result > 0; // -1:I/O error, 0:ok, 1:not found
        ok = result >= 0; //printf("rd %s=%s (found: %d) (rc: %d)\n", key, val, !not_found, result);

        KISSDB_close( &kdb );
    }
    return !!ok;
}
bool dbkv_write( const char *dbfile, const char *keystr, const char *val ) {
    KISSDB kdb = {0};
    int ok = 0 == KISSDB_open( &kdb, dbfile, KISSDB_OPEN_MODE_RWCREAT, DBKV_HASH_BUCKET, DBKV_LEN_KEY, DBKV_LEN_VALUE);
    if( ok ) {
        dbkv_key key = {0};
        strcpy( key, keystr ); //sprintf(key, "%.*s", strlen(keystr) > DBKV_LEN_KEY ? DBKV_LEN_KEY : strlen(keystr), keystr);

        int result = KISSDB_put( &kdb, key, val ); // -1:I/O error, 0:ok
        ok = result >= 0; //printf("wr (rc: %d)\n", result);

        KISSDB_close( &kdb );
    }
    return !!ok;
}

#ifdef DBKV_DEMO
#include <stdio.h>
int main( int argc, char **argv ) {
    dbkv_val val = "0"; // default value, if not found
    dbkv_read( "db.kvs", "/hello", val );

    printf("/hello=%s\n", val);

    val[0] ++; if( val[0] > '9' ) val[0] = '0';
    dbkv_write( "db.kvs", "/hello", val );
}
#define main main__
#endif // KBDV_DEMO
#endif // KBDV_C

