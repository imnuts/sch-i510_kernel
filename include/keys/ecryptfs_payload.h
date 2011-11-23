#ifdef CONFIG_ECRYPTFS_ENABLED

#ifndef __ECRYPTFS_PAYLOAD_H__
#define __ECRYPTFS_PAYLOAD_H__

#include "ecryptfs_constants.h"

/* custom payload that will be converted to kernel specific structure */
struct _ecryptfs_payload {
	
	uint32	id;		/* to identify SEC payload */
	
	uint32 	hash_algo; 	/* hashing algo used by ecryptfs to hash fek */
	
	uint32 	skek_bytes;	/* fekek bytes */
	
	uint8  	skek[MAX_KEY_BYTES]; /* fekek -> key used to encrypt the fek */
	
	uint8	sig[PASSWORD_SIG_SIZE + 1]; /* unique identifier for auth token */
	
	uint8	salt[SALT_SIZE]; /* standard salt for hash algorithms */
	
} __attribute__ ((packed));

typedef struct _ecryptfs_payload ecryptfs_payload;

#endif // __ECRYPTFS_PAYLOAD_H__
#endif // CONFIG_ECRYPTFS_ENABLED

