#ifndef _UNIX_CRYPT_H
#define _UNIX_CRYPT_H

int get_sp_salt(const char *username,
                char **ret_salt,
                char **ret_encpwd);

#endif
