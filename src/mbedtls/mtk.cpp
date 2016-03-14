#include "mtk.h"

VMINT g_bearer_hdl;

//VMSTR ca_path = "facebookcert.cer";

int ret_connect = -1;
int con_ret = -1;
int mbedtls_net_connect( mbedtls_net_context *ctx, const char *host, const char *port, int proto );
//int ret, len;
//mbedtls_net_context server_fd;
unsigned char buf[1024];
//const char *pers = "WifiWebClient";
// path to certification file

const char* CONNECT_IP_ADDRESS;
VMINT CONNECT_PORT;
// mbedtls_entropy_context entropy;
// mbedtls_ctr_drbg_context ctr_drbg;
// mbedtls_ssl_context ssl;
// mbedtls_ssl_config conf;

// mbedtls_x509_crt cacert;
// mbedtls_x509_crt clicert;
// mbedtls_pk_context pkey;
// uint32_t flags;

//SOCKADDR_IN       t_addr_in = {0};
//int               t_ai_socktype;   /* Socket type. */
//int               t_ai_protocol;   /* Protocol of socket. */
//uint32_t          t_ai_addrlen;    /* Length of socket address. */