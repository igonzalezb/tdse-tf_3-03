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

Las características que se barajaron fueron:





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


### Diagrama en bloques

<img width="559" height="462" alt="Diagrama_en_bloques" src="https://github.com/user-attachments/assets/9002c784-cc37-49ba-a3e2-2e07355b8f4d" />


## Requisitos y modos de uso

### Requisitos
- Mostrar en pantalla las lecturas de sensores en tiempo real.  
- Registrar y almacenar localmente (EEPROM/Flash) valores y configuraciones básicas.  
- Activar bomba de riego cuando humedad del suelo < umbral configurado y nivel de agua OK.  
- Encender iluminación artificial cuando intensidad de luz < umbral y según configuración.  
- Permitir configurar umbrales de humedad, horarios de riego/iluminación y activar/desactivar alarmas.  
- Detectar y notificar fallas (bomba sin agua, sensor desconectado, temperatura extrema).  
- Modo Test para activar manualmente cada componente y mostrar estado.

### Componente hardwaree (tentativos)
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


### Modos de operación:

- **Normal (flujo principal)**: muestra en pantalla los valores de los diferentes sensores, se puede ir cambiando lo que muestra con los botones. El sistema actúa dependiendo de las mediciones de los sensores.
- **Ajustes y configuraciones**: menú en el cual se pueden configurar y modificar los valores de humedad deseada, activar/desactivar alarmas, cantidad de luz externa deseada.
- **Falla o errores**: ante una falla o error, el sistema muestra un mensaje en pantalla indicando el problema, y en algunos casos detiene el funcionamiento para evitar daños. Además, se enciende una luz indicadora de error y se activa una alarma sonora.
- **Modo Test**: en este modo se pueden probar los diferentes componentes del sistema, como los sensores, bombas y luces.
