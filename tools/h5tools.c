/*
 * Copyright � 1998 NCSA
 *                  All rights reserved.
 *
 * Programmer:  Robb Matzke <matzke@llnl.gov>
 *              Thursday, July 23, 1998
 *
 * Purpose:	A library for displaying the values of a dataset in a human
 *		readable format.
 */
#include <assert.h>
#include <ctype.h>
#include <h5tools.h>
#include <hdf5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <H5private.h>

/*
 * If REPEAT_VERBOSE is defined then character strings will be printed so
 * that repeated character sequences like "AAAAAAAAAA" are displayed as
 *
 * 	'A' repeates 9 times
 *
 * Otherwise the format is more Perl-like
 *
 * 	'A'*10
 * 
 */
#define REPEAT_VERBOSE


/*
 * The output functions need a temporary buffer to hold a piece of the
 * dataset while it's being printed.  This constant sets the limit on the
 * size of that temporary buffer in bytes.  For efficiency's sake, choose the
 * largest value suitable for your machine (for testing use a small value).
 */
#if 1
#define H5DUMP_BUFSIZE	(1024*1024)
#else
#define H5DUMP_BUFSIZE	(1024)
#endif

#define OPT(X,S)	((X)?(X):(S))
#define ALIGN(A,Z)	((((A)+(Z)-1)/(Z))*(Z))
#define START_OF_DATA	0x0001
#define END_OF_DATA	0x0002

/* Variable length string datatype */
#define STR_INIT_LEN	4096		/*initial length		*/
typedef struct h5dump_str_t {
    char		*s;		/*allocate string		*/
    size_t		len;		/*length of actual value	*/
    size_t		nalloc;		/*allocated size of string	*/
} h5dump_str_t;

/* Special strings embedded in the output */
#define OPTIONAL_LINE_BREAK	"\001"

/* Output variables */
typedef struct h5dump_context_t {
    size_t		cur_column;	/*current column for output	*/
    int			need_prefix;	/*is line prefix needed?	*/
    int			ndims;		/*dimensionality		*/
    hsize_t		p_min_idx[H5S_MAX_RANK]; /*min selected index	*/
    hsize_t		p_max_idx[H5S_MAX_RANK]; /*max selected index	*/
    int			prev_multiline;	/*was prev datum multiline?	*/
    size_t		prev_prefix_len;/*length of previous prefix	*/
} h5dump_context_t;
    

/*-------------------------------------------------------------------------
 * Function:	h5dump_str_close
 *
 * Purpose:	Closes a string by releasing it's memory and setting the size
 *		information to zero.
 *
 * Return:	void
 *
 * Programmer:	Robb Matzke
 *              Monday, April 26, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
h5dump_str_close(h5dump_str_t *str)
{
    if (str && str->nalloc) {
	free(str->s);
	memset(str, 0, sizeof(h5dump_str_t));
    }
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_str_len
 *
 * Purpose:	Returns the length of the string, not counting the null
 *		terminator.
 *
 * Return:	Success:	Length of string
 *
 *		Failure:	0
 *
 * Programmer:	Robb Matzke
 *              Monday, April 26, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static size_t
h5dump_str_len(h5dump_str_t *str)
{
    return str->len;
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_str_append
 *
 * Purpose:	Formats variable arguments according to printf() format
 *		string and appends the result to variable length string STR.
 *
 * Return:	Success:	Pointer to buffer containing result.
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *              Monday, April 26, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static char *
h5dump_str_append(h5dump_str_t *str/*in,out*/, const char *fmt, ...)
{
    va_list	ap;

    va_start(ap, fmt);

    /* Make sure we have some memory into which to print */
    if (!str->s || str->nalloc<=0) {
	str->nalloc = STR_INIT_LEN;
	str->s = malloc(str->nalloc);
	assert(str->s);
	str->s[0] = '\0';
	str->len = 0;
    }

    while (1) {
	size_t avail = str->nalloc - str->len;
	size_t nchars = HDvsnprintf(str->s+str->len, avail, fmt, ap);
	if (nchars<avail) {
	    /* success */
	    str->len += nchars;
	    break;
	}
	/* Try again with twice as much space */
	str->nalloc *= 2;
	str->s = realloc(str->s, str->nalloc);
	assert(str->s);
    }

    va_end(ap);
    return str->s;
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_str_reset
 *
 * Purpose:	Reset the string to the empty value. If no memory is
 *		allocated yet then initialize the h5dump_str_t struct.
 *
 * Return:	Success:	Ptr to the buffer which contains a null
 *				character as the first element.
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *              Monday, April 26, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static char *
h5dump_str_reset(h5dump_str_t *str/*in,out*/)
{
    if (!str->s || str->nalloc<=0) {
	str->nalloc = STR_INIT_LEN;
	str->s = malloc(str->nalloc);
	assert(str->s);
    }

    str->s[0] = '\0';
    str->len = 0;
    return str->s;
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_str_trunc
 *
 * Purpose:	Truncate a string to be at most SIZE characters.
 *
 * Return:	Success:	Pointer to the string
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *              Monday, April 26, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static char *
h5dump_str_trunc(h5dump_str_t *str/*in,out*/, size_t size)
{
    if (size<str->len) {
	str->len = size;
	str->s[size] = '\0';
    }
    return str->s;
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_str_fmt
 *
 * Purpose:	Reformat a string contents beginning at character START
 *		according to printf format FMT. FMT should contain no format
 *		specifiers except possibly the `%s' variety. For example, if
 *		the input string is `hello' and the format is "<<%s>>" then
 *		the output value will be "<<hello>>".
 *
 * Return:	Success:	A pointer to the resulting string.
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *              Monday, April 26, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static char *
h5dump_str_fmt(h5dump_str_t *str/*in,out*/, size_t start, const char *fmt)
{
    char	_temp[1024], *temp=_temp;

    /* If the format string is simply "%s" then don't bother doing anything */
    if (!strcmp(fmt, "%s")) return str->s;

    /*
     * Save the input value if there is a `%' anywhere in FMT.  Otherwise
     * don't bother because we don't need a temporary copy.
     */
    if (strchr(fmt, '%')) {
	if ((str->len-start)+1>sizeof _temp) {
	    temp = malloc((str->len-start)+1);
	    assert(temp);
	}
	strcpy(temp, str->s+start);
    }

    /* Reset the output string and append a formatted version */
    h5dump_str_trunc(str, start);
    h5dump_str_append(str, fmt, temp);

    /* Free the temp buffer if we allocated one */
    if (temp != _temp) free(temp);
    return str->s;
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_prefix
 *
 * Purpose:	Renders the line prefix value into string STR.
 *
 * Return:	Success:	Pointer to the prefix.
 *
 * 		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *              Thursday, July 23, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static char *
h5dump_prefix(h5dump_str_t *str/*in,out*/, const h5dump_t *info,
	      hsize_t elmtno, int ndims, hsize_t min_idx[], hsize_t max_idx[])
{
    hsize_t	p_prod[H5S_MAX_RANK], p_idx[H5S_MAX_RANK];
    hsize_t	n, i=0;

    h5dump_str_reset(str);
    if (ndims>0) {
	/*
	 * Calculate the number of elements represented by a unit change in a
	 * certain index position.
	 */
	for (i=ndims-1, p_prod[ndims-1]=1; i>0; --i) {
	    p_prod[i-1] = (max_idx[i]-min_idx[i]) * p_prod[i];
	}

	/*
	 * Calculate the index values from the element number.
	 */
	for (i=0, n=elmtno; i<(hsize_t)ndims; i++) {
	    p_idx[i] = n / p_prod[i] + min_idx[i];
	    n %= p_prod[i];
	}

	/*
	 * Print the index values.
	 */
	for (i=0; i<(hsize_t)ndims; i++) {
	    if (i) h5dump_str_append(str, "%s", OPT(info->idx_sep, ","));
	    h5dump_str_append(str, OPT(info->idx_n_fmt, "%lu"),
			      (unsigned long)p_idx[i]);
	}
    } else {
	/* Scalar */
	h5dump_str_append(str, OPT(info->idx_n_fmt, "%lu"), (unsigned long)0);
    }

    /*
     * Add prefix and suffix to the index.
     */
    return h5dump_str_fmt(str, 0, OPT(info->idx_fmt, "%s: "));
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_escape
 *
 * Purpose:	Changes all "funny" characters in S into standard C escape
 *		sequences. If ESCAPE_SPACES is non-zero then spaces are
 *		escaped by prepending a backslash.
 *
 * Return:	Success:	S
 *
 *		Failure:	NULL if the buffer would overflow. The
 *				buffer has as many left-to-right escapes as
 *				possible before overflow would have happened.
 *
 * Programmer:	Robb Matzke
 *              Monday, April 26, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static char *
h5dump_escape(char *s/*in,out*/, size_t size, int escape_spaces)
{
    size_t	n = strlen(s);
    size_t	i;
    const char	*escape;
    char	octal[8];
    
    for (i=0; i<n; i++) {
	switch (s[i]) {
	case '"':
	    escape = "\\\"";
	    break;
	case '\\':
	    escape = "\\\\";
	    break;
	case '\b':
	    escape = "\\b";
	    break;
	case '\f':
	    escape = "\\f";
	    break;
	case '\n':
	    escape = "\\n";
	    break;
	case '\r':
	    escape = "\\r";
	    break;
	case '\t':
	    escape = "\\t";
	    break;
	case ' ':
	    escape = escape_spaces ? "\\ " : NULL;
	    break;
	default:
	    if (!isprint(*s)) {
		sprintf(octal, "\\%03o", (unsigned char)(s[i]));
		escape = octal;
	    } else {
		escape = NULL;
	    }
	    break;
	}

	if (escape) {
	    size_t esc_size = strlen(escape);
	    if (n+esc_size+1>size) return NULL; /*would overflow*/
	    memmove(s+i+esc_size, s+i, (n-i)+1); /*make room*/
	    memcpy(s+i, escape, esc_size); /*insert*/
	    n += esc_size;
	    i += esc_size - 1;
	}
    }
    return s;
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_sprint
 *
 * Purpose:	Renders the value pointed to by VP of type TYPE into variable
 *		length string STR.
 *
 * Return:	A pointer to memory containing the result or NULL on error.
 *
 * Programmer:	Robb Matzke
 *              Thursday, July 23, 1998
 *
 * Modifications:
 * 		Robb Matzke, 1999-04-26
 *		Made this function safe from overflow problems by allowing it
 *		to reallocate the output string.
 *
 *-------------------------------------------------------------------------
 */
static char *
h5dump_sprint(h5dump_str_t *str/*in,out*/, const h5dump_t *info,
	      hid_t type, void *vp)
{
    size_t	i, n, offset, size, dims[H5S_MAX_RANK], nelmts, start;
    char	*name, quote='\0';
    hid_t	memb;
    int		nmembs, j, k, ndims;
    const int	repeat_threshold = 8;
    static char	fmt_llong[8], fmt_ullong[8];

    /* Build default formats for long long types */
    if (!fmt_llong[0]) {
	sprintf(fmt_llong, "%%%sd", PRINTF_LL_WIDTH);
	sprintf(fmt_ullong, "%%%su", PRINTF_LL_WIDTH);
    }

    /* Append value depending on data type */
    start = h5dump_str_len(str);
    if (H5Tequal(type, H5T_NATIVE_DOUBLE)) {
	h5dump_str_append(str, OPT(info->fmt_double, "%g"), *((double*)vp));
	
    } else if (H5Tequal(type, H5T_NATIVE_FLOAT)) {
	h5dump_str_append(str, OPT(info->fmt_double, "%g"), *((float*)vp));
	
    } else if (info->ascii &&
	       (H5Tequal(type, H5T_NATIVE_SCHAR) ||
		H5Tequal(type, H5T_NATIVE_UCHAR))) {
	switch (*((char*)vp)) {
	case '"':
	    h5dump_str_append(str, "\\\"");
	    break;
	case '\\':
	    h5dump_str_append(str, "\\\\");
	    break;
	case '\b':
	    h5dump_str_append(str, "\\b");
	    break;
	case '\f':
	    h5dump_str_append(str, "\\f");
	    break;
	case '\n':
	    h5dump_str_append(str, "\\n");
	    break;
	case '\r':
	    h5dump_str_append(str, "\\r");
	    break;
	case '\t':
	    h5dump_str_append(str, "\\t");
	    break;
	default:
	    if (isprint(*((char*)vp))) {
		h5dump_str_append(str, "%c", *((char*)vp));
	    } else {
		h5dump_str_append(str, "\\%03o", *((unsigned char*)vp));
	    }
	    break;
	}
	
    } else if (H5T_STRING==H5Tget_class(type)) {
	size = H5Tget_size(type);
	quote = '\0';

	for (i=0; i<size; i++) {

	    /* Count how many times the next character repeats */
	    j=1;
	    while (i+j<size && ((char*)vp)[i]==((char*)vp)[i+j]) j++;

	    /*
	     * Print the opening quote.  If the repeat count is high enough
	     * to warrant printing the number of repeats instead of
	     * enumerating the characters, then make sure the character to be
	     * repeated is in it's own quote.
	     */
	    if (j>repeat_threshold) {
		if (quote) h5dump_str_append(str, "%c", quote);
		quote = '\'';
		h5dump_str_append(str, "%s%c", i?" ":"", quote);
	    } else if (!quote) {
		quote = '"';
		h5dump_str_append(str, "%s%c", i?" ":"", quote);
	    }

	    /* Print the character */
	    switch (((char*)vp)[i]) {
	    case '"':
		h5dump_str_append(str, "\\\"");
		break;
	    case '\\':
		h5dump_str_append(str, "\\\\");
		break;
	    case '\b':
		h5dump_str_append(str, "\\b");
		break;
	    case '\f':
		h5dump_str_append(str, "\\f");
		break;
	    case '\n':
		h5dump_str_append(str, "\\n");
		break;
	    case '\r':
		h5dump_str_append(str, "\\r");
		break;
	    case '\t':
		h5dump_str_append(str, "\\t");
		break;
	    default:
		if (isprint(((char*)vp)[i])) {
		    h5dump_str_append(str, "%c", ((char*)vp)[i]);
		} else {
		    h5dump_str_append(str, "\\%03o", ((unsigned char*)vp)[i]);
		}
		break;
	    }

	    /* Print the repeat count */
	    if (j>repeat_threshold) {
#ifdef REPEAT_VERBOSE
		h5dump_str_append(str, "%c repeats %d times", quote, j-1);
#else
		h5dump_str_append(str, "%c*%d", quote, j-1);
#endif
		quote = '\0';
		i += j-1;
	    }
	}
	if (quote) h5dump_str_append(str, "%c", quote);
	
    } else if (H5Tequal(type, H5T_NATIVE_INT)) {
	h5dump_str_append(str, OPT(info->fmt_int, "%d"),
			  *((int*)vp));
	
    } else if (H5Tequal(type, H5T_NATIVE_UINT)) {
	h5dump_str_append(str, OPT(info->fmt_uint, "%u"),
			  *((unsigned*)vp));
	
    } else if (H5Tequal(type, H5T_NATIVE_SCHAR)) {
	h5dump_str_append(str, OPT(info->fmt_schar, "%d"),
			  *((signed char*)vp));
	
    } else if (H5Tequal(type, H5T_NATIVE_UCHAR)) {
	h5dump_str_append(str, OPT(info->fmt_uchar, "%u"),
			  *((unsigned char*)vp));
	
    } else if (H5Tequal(type, H5T_NATIVE_SHORT)) {
	h5dump_str_append(str, OPT(info->fmt_short, "%d"),
			  *((short*)vp));
	
    } else if (H5Tequal(type, H5T_NATIVE_USHORT)) {
	h5dump_str_append(str, OPT(info->fmt_ushort, "%u"),
			  *((unsigned short*)vp));
	
    } else if (H5Tequal(type, H5T_NATIVE_LONG)) {
	h5dump_str_append(str, OPT(info->fmt_long, "%ld"),
			  *((long*)vp));
	
    } else if (H5Tequal(type, H5T_NATIVE_ULONG)) {
	h5dump_str_append(str, OPT(info->fmt_ulong, "%lu"),
			  *((unsigned long*)vp));
	
    } else if (H5Tequal(type, H5T_NATIVE_LLONG)) {
	h5dump_str_append(str, OPT(info->fmt_llong, fmt_llong),
			  *((long_long*)vp));
	
    } else if (H5Tequal(type, H5T_NATIVE_ULLONG)) {
	h5dump_str_append(str, OPT(info->fmt_ullong, fmt_ullong),
			  *((unsigned long_long*)vp));
	
    } else if (H5Tequal(type, H5T_NATIVE_HSSIZE)) {
	if (sizeof(hssize_t)==sizeof(int)) {
	    h5dump_str_append(str, OPT(info->fmt_int, "%d"),
			      *((int*)vp));
	} else if (sizeof(hssize_t)==sizeof(long)) {
	    h5dump_str_append(str, OPT(info->fmt_long, "%ld"),
			      *((long*)vp));
	} else {
	    h5dump_str_append(str, OPT(info->fmt_llong, fmt_llong),
			      *((int64_t*)vp));
	}
	
    } else if (H5Tequal(type, H5T_NATIVE_HSIZE)) {
	if (sizeof(hsize_t)==sizeof(int)) {
	    h5dump_str_append(str, OPT(info->fmt_uint, "%u"),
			      *((unsigned*)vp));
	} else if (sizeof(hsize_t)==sizeof(long)) {
	    h5dump_str_append(str, OPT(info->fmt_ulong, "%lu"),
			      *((unsigned long*)vp));
	} else {
	    h5dump_str_append(str, OPT(info->fmt_ullong, fmt_ullong),
			      *((uint64_t*)vp));
	}
	
    } else if (H5T_COMPOUND==H5Tget_class(type)) {
	nmembs = H5Tget_nmembers(type);
	h5dump_str_append(str, "%s", OPT(info->cmpd_pre, "{"));
	for (j=0; j<nmembs; j++) {
	    if (j) h5dump_str_append(str, "%s",
				     OPT(info->cmpd_sep,
					 ", " OPTIONAL_LINE_BREAK));

	    /* The name */
	    name = H5Tget_member_name(type, j);
	    h5dump_str_append(str, OPT(info->cmpd_name, ""), name);
	    free(name);

	    /* The value */
	    offset = H5Tget_member_offset(type, j);
	    memb = H5Tget_member_type(type, j);
	    size = H5Tget_size(memb);
	    ndims = H5Tget_member_dims(type, j, dims, NULL);
	    assert(ndims>=0 && ndims<=H5S_MAX_RANK);
	    for (k=0, nelmts=1; k<ndims; k++) nelmts *= dims[k];

	    if (nelmts>1) {
		h5dump_str_append(str, "%s", OPT(info->arr_pre, "["));
	    }
	    for (i=0; i<nelmts; i++) {
		if (i) {
		    h5dump_str_append(str, "%s",
				      OPT(info->arr_sep,
					  "," OPTIONAL_LINE_BREAK));
		}
		h5dump_sprint(str, info, memb, (char*)vp+offset+i*size);
	    }
	    if (nelmts>1) {
		h5dump_str_append(str, "%s", OPT(info->arr_suf, "]"));
	    }
	    H5Tclose(memb);
	}
	h5dump_str_append(str, "%s", OPT(info->cmpd_suf, "}"));
	
    } else if (H5T_ENUM==H5Tget_class(type)) {
	char enum_name[1024];
	if (H5Tenum_nameof(type, vp, enum_name, sizeof enum_name)>=0) {
	    h5dump_escape(enum_name, sizeof enum_name, TRUE);
	} else {
	    h5dump_str_append(str, "0x");
	    n = H5Tget_size(type);
	    for (i=0; i<n; i++) {
		h5dump_str_append(str, "%02x", ((unsigned char*)vp)[i]);
	    }
	}
	
    } else {
	h5dump_str_append(str, "0x");
	n = H5Tget_size(type);
	for (i=0; i<n; i++) {
	    h5dump_str_append(str, "%02x", ((unsigned char*)vp)[i]);
	}
    }

    return h5dump_str_fmt(str, start, OPT(info->elmt_fmt, "%s"));
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_ncols
 *
 * Purpose:	Count the number of columns in a string. This is the number
 *		of characters in the string not counting line-control
 *		characters.
 *
 * Return:	Success:	Width of string.
 *
 *		Failure:	0
 *
 * Programmer:	Robb Matzke
 *              Tuesday, April 27, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static size_t
h5dump_ncols(const char *s)
{
    size_t	i;
    
    for (i=0; *s; s++) if (*s>=' ') i++;
    return i;
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_simple_prefix
 *
 * Purpose:	If ctx->need_prefix is set then terminate the current line
 *		(if applicable), calculate the prefix string, and display it
 * 		at the start of a line.
 *
 * Return:	void
 *
 * Programmer:	Robb Matzke
 *              Monday, April 26, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
h5dump_simple_prefix(FILE *stream, const h5dump_t *info,
		     h5dump_context_t *ctx, hsize_t elmtno, int secnum)
{
    h5dump_str_t	prefix;

    memset(&prefix, 0, sizeof(h5dump_str_t));
    if (!ctx->need_prefix) return;
    
    /* Terminate previous line, if any */
    if (ctx->cur_column) {
	fputs(OPT(info->line_suf, ""), stream);
	putc('\n', stream);
	fputs(OPT(info->line_sep, ""), stream);
    }

    /* Calculate new prefix */
    h5dump_prefix(&prefix, info, elmtno, ctx->ndims,
		  ctx->p_min_idx, ctx->p_max_idx);

    /* Write new prefix to output */
    if (0==elmtno && 0==secnum && info->line_1st) {
	fputs(h5dump_str_fmt(&prefix, 0, info->line_1st),
	      stream);
    } else if (secnum && info->line_cont) {
	fputs(h5dump_str_fmt(&prefix, 0, info->line_cont),
	      stream);
    } else {
	fputs(h5dump_str_fmt(&prefix, 0, info->line_pre),
	      stream);
    }
    ctx->cur_column = ctx->prev_prefix_len = h5dump_str_len(&prefix);
    ctx->need_prefix = 0;

    /* Free string */
    h5dump_str_close(&prefix);
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_simple_data
 *
 * Purpose:	Prints some (NELMTS) data elements to output STREAM. The
 *		elements are stored in _MEM as type TYPE and are printed
 *		according to the format described in INFO. The CTX struct
 *		contains context information shared between calls to this
 *		function.  The FLAGS is a bit field that indicates whether
 *		the data supplied in this call falls at the beginning or end
 *		of the total data to be printed (START_OF_DATA and
 *		END_OF_DATA).
 *
 * Return:	void
 *
 * Programmer:	Robb Matzke
 *              Monday, April 26, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
h5dump_simple_data(FILE *stream, const h5dump_t *info,
		   h5dump_context_t *ctx/*in,out*/, unsigned flags,
		   hsize_t nelmts, hid_t type, void *_mem)
{
    unsigned char	*mem = (unsigned char*)_mem;
    hsize_t		i;		/*element counter		*/
    char		*s, *section;	/*a section of output		*/
    int			secnum;		/*section sequence number	*/
    size_t		size;		/*size of each datum		*/
    size_t		ncols=80;	/*available output width	*/
    h5dump_str_t	buffer;		/*string into which to render	*/
    int			multiline;	/*datum was multiline		*/

    /* Setup */
    memset(&buffer, 0, sizeof(h5dump_str_t));
    size = H5Tget_size(type);
    if (info->line_ncols>0) ncols = info->line_ncols;
    h5dump_simple_prefix(stream, info, ctx, 0, 0);
    
    for (i=0; i<nelmts; i++) {
	
	/* Render the element */
	h5dump_str_reset(&buffer);
	h5dump_sprint(&buffer, info, type, mem+i*size);
	if (i+1<nelmts || 0==(flags & END_OF_DATA)) {
	    h5dump_str_append(&buffer, "%s", OPT(info->elmt_suf1, ","));
	}
	s = h5dump_str_fmt(&buffer, 0, "%s");

	/*
	 * If the element would split on multiple lines if printed at our
	 * current location...
	 */
	if (1==info->line_multi_new &&
	    (ctx->cur_column + h5dump_ncols(s) +
	     strlen(OPT(info->elmt_suf2, " ")) +
	     strlen(OPT(info->line_suf, ""))) > ncols) {
	    if (ctx->prev_multiline) {
		/*
		 * ... and the previous element also occupied more than one
		 * line, then start this element at the beginning of a line.
		 */
		ctx->need_prefix = TRUE;
	    } else if ((ctx->prev_prefix_len + h5dump_ncols(s) +
			strlen(OPT(info->elmt_suf2, " ")) +
			strlen(OPT(info->line_suf, ""))) <= ncols) {
		/* 
		 * ...but *could* fit on one line otherwise, then we
		 * should end the current line and start this element on its
		 * own line.
		 */
		ctx->need_prefix = TRUE;
	    }
	}

	/*
	 * If the previous element occupied multiple lines and this element
	 * is too long to fit on a line then start this element at the
	 * beginning of the line.
	 */
	if (1==info->line_multi_new &&
	    ctx->prev_multiline &&
	    (ctx->cur_column + h5dump_ncols(s) +
	     strlen(OPT(info->elmt_suf2, " ")) +
	     strlen(OPT(info->line_suf, ""))) > ncols) {
	    ctx->need_prefix = TRUE;
	}
	
	/*
	 * Each OPTIONAL_LINE_BREAK embedded in the rendered string can cause
	 * the data to split across multiple lines.  We display the sections
	 * one-at a time.
	 */
	for (secnum=0, multiline=0;
	     (section=strtok(secnum?NULL:s, OPTIONAL_LINE_BREAK));
	     secnum++) {
	    /*
	     * If the current section plus possible suffix and end-of-line
	     * information would cause the output to wrap then we need to
	     * start a new line.
	     */
	    if ((ctx->cur_column + strlen(section) +
		 strlen(OPT(info->elmt_suf2, " ")) +
		 strlen(OPT(info->line_suf, ""))) > ncols) {
		ctx->need_prefix = 1;
	    }

	    /*
	     * Print the prefix or separate the beginning of this element
	     * from the previous element.
	     */
	    if (ctx->need_prefix) {
		if (secnum) multiline++;
		h5dump_simple_prefix(stream, info, ctx, i, secnum);
	    } else if (i && 0==secnum) {
		fputs(OPT(info->elmt_suf2, " "), stream);
		ctx->cur_column += strlen(OPT(info->elmt_suf2, " "));
	    }
	    
	    /* Print the section */
	    fputs(section, stream);
	    ctx->cur_column += strlen(section);
	}
	ctx->prev_multiline = multiline;
    }
    h5dump_str_close(&buffer);
}

	

/*-------------------------------------------------------------------------
 * Function:	h5dump_simple_dset
 *
 * Purpose:	Print some values from a dataset with a simple data space.
 *		This is a special case of h5dump_dset().
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *              Thursday, July 23, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
h5dump_simple_dset(FILE *stream, const h5dump_t *info, hid_t dset,
		   hid_t p_type)
{
    hid_t		f_space;		/*file data space	*/
    hsize_t		elmtno, i;		/*counters		*/
    int			carry;			/*counter carry value	*/
    hssize_t		zero[8];		/*vector of zeros	*/
    unsigned		flags;			/*buffer extent flags	*/

    /* Print info */
    h5dump_context_t	ctx;			/*print context		*/
    size_t		p_type_nbytes;		/*size of memory type	*/
    hsize_t		p_nelmts;		/*total selected elmts	*/

    /* Stripmine info */
    hsize_t		sm_size[H5S_MAX_RANK];	/*stripmine size	*/
    hsize_t		sm_nbytes;		/*bytes per stripmine	*/
    hsize_t		sm_nelmts;		/*elements per stripmine*/
    unsigned char	*sm_buf=NULL;		/*buffer for raw data	*/
    hid_t		sm_space;		/*stripmine data space	*/

    /* Hyperslab info */
    hssize_t		hs_offset[H5S_MAX_RANK];/*starting offset	*/
    hsize_t		hs_size[H5S_MAX_RANK];	/*size this pass	*/
    hsize_t		hs_nelmts;		/*elements in request	*/



    /*
     * Check that everything looks okay.  The dimensionality must not be too
     * great and the dimensionality of the items selected for printing must
     * match the dimensionality of the dataset.
     */
    memset(&ctx, 0, sizeof ctx);
    ctx.need_prefix = 1;
    f_space = H5Dget_space(dset);
    ctx.ndims = H5Sget_simple_extent_ndims(f_space);
    if ((size_t)(ctx.ndims)>NELMTS(sm_size)) return -1;

    /* Assume entire data space to be printed */
    for (i=0; i<(hsize_t)(ctx.ndims); i++) ctx.p_min_idx[i] = 0;
    H5Sget_simple_extent_dims(f_space, ctx.p_max_idx, NULL);
    for (i=0, p_nelmts=1; i<(hsize_t)(ctx.ndims); i++) {
	p_nelmts *= ctx.p_max_idx[i]-ctx.p_min_idx[i];
    }
    if (0==p_nelmts) return 0; /*nothing to print*/

    /*
     * Determine the strip mine size and allocate a buffer.  The strip mine is
     * a hyperslab whose size is manageable.
     */
    p_type_nbytes = H5Tget_size(p_type);
    for (i=ctx.ndims, sm_nbytes=p_type_nbytes; i>0; --i) {
	sm_size[i-1] = MIN (ctx.p_max_idx[i-1] - ctx.p_min_idx[i-1],
			    H5DUMP_BUFSIZE/sm_nbytes);
	sm_nbytes *= sm_size[i-1];
	assert(sm_nbytes>0);
    }
    sm_buf = malloc(sm_nbytes);
    sm_nelmts = sm_nbytes/p_type_nbytes;
    sm_space = H5Screate_simple(1, &sm_nelmts, NULL);

    /* The stripmine loop */
    memset(hs_offset, 0, sizeof hs_offset);
    memset(zero, 0, sizeof zero);
    for (elmtno=0; elmtno<p_nelmts; elmtno+=hs_nelmts) {

	/* Calculate the hyperslab size */
	if (ctx.ndims>0) {
	    for (i=0, hs_nelmts=1; i<(hsize_t)(ctx.ndims); i++) {
		hs_size[i] = MIN(ctx.p_max_idx[i]-hs_offset[i], sm_size[i]);
		hs_nelmts *= hs_size[i];
	    }
	    H5Sselect_hyperslab(f_space, H5S_SELECT_SET, hs_offset, NULL,
				hs_size, NULL);
	    H5Sselect_hyperslab(sm_space, H5S_SELECT_SET, zero, NULL,
				&hs_nelmts, NULL);
	} else {
	    H5Sselect_all(f_space);
	    H5Sselect_all(sm_space);
	    hs_nelmts = 1;
	}
	
	/* Read the data */
	if (H5Dread(dset, p_type, sm_space, f_space, H5P_DEFAULT, sm_buf)<0) {
	    return -1;
	}

	/* Print the data */
	flags = ((0==elmtno?START_OF_DATA:0) |
		 (elmtno+hs_nelmts>=p_nelmts?END_OF_DATA:0));
	h5dump_simple_data(stream, info, &ctx, flags, hs_nelmts, p_type,
			   sm_buf);
	
	/* Calculate the next hyperslab offset */
	for (i=ctx.ndims, carry=1; i>0 && carry; --i) {
	    hs_offset[i-1] += hs_size[i-1];
	    if (hs_offset[i-1]==(hssize_t)(ctx.p_max_idx[i-1])) {
		hs_offset[i-1] = ctx.p_min_idx[i-1];
	    } else {
		carry = 0;
	    }
	}
    }

    /* Terminate the output */
    if (ctx.cur_column) {
	fputs(OPT(info->line_suf, ""), stream);
	putc('\n', stream);
	fputs(OPT(info->line_sep, ""), stream);
    }
    H5Sclose(sm_space);
    H5Sclose(f_space);
    if (sm_buf) free(sm_buf);
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_simple_mem
 *
 * Purpose:	Print some values from memory with a simple data space.
 *		This is a special case of h5dump_mem().
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *              Thursday, July 23, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
h5dump_simple_mem(FILE *stream, const h5dump_t *info, hid_t type,
		  hid_t space, void *mem)
{
    hsize_t		i;			/*counters		*/
    size_t		size;			/*size of each element	*/
    hsize_t		nelmts;			/*total selected elmts	*/
    h5dump_context_t	ctx;			/*printing context	*/

    /*
     * Check that everything looks okay.  The dimensionality must not be too
     * great and the dimensionality of the items selected for printing must
     * match the dimensionality of the dataset.
     */
    memset(&ctx, 0, sizeof ctx);
    ctx.need_prefix = 1;
    ctx.ndims = H5Sget_simple_extent_ndims(space);
    if ((size_t)(ctx.ndims)>NELMTS(ctx.p_min_idx)) return -1;

    /* Assume entire data space to be printed */
    for (i=0; i<(hsize_t)(ctx.ndims); i++) ctx.p_min_idx[i] = 0;
    H5Sget_simple_extent_dims(space, ctx.p_max_idx, NULL);
    for (i=0, nelmts=1; i<(hsize_t)(ctx.ndims); i++) {
	nelmts *= ctx.p_max_idx[i] - ctx.p_min_idx[i];
    }
    if (0==nelmts) return 0; /*nothing to print*/
    size = H5Tget_size(type);

    /* Print it */
    h5dump_simple_data(stream, info, &ctx, START_OF_DATA|END_OF_DATA,
		       nelmts, type, mem);

    /* Terminate the output */
    if (ctx.cur_column) {
	fputs(OPT(info->line_suf, ""), stream);
	putc('\n', stream);
	fputs(OPT(info->line_sep, ""), stream);
    }
    
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_fixtype
 *
 * Purpose:	Given a file data type choose a memory data type which is
 *		appropriate for printing the data.
 *
 * Return:	Success:	Memory data type
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *              Thursday, July 23, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
hid_t
h5dump_fixtype(hid_t f_type)
{
    hid_t	m_type=-1, f_memb;
    hid_t	*memb=NULL;
    char	**name=NULL;
    int		nmembs=0, i, j, *ndims=NULL;
    size_t	size, offset, *dims=NULL, nelmts;

    size = H5Tget_size(f_type);
    switch (H5Tget_class(f_type)) {

    case H5T_INTEGER:
	/*
	 * Use the smallest native integer type of the same sign as the file
	 * such that the memory type is at least as large as the file type.
	 * If there is no memory type large enough then use the largest
	 * memory type available.
	 */
	if (size<=sizeof(char)) {
	    m_type = H5Tcopy(H5T_NATIVE_SCHAR);
	} else if (size<=sizeof(short)) {
	    m_type = H5Tcopy(H5T_NATIVE_SHORT);
	} else if (size<=sizeof(int)) {
	    m_type = H5Tcopy(H5T_NATIVE_INT);
	} else if (size<=sizeof(long)) {
	    m_type = H5Tcopy(H5T_NATIVE_LONG);
	} else {
	    m_type = H5Tcopy(H5T_NATIVE_LLONG);
	}
	H5Tset_sign(m_type, H5Tget_sign(f_type));
	break;
	
    case H5T_FLOAT:
	/*
	 * Use the smallest native floating point type available such that
	 * its size is at least as large as the file type.  If there is not
	 * native type large enough then use the largest native type.
	 */
	if (size<=sizeof(float)) {
	    m_type = H5Tcopy(H5T_NATIVE_FLOAT);
	} else if (size<=sizeof(double)) {
	    m_type = H5Tcopy(H5T_NATIVE_DOUBLE);
	} else {
	    m_type = H5Tcopy(H5T_NATIVE_LDOUBLE);
	}
	break;

    case H5T_STRING:
	m_type = H5Tcopy(f_type);
	H5Tset_cset(m_type, H5T_CSET_ASCII);
	break;

    case H5T_COMPOUND:
	/*
	 * We have to do this in two steps.  The first step scans the file
	 * type and converts the members to native types and remembers all
	 * their names and sizes, computing the size of the memory compound
	 * type at the same time.  Then we create the memory compound type
	 * and add the members.
	 */
	nmembs = H5Tget_nmembers(f_type);
	memb = calloc(nmembs, sizeof(hid_t));
	name = calloc(nmembs, sizeof(char*));
	ndims = calloc(nmembs, sizeof(int));
	dims = calloc(nmembs*4, sizeof(size_t));
	
	for (i=0, size=0; i<nmembs; i++) {

	    /* Get the member type and fix it */
	    f_memb = H5Tget_member_type(f_type, i);
	    memb[i] = h5dump_fixtype(f_memb);
	    H5Tclose(f_memb);
	    if (memb[i]<0) goto done;

	    /* Get the member dimensions */
	    ndims[i] = H5Tget_member_dims(f_type, i, dims+i*4, NULL);
	    assert(ndims[i]>=0 && ndims[i]<=4);
	    for (j=0, nelmts=1; j<ndims[i]; j++) nelmts *= dims[i*4+j];

	    /* Get the member name */
	    name[i] = H5Tget_member_name(f_type, i);
	    if (NULL==name[i]) goto done;

	    /*
	     * Compute the new offset so each member is aligned on a byte
	     * boundary which is the same as the member size.
	     */
	    size = ALIGN(size, H5Tget_size(memb[i])) +
		     nelmts * H5Tget_size(memb[i]);
	}

	m_type = H5Tcreate(H5T_COMPOUND, size);
	for (i=0, offset=0; i<nmembs; i++) {
	    H5Tinsert_array(m_type, name[i], offset, ndims[i], dims+i*4,
			    NULL, memb[i]);
	    for (j=0, nelmts=1; j<ndims[i]; j++) nelmts *= dims[i*4+j];
	    offset = ALIGN(offset, H5Tget_size(memb[i])) +
		     nelmts * H5Tget_size(memb[i]);
	}
	break;

    case H5T_ENUM:
	m_type = H5Tcopy(f_type);
	break;

    case H5T_TIME:
    case H5T_BITFIELD:
    case H5T_OPAQUE:
	/*
	 * These type classes are not implemented yet.
	 */
	break;

    default:
	/* What the heck? */
	break;
    }

 done:
    /* Clean up temp buffers */
    if (memb && name && ndims && dims) {
	for (i=0; i<nmembs; i++) {
	    if (memb[i]>=0) H5Tclose(memb[i]);
	    if (name[i]) free(name[i]);
	}
	free(memb);
	free(name);
	free(ndims);
	free(dims);
    }
    
    return m_type;
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_dset
 *
 * Purpose:	Print some values from a dataset DSET to the file STREAM
 *		after converting all types to P_TYPE (which should be a
 *		native type).  If P_TYPE is a negative value then it will be
 *		computed from the dataset type using only native types.
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *              Thursday, July 23, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
h5dump_dset(FILE *stream, const h5dump_t *info, hid_t dset, hid_t _p_type)
{
    hid_t	f_space;
    hid_t	p_type = _p_type;
    hid_t	f_type;
    int		status;
    h5dump_t	info_dflt;

    /* Use default values */
    if (!stream) stream = stdout;
    if (!info) {
	memset(&info_dflt, 0, sizeof info_dflt);
	info = &info_dflt;
    }
    if (p_type<0) {
	f_type = H5Dget_type(dset);
	p_type = h5dump_fixtype(f_type);
	H5Tclose(f_type);
	if (p_type<0) return -1;
    }

    /* Check the data space */
    f_space = H5Dget_space(dset);
    if (H5Sis_simple(f_space)<=0) return -1;
    H5Sclose(f_space);

    /* Print the data */
    status = h5dump_simple_dset(stream, info, dset, p_type);
    if (p_type!=_p_type) H5Tclose(p_type);
    return status;
}


/*-------------------------------------------------------------------------
 * Function:	h5dump_mem
 *
 * Purpose:	Displays the data contained in MEM. MEM must have the
 *		specified data TYPE and SPACE.  Currently only simple data
 *		spaces are allowed and only the `all' selection.
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *              Wednesday, January 20, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
h5dump_mem(FILE *stream, const h5dump_t *info, hid_t type, hid_t space,
	   void *mem)
{
    h5dump_t	info_dflt;
    
    /* Use default values */
    if (!stream) stream = stdout;
    if (!info) {
	memset(&info_dflt, 0, sizeof info_dflt);
	info = &info_dflt;
    }

    /* Check the data space */
    if (H5Sis_simple(space)<=0) return -1;
    return h5dump_simple_mem(stream, info, type, space, mem);
}
