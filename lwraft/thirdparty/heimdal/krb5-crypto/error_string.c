/*
 * Copyright (c) 2001, 2003, 2005 - 2006 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "krb5_locl.h"
#include "krb5-protos.h"

#undef __attribute__
#define __attribute__(x)

/*
 * Return buffer large enough to contain specified varargs format
 */
char *krb5_sprintf_alloc_buf(const char *fmt, ...)
{
    va_list ap;
    int len = 0;
    char buf[2] = {0};
    char *retBuf = NULL;

    va_start(ap, fmt);
#ifdef _WIN32
    len = _vscprintf(fmt, ap);
#else
    len = vsnprintf(buf, 0, fmt, ap);
#endif
    va_end(ap);
    retBuf = (char *) malloc(len + 1);
    return retBuf;
}

char *krb5_vsprintf_alloc_buf(const char *fmt, va_list args)
{
    int len = 0;
    char buf[2] = {0};
    char *retBuf = NULL;

#ifdef _WIN32
    len = _vscprintf(fmt, args);
#else
    len = vsnprintf(buf, 0, fmt, args);
#endif
    retBuf = (char *) malloc(len + 1);
    return retBuf;
}

/**
 * Clears the error message from the Kerberos 5 context.
 *
 * @param context The Kerberos 5 context to clear
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_clear_error_message(krb5_context context)
{
    HEIMDAL_MUTEX_lock(context->mutex);
    if (context->error_string)
	free(context->error_string);
    context->error_code = 0;
    context->error_string = NULL;
    HEIMDAL_MUTEX_unlock(context->mutex);
}

/**
 * Set the context full error string for a specific error code.
 * The error that is stored should be internationalized.
 *
 * The if context is NULL, no error string is stored.
 *
 * @param context Kerberos 5 context
 * @param ret The error code
 * @param fmt Error string for the error code
 * @param ... printf(3) style parameters.
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_set_error_message(krb5_context context, krb5_error_code ret,
		       const char *fmt, ...)
    __attribute__ ((format (printf, 3, 4)))
{
    va_list ap;

    va_start(ap, fmt);
    krb5_heim_vset_error_message (context, ret, fmt, ap);
    va_end(ap);
}

/**
 * Set the context full error string for a specific error code.
 *
 * The if context is NULL, no error string is stored.
 *
 * @param context Kerberos 5 context
 * @param ret The error code
 * @param fmt Error string for the error code
 * @param args printf(3) style parameters.
 *
 * @ingroup krb5_error
 */


KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_vset_error_message (krb5_context context, krb5_error_code ret,
			 const char *fmt, va_list args)
    __attribute__ ((format (printf, 3, 0)))
{
    va_list args_copy;
 
    if (context == NULL)
	return;

    HEIMDAL_MUTEX_lock(context->mutex);
    if (context->error_string) {
	free(context->error_string);
	context->error_string = NULL;
    }
#ifndef _WIN32
    va_copy(args_copy, args);
#else
    args_copy = args;
#endif
    context->error_code = ret;
    context->error_string = krb5_vsprintf_alloc_buf(fmt, args);
    if (context->error_string) {
        vsprintf(context->error_string, fmt, args_copy);
    }
    else {
	context->error_string = NULL;
    }
#ifndef _WIN32
    va_end(args_copy);
#endif
    HEIMDAL_MUTEX_unlock(context->mutex);
}

/**
 * Prepend the context full error string for a specific error code.
 * The error that is stored should be internationalized.
 *
 * The if context is NULL, no error string is stored.
 *
 * @param context Kerberos 5 context
 * @param ret The error code
 * @param fmt Error string for the error code
 * @param ... printf(3) style parameters.
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_prepend_error_message(krb5_context context, krb5_error_code ret,
			   const char *fmt, ...)
    __attribute__ ((format (printf, 3, 4)))
{
    va_list ap;

    va_start(ap, fmt);
    krb5_heim_vprepend_error_message(context, ret, fmt, ap);
    va_end(ap);
}

/**
 * Prepend the contexts's full error string for a specific error code.
 *
 * The if context is NULL, no error string is stored.
 *
 * @param context Kerberos 5 context
 * @param ret The error code
 * @param fmt Error string for the error code
 * @param args printf(3) style parameters.
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_vprepend_error_message(krb5_context context, krb5_error_code ret,
			    const char *fmt, va_list args)
    __attribute__ ((format (printf, 3, 0)))
{
    char *str = NULL, *str2 = NULL;

    if (context == NULL)
	return;

    HEIMDAL_MUTEX_lock(context->mutex);
    if (context->error_code != ret) {
	HEIMDAL_MUTEX_unlock(context->mutex);
	return;
    }

    str = krb5_vsprintf_alloc_buf(fmt, args);
    if (str) {
        vsprintf(str, fmt, args);
    }
    
    if (str == NULL) {
	HEIMDAL_MUTEX_unlock(context->mutex);
	return;
    }
    if (context->error_string) {
        str2 = krb5_sprintf_alloc_buf("%s: %s", str, context->error_string);
        if (str2) {
	    sprintf(str2, "%s: %s", str, context->error_string);
        }
	free(context->error_string);
	if (str2 == NULL)
	    context->error_string = NULL;
	else
	    context->error_string = str2;
	free(str);
    } else
	context->error_string = str;
    HEIMDAL_MUTEX_unlock(context->mutex);
}


/**
 * Return the error message in context. On error or no error string,
 * the function returns NULL.
 *
 * @param context Kerberos 5 context
 *
 * @return an error string, needs to be freed with
 * krb5_free_error_message(). The functions return NULL on error.
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION char * KRB5_LIB_CALL
krb5_get_error_string(krb5_context context)
{
    char *ret = NULL;

    HEIMDAL_MUTEX_lock(context->mutex);
    if (context->error_string)
	ret = strdup(context->error_string);
    HEIMDAL_MUTEX_unlock(context->mutex);
    return ret;
}

KRB5_LIB_FUNCTION krb5_boolean KRB5_LIB_CALL
krb5_have_error_string(krb5_context context)
{
    char *str;
    HEIMDAL_MUTEX_lock(context->mutex);
    str = context->error_string;
    HEIMDAL_MUTEX_unlock(context->mutex);
    return str != NULL;
}

/**
 * Return the error message for `code' in context. On memory
 * allocation error the function returns NULL.
 *
 * @param context Kerberos 5 context
 * @param code Error code related to the error
 *
 * @return an error string, needs to be freed with
 * krb5_free_error_message(). The functions return NULL on error.
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION const char * KRB5_LIB_CALL
krb5_heim_get_error_message(krb5_context context, krb5_error_code code)
{
    char *str = NULL;
    const char *cstr = NULL;
#if 0 /* TBD: Adam fix unused variable; used com_right_r() below*/
    char buf[128];
#endif
    int free_context = 0;

    if (code == 0)
	return strdup("Success");

    /*
     * The MIT version of this function ignores the krb5_context
     * and several widely deployed applications call krb5_get_error_message()
     * with a NULL context in order to translate an error code as a
     * replacement for error_message().  Another reason a NULL context
     * might be provided is if the krb5_init_context() call itself
     * failed.
     */
    if (context)
    {
        HEIMDAL_MUTEX_lock(context->mutex);
        if (context->error_string &&
            (code == context->error_code || context->error_code == 0))
        {
            str = strdup(context->error_string);
        }
        HEIMDAL_MUTEX_unlock(context->mutex);

        if (str)
            return str;
    }
    else
    {
        if (krb5_heim_init_context(&context) == 0)
            free_context = 1;
    }

#if 0 /* TBD: Adam */
    if (context)
        cstr = com_right_r(context->et_list, code, buf, sizeof(buf));
#endif

    if (free_context)
        krb5_heim_free_context(context);

    if (cstr)
        return strdup(cstr);

#if 0 /* TBD: Adam */
    cstr = error_message(code);
    if (cstr)
        return strdup(cstr);
#endif

    str = krb5_sprintf_alloc_buf("<unknown error: %d>", (int)code);
    if (str) {
        sprintf(str, "<unknown error: %d>", (int)code);
    }
    else {
	return NULL;
    }
    return str;
}


/**
 * Free the error message returned by krb5_get_error_message().
 *
 * @param context Kerberos context
 * @param msg error message to free, returned byg
 *        krb5_get_error_message().
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_free_error_message(krb5_context context, const char *msg)
{
    free(rk_UNCONST(msg));
}


#if 0 
/* TBD: adam isn't needed */
/**
 * Return the error string for the error code. The caller must not
 * free the string.
 *
 * This function is deprecated since its not threadsafe.
 *
 * @param context Kerberos 5 context.
 * @param code Kerberos error code.
 *
 * @return the error message matching code
 *
 * @ingroup krb5
 */

KRB5_LIB_FUNCTION const char* KRB5_LIB_CALL
krb5_get_err_text(krb5_context context, krb5_error_code code)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    const char *p = NULL;
    if(context != NULL)
	p = com_right(context->et_list, code);
    if(p == NULL)
	p = strerror(code);
    if (p == NULL)
	p = "Unknown error";
    return p;
}
#endif /* if 0 */
