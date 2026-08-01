<div align="center">

<img src="Imagenes/Logo FIUBA.png" alt="Universidad de Buenos Aires - Facultad de Ingeniería" width="600">

### Sistemas Embebidos

<br>

## Memoria del Trabajo Final

# Smartceta

### Sistema embebido para el cuidado automatizado de plantas

<br>

**Autores**

| Apellido y nombre | Padrón |
| --- | ---: |
| Carrizo, Ezequiel Ignacio | 105187 |
| Gonzalez Bigliardi, Iñaki | 107443 |
| Juncal, Franco Mariano | 106448 |

**Cuatrimestre de cursada:** segundo cuatrimestre de 2025

</div>

---

## Resumen

El presente trabajo describe el diseño y la implementación de Smartceta, un sistema embebido destinado a asistir el cuidado de una planta doméstica. El equipo desarrollado sensa la humedad del suelo, la intensidad de luz ambiente, el nivel de agua de un depósito y la temperatura y humedad del aire. A partir de estas variables controla una bomba de riego y una tira de iluminación, y permite consultar las mediciones y modificar parámetros mediante una pantalla LCD y cuatro pulsadores.

La unidad de control se implementó sobre una placa NUCLEO-F103RB. El firmware se organizó como una aplicación *bare-metal* cooperativa, con tareas periódicas, máquinas de estados, intercambio de eventos y una estructura de datos compartida. También se incorporaron un LED RGB, un buzzer, medición de corriente en las etapas de potencia, un modo de prueba y almacenamiento de las configuraciones en la memoria Flash interna.

El Producto mínimo Viable permitió integrar en una única plataforma la adquisición de datos de los sensores, el control de actuadores, la interfaz local y la persistencia de parámetros de configuración. Se logró la compilación sin errores en modo Release para comprobar el funcionamiento del dispositivo en una maceta real sin dependencia de una computadora. El Proyecto tuvo un enfoque de seguridad para priorizar la integridad del hardware ante posibles fallas o malos funcionamientos.

## Abstract

This report presents the design and implementation of Smartceta, an embedded system intended to assist with the care of a household plant. The system measures soil moisture, ambient light, water-tank level, air temperature, and relative humidity. These variables are used to control an irrigation pump and an artificial light source, while a local LCD and four push-buttons allow the user to inspect measurements and modify configuration parameters.

The control unit was implemented on an STM32 NUCLEO-F103RB board. Its firmware follows a cooperative bare-metal architecture based on periodic tasks, state machines, event queues, and shared application data. An RGB status LED, a buzzer, current-sensing circuits, a hardware test mode, and non-volatile configuration storage were also included.

The Minimum Viable Product (MVP) enabled the integration of sensor data acquisition, actuator control, the local user interface, and configuration parameter persistence into a single platform. The system was successfully compiled in Release mode without errors to verify the device's operation in a real plant pot without relying on a computer. The project adopted a safety-oriented approach, prioritizing hardware integrity in the event of potential failures or malfunctions.


## Registro de versiones

| Revisión | Cambios realizados | Fecha |
| :---: | --- | :---: |
| 0.1 | Entrega de memoria técnica | <span style="color:#0057b8">🔵 Confirmar fecha</span> |

*Tabla 0.1: registro de versiones del documento.*

---

# Índice general

- [Smartceta](#smartceta)
    - [Sistema embebido para el cuidado automatizado de plantas](#sistema-embebido-para-el-cuidado-automatizado-de-plantas)
  - [Convención para completar esta versión](#convención-para-completar-esta-versión)
  - [Resumen](#resumen)
  - [Abstract](#abstract)
  - [Registro de versiones](#registro-de-versiones)
- [Índice general](#índice-general)
- [Capítulo 1: Introducción general](#capítulo-1-introducción-general)
  - [1.1 Necesidad, motivación y objetivo](#11-necesidad-motivación-y-objetivo)
  - [1.2 Alcance y limitaciones](#12-alcance-y-limitaciones)
  - [1.3 Descripción general del sistema](#13-descripción-general-del-sistema)
  - [1.4 Análisis de sistemas similares](#14-análisis-de-sistemas-similares)
  - [1.5 Justificación del enfoque técnico](#15-justificación-del-enfoque-técnico)
- [Capítulo 2: Introducción específica](#capítulo-2-introducción-específica)
  - [2.1 Requisitos](#21-requisitos)
  - [2.2 Casos de uso](#22-casos-de-uso)
    - [2.2.1 Operación normal](#221-operación-normal)
    - [2.2.2 Configuración de parámetros](#222-configuración-de-parámetros)
    - [2.2.3 Gestión de fallas](#223-gestión-de-fallas)
    - [2.2.4 Prueba de componentes](#224-prueba-de-componentes)
  - [2.3 Descripción de los módulos principales](#23-descripción-de-los-módulos-principales)
    - [2.3.1 Unidad de control](#231-unidad-de-control)
    - [2.3.2 Sensores](#232-sensores)
    - [2.3.3 Actuadores y etapas de potencia](#233-actuadores-y-etapas-de-potencia)
    - [2.3.4 Interfaz local](#234-interfaz-local)
    - [2.3.5 Alimentación](#235-alimentación)
- [Capítulo 3: Diseño e implementación](#capítulo-3-diseño-e-implementación)
  - [3.1 Arquitectura general](#31-arquitectura-general)
  - [3.2 Diseño de hardware](#32-diseño-de-hardware)
    - [3.2.1 Unidad de control y periféricos](#321-unidad-de-control-y-periféricos)
    - [3.2.2 Entradas analógicas y calibración](#322-entradas-analógicas-y-calibración)
    - [3.2.3 Sensor DHT22](#323-sensor-dht22)
    - [3.2.4 Control de la bomba](#324-control-de-la-bomba)
    - [3.2.5 Control de iluminación e indicadores](#325-control-de-iluminación-e-indicadores)
    - [3.2.6 Interfaz de usuario](#326-interfaz-de-usuario)
    - [3.2.7 Esquemático, PCB y montaje](#327-esquemático-pcb-y-montaje)
    - [3.2.8 Pinout del sistema](#328-pinout-del-sistema)
    - [3.2.9 Lista de materiales](#329-lista-de-materiales)
  - [3.3 Diseño de firmware](#33-diseño-de-firmware)
    - [3.3.1 Arquitectura de ejecución](#331-arquitectura-de-ejecución)
    - [3.3.2 Tareas y periodicidades](#332-tareas-y-periodicidades)
    - [3.3.3 Adquisición y datos compartidos](#333-adquisición-y-datos-compartidos)
    - [3.3.4 Control de riego](#334-control-de-riego)
    - [3.3.5 Control de iluminación](#335-control-de-iluminación)
    - [3.3.6 Modos de operación](#336-modos-de-operación)
    - [3.3.7 Interfaz de usuario](#337-interfaz-de-usuario)
    - [3.3.8 Alarmas y recuperación](#338-alarmas-y-recuperación)
    - [3.3.9 Persistencia de configuraciones](#339-persistencia-de-configuraciones)
- [Capítulo 4: Ensayos y resultados](#capítulo-4-ensayos-y-resultados)
  - [4.1 Metodología general](#41-metodología-general)
  - [4.2 Pruebas funcionales del hardware](#42-pruebas-funcionales-del-hardware)
  - [4.3 Pruebas funcionales del firmware](#43-pruebas-funcionales-del-firmware)
  - [4.4 Pruebas de integración](#44-pruebas-de-integración)
  - [4.5 Consumo eléctrico](#45-consumo-eléctrico)
  - [4.6 Uso de memoria](#46-uso-de-memoria)
  - [4.7 Análisis temporal](#47-análisis-temporal)
    - [4.7.1 WCET experimental](#471-wcet-experimental)
  - [4.8 Cumplimiento de requisitos](#48-cumplimiento-de-requisitos)
  - [4.9 Documentación del desarrollo](#49-documentación-del-desarrollo)
- [Capítulo 5: Conclusiones](#capítulo-5-conclusiones)
  - [5.1 Resultados obtenidos](#51-resultados-obtenidos)
  - [5.2 Lecciones aprendidas](#52-lecciones-aprendidas)
  - [5.3 Trabajos necesarios antes del cierre](#53-trabajos-necesarios-antes-del-cierre)
  - [5.4 Posibles ampliaciones](#54-posibles-ampliaciones)
- [Capítulo 6: Uso de herramientas de inteligencia artificial](#capítulo-6-uso-de-herramientas-de-inteligencia-artificial)
- [Bibliografía y referencias](#bibliografía-y-referencias)

---

# Capítulo 1: Introducción general

## 1.1 Necesidad, motivación y objetivo

El cuidado de una planta doméstica requiere observar de forma periódica el estado del sustrato, la disponibilidad de agua, la iluminación y las condiciones ambientales. Cuando estas tareas dependen por completo de la intervención manual pueden producirse riegos insuficientes o excesivos, falta de luz y períodos prolongados sin supervisión. El problema resulta especialmente relevante cuando el usuario no puede mantener una rutina constante.

Smartceta se propuso como una plataforma de asistencia local capaz de medir las variables principales, presentarlas de manera comprensible y actuar sobre el riego y la iluminación. El objetivo no fue eliminar toda participación humana, sino reducir la frecuencia de las acciones rutinarias y aportar información para que el usuario pudiera tomar mejores decisiones.

Los objetivos específicos del proyecto fueron:

1. Sensar la humedad del suelo, la luz ambiente, el nivel de agua, la temperatura y la humedad relativa del aire.
2. Accionar una bomba de agua cuando el sustrato requiriera riego y hubiera agua disponible.
3. Encender una fuente de iluminación cuando la luz ambiente fuera insuficiente.
4. Mostrar las mediciones y el estado del sistema mediante una interfaz local.
5. Permitir la configuración de umbrales y conservarlos después de una desconexión.
6. Incorporar indicaciones visuales y sonoras, diagnóstico de corriente y un modo de prueba.
7. Mantener una arquitectura modular que admitiera ampliaciones posteriores.

## 1.2 Alcance y limitaciones

El alcance de la primera versión quedó definido de la siguiente manera.

**Funciones incluidas**

- Medición de humedad del suelo, luz, nivel de agua, temperatura y humedad ambiente.
- Lectura de dos señales destinadas al diagnóstico de corriente de bomba e iluminación.
- Riego mediante una bomba de corriente continua.
- Iluminación artificial mediante una tira LED de 5 V.
- Interfaz local con LCD de 16 × 2 caracteres y cuatro pulsadores.
- Señalización con LED RGB y buzzer.
- Configuración de umbrales y habilitación independiente del sonido y del LED de estado.
- Persistencia de configuraciones en la memoria Flash interna.
- Modo de prueba para sensores y actuadores.
- Código fuente para identificar y señalizar trece causas de falla, con tiempos de gracia para el arranque y las transiciones de los actuadores y notificación única al activar cada causa; su recuperación y robustez todavía requieren validación y correcciones puntuales.

**Funciones excluidas o postergadas**

- Alimentación autónoma mediante batería y panel solar.
- Conectividad Wi-Fi, Bluetooth o aplicación móvil.
- Gestión coordinada de varias macetas.
- Calefacción del sustrato o del ambiente.
- Medición de pH o nutrientes.
- Control remoto y almacenamiento histórico en la nube.

## 1.3 Descripción general del sistema

Smartceta se desarrolló como un sistema embebido destinado a supervisar las condiciones necesarias para el cuidado de una planta y actuar sobre ellas con una intervención mínima del usuario. La unidad central se implementó con una placa de desarrollo NUCLEO-F103RB, encargada de adquirir las mediciones, ejecutar la lógica de control y comandar tanto los actuadores como la interfaz local.

El sistema incorporó sensores para medir la humedad del suelo, la intensidad de luz ambiente, el nivel de agua disponible en el depósito y la temperatura y humedad del aire. También se incluyeron sensores para medir la corriente consumida por la bomba y por la tira LED. Estas últimas mediciones se plantearon como información de diagnóstico para detectar condiciones de funcionamiento anormales en las etapas de potencia.

A partir de los valores adquiridos y de los parámetros configurados por el usuario, el sistema determina cuándo regar la planta y cuándo encender la iluminación artificial. El riego utiliza una bomba controlada por modulación por ancho de pulso, generando una rampa de tensión durante el arranque y la detención. El riego inicia sólo con agua suficiente y humedad de suelo por debajo del umbral, y se detiene al alcanzar la consigna más una banda de 10 puntos porcentuales o al disminuir el nivel del depósito. La tira LED se controla por encendido y apagado con una banda de histéresis de 10 puntos. 

La interacción con el usuario se concentró en una pantalla LCD de 16 × 2 caracteres y cuatro pulsadores: siguiente, anterior, aceptar (Enter) y volver (Escape). En operación normal, la pantalla presenta de manera alternada las mediciones de los sensores. Desde el menú de configuración es posible modificar los umbrales de humedad del suelo, luz y nivel de agua, además de habilitar o deshabilitar de manera independiente las señales sonoras y el LED de estado. Los cinco valores seleccionados se almacenan en la memoria Flash del microcontrolador.

Como elementos de señalización se utilizaron un LED RGB y un buzzer activo. El color y el patrón del LED identifican el modo de operación, mientras que el buzzer informa acciones del sistema. Además de los modos normal y configuración, se incorpora un modo de prueba con cuatro lecturas de sensores y cuatro pruebas de actuadores. También contempla trece causas de falla, aplica tiempos de gracia para reducir falsos positivos y solicita el apagado de la bomba y de la tira LED al entrar en el modo falla. 

La figura 1.1 presenta la organización general de Smartceta.

```mermaid
flowchart LR
    subgraph ENTRADAS["Sensores y entradas"]
        HS["Humedad del suelo"]
        LDR["Luz ambiente · LDR"]
        NA["Nivel de agua"]
        DHT["DHT22 · temperatura y humedad ambiente"]
        IB["Medición de corriente de bomba"]
        IL["Medición de corriente de tira LED"]
        BTN["Cuatro pulsadores"]
    end

    MCU["NUCLEO-F103RB<br>STM32F103RBT6"]

    subgraph SALIDAS["Actuadores e interfaz local"]
        QB["Etapa MOSFET de bomba"]
        BOMBA["Bomba de agua"]
        QL["Etapa MOSFET de iluminación"]
        TIRA["Tira LED"]
        LCD["LCD 16 × 2"]
        RGB["LED RGB de estado"]
        DB["Driver de buzzer"]
        BUZ["Buzzer"]
    end

    HS -->|"Analógica · ADC1_IN8"| MCU
    LDR -->|"Analógica · ADC1_IN4"| MCU
    NA -->|"Analógica · ADC1_IN1"| MCU
    DHT -->|"Digital · PC6/EXTI6"| MCU
    IB -->|"Analógica · ADC1_IN11"| MCU
    IL -->|"Analógica · ADC1_IN10"| MCU
    BTN -->|"GPIO"| MCU

    MCU -->|"PWM · TIM1_CH4"| QB
    QB --> BOMBA
    MCU -->|"Digital · PC5"| QL
    QL --> TIRA
    MCU -->|"GPIO · bus de 4 bits"| LCD
    MCU -->|"PWM · TIM4"| RGB
    MCU -->|"GPIO"| DB
    DB --> BUZ

    BOMBA -. "Realimentación de corriente" .-> IB
    TIRA -. "Realimentación de corriente" .-> IL

    classDef sensor fill:#d9f0ff,stroke:#1674a8,color:#111;
    classDef control fill:#dff5df,stroke:#287a28,color:#111;
    classDef output fill:#ffe5d6,stroke:#b55724,color:#111;
    class HS,LDR,NA,DHT,IB,IL,BTN sensor;
    class MCU control;
    class QB,BOMBA,QL,TIRA,LCD,RGB,DB,BUZ output;
```

*Figura 1.1: diagrama en bloques funcional de Smartceta.*


## 1.4 Análisis de sistemas similares

Durante la definición del alcance se consideraron dos equipos de riego doméstico. El sistema Beday integra una bomba, una interfaz local y modalidades de riego manual, temporizado o condicionado por humedad; algunas variantes se alimentan mediante batería y panel solar [14]. El HCT-355 funciona como programador conectado a una canilla, admite tres programas y dispone de detección de lluvia, riego manual e interfaz local [15]. Ambos resuelven parte del problema, pero no integran el mismo conjunto de sensores, actuación e interfaz propuesto para Smartceta.

| Sistema analizado | Variables consideradas | Riego | Iluminación | Interfaz | Diferencia principal respecto de Smartceta |
| --- | --- | --- | --- | --- | --- |
| Sistema de riego automático Beday | Humedad del sustrato en la variante consultada | Automático por temporización, humedad o mando manual | No incluida | Pantalla y teclado  | Puede incorporar alimentación solar, pero no documenta supervisión ambiental ni diagnóstico de corriente [14] |
| HCT-355 | Lluvia y programación horaria | Automático mediante tres programas | No incluida | Pantalla y botones | Requiere conexión a canilla y no decide el riego a partir de la humedad del suelo [15] |
| Smartceta | Humedad del suelo, luz, nivel de agua, temperatura, humedad ambiente y señales de corriente | Automático por humedad | Automático por nivel de luz | LCD, cuatro botones, LED RGB y buzzer | - |

*Tabla 1.1: comparación conceptual de los sistemas considerados.*

<p align="center">
  <img src="Imagenes/Sistema Beday.png" alt="Sistema de riego automático Beday" width="430">
  <br>
  <em>Figura 1.2: sistema de riego automático Beday utilizado como referencia [14].</em>
</p>

<p align="center">
  <img src="Imagenes/Teswelltech HCT-355.png" alt="Programador de riego HCT-355" width="430">
  <br>
  <em>Figura 1.3: programador de riego HCT-355 utilizado como referencia [15].</em>
</p>


## 1.5 Justificación del enfoque técnico

El enfoque técnico se definió buscando un equilibrio entre funcionalidad, costo, disponibilidad de componentes y tiempo de implementación. Debido al alcance académico del trabajo, se priorizó la construcción de un prototipo autónomo en su operación, aunque alimentado desde una fuente externa, capaz de medir las variables principales asociadas al cuidado de una planta, actuar sobre ellas y ofrecer una interfaz amigable con el usuario. Las funciones que requerían infraestructura adicional, como conectividad inalámbrica, batería, panel solar, control de múltiples macetas y análisis de pH, se dejaron fuera de la primera versión.

Como unidad de control se eligió la placa NUCLEO-F103RB. Su microcontrolador STM32F103RBT6 ofrece las entradas y salidas necesarias para conectar sensores, pantalla, pulsadores y actuadores. Los conversores analógico-digitales permiten adquirir las señales de humedad del suelo, luz, nivel de agua y corriente; los temporizadores se utilizan para la señal PWM de la bomba, el LED RGB y la temporización del DHT22; y la memoria Flash interna permite conservar los parámetros sin incorporar una memoria externa. El entorno STM32CubeIDE y la biblioteca HAL facilitaron la configuración de periféricos y la integración de módulos [4].

Los sensores se seleccionaron por disponibilidad, bajo costo y facilidad de conexión. Se empleó un sensor resistivo YL-69 para la humedad del suelo, una LDR para la luz, un módulo resistivo para el nivel de agua y un DHT22 para temperatura y humedad ambiente. Esta selección es apropiada para un prototipo, aunque requiere calibración y no ofrece la durabilidad ni la exactitud de sensores industriales.

El riego se implementó con una bomba de corriente continua controlada mediante una etapa MOSFET de canal N y modulación por ancho de pulso. El PWM permite aplicar una rampa gradual tanto para el encendido como para el apagado de la bomba. La tira LED utiliza una segunda etapa MOSFET. Las dos ramas incluyen la medición corriente mediante una resistencia de paso para diagnóstico.

La interfaz local evita depender de un teléfono o de una red. El LCD, los pulsadores, el LED RGB y el buzzer proporcionan consulta, configuración y realimentación inmediata. La fuente externa de 5 V resulta más adecuada que una batería para alimentar de forma sostenida la placa, la bomba y la iluminación.

El firmware se organizó como un sistema *bare-metal* cooperativo con una base de tiempo de 1 ms. La separación en tareas aísla la adquisición, la interfaz y cada actuador; las colas de eventos desacoplan las órdenes; y una estructura de datos compartida concentra las mediciones. Para utilizar un único ADC, las tareas analógicas coordinan su acceso y realizan conversiones por interrupción, sin espera activa.

En suma, las deciciones adoptadas buscan proveer estabilidad, seguridad y facilidad de uso e implementación, además de eventualmente permitir la ampliación del sistema.


---

# Capítulo 2: Introducción específica

## 2.1 Requisitos

Los requisitos funcionales originales se conservaron con sus identificadores para mantener la trazabilidad con el diseño y los ensayos [5].

| Grupo | ID | Descripción |
| --- | :---: | --- |
| Indicadores | 0.1 | El sistema contará con una pantalla LCD para mostrar información al usuario. |
| Indicadores | 0.2 | El sistema contará con un LED indicador de estado. |
| Indicadores | 0.3 | El sistema contará con un buzzer para emitir señales sonoras. |
| Sensores | 1.1 | El sistema contará con un sensor de temperatura y humedad ambiente. |
| Sensores | 1.2 | El sistema contará con un sensor de luz ambiente. |
| Sensores | 1.3 | El sistema contará con un sensor de humedad del suelo. |
| Sensores | 1.4 | El sistema contará con un sensor de nivel de agua en el depósito. |
| Sensores | 1.5 | El sistema contará con un sensor de corriente para detectar fallas en la bomba. |
| Actuadores | 2.1 | El sistema contará con una bomba para regar la planta. |
| Actuadores | 2.2 | El sistema contará con una tira de luces LED para iluminar la planta. |
| Pulsadores | 3.0 | El sistema contará con pulsadores para interactuar con la aplicación. |
| Aplicación | 4.1 | La aplicación permitirá configurar el umbral de humedad del suelo. |
| Aplicación | 4.2 |La aplicación permitirá configurar si se desea que se active la iluminacion|
| Aplicación | 4.3 | La aplicación permitirá activar o desactivar las alarmas sonoras y visuales. |
| Aplicación | 4.4 | La aplicación permitirá visualizar las lecturas de los sensores en tiempo real. |
| Aplicación | 4.5 | La aplicación permitirá ingresar a un modo de prueba de los componentes. |
| Aplicación | 4.6 | El sistema almacenará localmente valores y configuraciones básicas en memoria no volátil. |
| Alarmas | 5.1 | El sistema contará con alarmas sonoras y visuales para notificar fallas. |
| Alarmas | 5.2 | El sistema activará las alarmas cuando detecte un nivel de agua bajo. |
| Alarmas | 5.3 | El sistema activará las alarmas cuando detecte una falla en la bomba o la tira LED mediante la medición de corriente. |

*Tabla 2.1: requisitos funcionales de Smartceta.*

Como restricciones de diseño se adoptaron una alimentación externa de 5 V, operación local sin conectividad, ejecución *bare-metal* y utilización de la memoria y los periféricos internos del STM32F103RBT6.

## 2.2 Casos de uso

### 2.2.1 Operación normal

| Elemento | Definición |
| --- | --- |
| Disparador | El sistema termina la inicialización o regresa desde otro modo. |
| Precondiciones | Alimentación estable, sensores conectados y configuraciones válidas. |
| Flujo principal | El sistema actualiza las mediciones, las presenta en el LCD y evalúa periódicamente los umbrales. Si el suelo está seco y hay suficiente agua, ordena encender la bomba. Si la luz es inferior al umbral, enciende la tira LED. |
| Flujos alternativos | El usuario cambia la pantalla, entra a configuración o mantiene ESC para entrar al modo de prueba. Una lectura inválida debe originar una indicación de falla. |
| Poscondición | Los actuadores quedan en el estado determinado por las mediciones y la configuración. |

*Tabla 2.2: caso de uso de operación normal.*

### 2.2.2 Configuración de parámetros

| Elemento | Definición |
| --- | --- |
| Disparador | El usuario presiona ENTER en modo normal. |
| Precondiciones | El LCD y los pulsadores se encuentran operativos. |
| Flujo principal | El usuario recorre sonido, luz, nivel de agua, humedad del suelo y habilitación del LED de estado; selecciona un parámetro; modifica su valor; confirma con ENTER; y el firmware registra el cambio en Flash. |
| Flujos alternativos | ESC abandona la pantalla de edición sin escribir un nuevo registro en Flash o regresa al modo normal. El requisito original también prevé volver a NORMAL después de un tiempo de inactividad [5]. |
| Poscondición | La configuración confirmada queda disponible en RAM y almacenada en memoria no volátil. |

*Tabla 2.3: caso de uso de configuración.*

### 2.2.3 Gestión de fallas

| Elemento | Definición |
| --- | --- |
| Disparador | `task_system_failure` detecta una o más de las trece causas contempladas: corriente anormal en bomba o tira LED, falla de sus etapas de potencia, temperatura fuera de rango, error persistente del DHT22, nivel de agua bajo o lectura inválida de un sensor. Los *timeouts*, tramas incompletas y errores de checksum del DHT22 se agrupan bajo la etiqueta «DHT22 No Resp.». |
| Precondiciones | Las tareas de adquisición y el monitoreo de fallas se encuentran activos. |
| Flujo principal | El gestor registra las causas activas y notifica al menú. Al ingresar en modo de falla, el firmware solicita apagar la bomba y la tira LED, presenta en el LCD el código y nombre de la causa y activa las indicaciones visuales o sonoras que estén habilitadas. El usuario puede recorrer las causas y el sistema realiza *autoscroll*. |
| Recuperación prevista | Si el sistema detecta que la falla fue solucionada (Por ejemplo, se recuperó la comunicación perdida con un sensor) se ofrece volver al modo normal mediante una pulsación sostenida de ESC. Dos detecciones consecutivas de sobrecorriente en una misma carga bloquean la recuperación y el sistema debe desenergizarse para volver a ser operado con normalidad. |

*Tabla 2.4: caso de uso de gestión de fallas.*

### 2.2.4 Prueba de componentes

| Elemento | Definición |
| --- | --- |
| Disparador | El usuario mantiene presionado ESC; el umbral interno es de 1500 ms después del antirrebote, por lo que el evento ocurre aproximadamente a los 1,55 s desde el flanco físico. |
| Precondiciones | El prototipo está conectado y el depósito contiene agua antes de probar la bomba.  |
| Flujo principal | El usuario recorre los componentes con siguiente/anterior, inicia una prueba con ENTER y la detiene con ESC. Un segundo ESC regresa al modo normal. |
| Componentes implementados | Nivel de agua, luz, humedad de suelo, humedad y temperatura ambiente, LED de estado, buzzer, bomba y tira LED. |
| Poscondición prevista | Las pruebas de sensores finalizan sin modificar salidas y toda prueba de actuador debe terminar con todas las cargas apagadas. |

*Tabla 2.5: caso de uso del modo de prueba.*

## 2.3 Descripción de los módulos principales

### 2.3.1 Unidad de control

La placa NUCLEO-F103RB incorpora un microcontrolador STM32F103RBT6 basado en Arm Cortex-M3, con 128 KiB de memoria Flash, 20 KiB de SRAM, conversores ADC de 12 bits, temporizadores y GPIO suficientes para el prototipo [1]. La placa también proporciona la interfaz ST-LINK para programación y depuración [2].

### 2.3.2 Sensores

| Variable | Elemento empleado | Interfaz | Alimentación | Uso en el sistema |
| --- | --- | --- | --- | --- |
| Humedad del suelo | YL-69 | Analógica | 3,3 V | Determina la necesidad de riego |
| Luz ambiente | Módulo Duaitek LIGHT-SENSOR con LDR y LM393 | Analógica AO; salida digital DO no utilizada | 3,3–5 VCC; 3,3 V en Smartceta | Determina el encendido de la tira LED |
| Nivel de agua | Módulo resistivo KY-059 | Analógica | 3–5 VCC; 3,3 V en Smartceta | Habilita el riego y detecta nivel bajo |
| Temperatura y humedad del aire | DHT22 | Digital con protocolo propietario | 3–5 VCC; 3,3 V en Smartceta | Información ambiental [3] |
| Corriente de bomba | Resistencia de medición y limitación por Zener | Analógica hacia ADC de 3,3 V | Componente pasivo | Diagnóstico de la etapa de riego |
| Corriente de iluminación | Resistencia de medición y limitación por Zener | Analógica hacia ADC de 3,3 V | Componente pasivo | Diagnóstico de la etapa de iluminación |

*Tabla 2.6: sensores y señales adquiridas.*

El módulo de luz Duaitek LIGHT-SENSOR incorpora un fotoresistor LDR, un comparador LM393 y un potenciómetro para ajustar el umbral de su salida digital. También dispone de una salida analógica relacionada de forma no lineal con la iluminación, que Smartceta convierte en un porcentaje relativo. Su alimentación admitida es de 3,3 a 5 VCC y la placa mide aproximadamente 30 × 14 mm [11].

El KY-059 detecta la presencia y el nivel relativo de agua mediante pistas conductoras expuestas. Posee una salida analógica, admite una alimentación de 3 a 5 VCC y presenta un área sensible aproximada de 40 × 16 mm sobre una placa de 65 × 20 × 8 mm.[10].

### 2.3.3 Actuadores y etapas de potencia

La bomba utilizada es una Duaitek WATER-PUMP-120LH, especificada para 3–6 VCC y un caudal máximo publicado de 120 L/h; en Smartceta se alimenta con 5 V [12]. La iluminacion es una tira LED 5050 flexible [13]. En el prototipo, el segmento instalado se alimenta desde la línea de 5 V mediante su etapa de potencia.

Cada carga se conecta mediante una etapa MOSFET de canal N FQPF13N06L en configuración de conmutación *low-side*. La bomba recibe PWM desde TIM1, mientras que la tira LED se controla como una salida digital. Un transistor MPSA42 maneja el buzzer y tres canales PWM controlan los colores del LED RGB.


### 2.3.4 Interfaz local

La interfaz está formada por un LCD de 16 × 2 caracteres conectado en modo de cuatro bits y cuatro pulsadores con activación por nivel bajo: siguiente, anterior, aceptar (ENTER) y cancelar (ESC). Un LED RGB identifica los modos y acciones principales; el buzzer genera un pulso, dos pulsos, una indicación continua o un patrón intermitente según el evento recibido.

### 2.3.5 Alimentación

Una fuente externa de 5 V ingresa por J3 y alimenta tanto las cargas como la entrada E5V de la NUCLEO mediante el pin 6 del conector CN7. Para utilizar E5V, el manual de la placa especifica 4,75–5,25 V, y una provisión de corriente máxima de 500 mA, el puente JP5 entre los pines 2 y 3 y la remoción de JP1 [17]. Los sensores y niveles lógicos utilizan 3,3 V provistos por la placa NUCLEO.

---

# Capítulo 3: Diseño e implementación

## 3.1 Arquitectura general

La arquitectura gestiona adquisición, supervisión, decisión, interfaz y actuación respetando la separación de módulos independientes de "Sensores", "Sistemas" y "Actuadores". Las tareas de sensores actualizan una estructura de datos común; `task_system_failure` evalúa las condiciones anormales; `task_menu` consulta los datos, administra los modos y produce eventos; `task_display` transfiere al LCD el contenido solicitado; y las tareas de actuadores consumen colas independientes. Los pulsadores generan eventos de usuario. Las configuraciones se escriben en la última página de la Flash.

```mermaid
flowchart LR
    subgraph Sensores
        HS["Humedad de suelo"]
        LU["Luz"]
        NA["Nivel de agua"]
        IB["Corriente de bomba"]
        IL["Corriente de LED"]
        DT["Temperatura y humedad ambiente"]
    end

    ADC["ADC1"]
    SD["Datos compartidos"]
    BTN["Cuatro pulsadores"]
    MEN["Menú y lógica de control"]
    FAL["Gestor centralizado de fallas"]
    FL["Configuración en Flash"]
    Q["Colas de eventos"]

    subgraph Actuadores
        LCD["LCD 16 × 2"]
        BOM["Bomba"]
        LED["Tira LED"]
        RGB["LED RGB"]
        BUZ["Buzzer"]
    end

    HS --> ADC
    LU --> ADC
    NA --> ADC
    IB --> ADC
    IL --> ADC
    ADC --> SD
    DT --> SD
    BTN --> MEN
    SD --> MEN
    SD --> FAL
    FAL -->|"evento de falla"| MEN
    FL <--> MEN
    MEN --> LCD
    MEN --> Q
    Q --> BOM
    Q --> LED
    Q --> RGB
    Q --> BUZ
```

*Figura 3.1: arquitectura funcional presente en el código fuente.*

## 3.2 Diseño de hardware

### 3.2.1 Unidad de control y periféricos

El reloj del sistema se obtiene del oscilador interno HSI dividido por dos y multiplicado por dieciséis mediante el PLL. El núcleo y APB2 operan a 64 MHz, APB1 a 32 MHz y el ADC recibe 64 MHz/6, aproximadamente 10,67 MHz.

| Periférico | Configuración relevante | Función |
| --- | --- | --- |
| ADC1 | 12 bits; canales seleccionados en tiempo de ejecución; muestreo de 239,5 ciclos | Cinco entradas analógicas |
| TIM1 CH4 | Prescaler 19; período 3199 | PWM de la bomba a 1 kHz |
| TIM2 | Prescaler 63; período 65535 | Base de 1 µs para decodificar el DHT22 |
| TIM4 CH1/CH3/CH4 | Prescaler 0; período 100 | PWM azul, verde y rojo del LED de estado |
| SysTick | 1 ms | Base temporal del planificador |
| EXTI6, atendida por EXTI9_5 | Flancos ascendentes y descendentes durante la adquisición | Captura de la trama DHT22 |

*Tabla 3.1: periféricos principales del STM32.*



### 3.2.2 Entradas analógicas y calibración

Las cinco señales analógicas comparten ADC1. Antes de cada conversión la tarea solicitante configura el canal y registra su propiedad en el *router* de *callbacks*. La conversión se inicia por interrupción; al finalizar, el valor se entrega únicamente a la tarea propietaria y se libera el recurso. Cada tarea posee además un tiempo de espera de 10 ms para evitar que una conversión perdida bloquee indefinidamente el módulo.

La humedad del suelo se calcula entre dos extremos incluidos en el firmware: 4095 para suelo seco y 1700 para suelo húmedo. El nivel de agua utiliza 0 para depósito vacío y 2200 para el nivel adoptado como lleno. La luz se escala entre 4095 para oscuridad y 0 para máxima iluminación. Todos los porcentajes se limitan al intervalo 0–100 %.

Las señales de corriente se expresan en miliamperes mediante referencias de un punto incluidas en el firmware. Para la bomba se adoptan 0 cuentas como 0 mA y 1556 cuentas como 128,5 mA; para la tira LED se utilizan 0 cuentas como 0 mA y 573 cuentas como 163,6 mA. El resultado se almacena como un entero en miliamperes. El gestor de fallas considera para la bomba un intervalo de 10 a 300 mA y para la tira LED uno de 30 a 200 mA, después de un tiempo de estabilización de 300 ms.

### 3.2.3 Sensor DHT22

La lectura del DHT22 se implementó sin espera activa prolongada. La línea de datos está conectada a PC6 y utiliza EXTI6, atendida mediante el vector compartido `EXTI9_5_IRQn`. La tarea inicia la comunicación cada 2000 ms, mantiene la línea en nivel bajo durante 2 ms y luego habilita la interrupción por ambos flancos. TIM2 mide la duración de los pulsos con resolución de 1 µs. Los pulsos altos mayores que 50 µs se interpretan como 1 lógicos. La trama recibida se valida mediante su suma de comprobación antes de actualizar los datos compartidos.

### 3.2.4 Control de la bomba

La bomba se conecta mediante un FQPF13N06L en conmutación *low-side* y se comanda desde PA11/TIM1_CH4. Su rango de alimentación publicado es de 3 a 6 VCC y su caudal máximo es de 120 L/h. Se opera el PWM a 1 kHz para generar una rampa controlada de encendido y de apagado. El firmware modifica el valor de comparación entre aproximadamente 480 y 3200 en pasos de 10 cada 1 ms. La inicialización avanza desde 0 a 480 para evitar enviar pulsos de tensión demasiado cortos que generen ruido pero que no alcancen a mover agua. El recorrido útil requiere aproximadamente 320 ms.

El sensado de corriente utiliza una resistencia de paso y una entrada limitada por un diodo Zener de 3V. El objetivo es detectar consumo excesivo y distinguir una bomba desconectada o bloqueada. El diodo proporciona un enclavamiento en 3V ante una subida repentina de la corriente que atraviesa la resistencia de *shunt*.

### 3.2.5 Control de iluminación e indicadores

La tira instalada utiliza LED 5050 y se conecta a una segunda etapa FQPF13N06L gobernada por PC5 como salida digital. La corriente se sensa por PC0/ADC1_IN10.

El LED RGB utiliza TIM4: PB6 para azul, PB8 para verde y PB9 para rojo. El firmware define verde fijo para modo normal, azul con parpadeo lento para configuración, rojo con parpadeo rápido para falla y violeta fijo para prueba. Durante el riego emplea un patrón adicional con parpadeo en tono celeste.

### 3.2.6 Interfaz de usuario

El LCD se conectó en modo de cuatro bits. `task_display` utiliza un doble búfer y actualiza el LCD de forma incremental: en cada invocación realiza una operación de posicionamiento o escribe un carácter, evitando concentrar la actualización completa de las dos filas en un único ciclo del planificador. Los pulsadores poseen *pull-up* y son activos en nivel bajo. La tarea de botones aplica un antirrebote temporal de 50 ms y, después de validarlo, reconoce una pulsación mantenida de ESC a los 1500 ms; desde el flanco físico, la latencia nominal total es cercana a 1,55 s.

El buzzer se acciona mediante un transistor MPSA42. Los patrones implementados incluyen un pulso de 80 ms, dos pulsos de 80 ms separados por 80 ms, sonido continuo y alternancia de 300 ms encendido/300 ms apagado.


### 3.2.7 Esquemático, PCB y montaje

La documentación eléctrica se encuentra en los siguientes archivos:

- [Esquemático completo](../hardware/TDSE-TF/TDSE-TF.pdf).
- [Capa de cobre](../hardware/TDSE-TF/TDSE-TF-B_Cu.pdf).
- [Plano de fabricación](../hardware/TDSE-TF/TDSE-TF-F_Fab.pdf).
- [Asignación de pines de la NUCLEO](../hardware/TDSE-TF/NucleoF103RB-pinout.jpg).
- [Configuración de alimentación externa de la NUCLEO](../hardware/Alimentación%20externa.pdf).
- [Fuente del esquemático](../hardware/TDSE-TF/TDSE-TF.kicad_sch).
- [Fuente de la PCB](../hardware/TDSE-TF/TDSE-TF.kicad_pcb).



### 3.2.8 Pinout del sistema

| Función | Pin STM32 | Periférico o modo |
| --- | --- | --- |
| Nivel de agua | PA1 | ADC1_IN1 |
| Luz ambiente | PA4 | ADC1_IN4 |
| Humedad del suelo | PB0 | ADC1_IN8 |
| Corriente de LED | PC0 | ADC1_IN10 |
| Corriente de bomba | PC1 | ADC1_IN11 |
| DHT22 | PC6 | GPIO/EXTI6 |
| PWM de bomba | PA11 | TIM1_CH4 |
| Tira LED | PC5 | Salida digital |
| LED RGB azul | PB6 | TIM4_CH1 |
| LED RGB verde | PB8 | TIM4_CH3 |
| LED RGB rojo | PB9 | TIM4_CH4 |
| Buzzer | PB13 | Salida digital |
| ESC | PB1 | Entrada con *pull-up* interno |
| Siguiente | PB2 | Entrada con *pull-up* interno |
| Anterior | PB14 | Entrada con *pull-up* interno |
| ENTER | PB15 | Entrada con *pull-up* interno |
| LCD RS | PA0 | Salida digital |
| LCD E | PA8 | Salida digital |
| LCD D4 | PB10 | Salida digital |
| LCD D5 | PB4 | Salida digital |
| LCD D6 | PB5 | Salida digital |
| LCD D7 | PA10 | Salida digital |
| USART2 TX/RX | PA2/PA3 | AF push-pull / entrada |

*Tabla 3.2: asignación de pines implementada.*

### 3.2.9 Lista de materiales

| Cantidad | Elemento | Valor o modelo | Observaciones |
| ---: | --- | --- | --- |
| 1 | Placa de desarrollo | NUCLEO-F103RB | Unidad de control |
| 1 | Sensor de humedad de suelo | YL-69 | Sensor resistivo |
| 1 | Sensor ambiental | DHT22 | Temperatura y humedad |
| 1 | Sensor de luz | Duaitek LIGHT-SENSOR | LDR + LM393 |
| 1 | Sensor de nivel | KY-059 | Resistivo; salida analógica |
| 1 | Bomba sumergible | Duaitek WATER-PUMP-120LH | |
| 1 | Segmento de tira LED | Tira LED 5050 | 30 cm de tira LED |
| 1 | LCD | 16 × 2 | Interfaz paralela de cuatro bits |
| 4 | Pulsadores | Normalmente abiertos | |
| 1 | LED RGB | Cátodo común | Indicador de estado |
| 1 | Buzzer | Buzzer activo | Aviso sonoro |
| 2 | MOSFET canal N | FQPF13N06L  | Bomba y tira LED |
| 1 | Transistor NPN | MPSA42 | Driver de buzzer |
| 2 | Diodos Zener | 1N4727A | Protección de entradas |
| 4 | Resistencias R3, R5, R6 y R7 | 330 Ω, 1/4 W |  |
| 1 | Resistencia R4 | 5,1 kΩ nominal, 1/4 W | |
| 1 | Resistencia de potencia R1 | 10 Ω, 2 W |  |
| 1 | Resistencia R2 | 3,3 Ω, 1/4 W | |
| 1 | Resistencia R8 | 3,3 kΩ, 1/4 W |  |
| 1 | Potenciómetro RV1 | 10 kΩ | Contraste del LCD |
| 2 | Tiras de pines hembra dobles | 2x19 | Conexión de la NUCLEO |
| — | Conectores de sensores y cargas | Conectores de 2, 3 y 4 vías | |
| 1 | PCB | Diseño propio | Prototipo |

*Tabla 3.3: lista de materiales utilizados.*


## 3.3 Diseño de firmware

### 3.3.1 Arquitectura de ejecución

Después de inicializar HAL, relojes y periféricos, `app_init()` inicializa las catorce tareas. `HAL_SYSTICK_Callback()` incrementa un contador cada 1 ms y `app_update()` consume los ticks pendientes. Por cada tick se ejecutan, siempre en el mismo orden, todas las funciones `update`.

El contador de ciclos DWT mide el tiempo de cada tarea y conserva su máximo observado. Esta instrumentación permite obtener el *Worst Case Execution Time* (WCET) experimental sin modificar la secuencia funcional.

### 3.3.2 Tareas y periodicidades

| Tarea | Función | Períodos de activación o evaluación |
| --- | --- | --- |
| `task_button` | Antirrebote y eventos de cuatro pulsadores | Evaluación cada 1 ms |
| `task_display` | Se actualiza el LCD de manera incremental | Cada 1 ms cuando existe contenido pendiente |
| `task_humidity` | Humedad de suelo | Solicitud cada 50 ms, sujeta al arbitraje del ADC |
| `task_light` | Luz ambiente | Solicitud cada 50 ms, sujeta al arbitraje del ADC |
| `task_water_level` | Nivel de agua | Solicitud cada 50 ms, sujeta al arbitraje del ADC |
| `task_pump_current` | Corriente de bomba | Solicitud cada 50 ms, sujeta al arbitraje del ADC |
| `task_led_current` | Corriente de iluminación | Solicitud cada 50 ms, sujeta al arbitraje del ADC |
| `task_dht22` | Temperatura y humedad del aire | Inicio de trama cada 2 s |
| `task_state_led` | Color y parpadeo del LED RGB | Cada 1 ms y por eventos |
| `task_buzzer` | Patrones sonoros | Cada 1 ms y por eventos |
| `task_pump` | Estado y rampa PWM | Cada 1 ms y por eventos |
| `task_led_strip` | Control de actuador de iluminación | Cada 1 ms y por eventos |
| `task_system_failure` | Supervisa y mantiene el conjunto de fallas | Evaluación cada 1 ms |
| `task_menu` | Modos, configuración, presentación de datos y decisiones de control | Cada 1 ms; refresco de pantalla 1 s; autoavance 5 s; luz 20 s; espera mínima de riego 40 s |

*Tabla 3.4: tareas principales del firmware.*

### 3.3.3 Adquisición y datos compartidos

Las tareas analógicas implementan pequeñas máquinas de estados: espera del período, solicitud del ADC, espera de conversión, procesamiento y publicación. El *router* de *callbacks* evita que una tarea consuma la conversión iniciada por otra. Los valores procesados se almacenan en la estructura de datos compartida `shared_data`, junto con el modo activo y los valores configurados.

El DHT22 utiliza una máquina separada con estados de inicio, captura, decodificación, validación y error. La adquisición por flancos evita bloquear el lazo principal durante toda la trama. El indicador de error se activa después de tres adquisiciones fallidas consecutivas, equivalentes a unos 6 s con el período nominal de 2 s.

### 3.3.4 Control de riego

Después de una orden ON u OFF, el menú espera como mínimo 40 s antes de habilitar una nueva decisión de inicio. Si al vencer ese plazo no corresponde cambiar el estado, la marca temporal permanece vencida y el criterio de arranque vuelve a evaluarse en cada ciclo de 1 ms. Mientras la máquina de la bomba no está en reposo, el control también se comprueba en cada ciclo. El arranque requiere que el nivel de agua sea mayor o igual que el mínimo configurado y que la humedad del suelo sea menor que el umbral. La parada se ordena cuando, con la bomba encendida, la humedad es mayor o igual a la configurada más un 10%, o cuando el nivel de agua cae por debajo del mínimo. La tarea de bomba aplica la rampa PWM de aproximadamente 320 ms descrita en 3.2.4.

### 3.3.5 Control de iluminación

Cada 20 s la tarea de menú consulta el estado de la tira. Si está apagada, ordena encenderla cuando la luz es menor o igual que el umbral configurado; si está encendida, ordena apagarla cuando la medición es mayor o igual que el umbral más 10%.

### 3.3.6 Modos de operación

```mermaid
stateDiagram-v2
    [*] --> NORMAL
    NORMAL --> SETUP: ENTER
    SETUP --> NORMAL: ESC
    NORMAL --> TEST: ESC mantenido ≈1,55 s
    TEST --> NORMAL: ESC sin prueba activa
    NORMAL --> FAILURE: evento de falla
    SETUP --> FAILURE: evento de falla
    TEST --> FAILURE: evento de falla
    FAILURE --> NORMAL: causas resueltas + ESC mantenido
    note right of FAILURE
      Dos fallas consecutivas de
      sobrecorriente bloquean el sistema
    end note
```

*Figura 3.2: modos contemplados por el código fuente.*

En `NORMAL` se presentan las mediciones y se evalúan los controles automáticos; `SETUP` permite editar y guardar cinco configuraciones; y `TEST` ofrece cuatro lecturas de sensores y cuatro pruebas de actuadores. Ante un evento de falla, el menú ingresa en `FAILURE`, solicita apagar la bomba y la tira LED y muestra las fallas activas.

### 3.3.7 Interfaz de usuario

En modo normal el LCD recorre cinco pantallas: humedad del suelo, humedad ambiente, temperatura ambiente, luz y nivel de agua. El cambio es manual con siguiente/anterior o automático cada 5 s. ENTER abre el menú de configuración y ESC mantenido abre el modo de prueba.

Los valores configurables y sus límites se muestran en la tabla 3.5.

| Parámetro | Valor inicial del firmware | Intervalo |
| --- | ---: | ---: |
| Sonidos | 1, habilitados | 0–1 |
| Umbral de luz | 50 % | 0–100 % |
| Nivel mínimo de agua | 40 % | 15–100 % |
| Humedad deseada del suelo | 70 % | 0–100 % |
| LED de estado | 1, habilitado | 0–1 |

*Tabla 3.5: cinco configuraciones predeterminadas.*


### 3.3.8 Alarmas y recuperación

`task_system_failure` evalúa trece posibles fallas: corriente anormal de la bomba, corriente anormal de la tira LED, temperatura fuera del intervalo de 0 a 35 °C, error persistente del DHT22, nivel de agua bajo y lecturas consideradas inválidas en los sensores analógicos. Los *timeouts*, tramas incompletas y errores de checksum del DHT22 se notifican mediante una única causa rotulada «DHT22 No Resp.». El diagnóstico comienza después de un período de gracia inicial de 150 ms para evitar falsos positivos; el DHT22 informa error después de tres fallos consecutivos; y las corrientes se comprueban 300 ms después de alcanzar ON u OFF, omitiendo las rampas. Los  umbrales de corriente se configuraron en 10–300 mA para la bomba y 30–200 mA para la tira LED.

Cada falla activa se conserva en un arreglo para que la interfaz pueda mostrarla de manera individual. El menú cambia al modo de falla y solicita apagar la bomba y la tira LED.

La restauración del sistema se habilita cuando `task_system_failure_can_restore()` determina que las fallas activas volvieron condiciones normales. El usuario debe mantener ESC para borrar las fallas y regresar a `NORMAL`. Dos detecciones independientes de sobrecorriente en la bomba o en la tira LED bloquean esta restauración y mantienen el sistema en la pantalla de bloqueo. Con esto, se busca que el usuario realmente verifique la causa del problema para evitar daños mayores. El sistema vuelve a su funcionamiento normal luego de desenergizarlo y volverlo a energizar.

### 3.3.9 Guardado de configuraciones

Las configuraciones se almacenan desde la dirección `0x0801FC00`, correspondiente a la última página de 1 KiB de la Flash. Cada registro ocupa cuatro bytes: índice y valor, ambos de 16 bits. Al iniciar, el firmware reconstruye el último valor válido de cada parámetro. Cuando se completan 256 registros, borra la página y escribe un estado consolidado.

---

# Capítulo 4: Ensayos y resultados

## 4.1 Metodología general

La verificación se dividió en cuatro niveles:

1. Ensayos eléctricos de alimentación, entradas y salidas.
2. Pruebas funcionales de cada módulo de firmware.
3. Integración.
4. Caracterización de consumo, memoria y tiempo de ejecución.


El instrumental utilizado fue:

- Multímetro digital Uni-t UT61E.
- Multímetro digital CEM DT-4000.
- Fuente de 5,1 V, 700 mA.
- Osciloscopio Hantek DSO2090.
- Depurador ST-LINK y STM32CubeIDE.
- Recipiente graduado, sustrato seco y húmedo y una referencia ambiental.

<span style="color:#0057b8"><strong>🔵 ANOTACIÓN:</strong> asociar a cada fila un enlace a una fotografía, captura de osciloscopio o archivo de datos.</span>

## 4.2 Pruebas funcionales del hardware

| ID | Ensayo | Procedimiento y criterio de aceptación | Resultado registrado |
| :---: | --- | --- | --- |
| HW-01 | Alimentación de 5 V y 3,3 V | Medir ambas líneas en reposo y con máxima carga. No deben salir de la tolerancia admitida por los componentes. | Fuente sin carga: 5,1 V; corriente nominal: 700 mA. Fuente con carga máxima (sistema, bomba y tira led encendidos): 4,77 V |
| HW-02 | LCD | Encender, recorrer todas las pantallas y comprobar legibilidad y ausencia de caracteres corruptos. | Funciona segun lo esperado |
| HW-03 | Pulsadores | Realizar al menos 10 pulsaciones breves por tecla y pulsaciones mantenidas de ESC. No deben observarse eventos dobles ni pérdidas. | Funciona segun lo esperado |
| HW-04 | Humedad del suelo | Medir en aire, sustrato seco, humedad intermedia y sustrato saturado. | Funciona segun lo esperado |
| HW-05 | Luz | Medir en oscuridad, condiciones normales e iluminación intensa. La indicación debe cubrir el intervalo útil sin saturación prematura. | Funciona segun lo esperado |
| HW-06 | Nivel de agua | Registrar vacío y varios niveles conocidos. | Funciona segun lo esperado |
| HW-07 | DHT22 | Comparar 10 lecturas con un instrumento de referencia en régimen estable. | Funciona segun lo esperado |
| HW-08 | Bomba y PWM | Observar PA11, verificar frecuencia, rampa, apagado eléctrico; controlar VGS, VDS y temperatura del MOSFET. | Funciona segun lo esperado |
| HW-09 | Tira LED | Accionar PC5, medir corriente y verificar apagado completo. | Funciona segun lo esperado |
| HW-10 | Medición de corriente | Aplicar cero, carga nominal y condición límite a ambos canales. | Funciona segun lo esperado |
| HW-11 | LED RGB y buzzer | Solicitar todos los patrones y comprobar color, frecuencia y sonido. | Funciona segun lo esperado |

*Tabla 4.1: protocolo de pruebas funcionales del hardware.*

## 4.3 Pruebas funcionales del firmware

| ID | Función | Procedimiento | Criterio | Resultado |
| :---: | --- | --- | --- | --- |
| FW-01 | Inicio | Arrancar con Flash vacía y con Flash previamente configurada. | Carga valores iniciales en el primer caso, conserva los últimos valores en el segundo y no dispara fallas antes de contar con muestras válidas. | Funciona segun lo esperado |
| FW-02 | Navegación normal | Recorrer manualmente las cinco variables y esperar el autoavance. | Orden correcto, texto válido y autoavance cercano a 5 s. | Funciona segun lo esperado |
| FW-03 | Configuración | Modificar los cinco parámetros, cancelar una edición, confirmar otra y dejar el menú inactivo. | Respeta límites, ESC restaura el valor anterior, ENTER guarda y la inactividad retorna a NORMAL. | Funciona segun lo esperado |
| FW-04 | Persistencia | Guardar valores, desconectar durante 30 s y volver a alimentar. Repetir después de múltiples escrituras y durante una compactación controlada. | Los valores recuperados coinciden con los confirmados y un corte no produce registros incompletos. | Funciona segun lo esperado |
| FW-05 | Antirrebote | Inyectar pulsaciones rápidas y observar los eventos. | Un evento por pulsación válida; ESC mantenido se detecta una vez. | Funciona segun lo esperado |
| FW-06 | Arbitraje ADC | Mantener las cinco tareas analógicas activas y registrar conversiones y *timeouts*. | Ningún resultado se asigna al canal incorrecto, no hay bloqueo y todo error invalida o marca como antigua la muestra previa. | Funciona segun lo esperado |
| FW-07 | Validación DHT22 | Probar lectura normal, sensor ausente y trama con *checksum* incorrecto. | Publica sólo tramas válidas, conserva el signo y la resolución acordada y señala los errores definidos. | Funciona segun lo esperado |
| FW-08 | Rampa de bomba | Enviar ON, OFF y órdenes durante una rampa. | Estado final coherente y ausencia de falsos diagnósticos de corriente. | Funciona segun lo esperado |
| FW-09 | Patrones de estado | Solicitar todos los eventos de LED y buzzer. | La salida coincide con la tabla de patrones y una orden nueva reemplaza la anterior de forma definida. | Funciona segun lo esperado |
| FW-10 | Modo de prueba | Probar cada una de las ocho opciones. | Cada opción ejecuta, informa y detiene la prueba de forma segura. | Funciona segun lo esperado |
| FW-11 | Estado de falla | Inyectar cada una de las trece causas de falla, incluida una durante cada prueba de actuador. | Identifica la causa, apaga cargas peligrosas, avisa y recupera de forma controlada. | Funciona segun lo esperado |

*Tabla 4.2: pruebas funcionales del firmware.*

## 4.4 Pruebas de integración

| ID | Escenario | Resultado esperado | Evidencia final |
| :---: | --- | --- | --- |
| INT-01 | Sensado del ambiente | Las cinco variables se actualizan y muestran con unidad válida. | Funciona segun lo esperado. Se muestra en el Video 4.1 (Minuto 1:10)|
| INT-02 | Iluminación automática | La tira enciende bajo el umbral y apaga sobre el umbral superior sin oscilación. | Funciona segun lo esperado. Se muestra en el Video 4.1 (Minuto 3:30) |
| INT-03 | Riego automático | Con suelo seco y agua suficiente, la bomba riega hasta alcanzar la condición de parada. | Funciona segun lo esperado. Se muestra en el Video 4.1 (Minuto 3:00) |
| INT-04 | Depósito con nivel bajo | La bomba no arranca o se detiene; se indica la causa. | Funciona segun lo esperado |
| INT-05 | Modo falla | Se desconecta un sensor y el sistema entra en modo falla. | Funciona segun lo esperado. Se muestra en el Video 4.1 (Minuto 7:20) |
| INT-06 | Configuración y reinicio | Los valores confirmados se mantienen después de quitar alimentación. | Funciona segun lo esperado. Se muestra en el Video 4.1 (Minuto 10:00) |
| INT-07 | Modo de prueba | Se prueban individualmente todos los sensores y actuadores. | Funciona segun lo esperado. Se muestra en el Video 4.1 (Minuto 5:30) |
| INT-08 | Operación prolongada | El sistema opera sin bloqueo, corrupción de pantalla ni activación espuria. | Funciona segun lo esperado. |

*Tabla 4.3: escenarios de integración y evidencia final.*

El video 4.1 Muestra las diferentes funcionalidades del proyecto completamente integrado.

<p align="center">
  <a href="https://youtu.be/_TpPSb_1oB8" target="_blank">
    <img src="./Imagenes/portada_video.png" alt="Video Presentación SMARTCETA" width="500" />
  </a>
</p>

*Video 4.1: Video demostracion de integracion de la Smartceta.*

## 4.5 Consumo eléctrico

La medición debe realizarse sobre la entrada de 5 V, no únicamente sobre la NUCLEO, porque la bomba y la tira LED dominan el consumo. Para cada estado se registran tensión y corriente después de alcanzar régimen. La potencia de entrada se calcula como:

$$
P\,[\mathrm{W}] = \frac{V\,[\mathrm{V}] \cdot I\,[\mathrm{mA}]}{1000}
\tag{4.1}
$$

| Estado | Tensión de entrada | Corriente | Potencia | Observaciones |
| --- | ---: | ---: | ---: | --- |
| Sistema en reposo | 4,9 V | 100 mA |  490 mW | LCD, LED de estado y sensores activos |
| Sólo tira LED |  4,81 V | 147 mA | 707 mW | Medicion en modo test |
| Bomba en régimen | 4,82 V | 127 mA | 612 mW | Después de la rampa |
| Bomba + tira LED | 4,77 V | 267 mA | 1,27 W | Peor caso sostenido |
| Modo falla | 4,92 V | 116 mA | 0,57 W | LED y buzzer de alarma |
| Maximo consumo | 4,77 V | 367 mA | 1,75 W | Sistema funcionando + LED + Bomba |

*Tabla 4.4: consumo del prototipo.*

La fuente debe soportar el peor caso sostenido y el pico de arranque con un cierto margen. Se estableció el margen de seguridad (M) en un 90%:

$$
I_{\text{fuente}} \geq I_{\text{pico medido}} \cdot M
\tag{4.2}
$$

$$
  M = 90 \%
$$

$$
  I_{\text{pico medido}} = 367 mA
$$

$$
I_{\text{fuente}} = 700 mA
$$

## 4.6 Uso de memoria

| Memoria | Total | Usada por la compilación `Release` | Libre |
| --- | ---: | ---: | ---: |
| Flash física | 131 072 B | 25 176 B (19,2 %) | 105 896 B |
| Flash destinada a la aplicación | 130 048 B, después de reservar 1 KiB | 25 176 B (19,4 %) | 104 872 B |
| SRAM | 20 480 B | 4028 B (19,7 %) | 16 452 B |

*Tabla 4.5: utilización de memoria.*

## 4.7 Análisis temporal

### 4.7.1 WCET experimental

| Tarea | Período de invocación | WCET medido | Utilización \(C_i/T_i\) |
| --- | ---: | ---: | ---: |
| Botones | 1000 µs | 17 µs | 1,7 % |
| Display | 1000 µs | 87 µs | 8,7 % |
| Humedad de suelo | 1000 µs | 22 µs | 2,2 % |
| Luz | 1000 µs | 32 µs | 3,2 % |
| Nivel de agua | 1000 µs | 22 µs | 2,2 % |
| Corriente de bomba | 1000 µs | 22 µs | 2,2 % |
| Corriente de LED | 1000 µs | 27 µs | 2,7 % |
| DHT22 | 1000 µs | 111 µs | 11,1 % |
| LED RGB | 1000 µs | 14 µs | 1,4 % |
| Buzzer | 1000 µs | 9 µs | 0,9 % |
| Bomba | 1000 µs | 13 µs | 1,3 % |
| Tira LED | 1000 µs | 7 µs | 0,7 % |
| Gestor de fallas | 1000 µs | 6 µs | 0,6 % |
| Menú | 1000 µs | 251 µs | 25,1 % |
| **Cota por suma de WCET individuales** | **1000 µs** | **640 µs** | **64,0 %** |

*Tabla 4.6: WCET experimental de las tareas.*

Los valores se obtuvieron del máximo registrado por DWT para cada tarea. La suma de 640 µs es una cota conservadora: los máximos individuales pueden haber ocurrido en ciclos diferentes y, por lo tanto, no reemplaza una medición directa del WCET del ciclo completo.

La cota de utilización se calcula como:

$$
U = \frac{\sum_{i=1}^{14} C_i}{1000\ \mu s}
\tag{4.3}
$$

La cota de utilización es \(U = 64,0 \%\), por lo que existe un sobrante de unos 360 µs por ciclo.

El criterio mínimo es que el WCET del ciclo completo sea menor que 1000 µs.

## 4.8 Cumplimiento de requisitos

En la tabla 4.7 se muestra cada requisito impuesto previamente y se establece si se cumple o no.

| Grupo | ID | Descripción | ¿Se cumple? |
| --- | :---: | --- | --- |
| Indicadores | 0.1 | El sistema contará con una pantalla LCD para mostrar información al usuario. | Sí |
| Indicadores | 0.2 | El sistema contará con un LED indicador de estado. | Sí |
| Indicadores | 0.3 | El sistema contará con un buzzer para emitir señales sonoras. | Sí |
| Sensores | 1.1 | El sistema contará con un sensor de temperatura y humedad ambiente. | Sí |
| Sensores | 1.2 | El sistema contará con un sensor de luz ambiente. | Sí |
| Sensores | 1.3 | El sistema contará con un sensor de humedad del suelo. | Sí |
| Sensores | 1.4 | El sistema contará con un sensor de nivel de agua en el depósito. | Sí |
| Sensores | 1.5 | El sistema contará con un sensor de corriente para detectar fallas en la bomba. | Sí |
| Actuadores | 2.1 | El sistema contará con una bomba para regar la planta. | Sí |
| Actuadores | 2.2 | El sistema contará con una tira de luces LED para iluminar la planta. | Sí |
| Pulsadores | 3.0 | El sistema contará con pulsadores para interactuar con la aplicación. | Sí |
| Aplicación | 4.1 | La aplicación permitirá configurar el umbral de humedad del suelo. | Sí |
| Aplicación | 4.2 |La aplicación permitirá configurar si se desea que se active la iluminacion| Sí |
| Aplicación | 4.3 | La aplicación permitirá activar o desactivar las alarmas sonoras y visuales. | Sí |
| Aplicación | 4.4 | La aplicación permitirá visualizar las lecturas de los sensores en tiempo real. | Sí |
| Aplicación | 4.5 | La aplicación permitirá ingresar a un modo de prueba de los componentes. | Sí |
| Aplicación | 4.6 | El sistema almacenará localmente valores y configuraciones básicas en memoria no volátil. | Sí |
| Alarmas | 5.1 | El sistema contará con alarmas sonoras y visuales para notificar fallas. | Sí |
| Alarmas | 5.2 | El sistema activará las alarmas cuando detecte un nivel de agua bajo. | Sí |
| Alarmas | 5.3 | El sistema activará las alarmas cuando detecte una falla en la bomba o la tira LED mediante la medición de corriente. | Sí |

*Tabla 4.7: Requisitos de la tabla 2.1 y la verificación de su cumplimiento.*


## 4.9 Documentación del desarrollo

| Elemento | Ubicación | Observaciones |
| --- | --- | --- |
| Requisitos y casos de uso | [`REQUISITOS.md`](../REQUISITOS.md) | Disponible |
| Proyecto STM32CubeIDE | [`tdse-tf_3-03/`](../tdse-tf_3-03/) | Disponible |
| Código de aplicación | [`tdse-tf_3-03/app/`](../tdse-tf_3-03/app/) | Disponible |
| Configuración de periféricos | [`tdse-tf_3-03/Core/`](../tdse-tf_3-03/Core/) | Disponible |
| Artefactos de compilación `Release` | [`tdse-tf_3-03/Release/`](../tdse-tf_3-03/Release/) | ELF, MAP, listado y manifiestos de la compilación limpia |
| Esquemático y PCB | [`hardware/TDSE-TF/`](../hardware/TDSE-TF/) | Disponible |
| Alimentación externa de la NUCLEO | [`hardware/Alimentación externa.pdf`](../hardware/Alimentación%20externa.pdf) | Extracto de UM1724 Rev. 17 con condiciones de E5V |
| Memoria técnica | [`Memoria Tecnica/Memoria Tecnica.md`](Memoria%20Tecnica.md) | Disponible |
| Video final | [Link video](https://www.youtube.com/watch?v=_TpPSb_1oB8) | Disponible |

*Tabla 4.9: documentación asociada al desarrollo.*

---

# Capítulo 5: Conclusiones

## 5.1 Resultados obtenidos

El proyecto permitió diseñar un sistema embebido que reúne las funciones principales necesarias para asistir el cuidado de una planta. Sobre una única NUCLEO-F103RB se integraron cinco entradas analógicas, un sensor digital, una pantalla, cuatro pulsadores y cuatro actuadores, incluyendo dos cargas de potencia.

La arquitectura cooperativa y modular separó las responsabilidades de sensado, evaluación y actuación. El arbitraje del ADC organiza el uso concurrente de un único conversor sin introducir esperas activas prolongadas, mientras que las colas de eventos desacoplan la lógica de menú de las salidas. La persistencia en Flash conserva consignas en las pruebas informadas, pero aún requiere reservar la página y robustecer su integridad.

La revisión actual corrigió la condición de parada del riego, implementó una histéresis de iluminación que conserva el estado, expresó los canales de corriente en miliamperes y evitó la publicación repetitiva de una misma falla. El gestor contempla trece causas, una gracia inicial de 150 ms, una espera de 300 ms para diagnosticar los actuadores en estado estable y un filtro de tres fallos consecutivos para el DHT22. Una compilación limpia de las catorce tareas concluyó sin errores ni advertencias y ocupó 25 176 B de Flash y 4028 B de SRAM, equivalentes al 19,2 % y 19,7 % de los recursos físicos, respectivamente.

Estas mejoras no equivalen por sí solas a una validación de seguridad. El reposo de la bomba conserva aproximadamente 15 % de *duty*, no existe tiempo máximo de riego, las colas pueden perder eventos sin diagnóstico, los modos de configuración y prueba no garantizan la detención de una carga previamente activa, y las muestras analógicas carecen de validez y antigüedad. La recuperación de algunas fallas, la integridad de Flash y las protecciones de potencia también requieren cierre. En consecuencia, Smartceta constituye un prototipo integrado y extensible, pero no debe presentarse como un sistema de riego autónomo seguro hasta resolver y ensayar esos puntos.

<span style="color:#008000"><strong>🟢 VALORES EXPERIMENTALES A COMPLETAR:</strong> error máximo de sensado = ___; consumo máximo = ___ W; utilización máxima de CPU = ___ %.</span>

## 5.2 Lecciones aprendidas

La integración de sensores de distinta naturaleza mostró la importancia de separar adquisición, conversión y publicación de datos. El esquema de propiedad del ADC evita interferencias entre canales y puede reutilizarse en otros proyectos con recursos compartidos.

El desarrollo de las etapas de potencia puso de manifiesto que la existencia de una orden de software no garantiza una acción segura. El control de una bomba requiere considerar corriente de arranque, protección inductiva, disponibilidad de agua, tiempo máximo de funcionamiento y una vía de apagado ante fallas.

La interfaz basada en eventos simplificó la incorporación de modos y patrones de señalización, pero también evidenció la necesidad de diseñar explícitamente el tratamiento de colas llenas y de eventos prioritarios. Una falla crítica no debe competir en igualdad de condiciones con una pulsación de usuario.

Por último, la trazabilidad entre requisito, implementación y ensayo permitió detectar diferencias que no resultaban evidentes al observar módulos aislados, como la interpretación de “intensidad de iluminación”, la necesidad de validar de forma independiente las alarmas sonoras y visuales y los riesgos de navegación y apagado dentro del modo de prueba.



## 5.3 Posibles ampliaciones

Una vez cerrada la base funcional, el sistema podría incorporar sensores capacitivos de suelo, regulación PWM de la iluminación, registro histórico, reloj de tiempo real, conectividad inalámbrica, control de varias macetas, gabinete resistente a humedad y alimentación mediante batería y energía solar. Estas ampliaciones deben abordarse después de resolver la seguridad y la repetibilidad del prototipo actual.

---

# Capítulo 6: Uso de herramientas de inteligencia artificial

Durante la preparación de esta versión de la memoria se utilizó una herramienta de inteligencia artificial para analizar la estructura de dos informes de referencia [8], [9], recorrer los archivos del proyecto, proponer una organización documental y redactar una primera versión integral. Las afirmaciones técnicas se contrastaron con `REQUISITOS.md`, el esquemático, la asignación de pines, el código fuente, las hojas de datos del *hardware*, el registro interno de mediciones [16] y una compilación limpia de la configuración `Release`.

| Participante | Herramienta | Uso | Verificación humana requerida |
| --- | --- | --- | --- |
| Juncal, Franco Mariano | ChatGPT/Codex | Análisis, generación y diagnóstico de código; generación de documentos | Revisión de la lógica del código, ensayo de implementaciones y corrección humana de las redacciones |
| Gonzalez Bigliardi, Iñaki | Gemini | Generación de texto de commits, ayuda y diagnóstico de código | Revisión y corrección del texto, y verificacion del codigo. |
| Carrizo, Ezequiel Ignacio | Gemini | Generación y diagnóstico de código. Ayuda con la solución de problemas de EMI. | Revisión del código. Ensayos de laboratorio para determinar la reducción real de las interferencias. |

*Tabla 6.1: uso declarado de herramientas de inteligencia artificial.*

---

# Bibliografía y referencias

[1] STMicroelectronics, [*DS5319 — STM32F103x8/STM32F103xB: medium-density performance line Arm-based 32-bit MCU*](https://www.st.com/en/microcontrollers-microprocessors/stm32f103/documentation.html), hoja de datos, consultada el 25/07/2026.

[2] STMicroelectronics, [*UM1724 — STM32 Nucleo-64 boards (MB1136)*](https://www.st.com/resource/en/user_manual/dm00105823.pdf), revisión 17, manual de usuario, consultado el 25/07/2026.

[3] Aosong Electronics, [*AM2302/DHT22 temperature and humidity sensor*](https://asairsensors.com/product/am2302-dht22-temperature-and-humidity-sensor/), documentación del fabricante, consultada el 25/07/2026.

[4] STMicroelectronics, [*STM32CubeF1 embedded software package*](https://www.st.com/en/embedded-software/stm32cubef1.html), página oficial del paquete HAL y LL para STM32F1, consultada el 25/07/2026.

[5] Equipo Smartceta, [*Requisitos y modos de uso*](../REQUISITOS.md), segundo cuatrimestre de 2025.

[6] Equipo Smartceta, [*Esquemático eléctrico TDSE-TF*](../hardware/TDSE-TF/TDSE-TF.pdf).

[7] Equipo Smartceta, [*Proyecto de firmware STM32CubeIDE*](../tdse-tf_3-03/).

[8] Embebidos-Fran-Marcos-Nacho, [*Memoria técnica del proyecto tdse-tf_1-2*](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/tree/Memoria-final-y-video), consultada el 25/07/2026 como referencia de estructura documental.

[9] M. G. Villafañe, [*Memoria del Trabajo Final — Smartlock*](https://github.com/mgvillafane/SE_TP_Smartlock/tree/main), consultada el 25/07/2026 como referencia de estructura documental.

[10] Mercado Libre Argentina, [*Sensor de detección de agua de lluvia KY-059 Arduino*](https://www.mercadolibre.com.ar/sensor-de-deteccion-de-agua-de-lluvia-ky059-arduino/up/MLAU128801427?pdp_filters=item_id:MLA1605215490), ficha comercial consultada el 23/07/2026.

[11] Mercado Libre Argentina, [*Módulo sensor de luz con LDR fotoresistor compatible con Arduino*](https://www.mercadolibre.com.ar/modulo-sensor-de-luz-con-ldr-fotoresistor-compatible-con-arduino/p/MLA32487827?pdp_filters=item_id:MLA1666016788), modelo Duaitek LIGHT-SENSOR, ficha comercial consultada el 23/07/2026.

[12] Mercado Libre Argentina, [*Bomba sumergible Duaitek WATER-PUMP-120LH, 3–6 V*](https://www.mercadolibre.com.ar/bomba-sumergible-duaitek-water-pump-120lh-3v-6v-120lh-para-acuario/p/MLA39116120?pdp_filters=item_id:MLA1456284509), ficha comercial consultada el 23/07/2026.

[13] Mercado Libre Argentina, [*Tira LED Veoquiero 5050 de 2 m, set ×2*](https://www.mercadolibre.com.ar/tira-led-2m-set-x2-a-pila-flexible-interior-5050-veoquiero/p/MLA67484374?pdp_filters=item_id:MLA3131487374), ficha comercial consultada el 23/07/2026.

[14] Mercado Libre México, [*Sistema automático de riego por goteo solar Beday para 15 plantas*](https://www.mercadolibre.com.mx/sistema-automatico-de-riego-por-goteo-solar-beday-15-plants/p/MLM2079479850), ficha comercial consultada el 25/07/2026.

[15] Eshine, [*HCT-355 Water Timer — instruction manual*](https://eshine-t.com/wp-content/uploads/2025/09/Manual-HCT-355-EN.pdf), manual de usuario consultado el 25/07/2026.

[16] Equipo Smartceta, [*Mediciones SMARTCETA*](../hardware/Mediciones%20SMARTCETA.txt), registro interno de mediciones de banco actualizado el 24/07/2026.

[17] STMicroelectronics, [*UM1724 — STM32 Nucleo-64 boards: external power supply inputs VIN and E5V*](../hardware/Alimentación%20externa.pdf), revisión 17, extracto local de las páginas 22–23.


---


**Fin de la memoria técnica**
