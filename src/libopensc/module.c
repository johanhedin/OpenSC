/*
 * module.c: Dynamic linking loader
 *
 * Copyright (C) 2002  Antti Tapaninen <aet@cc.hut.fi>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "internal.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>

int sc_module_open(struct sc_context *ctx, void **mod_handle, const char *filename)
{
	const char *error;
	void *handle;

	assert(ctx != NULL);

	if (!filename)
		return SC_ERROR_UNKNOWN;

	handle = dlopen(filename, RTLD_LAZY);

	if ((error = dlerror()) != NULL) {
		if (ctx->debug)
			debug(ctx, "sc_module_open: %s\n", error);
		return SC_ERROR_UNKNOWN;
	}
	*mod_handle = handle;
	return SC_SUCCESS;
}

int sc_module_close(struct sc_context *ctx, void *mod_handle)
{
	const char *error;

	assert(ctx != NULL);

	if (!mod_handle)
		return SC_ERROR_UNKNOWN;

	dlclose(mod_handle);

	if ((error = dlerror()) != NULL) {
		if (ctx->debug)
			debug(ctx, "sc_module_close: %s\n", error);
		return SC_ERROR_UNKNOWN;
	}
	return SC_SUCCESS;
}

int sc_module_get_address(struct sc_context *ctx, void *mod_handle, void **sym_address, const char *sym_name)
{
	const char *error;
	char name[256];
	void *address;

	assert(ctx != NULL);

	if (!mod_handle || !sym_name)
		return SC_ERROR_UNKNOWN;

	/* Some platforms might need a leading underscore for the symbol */
	name[0] = '_';
	strncpy(&name[1], sym_name, sizeof(name) - 1);

	address = dlsym(mod_handle, name);

	/* Failed? Try again without the leading underscore */
	if (address == NULL)
		address = dlsym(mod_handle, sym_name);

	if ((error = dlerror()) != NULL) {
		if (ctx->debug)
			debug(ctx, "sc_module_get_address: %s\n", error);
		return SC_ERROR_UNKNOWN;
	}
	*sym_address = address;
	return SC_SUCCESS;
}

#elif defined(_WIN32)
#include <windows.h>

int sc_module_open(struct sc_context *ctx, void **mod_handle, const char *filename)
{
	void *handle;

	assert(ctx != NULL);

	if (!filename)
		return SC_ERROR_UNKNOWN;

	handle = LoadLibrary(filename);

	if (handle == NULL) {
		if (ctx->debug)
			/* TODO: GetLastError */
			debug(ctx, "sc_module_open: unknown error");
		return SC_ERROR_UNKNOWN;
	}
	*mod_handle = handle;
	return SC_SUCCESS;
}

int sc_module_close(struct sc_context *ctx, void *mod_handle)
{
	assert(ctx != NULL);

	if (!mod_handle)
		return SC_ERROR_UNKNOWN;

	FreeLibrary(mod_handle);
	/* TODO: GetLastError */

	return SC_SUCCESS;
}

int sc_module_get_address(struct sc_context *ctx, void *mod_handle, void **sym_address, const char *sym_name)
{
	void *address;

	assert(ctx != NULL);

	if (!mod_handle || !sym_name)
		return SC_ERROR_UNKNOWN;


	address = GetProcAddress(mod_handle, sym_name);

	if (address == NULL) {
		if (ctx->debug)
			/* TODO: GetLastError */
			debug(ctx, "sc_module_get_address: unknown error");
		return SC_ERROR_UNKNOWN;
	}
	*sym_address = address;
	return SC_SUCCESS;
}

#else

int sc_module_open(struct sc_context *ctx, void **mod_handle, const char *filename)
{
	return SC_ERROR_UNKNOWN;
}

int sc_module_close(struct sc_context *ctx, void *mod_handle)
{
	return SC_ERROR_UNKNOWN;
}

int sc_module_get_address(struct sc_context *ctx, void *mod_handle, void **sym_address, const char *sym_name)
{
	return SC_ERROR_UNKNOWN;
}

#endif
