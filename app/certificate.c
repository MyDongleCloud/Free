#define _XOPEN_SOURCE 700
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include "macro.h"

unsigned int certificateGetExpiration(char *path) {
	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
	X509 *cert;
	FILE *fp = fopen(path, "r");
	if (!fp)
		return 0;
	cert = PEM_read_X509(fp, NULL, NULL, NULL);
	fclose(fp);
	if (!cert)
		return 0;
	const ASN1_TIME *expiration_date = X509_get_notAfter(cert);
	if (!expiration_date) {
		X509_free(cert);
		return 0;
	}
	BIO *bio = BIO_new(BIO_s_mem());
	if (!bio)
		return 0;
	ASN1_TIME_print(bio, expiration_date);
	char *buf;
	BIO_get_mem_data(bio, &buf);
	PRINTF("Expiration: %s\n", buf);
	struct tm tm_info;
	memset(&tm_info, 0, sizeof(struct tm));
	if (strptime(buf, "%b %d %H:%M:%S %Y %Z", &tm_info) == NULL)
		return 0;
	time_t epoch_time = mktime(&tm_info);
	if (epoch_time == (time_t) - 1)
		return 0;
	BIO_free(bio);
	X509_free(cert);
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	return (unsigned int)epoch_time;
}
