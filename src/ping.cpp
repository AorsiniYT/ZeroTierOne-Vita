// --- INCLUDES VITA KERNEL NECESARIOS ---
#include "ping.h"
#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/net/net.h>
#include <psp2/ctrl.h>
#include <cstring>
#include <cstdio>
#include "common/debugScreen.h"
#include "debug.h"
#define printf psvDebugScreenPrintf
#define BOOST_ASIO_DISABLE_THREADS
#include <boost/asio.hpp>
#include <boost/asio/ip/icmp.hpp>
#include <chrono>

#ifndef SceNetTimeval_defined
#define SceNetTimeval_defined
typedef struct SceNetTimeval {
    long tv_sec;
    long tv_usec;
} SceNetTimeval;
#endif

// Implementación de la función de checksum RFC1071
uint16_t in_cksum(uint16_t *ptr, int nbytes) {
    uint32_t sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1)
        sum += *(uint8_t *)ptr;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (uint16_t)(~sum);
}

void run_ping_test(const char* ip_addr) {
    if (!ip_addr || strlen(ip_addr) == 0) {
        printf("[Ping Test] No se recibió IP destino. Abortando.\n");
        vita_debug_log("[Ping Test] No se recibió IP destino. Abortando.\n");
        return;
    }
    printf("\n[Ping Test] Ping a %s (pulsa círculo para salir)\n", ip_addr);
    vita_debug_log("[LOG] Iniciando ping a %s\n", ip_addr);

    SceCtrlData ctrl;
    bool salir = false;
    // Detectar si la IP es local (privada)
    auto is_local_ip = [](const char* ip) {
        // 10.x.x.x, 192.168.x.x, 172.16.x.x - 172.31.x.x
        int a, b, c, d;
        if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
            if (a == 10) return true;
            if (a == 192 && b == 168) return true;
            if (a == 172 && b >= 16 && b <= 31) return true;
        }
        return false;
    };
    int delay_ms = is_local_ip(ip_addr) ? 1 : 10;
    while (!salir) {
        // --- MODO BOOST.ASIO SINGLE-THREADED ---
        try {
            boost::asio::io_context io_context;
            boost::asio::ip::icmp::resolver resolver(io_context);
            auto endpoints = resolver.resolve(boost::asio::ip::icmp::v4(), ip_addr, "");
            boost::asio::ip::icmp::endpoint destination = *endpoints.begin();
            boost::asio::ip::icmp::socket socket(io_context, boost::asio::ip::icmp::v4());

            // Crear paquete ICMP Echo Request
            std::string body = "Ping Vita Boost";
            boost::asio::streambuf request;
            std::ostream os(&request);
            uint8_t type = 8, code = 0;
            uint16_t id = 1, seq = 0x1234;
            uint16_t checksum = 0;
            os.put(type); os.put(code);
            os.put(0); os.put(0); // checksum placeholder
            os.put(id >> 8); os.put(id & 0xFF);
            os.put(seq >> 8); os.put(seq & 0xFF);
            os << body;
            std::vector<uint8_t> data(boost::asio::buffer_cast<const uint8_t*>(request.data()), boost::asio::buffer_cast<const uint8_t*>(request.data()) + request.size());
            uint32_t sum = 0;
            for (size_t i = 0; i < data.size(); i += 2) {
                uint16_t word = data[i] << 8;
                if (i + 1 < data.size()) word |= data[i + 1];
                sum += word;
            }
            while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
            checksum = ~sum;
            data[2] = checksum >> 8;
            data[3] = checksum & 0xFF;

            // Enviar paquete
            auto start = std::chrono::steady_clock::now();
            socket.send_to(boost::asio::buffer(data), destination);
            printf("[Boost.Asio] Paquete ICMP enviado (%zu bytes)\n", data.size());
            vita_debug_log("[Boost.Asio] Paquete ICMP enviado (%zu bytes)\n", data.size());

        // Esperar respuesta con timeout manual y chequeo de botón
        std::array<uint8_t, 256> reply_buf;
        boost::asio::ip::icmp::endpoint sender;
        socket.non_blocking(true); // modo no bloqueante
        size_t len = 0;
        bool received = false;
        auto t_start = std::chrono::steady_clock::now();
        double ms = 0.0;
        for (int i = 0; i < 20; ++i) { // Espera hasta 2 segundos (20x100ms)
            try {
                len = socket.receive_from(boost::asio::buffer(reply_buf), sender);
            } catch (...) {
                len = 0;
            }
            if (len > 0) {
                auto t_recv = std::chrono::steady_clock::now();
                ms = std::chrono::duration<double, std::milli>(t_recv - start).count();
                received = true;
                break;
            }
            // Chequear botón círculo durante la espera
            sceCtrlPeekBufferPositive(0, &ctrl, 1);
            if (ctrl.buttons & SCE_CTRL_CIRCLE) {
                salir = true;
                break;
            }
            sceKernelDelayThread(delay_ms*1000); // delay dinámico
        }
        if (received) {
            printf("[Boost.Asio] Respuesta recibida (%zu bytes) en %.2f ms\n", len, ms);
            vita_debug_log("[Boost.Asio] Respuesta recibida (%zu bytes) en %.2f ms\n", len, ms);
        } else if (!salir) {
            printf("[Boost.Asio] No se recibió respuesta (timeout)\n");
            vita_debug_log("[Boost.Asio] No se recibió respuesta (timeout)\n");
        }
        } catch (const std::exception& e) {
            printf("[Boost.Asio] Error: %s\n", e.what());
            vita_debug_log("[Boost.Asio] Error: %s\n", e.what());
        }
        // Esperar un poco antes del siguiente ping
        sceKernelDelayThread(500*1000); // 500 ms
        // Comprobar si se pulsa círculo
        sceCtrlPeekBufferPositive(0, &ctrl, 1);
        if (ctrl.buttons & SCE_CTRL_CIRCLE) {
            salir = true;
        }
    }

}