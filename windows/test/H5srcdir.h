/* If you are reading this file and it has a '.h' suffix, it was automatically
 * generated from the '.in' version.  Make changes there.
 */

/* Set the 'srcdir' path from configure time */
static const char *config_srcdir = ".";

/* Buffer to construct path in and return pointer to */
static char srcdir_path[1024] = "";

/* Buffer to construct file in and return pointer to */
static char srcdir_testpath[1024] = "";

/* Append the test file name to the srcdir path and return the whole string */
static const char *H5_get_srcdir_filename(const char *filename)
{
    const char *srcdir = HDgetenv("srcdir");

    /* Check for using the srcdir from configure time */
    if(NULL == srcdir)
        srcdir = config_srcdir;

    /* Build path to test file */
    if((HDstrlen(srcdir) + HDstrlen(filename) + 2) < sizeof(srcdir_testpath)) {
        HDstrcpy(srcdir_testpath, srcdir);
        HDstrcat(srcdir_testpath, "/");
        HDstrcat(srcdir_testpath, filename);
        return(srcdir_testpath);
    } /* end if */
    else
        return(NULL);
}

/* Just return the srcdir path */
static const char *H5_get_srcdir(void)
{
    const char *srcdir = HDgetenv("srcdir");

    /* Check for using the srcdir from configure time */
    if(NULL == srcdir)
        srcdir = config_srcdir;

    /* Build path to all test files */
    if((HDstrlen(srcdir) + 2) < sizeof(srcdir_path)) {
        HDstrcpy(srcdir_path, srcdir);
        HDstrcat(srcdir_path, "/");
        return(srcdir_path);
    } /* end if */
    else
        return(NULL);
}

