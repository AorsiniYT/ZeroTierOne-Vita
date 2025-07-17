#pragma once
// Comunicación UDP y paso de paquetes

#ifdef __cplusplus
extern "C" {
#endif

int ztl_transport_init(void);
int ztl_transport_send(const void* data, int len);
int ztl_transport_recv(void* buf, int buflen);
void ztl_transport_close(void);

#ifdef __cplusplus
}
#endif
