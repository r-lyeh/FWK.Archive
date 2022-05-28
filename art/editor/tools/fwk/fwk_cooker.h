// -----------------------------------------------------------------------------
// asset pipeline framework
// - rlyeh, public domain.
//
// all cooked assets are stored inside zip file at root folder, which acts as an asset database.
// during game boot, the database gets rebuilt as follows: (note: step 0 is an optional optimization)
// 0. for N given cores, split list of infiles into N zipfiles. then, parallelize cookers.
// 1. compare local disk files against file in zip database. for each mismatch do:
// 2. - invalidate its entry in database, if local file was removed from disk.
// 3. - write its *cooked* contents into database, if local file was created or modified from disk.
// 4. mount any existing zipfile(s) after cooking.
//
// notes: meta-datas from every raw asset are stored into comment field, inside .cook.zip archive.
// @todo: fix leaks
// @todo: symlink exact files
// @todo: idle threads should steal jobs from busy threads (maybe use jobs/coroutines for this?) ...
// @todo: ... and should compress them in the original cook[N] bucket

enum COOKER_FLAGS {
    COOKER_SYNC = 0,
    COOKER_ASYNC = 1,
    COOKER_CANCELABLE = 2,
};

API void cooker_config( const char *art_path, const char *tools_path, const char *fwk_ini ); // "art/", "art/tools/", "fwk.ini"
API bool cooker_start( const char *masks, int flags ); // "**"
API void cooker_stop();
API void cooker_cancel();
API int  cooker_progress(); // [0..100]
API int  cooker_jobs();     // [0..N]

