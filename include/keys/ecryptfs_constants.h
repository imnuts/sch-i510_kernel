#ifdef CONFIG_ECRYPTFS_ENABLED

#ifndef __ECRYPTFS_CONSTANTS_H__
#define __ECRYPTFS_CONSTANTS_H__

typedef unsigned short	uint16;
typedef unsigned char 	uint8;
typedef unsigned long 	uint32;

#define MAX_KEY_BYTES		32
#define PASSWORD_SIG_SIZE	16
#define SALT_SIZE		8
#define FEK_HASH_ALGO		10 // ? 
#define SEC_ID			0xcafebabe
#define ECRYPTFS_MAX_OPTIONS	1024

#define AUTH_TOKEN_TYPE		"user"
#define ECRYPTFS_MOUNT_DEVICE	"ecryptfs"

// ecryptfs options
#define ECRYPTFS_PLAINTEXT_PASSTHOUGH	"ecryptfs_passthrough"
#define ECRYPTFS_SIGNATURE		"ecryptfs_sig="
#define ECRYPTFS_CIPHER			"ecryptfs_cipher="
#define ECRYPTFS_KEY_BYTES		"ecryptfs_key_bytes="
#define ECRYPTFS_UNLINK_SIGNATURE	"ecryptfs_unlink_sig"

#define ECRYPTFS_FEK_CIPHER		"aes"

// this is versions of ecryptfs module
#define ECRYPTFS_MAJOR_VERSION 0x00
#define ECRYPTFS_MINOR_VERSION 0x04
#define ECRYPTFS_VERSION ((ECRYPTFS_MAJOR_VERSION << 8) | ECRYPTFS_MINOR_VERSION)

#define ECRYPTFS_PWD_PAYLOAD_TYPE	0 // password

#endif // __ECRYPTFS_CONSTANTS_H__
#endif // CONFIG_ECRYPTFS_ENABLED
