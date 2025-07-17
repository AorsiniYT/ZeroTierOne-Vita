#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <psp2/rtc.h>
#include <stdlib.h>

#include <psp2/kernel/clib.h>
#include "debug.h"

#ifdef __vita__
#define pthread_mutex_t         int
#define pthread_mutex_init(x,y) (void)(x),0
#define pthread_mutex_lock(x)   (void)(x)
#define pthread_mutex_unlock(x) (void)(x)
static pthread_mutex_t print_mutex;
bool vita_debug_init() { return true; }
#else
#include <pthread.h>
static pthread_mutex_t print_mutex;
bool vita_debug_init() {
  if (pthread_mutex_init(&print_mutex, NULL) != 0) {
    return false;
  }
  return true;
}
#endif

void vita_debug_log(const char *s, ...) {
  pthread_mutex_lock(&print_mutex);

  char* buffer = malloc(8192);
  if (!buffer) {
    pthread_mutex_unlock(&print_mutex);
    return;
  }

  SceDateTime time;
  sceRtcGetCurrentClock(&time, 0);

  snprintf(buffer, 26, "%04d%02d%02d %02d:%02d:%02d.%06d ",
           time.year, time.month, time.day,
           time.hour, time.minute, time.second,
           time.microsecond);

  va_list va;
  va_start(va, s);
  int len = vsnprintf(&buffer[25], 8000, s, va);
  va_end(va);

  // Abrir archivo de log en la carpeta de zt_storage
  FILE* logf = fopen("ux0:data/zerotierone/zerotierlite.log", "a");
  if (logf) {
    fprintf(logf, "%s", buffer);
    if (buffer[len + 24] != '\n') {
        fprintf(logf, "\n");
    }
    fflush(logf);
    fclose(logf);
  } else {
    // Si no se pudo abrir el log, mostrar por pantalla
    printf("[ZeroTierLite] No se pudo abrir el archivo de log. Mensaje: %s\n", &buffer[25]);
  }

#ifdef __vita__
  // También imprimir por mdns_log (sceClibPrintf)
  sceClibPrintf("%s", &buffer[25]);
#endif

  free(buffer);

  pthread_mutex_unlock(&print_mutex);
}

