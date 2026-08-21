# XIAO WiFi Analyzer v1.0.0

Analizador Wi-Fi portátil, autónomo y educativo sobre una **Seeed Studio XIAO ESP32-C5**, con dashboard web completo y, opcionalmente, una interfaz física en una pantalla táctil circular.

## Qué incluye este release

- 🟢 **`XIAO_WiFi_Analyzer_Basico`** — AP + escáner + dashboard web. Cero dependencias de terceros.
- 🔵 **`XIAO_WiFi_Analyzer_Pantalla`** — todo lo anterior + Round Display for XIAO (4 pantallas táctiles).
- 📖 Documentación completa paso a paso: hardware, instalación del entorno, ambas guías de firmware, referencia de la API REST, mapeo completo de pines, y una guía de solución de problemas con **todos los errores reales** encontrados al desarrollar esto sobre hardware físico (no son suposiciones ni copiar-pegar genérico).

## Instalación rápida

1. Lee [`docs/01-requisitos-hardware.md`](docs/01-requisitos-hardware.md) — presta especial atención al aviso de la antena.
2. Sigue [`docs/02-instalacion-entorno.md`](docs/02-instalacion-entorno.md) para dejar Arduino IDE listo.
3. Empieza siempre por [`docs/03-guia-version-basica.md`](docs/03-guia-version-basica.md).
4. Si quieres la pantalla, continúa con [`docs/04-guia-version-pantalla.md`](docs/04-guia-version-pantalla.md).
5. Cualquier error de compilación → [`docs/05-solucion-problemas.md`](docs/05-solucion-problemas.md) — está prácticamente garantizado que ya lo hemos visto y documentado.

## Alcance ético

Proyecto exclusivamente **pasivo y educativo**. No implementa deauth, evil twin, interceptación de tráfico, fuerza bruta ni ningún tipo de ataque. Ver el README para el detalle completo.

## Placa validada

Todo probado sobre hardware real: Seeed Studio XIAO ESP32-C5 + Round Display for XIAO, con Arduino IDE 2.x y core `esp32` 3.3.11.
