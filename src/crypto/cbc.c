/*
 * Copyright (C) 2009 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <assert.h>
#include <gpxe/crypto.h>
#include <gpxe/cbc.h>

/** @file
 *
 * Cipher-block chaining
 *
 */

/**
 * XOR data blocks
 *
 * @v src		Input data
 * @v dst		Second input data and output data buffer
 * @v len		Length of data
 */
static void cbc_xor ( const void *src, void *dst, size_t len ) {
	const uint32_t *srcl = src;
	uint32_t *dstl = dst;
	unsigned int i;

	/* Assume that block sizes will always be dword-aligned, for speed */
	assert ( ( len % sizeof ( *srcl ) ) == 0 );

	for ( i = 0 ; i < ( len / sizeof ( *srcl ) ) ; i++ )
		dstl[i] ^= srcl[i];
}

/**
 * Encrypt data
 *
 * @v ctx		Context
 * @v src		Data to encrypt
 * @v dst		Buffer for encrypted data
 * @v len		Length of data
 * @v cipher		Underlying cipher algorithm
 * @v cbc_ctx		CBC context
 */
void cbc_encrypt ( void *ctx, const void *src, void *dst, size_t len,
		   struct cipher_algorithm *cipher, void *cbc_ctx ) {
	size_t blocksize = cipher->blocksize;

	assert ( ( len % blocksize ) == 0 );

	while ( len ) {
		cbc_xor ( src, cbc_ctx, blocksize );
		cipher_encrypt ( cipher, ctx, cbc_ctx, dst, blocksize );
		memcpy ( cbc_ctx, dst, blocksize );
		dst += blocksize;
		src += blocksize;
		len -= blocksize;
	}
}

/**
 * Decrypt data
 *
 * @v ctx		Context
 * @v src		Data to decrypt
 * @v dst		Buffer for decrypted data
 * @v len		Length of data
 * @v cipher		Underlying cipher algorithm
 * @v cbc_ctx		CBC context
 */
void cbc_decrypt ( void *ctx, const void *src, void *dst, size_t len,
		   struct cipher_algorithm *cipher, void *cbc_ctx ) {
	size_t blocksize = cipher->blocksize;

	assert ( ( len % blocksize ) == 0 );

	while ( len ) {
		cipher_decrypt ( cipher, ctx, src, dst, blocksize );
		cbc_xor ( cbc_ctx, dst, blocksize );
		memcpy ( cbc_ctx, src, blocksize );
		dst += blocksize;
		src += blocksize;
		len -= blocksize;
	}
}