#ifndef _mtk_
#define _mtk_

#include "net.h"
#include "debug.h"
#include "ssl.h"
#include "entropy.h"
#include "ctr_drbg.h"
#include "error.h"  
#include "certs.h" 
#include "vmsys.h"
#include "vmsock.h"

#define VM_IS_SUCCEEDED(x) (((x)>=0)?VM_TRUE:VM_FALSE)    /* Use this macro to determine if a VM_RESULT is a success or not. */

extern VMINT g_bearer_hdl;

extern VMSTR ca_path;

extern int ret_connect;
extern int con_ret;
extern int mbedtls_net_connect( mbedtls_net_context *ctx, const char *host, const char *port, int proto );
//extern int ret, len;
//extern mbedtls_net_context server_fd;
extern unsigned char buf[1024];
//extern const char *pers;

extern void myprint(char *str);
extern void myprintln(char *str);

extern const char* CONNECT_IP_ADDRESS;
extern VMINT CONNECT_PORT;

// extern SOCKADDR_IN       t_addr_in = {0};
// extern int               t_ai_socktype;   /* Socket type. */
// extern int               t_ai_protocol;   /* Protocol of socket. */
// extern uint32_t          t_ai_addrlen;    /* Length of socket address. */

// extern mbedtls_entropy_context entropy;
// extern mbedtls_ctr_drbg_context ctr_drbg;
// extern mbedtls_ssl_context ssl;
// extern mbedtls_ssl_config conf;

// extern mbedtls_x509_crt cacert;
// extern mbedtls_x509_crt clicert;
// extern mbedtls_pk_context pkey;
// extern uint32_t flags;
#endif