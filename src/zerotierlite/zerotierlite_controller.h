// zerotierlite_controller.h
#pragma once
#include "../planet_reader.h"

// Intenta conectar por UDP a los endpoints del planet usando la identidad local
// Devuelve true si al menos uno responde
#include "identity.h"
bool conectar_a_controladores(const PlanetData& planet, ztl_identity_t* identidad_local);
