#ifdef CONFIG_ECRYPTFS_ENABLED

#include <keys/ecryptfs_constants.h>
#include <keys/ecryptfs_payload.h>
#include "../../fs/ecryptfs/ecryptfs_kernel.h"

int isSecPayLoad(void *payload, long plen) {
	if(plen == sizeof(ecryptfs_payload)) {
		ecryptfs_payload *pl = (ecryptfs_payload*)payload;
		return (pl->id == SEC_ID);
	}
	return 0;
}

void convertSecPayloadToEcryptfsPayload(void *secPayload, void *ecryptfs_kernel_payload) {

	ecryptfs_payload *sPl = (ecryptfs_payload*)secPayload;
	struct ecryptfs_auth_tok *kPl = (struct ecryptfs_auth_tok*)ecryptfs_kernel_payload;
	
	kPl->version = ECRYPTFS_VERSION;
	kPl->token_type = ECRYPTFS_PWD_PAYLOAD_TYPE;
	
	kPl->token.password.hash_algo = sPl->hash_algo;
	
	kPl->token.password.session_key_encryption_key_bytes = sPl->skek_bytes;
	kPl->token.password.flags = ECRYPTFS_SESSION_KEY_ENCRYPTION_KEY_SET;
	
	memcpy(kPl->token.password.session_key_encryption_key, sPl->skek, sPl->skek_bytes);
	memcpy(kPl->token.password.signature, sPl->sig, PASSWORD_SIG_SIZE);
	memcpy(kPl->token.password.salt, sPl->salt, SALT_SIZE);
}

#endif // CONFIG_ECRYPTFS_ENABLED
