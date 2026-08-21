# 1. Requisitos de hardware

## Imprescindible para la versión básica

| Componente | Notas |
|---|---|
| **Seeed Studio XIAO ESP32-C5** | El chip: RISC-V, Wi-Fi 6 dual-band (2.4GHz + 5GHz), Bluetooth 5 LE, 802.15.4. [Página oficial](https://www.seeedstudio.com/Seeed-Studio-XIAO-ESP32C5-p-6609.html?sensecap_affiliate=w2q4ssn) · [Wiki](https://wiki.seeedstudio.com/xiao_esp32c5_getting_started/) |
| **Antena externa (incluida en la caja)** | ⚠️ **Crítico**. Sin ella conectada al conector U.FL, el alcance de escaneo es prácticamente inútil (solo se detectan 1-2 redes con señal pésima). Ver aviso más abajo. |
| **Cable USB-C** | Para alimentar, programar y depurar por Serial. Usa un cable de datos real, no uno "solo carga". |
| **Ordenador con Windows/macOS/Linux** | Para Arduino IDE. No hace falta después de cargar el firmware. |

## Adicional para la versión con pantalla

| Componente | Notas |
|---|---|
| **Round Display for XIAO (1.28")** | Pantalla circular táctil, 240×240px, controlador GC9A01 + táctil capacitivo CHSC6x. [Comprar](https://www.seeedstudio.com/1-28-Round-Touch-Display-for-Seeed-Studio-XIAO-ESP32.html?sensecap_affiliate=w2q4ssn) · [Wiki oficial](https://wiki.seeedstudio.com/get_start_round_display/) |
| Cinta desoldadora o desoldador | Solo si vas a soldar tú mismo los pines — ver aviso de soldadura abajo. |
| Multímetro (opcional pero recomendable) | Útil para comprobar continuidad si algo no responde tras soldar. |

---

## ⚠️ Aviso crítico: la antena

La XIAO ESP32-C5 tiene una antena residual mínima integrada en el propio PCB, pensada solo como respaldo de fábrica — **no la uses como antena principal**. Sin la antena externa conectada al conector U.FL:

- El escaneo detecta muy pocas redes (a veces solo 1-2).
- El RSSI de esas pocas redes es pésimo (por ejemplo, -86 dBm en una red que en realidad está a plena potencia).
- La conexión del propio Access Point puede volverse inestable.

**Antes de reportar cualquier problema de "detecta pocas redes" o "señal débil", confirma que la antena está conectada.** Es la causa más común y más fácil de pasar por alto.

Cómo conectarla: localiza el conector U.FL (diminuto, tipo "click") en la placa, alinea el conector de la antena y presiona con firmeza hasta notar que encaja. Si tiene adhesivo, pégala en algún punto donde quede lo más estirada posible (evita doblarla mucho o enrollarla, ya que distorsiona su geometría eléctrica).

## ⚠️ Aviso crítico: soldadura de los pines (solo versión con pantalla)

Si tu XIAO ESP32-C5 viene sin pines soldados y necesitas soldarlos tú mismo para montar la Round Display encima:

- **Un puente de estaño entre pines contiguos puede causar fallos muy difíciles de diagnosticar.** En el desarrollo de este proyecto, un puente entre pines causó simultáneamente: pantalla completamente en negro Y necesidad de reflashear el firmware tras cada reset (porque uno de los pines puenteados, D1, es también GPIO0, el pin de arranque del chip).
- **Revisa cada pin con lupa o la cámara del móvil en macro** tras soldar, confirmando que no hay estaño conectando dos pines vecinos.
- Si tienes un multímetro, usa el modo continuidad entre pines adyacentes — si pita y no deberían estar conectados, hay un puente.
- Ver [`05-solucion-problemas.md`](05-solucion-problemas.md#fallo-1-puente-de-soldadura) para el diagnóstico completo de este problema.

## Alimentación

El proyecto está pensado para funcionar con:
- El propio cable USB-C conectado a un ordenador, **o**
- Una power bank USB estándar.

No se ha modificado ni se necesita modificar el circuito de alimentación de la placa para nada de lo incluido en este repositorio.
