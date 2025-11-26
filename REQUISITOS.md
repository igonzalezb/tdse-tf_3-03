<img width="1774" height="300" alt="encabezado clasico ajustado" src="https://github.com/user-attachments/assets/e3e593e2-ef8b-4c69-9c9f-21c9299c43ea" />

# Smartceta

Fecha: 2C 2025

Integrantes:

- Carrizo, Ezequiel Ignacio - 105187
- Gonzalez Bigliardi, Iñaki - 107443
- Juncal, Franco Mariano - 106448


## Selección de proyecto a implementar

### Objetivo del proyecto y resultados esperados

El objetivo de este proyecto es diseñar e implementar una maceta «inteligente» que facilite el cuidado de una planta doméstica sin necesidad de supervisión constante. El sistema medirá condiciones ambientales y del sustrato para:

- Regar automáticamente según humedad del suelo y nivel de agua disponible.
- Controlar iluminación artificial según intensidad de luz externa y necesidades de la planta.
- Alertar al usuario en caso de errores, bajo nivel de agua o condiciones críticas.
- Permitir configuración de umbrales y modos de funcionamiento desde la interfaz local (pantalla + botones).

Resultados esperados:

- Sistema funcional con lectura de sensores y actuación sobre bomba y luces.
- Interfaz local con menú de configuración.
- Modo test y manejo de fallas con alarmas visuales y sonoras.

### Variantes del proyecto

Se consideraron distintas variantes del proyecto, con funcionalidades diversas. Los principales ejes de selección fueron el costo económico y el tiempo de implementación.

Se evaluaron las siguientes variantes en función de costo y tiempo:

| Funcionalidad                                         | Costo     | Descartado? | Motivo           |
|-------------------------------------------------------|-----------|-------------|------------------|
| Funcionamiento autónomo (baterías y panel solar)      | Alto      | Sí    | Alto costo y poco tiempo para implementación |
| Comunicación Wi‑Fi + App móvil                        | Medio     | Sí    | Complejidad y corto tiempo para implementación |
| Comunicación Bluetooth local                          | Bajo      | Sí    | Alta complejidad de implementación en el tiempo disponible |
| Panel LCD + botones locales                           | Bajo      | No    |                       |
| Sensor de nivel de agua                               | Bajo      | No    |                       |
| Sistema de múltiples macetas (red)                    | Alto      | Sí    | Fuera del alcance inicial |
| Riego automatizado                                    | Alto      | No    | 
| Calefacción incorporada                               | Alto      | Sí    | Por alto costo y dificultad de implementación  |
| Iluminación                                           | Medio     | No    |
| Sistema de alertas sonoras y mediante LED de estado.  | Bajo      | No    |
| Sensado de intensidad de luz                          | Bajo      | No    |    
| Sensado de nutrientes y ph del sustrato               | Alto      | Sí    | Alto costo de los sensores |

<p align="center"><em>Tabla 1: Variantes del proyecto.</em></p>

### Diagrama en bloques

En la siguiente figura se muestra el diagrama en bloques con los principales módulos del proyecto:

<p align="center">
  <img width="559" height="462" alt="Diagrama_en_bloques" src="https://github.com/user-attachments/assets/9002c784-cc37-49ba-a3e2-2e07355b8f4d" />
  <br>
  <sub>Figura 1: Diagrama en bloques del sistema</sub>
</p>

### Componente hardware (tentativos)

- Placa nucleo F103RB.  
- Sensor humedad suelo YL69.  
- Sensor DHT22 (temperatura y humedad ambiente).  
- Sensor de luz (LDR).  
- Sensor nivel de agua (módulo resistivo).  
- Bomba sumergible y driver.  
- Tira LED blanco frío + driver PWM.  
- Pantalla LCD/16x2 + 4 botones (Siguiente, anterior, enter, escape).  
- Buzzer y LED RGB indicador de estado.
- Sensor de corriente.

## Requisitos y modos de uso

### Requisitos

A continuación se detallan los requisitos funcionales del sistema propuesto:

| Grupo | ID | Descripción |
| :---- | :---- | :---- |
| Indicadores | 0.1 | El sistema contará con una pantalla LCD para mostrar información al usuario |
|   | 0.2 | Led indicador de estado del sistema |
|   | 0.3 | El sistema contará con un buzzer para emitir señales sonoras |
| Sensores | 1.1 | El sistema contará con un sensor de temperatura y humedad ambiente |
|  | 1.2 | El sistema contará con sensor de luz ambiente |
|  | 1.3 | El sistema contará con un sensor de humedad del suelo |
|  | 1.4 | El sistema contará con un sensor de nivel de agua en el depósito |
|   | 1.5 | El sistema contará con un sensor de corriente para detectar fallas en la bomba |
| Actuadores | 2.1 | El sistema contará con una bomba para regar la planta |
|  | 2.2 | El sistema contará con una tira de luces LED para iluminar la planta |
| Pulsadores | 3.0 | El sistema contará con pulsadores para interactuar con la aplicación |
| Aplicación | 4.1 | La aplicación deberá permitirle al usuario configurar los umbrales de humedad del suelo |
|  | 4.3 | La aplicación deberá permitirle al usuario configurar la cantidad/intensidad de iluminación |
|  | 4.4 | La aplicación deberá permitirle al usuario activar o desactivar las alarmas sonoras y visuales |
|  | 4.5 | La aplicación deberá permitirle al usuario visualizar las lecturas de los sensores en tiempo real |
|  | 4.6 | La aplicación deberá permitirle al usuario ingresar al modo test para probar los componentes del sistema |
| | 4.7 | Registrar y almacenar localmente (EEPROM/Flash) valores y configuraciones básicas |
| Alarmas | 5.1 | El sistema deberá contar con alarmas sonoras y visuales para notificar fallas |
|  | 5.2 | El sistema deberá activar las alarmas cuando el sensor de nivel de agua detecte bajo nivel |
|  | 5.3 | El sistema deberá activar las alarmas cuando el sensor de corriente detecte una falla en la bomba |

<p align="center"><em>Tabla 2: Requisitos del proyecto</em></p>


### Modos de operación:

- **Normal (flujo principal)**: muestra en pantalla los valores de los diferentes sensores, se puede ir cambiando lo que muestra con los botones. El sistema actúa dependiendo de las mediciones de los sensores.
- **Ajustes y configuraciones**: menú en el cual se pueden configurar y modificar los valores de humedad deseada, activar/desactivar alarmas, cantidad de luz externa deseada.
- **Falla o errores**: ante una falla o error, el sistema muestra un mensaje en pantalla indicando el problema, y en algunos casos detiene el funcionamiento para evitar daños. Además, se enciende una luz indicadora de error y se activa una alarma sonora.
- **Modo Test**: en este modo se pueden probar los diferentes componentes del sistema, como los sensores, bombas y luces.

#### Caso de uso 1: Normal

| Elemento | Definición |
| :---- | :---- |
| Disparador | El sistema se enciende. |
| Precondiciones | La Smartceta está energizada. Los sensores están conectados y operativos. Existen valores de configuración válidos almacenados (umbrales de humedad, luz, etc.). El nivel de agua es suficiente para permitir el riego. |
| Flujo principal | El sistema realiza lecturas periódicas de humedad de suelo, temperatura, humedad ambiente y luz. Muestra los valores en el display. Si la humedad del suelo está por debajo del umbral y hay agua, activa la bomba. Si la luz es insuficiente según la configuración, enciende la iluminación. Una vez actuado, vuelve a monitoreo continuo. |
| Flujos alternativos | a. Se detecta condición fuera de rango, pero no crítica (ej.: nivel de agua bajo, nivel de temperatura extrema) → se alerta al usuario en el display y/o LED.  |

<p align="center"><em>Tabla 3: Caso de uso 1: Funcionamiento normal (modo automático)</em></p>

#### Caso de uso 2: Configuración

| Elemento | Definición |
| :---- | :---- |
| Disparador | El usuario presiona la tecla ENTER estando en modo normal. |
| Precondiciones | El sistema está encendido y respondiendo a las teclas. Existen parámetros configurables (umbrales de humedad, luz, tiempos de riego, habilitación de alarmas, etc.). |
| Flujo principal | El sistema entra al menú de configuración y muestra la primera opción. El usuario navega con las teclas (siguiente/anterior) y selecciona un parámetro con ENTER. Con las teclas ajusta el valor deseado dentro de un rango permitido y confirma con ENTER. El sistema guarda los nuevos valores en memoria no volátil y vuelve al menú principal o al modo normal tras seleccionar “Salir”. |
| Flujos alternativos | a. El usuario presiona ESC → se cancela la modificación actual y se vuelve al menú anterior o al modo normal. b. El usuario deja de interactuar durante un tiempo de inactividad → el sistema sale automáticamente de configuración y vuelve al modo normal. |

<p align="center"><em>Tabla 4: Caso de uso 2: Configuración de parámetros</em></p>


#### Caso de uso 3: Gestión de fallas y alarmas

| Elemento | Definición |
| :---- | :---- |
| Disparador | Se detecta una falla crítica (por ejemplo: sensor desconectado o error de lectura, exceso de corriente en la bomba). |
| Precondiciones | El sistema está encendido y realizando monitoreo normal. Existen umbrales definidos para considerar una condición como falla crítica. |
| Flujo principal | Ante la detección de la falla crítica, el sistema enciende el LED en color rojo y activa el buzzer con un patrón de alarma. Se muestra en el display un mensaje indicando el tipo de falla y la acción recomendada (revisar sensor, etc.). En caso necesario, se bloquean las acciones que puedan dañar el hardware (por ejemplo, deshabilitar el riego). El sistema permanece en este estado hasta que el usuario lo indique. |
| Flujos alternativos | a. El usuario reconoce la falla (presiona ENTER u otra tecla definida) → se vuelve al modo normal. b. Se detectan múltiples fallas → el sistema las indica secuencialmente. |

<p align="center"><em>Tabla 5: Caso de uso 3: Gestión de fallas y alarmas</em></p> 

#### Caso de uso 4: Test de hardware

| Elemento | Definición |
| :---- | :---- |
| Disparador | El usuario selecciona la opción “Modo Test” mediante una combinación de teclas. |
| Precondiciones | El sistema está encendido. Todos los componentes a testear (bomba, LEDs, buzzer, sensores, display) están conectados físicamente. Se cuenta con nivel de agua suficiente para probar la bomba sin dañarla. |
| Flujo principal | El sistema ingresa al modo test y muestra una lista de elementos a probar. El usuario va seleccionando cada componente (bomba, LEDs, buzzer, sensores, etc.). Para cada selección, el sistema activa el componente (por ejemplo, enciende la bomba unos segundos, enciende el LED, genera un tono con el buzzer) y muestra en el display el estado (“OK” / “Falla” o el valor leído en el caso de sensores). Finalizado el test, el sistema ofrece salir del modo test y volver al modo normal. |
| Flujos alternativos | a. El usuario presiona ESC en cualquier momento → se interrumpe la prueba actual y se vuelve al modo normal. |

<p align="center"><em>Tabla 6: Caso de uso 4: Modo Test de hardware</em></p> 