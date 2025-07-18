#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <string>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>
#include "debug.h"
#include "planet_reader.h"

// Lee y parsea el archivo planet, usando el puerto de zerotier-one.port si existe
PlanetData leer_planet(const std::string& path_planet, const std::string& path_port) {
    // Verificar existencia y permisos de planet
    SceIoStat stat_planet;
    int res_planet = sceIoGetstat(path_planet.c_str(), &stat_planet);
    if (res_planet < 0) {
        vita_debug_log("[PLANET_READER] planet NO existe o sin permisos: %s", path_planet.c_str());
    } else {
        vita_debug_log("[PLANET_READER] planet existe: %s, size=%u, mode=0x%X", path_planet.c_str(), (unsigned int)stat_planet.st_size, stat_planet.st_mode);
    }

    // Verificar existencia y permisos de zerotier-one.port
    int puerto_por_defecto = 9993;
    if (!path_port.empty()) {
        SceIoStat stat_port;
        int res_port = sceIoGetstat(path_port.c_str(), &stat_port);
        if (res_port < 0) {
            vita_debug_log("[PLANET_READER] zerotier-one.port NO existe o sin permisos: %s", path_port.c_str());
        } else {
            vita_debug_log("[PLANET_READER] zerotier-one.port existe: %s, size=%u, mode=0x%X", path_port.c_str(), (unsigned int)stat_port.st_size, stat_port.st_mode);
        }
        std::ifstream fport(path_port);
        if (fport.is_open()) {
            std::string port_str;
            std::getline(fport, port_str);
            try {
                puerto_por_defecto = std::stoi(port_str);
                vita_debug_log("[PLANET_READER] Puerto leído de archivo: %d", puerto_por_defecto);
            } catch (...) {
                puerto_por_defecto = 9993;
                vita_debug_log("[PLANET_READER] Error al leer puerto, usando por defecto: 9993");
            }
        } else {
            vita_debug_log("[PLANET_READER] No se pudo abrir archivo de puerto, usando por defecto: 9993");
        }
    } else {
        vita_debug_log("[PLANET_READER] No se especificó archivo de puerto, usando por defecto: 9993");
    }

    PlanetData data;
    vita_debug_log("[PLANET_READER] Leyendo planet binario: %s", path_planet.c_str());
    std::ifstream f(path_planet, std::ios::binary);
    if (!f.is_open()) {
        vita_debug_log("[PLANET_READER] No se pudo abrir planet: %s", path_planet.c_str());
        return data;
    }
    // Leer todo el archivo en un buffer
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (buffer.empty()) {
        vita_debug_log("[PLANET_READER] planet vacío");
        return data;
    }
    // Nuevo parser: formato planet_inspect.py
    if (buffer.size() < 16) {
        vita_debug_log("[PLANET_READER] planet demasiado pequeño para cabecera");
        return data;
    }
    unsigned int p = 0;
    // Encabezado: versión, longitud, world_id
    uint32_t version = buffer[p] | (buffer[p+1]<<8) | (buffer[p+2]<<16) | (buffer[p+3]<<24);
    uint32_t length = buffer[p+4] | (buffer[p+5]<<8) | (buffer[p+6]<<16) | (buffer[p+7]<<24);
    uint64_t world_id = 0;
    for (int i=0; i<8; ++i) world_id |= ((uint64_t)buffer[p+8+i]) << (8*i);
    vita_debug_log("[PLANET_READER] version=%u, length=%u, world_id=0x%llx", version, length, world_id);
    p += 16;
    // Cada endpoint: 4 bytes tipo, 4 bytes IP, 2 bytes puerto, 32 bytes clave pública
    unsigned int total_endpoints = 0;
    while (p + 42 <= buffer.size()) {
        PlanetRoot r;
        PlanetEndpoint ep;
        // Tipo (no usado en C++, solo para debug)
        uint32_t tipo = buffer[p] | (buffer[p+1]<<8) | (buffer[p+2]<<16) | (buffer[p+3]<<24);
        p += 4;
        // IP
        char ipstr[16];
        sprintf(ipstr, "%u.%u.%u.%u", buffer[p], buffer[p+1], buffer[p+2], buffer[p+3]);
        ep.ip = std::string(ipstr);
        p += 4;
        // Puerto (little endian)
        ep.port = buffer[p] | (buffer[p+1]<<8);
        if (ep.port == 0) ep.port = puerto_por_defecto;
        p += 2;
        // Clave pública (hex)
        char pubkey[65] = {0};
        for (int i=0; i<32; ++i) sprintf(pubkey+i*2, "%02x", buffer[p+i]);
        r.identity = std::string(pubkey);
        p += 32;
        r.endpoints.push_back(ep);
        data.roots.push_back(r);
        total_endpoints++;
        vita_debug_log("[PLANET_READER] Endpoint: tipo=%u, ip=%s, port=%d, pubkey=%s", tipo, ep.ip.c_str(), ep.port, r.identity.c_str());
    }
    vita_debug_log("[PLANET_READER] Lectura binaria finalizada. Endpoints: %u", total_endpoints);
    return data;
}
