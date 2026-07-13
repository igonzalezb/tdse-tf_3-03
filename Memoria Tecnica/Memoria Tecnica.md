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

<!-- TODO: indicar ciudad y período efectivo de realización del trabajo. -->

*Este trabajo fue realizado en [ciudad], entre [mes y año] y [mes y año].*

</div>

---

## Resumen

<!--
TODO: redactar dos o tres párrafos breves y autocontenidos. Explicar:
- qué problema resuelve Smartceta;
- qué se diseñó e implementó;
- cuáles fueron los resultados principales;
- qué encontrará el lector en esta memoria.

No incluir referencias bibliográficas, notas al pie, siglas sin definir ni
formatos especiales. El resumen debe poder entenderse de manera aislada.
-->

[Completar el resumen del trabajo.]

---

## Abstract (opcional)

<!-- TODO: incluir la traducción al inglés del resumen o eliminar esta sección. -->

[Complete the abstract or remove this section.]

---

## Agradecimientos (opcional)

<!-- TODO: completar o eliminar esta sección. -->

[Completar los agradecimientos.]

---

## Registro de versiones

| Revisión | Cambios realizados | Fecha |
| :---: | --- | :---: |
| 0.1 | Creación de la estructura inicial de la memoria técnica | 13/07/2026 |
| [X.Y] | [Descripción de los cambios] | [dd/mm/aaaa] |

*Tabla 0.1: registro de versiones del documento.*

---

# Índice general

- [Capítulo 1: Introducción general](#capítulo-1-introducción-general)
  - [1.1 Necesidad, motivación y objetivo](#11-necesidad-motivación-y-objetivo)
  - [1.2 Alcance y limitaciones](#12-alcance-y-limitaciones)
  - [1.3 Descripción general del sistema](#13-descripción-general-del-sistema)
  - [1.4 Análisis de sistemas similares](#14-análisis-de-sistemas-similares)
  - [1.5 Justificación del enfoque técnico](#15-justificación-del-enfoque-técnico)
- [Capítulo 2: Introducción específica](#capítulo-2-introducción-específica)
  - [2.1 Requisitos](#21-requisitos)
  - [2.2 Casos de uso](#22-casos-de-uso)
  - [2.3 Descripción de los módulos principales](#23-descripción-de-los-módulos-principales)
- [Capítulo 3: Diseño e implementación](#capítulo-3-diseño-e-implementación)
  - [3.1 Arquitectura general](#31-arquitectura-general)
  - [3.2 Diseño de hardware](#32-diseño-de-hardware)
  - [3.3 Diseño de firmware](#33-diseño-de-firmware)
- [Capítulo 4: Ensayos y resultados](#capítulo-4-ensayos-y-resultados)
  - [4.1 Pruebas funcionales del hardware](#41-pruebas-funcionales-del-hardware)
  - [4.2 Pruebas funcionales del firmware](#42-pruebas-funcionales-del-firmware)
  - [4.3 Pruebas de integración](#43-pruebas-de-integración)
  - [4.4 Medición y análisis de consumo](#44-medición-y-análisis-de-consumo)
  - [4.5 Uso de memoria y análisis del build](#45-uso-de-memoria-y-análisis-del-build)
  - [4.6 Análisis temporal del firmware](#46-análisis-temporal-del-firmware)
  - [4.7 Gestión de bajo consumo](#47-gestión-de-bajo-consumo)
  - [4.8 Cumplimiento de requisitos](#48-cumplimiento-de-requisitos)
  - [4.9 Comparación con sistemas similares](#49-comparación-con-sistemas-similares)
  - [4.10 Documentación del desarrollo](#410-documentación-del-desarrollo)
- [Capítulo 5: Conclusiones](#capítulo-5-conclusiones)
  - [5.1 Resultados obtenidos](#51-resultados-obtenidos)
  - [5.2 Lecciones aprendidas](#52-lecciones-aprendidas)
  - [5.3 Próximos pasos](#53-próximos-pasos)
- [Capítulo 6: Uso de herramientas de inteligencia artificial](#capítulo-6-uso-de-herramientas-de-inteligencia-artificial)
- [Bibliografía y referencias](#bibliografía-y-referencias)
- [Anexos](#anexos-opcional)

<!--
TODO: al preparar la versión final, generar también los índices de figuras y
tablas si la herramienta de edición utilizada lo permite.
-->

---

# Capítulo 1: Introducción general

## 1.1 Necesidad, motivación y objetivo

<!--
TODO: presentar el problema para un lector que no conoce el proyecto. Explicar
la necesidad de automatizar el cuidado de una planta doméstica y el valor que
agrega la solución. Usar tiempo pasado para describir el trabajo realizado.
-->

El trabajo tuvo como objetivo diseñar e implementar una maceta inteligente que facilitara el cuidado de una planta doméstica sin necesidad de supervisión constante. El sistema fue planteado para medir condiciones ambientales y del sustrato, realizar el riego y la iluminación de forma automática, alertar al usuario ante condiciones anormales y permitir la configuración mediante una interfaz local.

<!-- TODO: ampliar la motivación, los desafíos técnicos y los resultados esperados. -->

## 1.2 Alcance y limitaciones

<!--
TODO: describir qué funciones quedaron dentro del alcance final y cuáles se
descartaron o pospusieron. Tomar como base la tabla de variantes de REQUISITOS.md.
Incluir, como mínimo, las decisiones sobre autonomía energética, conectividad,
múltiples macetas, calefacción y sensado de nutrientes o pH.
-->

[Completar el alcance y las limitaciones del trabajo.]

## 1.3 Descripción general del sistema

El sistema se organizó alrededor de una placa NUCLEO-F103RB conectada a sensores ambientales y del sustrato, actuadores de riego e iluminación y una interfaz local compuesta por pantalla, pulsadores e indicadores.

La figura 1.1 presenta el diagrama en bloques general definido durante la etapa de requisitos.

<p align="center">
  <img width="559" alt="Diagrama en bloques general de Smartceta" src="https://github.com/user-attachments/assets/9002c784-cc37-49ba-a3e2-2e07355b8f4d" />
  <br>
  <em>Figura 1.1: diagrama en bloques general de Smartceta.</em>
</p>

<!-- TODO: reemplazar la imagen remota por una versión definitiva almacenada en el repositorio y explicar brevemente el flujo de información y energía. -->

## 1.4 Análisis de sistemas similares

<!--
TODO: relevar productos o proyectos comparables y citar sus fuentes. Presentar
una tabla con sus principales características, ventajas y limitaciones. En este
capítulo todavía no comparar contra los resultados finales de Smartceta.
-->

| Sistema analizado | Sensores | Riego | Iluminación | Interfaz | Observaciones |
| --- | --- | --- | --- | --- | --- |
| [Producto o proyecto 1] | [Completar] | [Completar] | [Completar] | [Completar] | [Completar] |
| [Producto o proyecto 2] | [Completar] | [Completar] | [Completar] | [Completar] | [Completar] |

*Tabla 1.1: sistemas similares considerados como referencia.*

## 1.5 Justificación del enfoque técnico

<!--
TODO: justificar la solución adoptada: control local, NUCLEO-F103RB, sensores,
bomba, iluminación regulable y estrategia de alarmas. Explicar los criterios de
costo, disponibilidad, complejidad y tiempo de implementación.
-->

[Completar la justificación del enfoque técnico.]

---

# Capítulo 2: Introducción específica

## 2.1 Requisitos

Los requisitos funcionales definidos para Smartceta se resumen en la tabla 2.1.

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
| Aplicación | 4.1 | La aplicación permitirá configurar los umbrales de humedad del suelo. |
| Aplicación | 4.3 | La aplicación permitirá configurar la cantidad o intensidad de iluminación. |
| Aplicación | 4.4 | La aplicación permitirá activar o desactivar las alarmas sonoras y visuales. |
| Aplicación | 4.5 | La aplicación permitirá visualizar las lecturas de los sensores en tiempo real. |
| Aplicación | 4.6 | La aplicación permitirá ingresar a un modo de prueba de los componentes. |
| Aplicación | 4.7 | El sistema registrará y almacenará localmente valores y configuraciones básicas en memoria no volátil. |
| Alarmas | 5.1 | El sistema contará con alarmas sonoras y visuales para notificar fallas. |
| Alarmas | 5.2 | El sistema activará las alarmas cuando se detecte un nivel de agua bajo. |
| Alarmas | 5.3 | El sistema activará las alarmas cuando se detecte una falla en la bomba mediante la medición de corriente. |

*Tabla 2.1: requisitos funcionales de Smartceta.*

<!-- TODO: revisar la redacción y el cumplimiento de cada requisito contra la implementación final. Agregar requisitos no funcionales si fueron definidos. -->

## 2.2 Casos de uso

### 2.2.1 Operación normal

| Elemento | Definición |
| --- | --- |
| Disparador | El sistema se enciende. |
| Precondiciones | La Smartceta está energizada, los sensores están operativos y existen valores de configuración válidos. |
| Flujo principal | Se leen periódicamente los sensores, se muestran sus valores y se accionan el riego y la iluminación cuando corresponde. |
| Flujos alternativos | Se informa al usuario cualquier condición fuera de rango o falla detectada. |

*Tabla 2.2: caso de uso de operación normal.*

<!-- TODO: ajustar el caso de uso al comportamiento final implementado. -->

### 2.2.2 Configuración de parámetros

| Elemento | Definición |
| --- | --- |
| Disparador | El usuario ingresa al menú de configuración desde la interfaz local. |
| Precondiciones | El sistema está encendido y responde a los pulsadores. |
| Flujo principal | El usuario navega por el menú, modifica un parámetro, confirma el valor y el sistema lo almacena. |
| Flujos alternativos | El usuario cancela la modificación o el sistema vuelve al modo normal por inactividad. |

*Tabla 2.3: caso de uso de configuración de parámetros.*

<!-- TODO: documentar las teclas, los rangos configurables y los tiempos definitivos. -->

### 2.2.3 Gestión de fallas y alarmas

| Elemento | Definición |
| --- | --- |
| Disparador | El sistema detecta una condición de falla. |
| Precondiciones | El sistema está encendido y ejecuta el monitoreo de sensores y actuadores. |
| Flujo principal | Se informa la falla mediante la pantalla, el LED de estado y el buzzer; además, se bloquean las acciones que puedan dañar el hardware. |
| Flujos alternativos | El usuario reconoce la falla o se detectan varias fallas simultáneas. |

*Tabla 2.4: caso de uso de gestión de fallas y alarmas.*

<!-- TODO: identificar las fallas realmente implementadas y su mecanismo de recuperación. -->

### 2.2.4 Prueba de hardware

| Elemento | Definición |
| --- | --- |
| Disparador | El usuario selecciona el modo de prueba mediante la interfaz local. |
| Precondiciones | Los componentes que se probarán están conectados y el depósito posee agua suficiente. |
| Flujo principal | El usuario selecciona cada componente, el sistema ejecuta la prueba e informa el resultado o el valor leído. |
| Flujos alternativos | El usuario interrumpe la prueba y regresa al modo normal. |

*Tabla 2.5: caso de uso de prueba de hardware.*

<!-- TODO: ajustar la secuencia a la implementación final y enumerar los componentes incluidos. -->

## 2.3 Descripción de los módulos principales

<!--
En esta sección se describen módulos, componentes, bibliotecas o protocolos
utilizados pero no diseñados por el equipo. Las decisiones propias se desarrollan
en el capítulo 3.
-->

### 2.3.1 Unidad de control NUCLEO-F103RB

<!-- TODO: describir las características relevantes del STM32F103RB y citar su documentación. -->

[Completar la descripción de la unidad de control.]

### 2.3.2 Sensores

<!-- TODO: describir los sensores finalmente utilizados, sus rangos, interfaces, alimentación y criterios de selección. -->

- Temperatura y humedad ambiente: DHT22.
- Luz ambiente: LDR.
- Humedad del suelo: YL-69.
- Nivel de agua: módulo resistivo.
- Corriente de la bomba: [completar modelo].

### 2.3.3 Actuadores y etapas de potencia

<!-- TODO: describir la bomba, la tira LED y sus respectivos drivers. -->

[Completar la descripción de los actuadores y sus drivers.]

### 2.3.4 Interfaz local e indicadores

<!-- TODO: describir el LCD, los pulsadores, el buzzer y el LED indicador de estado. -->

[Completar la descripción de la interfaz local.]

---

# Capítulo 3: Diseño e implementación

## 3.1 Arquitectura general

<!--
TODO: presentar el diagrama detallado de hardware y firmware. Explicar el flujo
de datos, las dependencias entre módulos y las decisiones de partición. Si el
diagrama difiere del presentado en el capítulo 1, aclarar el motivo.
-->

[Completar la arquitectura general implementada.]

## 3.2 Diseño de hardware

<!--
Documentar problemas encontrados, criterios utilizados y justificación de cada
decisión. Incluir esquemas legibles, cálculos y referencias a hojas de datos.
-->

### 3.2.1 Unidad de control y asignación de periféricos

<!-- TODO: incluir reloj, ADC, temporizadores, GPIO, comunicaciones y demás periféricos utilizados. -->

[Completar la configuración de la unidad de control.]

### 3.2.2 Acondicionamiento de sensores

<!-- TODO: explicar circuitos de adaptación, filtrado, calibración y protección de cada entrada. -->

[Completar el diseño de las entradas de sensores.]

### 3.2.3 Control de la bomba y medición de corriente

<!-- TODO: incluir esquema, dimensionamiento del driver, protecciones y método de detección de fallas. -->

[Completar el diseño de la etapa de riego.]

### 3.2.4 Control de iluminación

<!-- TODO: describir el driver, la modulación PWM, los límites eléctricos y la disipación. -->

[Completar el diseño de la etapa de iluminación.]

### 3.2.5 Interfaz de usuario y alarmas

<!-- TODO: documentar las conexiones del LCD, pulsadores, buzzer y LED de estado. -->

[Completar el diseño de la interfaz de usuario.]

### 3.2.6 Alimentación y protecciones

<!-- TODO: incluir diagrama de alimentación, tensiones, corrientes previstas, márgenes y protecciones. -->

[Completar el diseño de alimentación.]

### 3.2.7 Esquemático, placa y montaje

<!-- TODO: incluir vistas del esquemático, PCB, fabricación y fotografías del montaje final. -->

[Completar la documentación del montaje.]

### 3.2.8 Pinout del sistema

| Función | Componente | Pin del componente | Pin del STM32 | Periférico | Observaciones |
| --- | --- | --- | --- | --- | --- |
| [Completar] | [Completar] | [Completar] | [Completar] | [Completar] | [Completar] |

*Tabla 3.1: asignación de pines del sistema.*

### 3.2.9 Lista de materiales

<!-- TODO: consolidar la información de hardware/BOM.txt y agregar cantidades, referencias y costos si corresponde. -->

| Cantidad | Referencia | Componente | Valor o modelo | Observaciones |
| ---: | --- | --- | --- | --- |
| [Completar] | [Completar] | [Completar] | [Completar] | [Completar] |

*Tabla 3.2: lista de materiales del prototipo.*

## 3.3 Diseño de firmware

<!--
Documentar la arquitectura, las máquinas de estado y las decisiones propias.
Incluir sólo fragmentos de código relevantes, con líneas numeradas y explicación.
Agregar un enlace permanente al repositorio en la versión final.
-->

### 3.3.1 Arquitectura de ejecución

<!-- TODO: explicar el planificador cooperativo o mecanismo de ejecución utilizado, la base de tiempo y la inicialización. -->

[Completar la arquitectura de ejecución.]

### 3.3.2 Tareas y periodicidades

| Tarea o módulo | Responsabilidad | Período o disparador | Entradas | Salidas |
| --- | --- | ---: | --- | --- |
| [Completar] | [Completar] | [Completar] | [Completar] | [Completar] |

*Tabla 3.3: tareas principales del firmware.*

### 3.3.3 Adquisición y procesamiento de sensores

<!-- TODO: describir muestreo, filtrado, conversiones, calibración, detección de valores inválidos y unidades. -->

[Completar la adquisición y el procesamiento de sensores.]

### 3.3.4 Control de riego

<!-- TODO: documentar los umbrales, la lógica de histéresis, las temporizaciones, los enclavamientos y las fallas. -->

[Completar la lógica de control de riego.]

### 3.3.5 Control de iluminación

<!-- TODO: documentar la relación entre la medición de luz, la consigna y el PWM aplicado. -->

[Completar la lógica de control de iluminación.]

### 3.3.6 Máquina de estados y modos de operación

<!-- TODO: incluir un diagrama de estados para los modos normal, configuración, falla y prueba, con eventos y condiciones de transición. -->

[Completar la máquina de estados del sistema.]

### 3.3.7 Interfaz de usuario

<!-- TODO: incluir el mapa de pantallas, la navegación, el tratamiento de pulsadores y ejemplos de mensajes. -->

[Completar el diseño de la interfaz de usuario.]

### 3.3.8 Alarmas y recuperación ante fallas

<!-- TODO: describir fallas detectables, prioridad, indicación, acciones seguras y recuperación. -->

[Completar la gestión de alarmas y fallas.]

### 3.3.9 Persistencia de configuraciones

<!-- TODO: describir qué parámetros se almacenan, dónde, con qué formato y cómo se validan al iniciar. -->

[Completar la persistencia de configuraciones.]

### 3.3.10 Diagnóstico y registro

<!-- TODO: describir la consola, los mensajes de diagnóstico y las herramientas utilizadas durante el desarrollo. -->

[Completar los mecanismos de diagnóstico.]

---

# Capítulo 4: Ensayos y resultados

<!--
Para cada ensayo indicar objetivo, instrumental, configuración, procedimiento,
criterio de aceptación, resultados, evidencia y análisis. Incluir fotografías,
tablas o gráficos. Referir cada figura o tabla antes de insertarla.
-->

## 4.1 Pruebas funcionales del hardware

| Ensayo | Procedimiento | Criterio de aceptación | Resultado | Estado | Evidencia |
| --- | --- | --- | --- | :---: | --- |
| [Completar] | [Completar] | [Completar] | [Completar] | [Pendiente] | [Figura o enlace] |

*Tabla 4.1: pruebas funcionales del hardware.*

## 4.2 Pruebas funcionales del firmware

| Ensayo | Procedimiento | Criterio de aceptación | Resultado | Estado | Evidencia |
| --- | --- | --- | --- | :---: | --- |
| [Completar] | [Completar] | [Completar] | [Completar] | [Pendiente] | [Registro o figura] |

*Tabla 4.2: pruebas funcionales del firmware.*

## 4.3 Pruebas de integración

<!-- TODO: documentar pruebas de extremo a extremo para riego, iluminación, interfaz, alarmas y modo de prueba. Incluir enlaces a videos si existen. -->

| Escenario | Módulos involucrados | Resultado esperado | Resultado obtenido | Estado |
| --- | --- | --- | --- | :---: |
| [Completar] | [Completar] | [Completar] | [Completar] | [Pendiente] |

*Tabla 4.3: pruebas de integración.*

## 4.4 Medición y análisis de consumo

<!-- TODO: indicar punto de medición, instrumentos, modos evaluados, tensión, corriente y potencia. Analizar el peor caso. -->

| Modo de operación | Tensión [V] | Corriente [mA] | Potencia [W] | Observaciones |
| --- | ---: | ---: | ---: | --- |
| [Completar] | [Completar] | [Completar] | [Completar] | [Completar] |

*Tabla 4.4: consumo medido en los modos de operación.*

## 4.5 Uso de memoria y análisis del build

<!-- TODO: incluir captura o reporte del build con uso de Flash y RAM, configuración de compilación y margen disponible. -->

| Recurso | Disponible | Utilizado | Utilización |
| --- | ---: | ---: | ---: |
| Flash | [Completar] | [Completar] | [Completar] |
| RAM | [Completar] | [Completar] | [Completar] |

*Tabla 4.5: utilización de memoria del firmware.*

## 4.6 Análisis temporal del firmware

### 4.6.1 Metodología de medición

<!-- TODO: describir cómo se midieron los tiempos de ejecución y bajo qué condiciones. -->

[Completar la metodología de medición temporal.]

### 4.6.2 Tiempo de ejecución de peor caso

| Tarea | Período $T_i$ | WCET $C_i$ | $C_i/T_i$ | Observaciones |
| --- | ---: | ---: | ---: | --- |
| [Completar] | [Completar] | [Completar] | [Completar] | [Completar] |

*Tabla 4.6: tiempo de ejecución de peor caso por tarea.*

### 4.6.3 Factor de utilización de CPU

El factor de utilización de CPU se calcula mediante:

$$
U = \sum_{i=1}^{n}\frac{C_i}{T_i}
\tag{4.1}
$$

<!-- TODO: calcular el valor con las mediciones finales, indicar la hipótesis de planificación y analizar el margen temporal. -->

[Completar el cálculo y el análisis de utilización.]

## 4.7 Gestión de bajo consumo

<!-- TODO: describir las técnicas implementadas o justificar por qué no se utilizaron. Relacionar la decisión con las mediciones de consumo. -->

[Completar el análisis de bajo consumo.]

## 4.8 Cumplimiento de requisitos

| ID | Requisito resumido | Evidencia | Estado final | Observaciones |
| :---: | --- | --- | :---: | --- |
| 0.1 | Pantalla LCD | [Completar] | [Pendiente] | [Completar] |
| 0.2 | LED de estado | [Completar] | [Pendiente] | [Completar] |
| 0.3 | Buzzer | [Completar] | [Pendiente] | [Completar] |
| 1.1 | Temperatura y humedad ambiente | [Completar] | [Pendiente] | [Completar] |
| 1.2 | Luz ambiente | [Completar] | [Pendiente] | [Completar] |
| 1.3 | Humedad del suelo | [Completar] | [Pendiente] | [Completar] |
| 1.4 | Nivel de agua | [Completar] | [Pendiente] | [Completar] |
| 1.5 | Corriente de la bomba | [Completar] | [Pendiente] | [Completar] |
| 2.1 | Bomba de riego | [Completar] | [Pendiente] | [Completar] |
| 2.2 | Iluminación LED | [Completar] | [Pendiente] | [Completar] |
| 3.0 | Pulsadores | [Completar] | [Pendiente] | [Completar] |
| 4.1 | Configuración de humedad | [Completar] | [Pendiente] | [Completar] |
| 4.3 | Configuración de iluminación | [Completar] | [Pendiente] | [Completar] |
| 4.4 | Configuración de alarmas | [Completar] | [Pendiente] | [Completar] |
| 4.5 | Visualización de sensores | [Completar] | [Pendiente] | [Completar] |
| 4.6 | Modo de prueba | [Completar] | [Pendiente] | [Completar] |
| 4.7 | Persistencia local | [Completar] | [Pendiente] | [Completar] |
| 5.1 | Notificación de fallas | [Completar] | [Pendiente] | [Completar] |
| 5.2 | Alarma por nivel de agua bajo | [Completar] | [Pendiente] | [Completar] |
| 5.3 | Alarma por falla de bomba | [Completar] | [Pendiente] | [Completar] |

*Tabla 4.7: cumplimiento final de los requisitos.*

## 4.9 Comparación con sistemas similares

<!-- TODO: retomar los sistemas presentados en la sección 1.4 y agregar Smartceta a la comparación a partir de resultados medidos. -->

| Característica | Sistema 1 | Sistema 2 | Smartceta |
| --- | --- | --- | --- |
| [Completar] | [Completar] | [Completar] | [Completar] |

*Tabla 4.8: comparación entre Smartceta y sistemas similares.*

## 4.10 Documentación del desarrollo

| Elemento | Ubicación | Estado | Observaciones |
| --- | --- | :---: | --- |
| Código fuente | `tdse-tf_3-03/` | [Completar] | [Completar] |
| Esquemático y PCB | `hardware/TDSE-TF/` | [Completar] | [Completar] |
| Lista de materiales | `hardware/BOM.txt` | [Completar] | [Completar] |
| Memoria técnica | `Memoria Tecnica/` | En elaboración | Documento actual. |
| Video de funcionamiento | [Agregar enlace] | [Completar] | [Completar] |

*Tabla 4.9: documentación asociada al desarrollo.*

---

# Capítulo 5: Conclusiones

## 5.1 Resultados obtenidos

<!-- TODO: resumir de forma breve los principales aportes y resultados comprobados. No introducir información nueva. -->

- [Completar el primer resultado obtenido.]
- [Completar el segundo resultado obtenido.]
- [Completar el tercer resultado obtenido.]

## 5.2 Lecciones aprendidas

<!-- TODO: describir aprendizajes técnicos y de gestión respaldados por el desarrollo. -->

- [Completar una lección aprendida.]
- [Completar una lección aprendida.]

## 5.3 Próximos pasos

<!-- TODO: enumerar mejoras o extensiones posibles, vinculadas con las limitaciones y los resultados. -->

- [Completar una mejora futura.]
- [Completar una mejora futura.]

---

# Capítulo 6: Uso de herramientas de inteligencia artificial

<!--
TODO: confirmar si esta sección forma parte de los requisitos de la entrega.
Documentar de forma transparente las herramientas utilizadas, por quién, con qué
objetivo y cómo se verificaron o modificaron sus resultados. Eliminar el capítulo
si no corresponde y actualizar el índice.
-->

| Integrante | Herramienta | Uso realizado | Verificación o aporte propio |
| --- | --- | --- | --- |
| [Completar] | [Completar] | [Completar] | [Completar] |

*Tabla 6.1: uso de herramientas de inteligencia artificial durante el trabajo.*

---

# Bibliografía y referencias

<!--
TODO: utilizar un formato uniforme, preferentemente IEEE. Incluir hojas de datos,
manuales, notas de aplicación, productos comparables y repositorio. Toda fuente
debe ser citada desde el cuerpo del documento.
-->

[1] STMicroelectronics, *STM32F103x8/STM32F103xB datasheet*, [versión y fecha].

[2] STMicroelectronics, *NUCLEO-F103RB user manual*, [versión y fecha].

[3] [Fabricante], *DHT22 datasheet*, [versión y fecha].

[4] [Completar las referencias restantes].

---

# Anexos (opcional)

<!--
Usar anexos sólo para información necesaria que interrumpiría la lectura del
cuerpo principal: resultados extensos, planos completos, protocolos de ensayo o
fragmentos de código complementarios. Eliminar esta sección si no se utiliza.
-->

## Anexo A: [Título]

[Completar el anexo o eliminar esta sección.]

---

**Fin de la memoria técnica**
