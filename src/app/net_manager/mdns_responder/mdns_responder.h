#pragma once

/* mDNS responder for STA mode: advertises <hostname>.local and announces the
 * _http._tcp service (port 80). A plain lwIP module (like dhcpserver/
 * dnsserver): netif and hostname are supplied by the caller; holding the
 * lwIP core lock while calling is the caller's responsibility. */

struct netif;

/* Call once (mdns_resp_init). */
void mdns_responder_init(void);

/* Registers <hostname>.local + the _http service on nif. */
void mdns_responder_start(struct netif* nif, const char* hostname);

/* Removes nif's mDNS registration. */
void mdns_responder_stop(struct netif* nif);
