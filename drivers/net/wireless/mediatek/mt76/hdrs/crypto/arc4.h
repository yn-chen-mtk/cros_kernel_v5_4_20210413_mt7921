#undef arc4_setkey
#define arc4_setkey LINUX_BACKPORT(arc4_setkey)
#undef arc4_crypt
#define arc4_crypt LINUX_BACKPORT(arc4_crypt)
#include <crypto/backport-arc4.h>
