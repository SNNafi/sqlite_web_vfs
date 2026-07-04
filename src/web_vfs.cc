/*
** SQLite loadable extension providing web VFS
*/
#include <sqlite3ext.h>
extern "C" {
SQLITE_EXTENSION_INIT1
}
#include "web_vfs.h"

/*************************************************************************************************/

/*
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
// Chwilio: renamed entry point + VFS name so the app-bundled build is fully isolated
// from a user-added stock web_vfs.dylib (which exports sqlite3_webvfs_init / registers "web").
// Distinct symbol avoids RTLD_GLOBAL interposition; distinct VFS name ("web_chwilio") means
// only this build provides it, so the app's `vfs=web_chwilio` URI routes deterministically here.
extern "C" int sqlite3_webvfs_chwilio_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi) {
    SQLITE_EXTENSION_INIT2(pApi);
    int rc = SQLITE_OK;
    rc = (new WebVFS::VFS())->Register("web_chwilio");
    return rc != SQLITE_OK ? rc : SQLITE_OK_LOAD_PERMANENTLY;
}
