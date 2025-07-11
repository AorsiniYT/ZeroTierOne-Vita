#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Muestra el teclado IME y guarda el texto en out. Devuelve 1 si se confirmó, 0 si se canceló.
int ime_get_text(char* out, int max_len, const char* title, const char* initial);

#ifdef __cplusplus
}
#endif
