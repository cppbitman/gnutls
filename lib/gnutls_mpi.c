/*
 *      Copyright (C) 2001,2002 Nikos Mavroyanopoulos
 *
 * This file is part of GNUTLS.
 *
 *  The GNUTLS library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public   
 *  License as published by the Free Software Foundation; either 
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */

#include <gnutls_int.h>
#include <libasn1.h>
#include <gnutls_errors.h>

/* Functions that refer to the libgcrypt library.
 */
 
void _gnutls_mpi_release( GNUTLS_MPI* x) {
	if (*x==NULL) return;
	gcry_mpi_release(*x);
	*x=NULL;
}

int _gnutls_mpi_scan( GNUTLS_MPI *ret_mpi, const opaque *buffer, size_t *nbytes ) {
	return gcry_mpi_scan( ret_mpi, GCRYMPI_FMT_USG, buffer, nbytes);
}

int _gnutls_mpi_scan_pgp( GNUTLS_MPI *ret_mpi, const opaque *buffer, size_t *nbytes ) {
	return gcry_mpi_scan( ret_mpi, GCRYMPI_FMT_PGP, buffer, nbytes);
}

int _gnutls_mpi_print( opaque *buffer, size_t *nbytes, const GNUTLS_MPI a ) {
	return gcry_mpi_print( GCRYMPI_FMT_USG, buffer, nbytes, a);
}

/* Always has the first bit zero */
int _gnutls_mpi_print_lz( opaque *buffer, size_t *nbytes, const GNUTLS_MPI a ) {
	return gcry_mpi_print( GCRYMPI_FMT_STD, buffer, nbytes, a);
}


/* this function reads an integer
 * from asn1 structs. Combines the read and mpi_scan
 * steps.
 */
int _gnutls_x509_read_int( ASN1_TYPE node, char* value, char* tmpstr, int tmpstr_size, GNUTLS_MPI* ret_mpi) {
int len, result;

	len = tmpstr_size - 1;
	result = asn1_read_value( node, value, tmpstr, &len);
	if (result != ASN1_SUCCESS) {
		gnutls_assert();
		return _gnutls_asn2err(result);
	}

	if (_gnutls_mpi_scan( ret_mpi, tmpstr, &len) != 0) {
		gnutls_assert();
		return GNUTLS_E_MPI_SCAN_FAILED;
	}

	return 0;
}

/* front end for asn1_create_element.
 */
asn1_retCode _gnutls_asn1_create_element(ASN1_TYPE definitions,char *source_name,
                                 ASN1_TYPE *element, char *dest_name) {

	*element = ASN1_TYPE_EMPTY;
	
	return asn1_create_element( definitions, source_name, element, dest_name);
                                 
}
