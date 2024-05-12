# Análisis de flujo y congestion en redes utilizando simulación discreta y un protocolo.

## Resumen:

En este trabajo se analiza el tráfico de red bajo tasas de datos acotadas y tamaño de buffers limitados.
Con el fin de diseñar y proponer soluciones de control de congestión y flujo sobre modelos de red, utilizando simulación discreta con la herramienta Omnet++, bajo el lenguaje c++.
Para ello diseñamos un protocolo de control de flujo y congestión inspirándonos en la sencillez para mantener la eficiencia de la red y la complejidad algorítmica.
Obtenemos conclusiones mediante gráficos y el modelo de red en base a los datos obtenidos de la simulación.

## Introducción:

<!--
- Definir el problema y contextualizar al lector con definiciones básicas.
    + Ej: "Nosotros en las redes vamos a encontrar tal y tal problema ...
           , el flujo esta tal cosa, la congestion tal otra ..."
-->

Trabajamos con redes, las cuales son una abstracción de nodos interconectados e intercomunicados a traves de sus conexiones, utilizadas para transmitir datos entre diferentes puntos de la misma. Donde los datos se transmiten en paquetes.

Las redes están formadas por distintos tipos de nodos que:

- Generan paquetes (generadores).
- Retransmiten/procesan paquetes.
- Consumen paquetes (sumidero).

![Red Básica](/IMGs/red-basica.png)

El principal objetivo de una red es transmitir paquetes de un punto a otro. Pero al analizar distintos casos de estudio, podemos darnos cuenta de un problema, se pierden paquetes y nunca llegan a destino.

Los enlaces que conectan nodos tienen velocidades limitadas, donde algunos tienen tiempos de procesamiento por paquete por lo que necesitan buffers de algún tamaño para guardarlos y no perderlos. Sumado a esto, el nodo generador también tiene una tasa de generación de paquetes.
Estas variables son las que al cambiar pueden producir que se pierdan paquetes. Podemos subdividir este problema cuando no controlas las siguientes partes:

- **Congestion**: Cuando hay demasiados paquetes presentes en una red (o en una parte de ella) para la capacidad de los buffers generando que se pierdan algunos.

- **Flujo**: Se da cuando el emisor envía paquetes a una velocidad mayor a la que el receptor puede aceptarlos, generando una saturación en el receptor, lo cual puede provocar nuevamente pérdida de paquetes.

Para lograrlo, diseñaremos un protocolo. Los protocolos son conjuntos de reglas de comunicación, que en nuestro caso permitirán controlar el flujo y la congestión en la red para evitar la perdida de paquetes.

<!--
Describir el estado del arte. (trabajos previos) + Ej: "En la actualidad se han propuesto varios protocolos para controlar la congestion y flujo en redes, como TCP, ..."
-->

En la actualidad se han propuesto varios protocolos para controlar la congestion y flujo en redes. Por ejemplo, TCP el cual garantiza que el receptor reciba todos los paquetes enviados desde el emisor, pero para lograrlo permite perdida de paquetes, ¿como soluciona esto? revisándolos.
Algunos intentan solucionar este problema evitando que se pierdan de ninguna forma, esto lo hacen con paquetes de control para no saturar ningún buffer de la red.
Otros implementan una mezcla de ambos, para garantizar la entrega de paquetes y evitar la saturación de buffers.

<!--
- Metodologia de trabajo.
    + Ej: "Nosotros vamos a trabajar con simulacion discreta, que es ...
           y emplearems un protocolo que vamos a proponer, para analizar el flujo y congestion en redes ..."
-->

Para trabajar sobre redes utilizamos el simulador de eventos discretos **(Omnet++)**. Elegimos esta herramienta ya que nos permite previsualizar la red y el efecto del diseño de nuestros protocolos en ella, cosa que no se podría ver con la misma facilidad en la vida real sobre una red física existente. Por ejemplo, nos permite controlar los tiempos y demás variables de la red.

<!--
- Presentación de nuestros casos de estudio.
   + Explicar caso 1: su ventaja, problemas, etc.
   + Explicar caso 2: su ventaja, problemas, etc.
   + Obs: Incluir las primeras graficas de la parte 1, con las conculciones y problemas que encontramos osea interpretarlas (ej, aca podemos ver que los paquetes, buffers, paquetes enviado, tal y tal cosa ...)
-->

Para empezar a ver los problemas de congestión y el control de flujo así como también ver la manera en la que nuestro protocolo funciona, tenemos dos casos de estudios para poder analizar más en detalle las cosas explicadas anteriormente.

En los diferentes casos de estudio van a cambiar las tasas de transferencia de datos en los distintos enlaces generando el fenómeno **cuello de botella**. Es decir que nos encontramos un enlace con menor tasa de transferencia que el resto de los enlaces de la red, lo que genera que se pierdan paquetes.

Para ello vemos algunas componentes y los enlaces existentes en nuestra red (abstracta) de estudio:

- **NodeTx**: Modulo que genera paquetes, compuesto de:

  - **Generador**: Genera paquetes.
  - **Queue**: Buffer que almacena paquetes. (tamaño 200pkt)

- **NodeRx**: Modulo que recibe paquetes, compuesto de:

  - **Queue**: Buffer que almacena paquetes. (tamaño 200pkt)
  - **Sink**: Consumidor de paquetes.

- **Queue**: Nodo intermedio que recibe paquetes, los almacena en un buffer (tamaño 200pkt), procesa y reenvia.

Luego tenemos los siguientes enlaces que conectan los nodos en la red:

![Red parte 1](/IMGs/red-parte1.png)

Pero internamente **NodeTx** y **NodeRx** tienen los siguientes enlaces:

![NodeTx parte 1](/IMGs/NodeTx-parte1.png)

![NodeRx parte 1](/IMGs/NodeRx-parte1.png)
Los casos de estudio son:

- **Caso 1**:
  - NodeTx a Queue: datarate = 1 Mbps y delay = 100 us
  - Queue a NodeRx: datarate = 1 Mbps y delay = 100 us
  - NodeRx.queue a Sink: datarate = 0.5 Mbps
- **Caso 2**:
  - NodeTx a Queue: datarate = 1 Mbps y delay = 100 us
  - Queue a NodeRx: datarate = 0.5 Mbps y delay = 100 us
  - NodeRx.queue a Sink: datarate = 1 Mbps

Ejecutamos la simulación durante `200s` para cada caso y obtuvimos las siguientes conclusiones:

**Caso de estudio 1**

En la siguiente gráfica podemos ver como se llenan los buffers a lo largo del tiempo:

![Ocupación de buffers parte 1 - caso 1](/GRAFICAS/buffers-parte1-caso1.png)

Analizando, podemos notar tres cosas:

- El buffer de **NodeTx** varia mucho a lo largo del tiempo debido a que el generador de paquetes envía paquetes a una tasa definida por `exponential(10ms)` esto es mas rápido de lo que su **queue** puede enviarlos. Notar que nunca supera los `200` paquetes por lo tanto no descarta paquetes por falta de espacio.
- El buffer **intermedio Nx** se mantiene constantemente en `1` debido a que recibe paquetes a la misma tasa que los envía. (i.e `1Mbps`). Nuevamente no descarta paquetes por falta de espacio.
- El buffer de **NodeRx** es particular, ya que podemos ver como su buffer va creciendo de forma linea y luego se mantiene constante en `200` paquetes, esto pasa porque empezó a descartar paquetes por falta de espacio, debido a que recibe paquetes a una tasa de `1Mbps` pero solo puede enviarlos a `0.5Mbps` generando un **cuello de botella** en el enlace entre **NodeRx.queue** y **Sink**.

Veamos ahora cuando y donde se descartan paquetes:

![Paquetes descartados parte 1 - caso 1](/GRAFICAS/pkt-descartados-parte1-caso1.png)

En la gráfica podemos ver claramente como la única parte donde se descartan paquetes es en el buffer de **NodeRx.queue**. Esto se debe justamente al **cuello de botella** que mencionamos anteriormente. Notar que el tiempo en el que se descartan paquetes es a partir de los `40s` aproximadamente y coincide con el momento en el que el buffer de **NodeRx** empieza a mantenerse constante en `200` paquetes.

**Caso de estudio 2**

En la siguiente gráfica podemos ver como se llenan los buffers a lo largo del tiempo:

![Ocupación de buffers parte 1 - caso 2](/GRAFICAS/buffers-parte1-caso2.png)

Analizando, podemos compáralo con el caso anterior y notar que:

- El buffer de **NodeTx** se mantiene igual que en el caso anterior debido a que la tasa de generación de paquetes es la misma y la tasa de envío también.
- El buffer **intermedio Nx** copia el comportamiento de Rx en el anterior caso debido a que la tasa de transferencia entre Nx.queue y Rx.queue es de `0.5Mbps` mientras que los paquetes llegan a Nx.queue con una tasa de `1Mbps` generando un **cuello de botella**.
- En el buffer **NodeRx** el comportamiento es similar al de Nx en el anterior caso, debido a que ahora recibe paquetes a una tasa de `0.5Mbps` menor que la tasa `1Mbps` de salida, evitando el cuello de botella.

Veamos ahora cuando y donde se descartan paquetes:

![Paquetes descartados parte 1 - caso 2](/GRAFICAS/pkt-descartados-parte1-caso2.png)

Claramente se ve como la única queue que descarta paquetes es la de **NodeNx** debido a lo mencionado anteriormente. Notar que ocurre lo mismo que en el caso anterior, a partir de los `40s` aproximadamente.

<!--
En el enunciado dice que hay que contestar las siguientes preguntas de la PARTE DE TAREA ANÁLISIS:
- ¿Qué diferencia observa entre el caso de estudio 1 y 2?
- ¿Cuál es la fuente limitante en cada uno?
- Investigue sobre la diferencia entre control de flujo y control de congestión (ver Figura 6-22 del libro Tanenbaum).
-->

## Métodos:

UMBRAL && ESPERA

<!--
Una sección que describir nuestra propuesta de solución:
- Describimos el algoritmo.
- Como llegamos a esa idea.
- Una pequeña hipotesis de porque creemos que va a funcionar.
-->

## Resultados:

<!--
- Una seccion que detalla los resultados de los algoritmos diseñados con graficos tanto para el caso 1 y caso 2.
- Concluciones de los resultados.

Opcional: Si no llegamos directamente al algoritmo, se puede incluir algunas modificacion que le fuimos haciendo al algoritmo para que funcionen mejor. (Es decir algunas variables globales, parametros, que no cambia tanto codigo, etc.)
Importante: Implementar todo el algoritmo de nuevo no se considera modificacion,  eso se ecribe en otro paper.

(Aca podemos agregar el caso 3 nuestro y explicar porque esta bueno, etc.)


Agregar graficos de carga ofrecida vs carga util explicada en el video:
https://www.youtube.com/watch?v=W8r8zSPjeAs&feature=youtu.be

El enunciado dice que hay que contestar las siguientes preguntas de la PARTE TAREA DE DISEÑO:
- ¿Cómo cree que se comporta su algoritmo de control de flujo y congestión?
- ¿Funciona para el caso de estudio 1 y 2 por igual? ¿Por qué?
-->

## Discusiones:

<!--
Una seccion con los logros, limitaiones y posibles mejoras de nuestro algoritmo propuesto.
-->

## Referencias:

<!--
- Todas las referencias que usamos en el trabajo. LIBROS, PAPERS, WEB, ETC.
(Nosotros usamos el manual de omnet++ y  quizas algo más ...).

Si agregamos imagenes de tanembaun para explicar algo de flujo y congestion, tambien se debe referenciar.
-->
