#ifndef NETWORK_H
#define NETWORK_H

void send_encrypted_data(const char* data);
int connect_to_server();
void cleanup_network();

#endif
