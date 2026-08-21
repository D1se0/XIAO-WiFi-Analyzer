# Contribuir a XIAO WiFi Analyzer

Gracias por tu interés. Este proyecto se mantiene con el mismo criterio con el que se construyó:

- **Nada de funcionalidades ofensivas.** Pull requests que añadan deauth, evil twin, interceptación de tráfico, o cualquier función activa contra redes de terceros se rechazarán directamente, sin importar la calidad del código.
- **Cero dependencias innecesarias.** La versión básica funciona sin ninguna librería de terceros — que siga siendo así. Cualquier dependencia nueva debe justificar claramente por qué no se puede evitar.
- **Documenta lo que te costó averiguar.** Si arreglas un bug que no fue obvio, añade la causa y la solución a `docs/05-solucion-problemas.md`, no solo el fix en el código. El valor de este repositorio está tanto en el firmware como en las horas de depuración documentadas.

## Cómo probar tus cambios

No hay tests automatizados (es firmware embebido para hardware físico concreto). Antes de un PR:
1. Compila y sube ambas versiones (básica y con pantalla) en una XIAO ESP32-C5 real.
2. Confirma que el dashboard web sigue funcionando igual.
3. Si tocaste algo de `display_ui.cpp`, confirma que el táctil sigue respondiendo tras un reset (no solo tras un flasheo limpio).
