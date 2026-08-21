#ifndef API_HANDLERS_H
#define API_HANDLERS_H

#include <WebServer.h>

void registerApiRoutes(WebServer &server);
void registerStaticRoutes(WebServer &server);
void registerCaptivePortalRoutes(WebServer &server);

#endif