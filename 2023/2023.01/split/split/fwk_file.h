// -----------------------------------------------------------------------------
// files, cache and virtual filesystem (registered directories and/or compressed zip archives).
// - rlyeh, public domain.
//
// - note: vfs_mount() order matters (last mounts have higher priority).
// - note: directory/with/trailing/slash/ as mount_point, or zip/tar/pak archive otherwise.
//
// @todo: file_mmap
// @todo: file_find() from first file_scan()

// physical filesystem. files

API const char** file_list(const char *path, const char *masks); // **.png;*.c
API bool         file_write( const char *file, const void *ptr, int len );
API bool         file_append( const char *file, const void *ptr, int len );
API char *       file_read(const char *filename);
API char *       file_load(const char *filename, int *len);
API uint64_t     file_size(const char *pathfile);
API bool         file_directory(const char *pathfile);

API char *       file_pathabs(const char *pathfile); // ../dir/./file.ext -> c:/prj/dir/file.ext
API char *       file_path(const char *pathfile); // c:/prj/dir/file.ext -> c:/prj/dir/
API char *       file_name(const char *pathfile); // c:/prj/dir/file.ext -> file.ext
API char *       file_ext(const char *pathfile); // c:/prj/dir/file.ext -> .ext
API char *       file_id(const char *pathfile); // c:/prj/dir/file.ext -> file/dir/prj (name then alphabetical)
API char *       file_normalize(const char *pathfile); // c:/prj/dir/file.ext -> c/prj/dir/file_ext
//API char *     file_normalize_with_folder(const char *pathfile); // c:/prj/dir/file.ext -> dir/file_ext
API char *       file_counter(const char *pathfile); // fwk.ini -> fwk(001).ini -> fwk(002).ini [...]

API uint64_t     file_stamp(const char *pathfile); // 20210319113316 (datetime in base10)
API uint64_t     file_stamp_epoch(const char *pathfile); // 1616153596 (seconds since unix epoch)

API bool         file_exist(const char *pathfile);
API bool         file_delete(const char *pathfile);
API bool         file_copy(const char *src, const char *dst);
API bool         file_move(const char *src, const char *dst);

API FILE*        file_temp();
API char*        file_tempname();

// compressed files

API array(char*) file_zip_list(const char *zipfile);
API array(char)  file_zip_extract(const char *zipfile, const char *filename);
API bool         file_zip_append(const char *zipfile, const char *filename, int clevel);
API bool         file_zip_appendmem(const char *zipfile, const char *entryname, const void *ptr, unsigned len, int clevel);

// storage (emscripten only)

// Mounts local storage folder for writing. Useful for Emscripten only. @path_folder: "/save" for example
// Reads local storage to memory. Usually call it one time only, after mount. Useful for Emscripten only.
// Writes memory contents to local storage. Usually call it after all fclose
API void         storage_mount(const char* folder);
API void         storage_read();
API void         storage_flush();

// virtual filesystem

API bool         vfs_mount(const char *mount_point);
API const char** vfs_list(const char *masks); // **.png;*.c

API char *       vfs_read(const char *pathfile);
API char *       vfs_load(const char *pathfile, int *size);
API int          vfs_size(const char *pathfile);

API const char * vfs_resolve(const char *fuzzyname); // guess best match. @todo: fuzzy path
API const char * vfs_find(const char *pathfile); // returns filename to extracted temporary file, so it can be read by foreign/3rd party libs
API FILE*        vfs_handle(const char *pathfile); // same as above, but returns file handle instead. preferred way, will clean descriptors at exit

// cache

API void *       cache_insert(const char *key, void *value, int size);
API void *       cache_lookup(const char *key, int *size);

// ini

// @todo: evaluate alt api
// int count = ini_count(filename);
// char *key = ini_key(filename, id);
// char *val = ini_val(filename, id);

typedef map(char*,char*) ini_t;

API ini_t        ini(const char *filename);
API ini_t        ini_from_mem(const char *data);
API bool         ini_write(const char *filename, const char *section, const char *key, const char *value);
