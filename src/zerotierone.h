#pragma once

#ifdef __cplusplus
extern "C" {
#endif
// API Vita para ZeroTierOne
void zerotierone_init();
void zerotierone_join(const char* network_id);
void zerotierone_leave(const char* network_id);
void zerotierone_status();
#ifdef __cplusplus
}
#endif
