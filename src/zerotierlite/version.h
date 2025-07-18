#ifndef ZEROTIERLITE_VERSION_H
#define ZEROTIERLITE_VERSION_H

// Versión de ZeroTierOne compatible (1.14.2)
#define ZEROTIERLITE_VERSION_MAJOR 1
#define ZEROTIERLITE_VERSION_MINOR 14
#define ZEROTIERLITE_VERSION_REVISION 2

// Función para obtener la versión como string
static inline const char* zerotierlite_version_string() {
    return "1.14.2";
}

#endif // ZEROTIERLITE_VERSION_H
