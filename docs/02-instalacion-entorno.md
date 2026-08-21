# 2. Instalación del entorno

Esta guía instala **solo lo necesario para la versión básica**. Si vas a montar la versión con pantalla, instala primero esto, confirma que la versión básica funciona, y luego sigue con [`04-guia-version-pantalla.md`](04-guia-version-pantalla.md) para las librerías adicionales.

## 2.1 Instalar Arduino IDE

1. Descarga **Arduino IDE 2.x** (versión estable más reciente) desde [arduino.cc/en/software](https://www.arduino.cc/en/software).
2. Instálalo con las opciones por defecto.

## 2.2 Añadir el paquete de placas de Espressif

1. Abre Arduino IDE.
2. Ve a `File → Preferences` (Windows/Linux) o `Arduino IDE → Settings` (macOS).
3. En **"Additional boards manager URLs"**, añade exactamente esta URL:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
   Si ya tienes otras URLs ahí, sepáralas con una coma.
4. Pulsa **OK**.

## 2.3 Instalar el core `esp32`

1. Ve a `Tools → Board → Boards Manager...`
2. Busca `esp32`.
3. Instala el paquete **"esp32" de Espressif Systems**.
4. **Versión mínima recomendada: 3.3.5.** Este proyecto se ha probado y validado con la **3.3.11**. Versiones anteriores pueden no reconocer la placa `XIAO_ESP32C5` en absoluto (es un chip muy reciente).

> Si tras instalar el core no aparece `XIAO_ESP32C5` en la lista de placas, actualiza a la versión más reciente disponible del paquete `esp32` — el soporte de este chip concreto se ha ido puliendo en versiones sucesivas.

## 2.4 Seleccionar la placa correcta

1. Conecta la XIAO ESP32-C5 por USB-C.
2. Ve a `Tools → Board → esp32 → XIAO_ESP32C5`.
3. Ve a `Tools → Port` y selecciona el puerto que ha aparecido (en Windows suele ser `COMx`; en macOS/Linux, algo como `/dev/cu.usbmodemXXXX` o `/dev/ttyACM0`).

Si no aparece ningún puerto nuevo al conectar la placa, revisa la sección de [solución de problemas](05-solucion-problemas.md#no-aparece-el-puerto).

## 2.5 Ajustes de compilación recomendados (opcional, pero útil)

- `Tools → Erase All Flash Before Sketch Upload → Enabled` — ayuda si tienes problemas de subida intermitentes.
- `Tools → Upload Speed` — si tienes fallos de conexión al flashear, baja de 921600 a 460800.
- `File → Preferences → Show verbose output during: compile` — actívalo si necesitas ver el detalle completo de un error de compilación (a veces el panel inferior de Arduino IDE recorta el mensaje real).

## 2.6 Prueba mínima antes de continuar

Antes de tocar el firmware del proyecto, confirma que el entorno funciona con este sketch mínimo:

```cpp
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("HOLA");
}

void loop() {
  Serial.println("HOLA");
  delay(1000);
}
```

1. Pégalo en un sketch nuevo (`File → New Sketch`).
2. Compílalo (`Sketch → Verify/Compile`).
3. Súbelo (`Sketch → Upload`).
4. Abre el Monitor Serie (`Tools → Serial Monitor`, o `Ctrl+Shift+M`), confirma **115200 baudios**.
5. Deberías ver `HOLA` apareciendo una vez por segundo.

### Si la subida falla con `Failed to start stub flasher` o similar

Esto es un comportamiento conocido en varias placas ESP32 recientes con USB-Serial-JTAG nativo (C3/C5/C6/S3), no un fallo de tu placa. La conexión inicial funciona (se identifica el chip, el MAC, etc.) pero el auto-reset hacia modo bootloader a veces no llega a tiempo.

**Solución — modo bootloader manual:**
1. Mantén pulsado el botón físico **BOOT** de la placa.
2. Sin soltarlo, pulsa y suelta **RESET**.
3. Suelta **BOOT**.
4. Pulsa `Upload` en Arduino IDE inmediatamente.

Si esto se repite en cada subida, puedes dejarlo como parte normal de tu flujo de trabajo con esta placa — no indica ningún problema real de hardware.

Con `HOLA` apareciendo cada segundo, tu entorno está listo. Continúa con [`03-guia-version-basica.md`](03-guia-version-basica.md).
