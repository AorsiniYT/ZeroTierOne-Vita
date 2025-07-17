#include "peer.h"
#include "transport.h"
#include <string.h>

int ztl_peer_init(ztl_peer_t* peer, const ztl_identity_t* id) {
    memset(peer, 0, sizeof(ztl_peer_t));
    if (id) peer->identity = *id;
    return 0;
}

int ztl_peer_authenticate(ztl_peer_t* peer) {
    // Enviar paquete de hello/discovery al controller de ZeroTierOne
    // Controller público: 35.205.255.45:9993 (ejemplo)
    // El paquete debe incluir el address y parte de la identidad
    unsigned char hello[64];
    memset(hello, 0, sizeof(hello));
    // Formato simple: [address(8)][public_key(32)][padding]
    memcpy(hello, peer->identity.address, 8);
    memcpy(hello+8, peer->identity.public_key, 32);
    // Enviar por UDP
    // NOTA: ztl_transport_send debe enviar a 35.205.255.45:9993
    // (esto requiere que ztl_transport_send use ese destino temporalmente)
    int sent = ztl_transport_send(hello, sizeof(hello));
    if (sent == sizeof(hello)) {
        peer->authenticated = 1; // Marcar como autenticado (simulado)
        return 0;
    }
    return -1;
}
