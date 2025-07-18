
#ifndef PLANET_READER_H
#define PLANET_READER_H

#include <string>
#include <vector>
#include <stdexcept>

struct PlanetEndpoint {
    std::string ip;
    int port;
};

struct PlanetRoot {
    std::string identity;
    std::vector<PlanetEndpoint> endpoints;
};

struct PlanetData {
    std::vector<PlanetRoot> roots;
};

// Lee planet y zerotier-one.port desde las rutas dadas y retorna la estructura PlanetData
PlanetData leer_planet(const std::string& planet_path, const std::string& port_path = "");

// Guarda el planet embebido en la ruta destino
void guardar_planet_binario(const std::string& destino);

#endif // PLANET_READER_H
