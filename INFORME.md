# Análisis de flujo y congestion en redes utilizando simulación discreta y un protocolo.

---

## Resumen:

En este trabajo se analiza el tráfico de red bajo tasas de datos acotadas y tamaño de buffers limitados.
Con el fin de diseñar y proponer soluciones de control de congestión y flujo sobre modelos de red, utilizando simulación discreta con la herramienta Omnet++, bajo el lenguaje c++.
Para ello diseñamos un protocolo de control de flujo y congestión inspirándonos en la sencillez para mantener la eficiencia de la red y la complejidad algorítmica.
Obtenemos conclusiones mediante gráficos y el modelo de red en base a los datos obtenidos de la simulación.

---

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

![Red Básica](./IMGs/red-basica.png){width=250 height=auto}

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
- Metodología de trabajo.
    + Ej: "Nosotros vamos a trabajar con simulación discreta, que es ...
           y emplearemos un protocolo que vamos a proponer, para analizar el flujo y congestion en redes ..."
-->

Para trabajar sobre redes utilizamos el simulador de eventos discretos **(Omnet++)**. Elegimos esta herramienta ya que nos permite previsualizar la red y el efecto del diseño de nuestros protocolos en ella, cosa que no se podría ver con la misma facilidad en la vida real sobre una red física existente. Por ejemplo, nos permite controlar los tiempos y demás variables de la red.

<!--
- Presentación de nuestros casos de estudio.
   + Explicar caso 1: su ventaja, problemas, etc.
   + Explicar caso 2: su ventaja, problemas, etc.
   + Obs: Incluir las primeras gráficas de la parte 1, con las conclusiones y problemas que encontramos osea interpretarlas (ej, aca podemos ver que los paquetes, buffers, paquetes enviado, tal y tal cosa ...)
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

![Red parte 1](./IMGs/red-parte1.png){width=250 height=auto}

Pero internamente **NodeTx** y **NodeRx** tienen los siguientes enlaces:

![NodeTx parte 1](./IMGs/NodeTx-parte1.png){width=auto height=250} ![NodeRx parte 1](./IMGs/NodeRx-parte1.png){width=auto height=250}

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

![Ocupación de buffers parte 1 - caso 1](./GRAFICAS/buffers-parte1-caso1.png){width=auto height=500}

Analizando, podemos notar tres cosas:

- El buffer de **NodeTx** varia mucho a lo largo del tiempo debido a que el generador de paquetes envía paquetes a una tasa definida por `exponential(10ms)` esto es mas rápido de lo que su **queue** puede enviarlos. Notar que nunca supera los `200` paquetes por lo tanto no descarta paquetes por falta de espacio.
- El buffer **intermedio Nx** se mantiene constantemente en `1` debido a que recibe paquetes a la misma tasa que los envía. (i.e `1Mbps`). Nuevamente no descarta paquetes por falta de espacio.
- El buffer de **NodeRx** es particular, ya que podemos ver como su buffer va creciendo de forma linea y luego se mantiene constante en `200` paquetes, esto pasa porque empezó a descartar paquetes por falta de espacio, debido a que recibe paquetes a una tasa de `1Mbps` pero solo puede enviarlos a `0.5Mbps` generando un **cuello de botella** en el enlace entre **NodeRx.queue** y **Sink**.

Veamos ahora cuando y donde se descartan paquetes:

![Paquetes descartados parte 1 - caso 1](./GRAFICAS/pkt-descartados-parte1-caso1.png){width=auto height=320}

En la gráfica podemos ver claramente como la única parte donde se descartan paquetes es en el buffer de **NodeRx.queue**. Esto se debe justamente al **cuello de botella** que mencionamos anteriormente. Notar que el tiempo en el que se descartan paquetes es a partir de los `40s` aproximadamente y coincide con el momento en el que el buffer de **NodeRx** empieza a mantenerse constante en `200` paquetes.

**Caso de estudio 2**

En la siguiente gráfica podemos ver como se llenan los buffers a lo largo del tiempo:

![Ocupación de buffers parte 1 - caso 2](./GRAFICAS/buffers-parte1-caso2.png){width=auto height=500}

Analizando, podemos compáralo con el caso anterior y notar que:

- El buffer de **NodeTx** se mantiene igual que en el caso anterior debido a que la tasa de generación de paquetes es la misma y la tasa de envío también.
- El buffer **intermedio Nx** copia el comportamiento de Rx en el anterior caso debido a que la tasa de transferencia entre Nx.queue y Rx.queue es de `0.5Mbps` mientras que los paquetes llegan a Nx.queue con una tasa de `1Mbps` generando un **cuello de botella**.
- En el buffer **NodeRx** el comportamiento es similar al de Nx en el anterior caso, debido a que ahora recibe paquetes a una tasa de `0.5Mbps` menor que la tasa `1Mbps` de salida, evitando el cuello de botella.

Veamos ahora cuando y donde se descartan paquetes:

![Paquetes descartados parte 1 - caso 2](./GRAFICAS/pkt-descartados-parte1-caso2.png){width=auto height=320}

Claramente se ve como la única queue que descarta paquetes es la de **NodeNx** debido a lo mencionado anteriormente. Notar que ocurre lo mismo que en el caso anterior, a partir de los `40s` aproximadamente.

<!--
En el enunciado dice que hay que contestar las siguientes preguntas de la PARTE DE TAREA ANÁLISIS:
- ¿Qué diferencia observa entre el caso de estudio 1 y 2?
- ¿Cuál es la fuente limitante en cada uno?
- Investigue sobre la diferencia entre control de flujo y control de congestión (ver Figura 6-22 del libro Tanenbaum).
-->

El control de flujo y el control de congestión son dos conceptos fundamentales en redes de computadoras que se relacionan con la administración del tráfico de datos, pero tienen objetivos y enfoques diferentes, veamos una imagen representativa de esto y analicemos la diferencia entre ambos conceptos:

![Control de flujo y congestión](./IMGs/congestion-y-flujo.jpeg){width=auto height=400}

**Control de flujo:**

- El control de flujo se refiere a la gestión de la velocidad de transferencia de datos entre dos puntos en una red para garantizar que el receptor no se sature con más datos de los que puede manejar.
- Se ocupa de asegurar que el emisor no envíe más datos de los que el receptor puede procesar o almacenar temporalmente en su búfer.
- El objetivo principal del control de flujo es garantizar la eficiencia y la fiabilidad en la transmisión de datos, evitando la congestión en la red.

**Control de congestión:**

- El control de congestión se refiere a la gestión del tráfico de datos en una red para evitar la saturación de los recursos de la red y la degradación del rendimiento.
- Se ocupa de evitar que se produzcan situaciones en las que la capacidad de la red se vea superada por la cantidad de datos que se están transmitiendo, lo que podría llevar a la pérdida de paquetes, la retransmisión excesiva y la congestión en la red.
- El objetivo principal del control de congestión es garantizar un uso eficiente de los recursos de red y mantener un rendimiento óptimo, evitando la saturación y el colapso de la red.

En resumen, mientras que el control de flujo se centra en la relación entre el emisor y el receptor para evitar que el receptor se sobrecargue, el control de congestión se centra en la gestión del tráfico de datos en toda la red para evitar la saturación de los recursos de la red.

---

## Métodos:

<!--
Una sección que describir nuestra propuesta de solución:
- Describimos el algoritmo.
- Como llegamos a esa idea.
- Una pequeña hipótesis de porque creemos que va a funcionar.
-->

A continuación presentamos algunas modificación que se hicieron en la red:

![Red parte 2](./IMGs/red-parte2.png){width=350 height=auto}

Notar que ahora tenemos dos **queue** intermedios, pero uno es unicamente para manejar paquetes de control, que serán enviados por el protocolo que diseñamos.

Por otra parte también se modificaron los enlaces internos de **NodeTx** y **NodeRx**:

El funcionamiento de **traTx** es similar que al de la **queue** nada mas que ahora no solo envía paquetes a **queue0** sino que también puede recibir paquetes de **queue1** y procesarlos.

![NodeTx parte 2](./IMGs/NodeTx-parte2.png){width=auto height=250}

El funcionamiento de **traRx** es similar que al de la **queue** nada mas que ahora no solo recibe paquetes de **queue0** sino que también puede enviar paquetes a **queue1** de control.

![NodeRx parte 2](./IMGs/NodeRx-parte2.png){width=auto height=250}

### Protocolo diseñado: **UMBRAL && ESPERA**

Nuestro protocolo recibe como nombre **UMBRAL && ESPERA** ya que usamos cotas en los buffers para evitar la pérdida de paquetes y creamos un tipo de paquetes para el control encargado de avisar al emisor que deje de enviar paquetes.

El algoritmo hace lo siguiente, dependiendo los casos de estudio, tenemos cotas para los buffers de `80%` y `50%` del total de almacenamiento del mismo, para nuestro caso los buffer tienen un almacenamiento de `200` paquetes, Por lo tanto el `80% * Almacenamiento(200) = cota1` es decir que tenemos una cota en `160` paquetes, análogamente para la cota de `50%`.

El funcionamiento se da a continuación:

- Se envían los paquetes normalmente hacia los nodos, en nuestro caso enviamos paquetes desde el nodo **Node.traTx** hacia **queue0**.

Notar que en nuestra red podemos tener dos situaciones de cuello de botella:

- Que se sature el buffer **queue0**.
- Que se sature el buffer de **NodeRx.traRx**.

Para ello nuestro protocolo intenta detectar antes que se sature alguno de ellos (o ambos), esto lo logramos revisando que no se supere la cota.

Antes de empezar a explicar el procedimiento del protocolo, vamos a definir el comportamiento adicional de **NodeRx**. Este se encarga de avisar por la ruta **NodeRx.queue ---> queue1 ---> NodeTx.queue** el estado actual de los buffers en la red. Para ello enviamos mensajes/paquetes de control sucesivamente informado con un booleano si algún buffer supera alguna cota. Consiguiendo de esta manera que el NodeTx siempre sepa el estado de la red para controla la transmisión de paquetes, evitando la saturación de los buffers en la red.

#### Caso 1: Que se sature el buffer queue0.

- Supongamos ahora que la tasa de transferencia de **queue0** ----> **NodeTx.traRx** es menor que la tasa de transferencia **NodeTx.traTx** ----> **queue0** que en general es uno de nuestro caso de estudio, en este caso se produce un cuello de botella con lo cual los paquete que van llegado a **queue0** irán almacenándose continuadamente.
- Irremediablemente si continuamos enviando paquetes, el buffer **queue0** se irá llenando gradualmente.
- Cuando se supere la cota, nuestro protocolo detectará esto, por lo cual encolamos un paquete creado por nosotros en la primera posición de la cola del **queue0**, que será enviado inmediatamente a **NodeRx** que este a su vez tiene un mecanismo que identifica este paquete como importante.
- Cuando lo detecta enviamos un paquete (indicando que se debe parar la transmisión) hacia el buffer **queue1** que éste a su vez envía el paquete a **NodeTx** indicando que se dejen de enviar paquetes para evitar la saturación de los buffers.
- En nuestro algoritmo, **NodeRx** envía constantemente paquetes a **queue1**, esto paquetes son enviados a **NodeTx** indicando que siga transmitiendo.
- Mientras **NodeTx** no reciba un paquete que indique que deba parar la transmisión, éste seguirá retransmitiendo.
- Una vez que **NodeTx** paró de transmitir porque recibió un paquete indicando que debía para, este seguirá sin retransmitir hasta que le indiquen lo contrario.
- Cuando los buffers dejen de superar la cota establecida, el último buffer que dejó de superar la cota, enviará un paquete hacia **NodeTx** indicando que reanude la retransmisión.
- Para el caso 1, el buffer **queue0** será el responsable de enviar el paquete para que se deba reanudar la transmisión.

#### Caso 2: Que se sature el buffer de NodeRx.traRx.

- Supongamos ahora que la congestión se produce en el buffer interno de **NodeRx** es decir en **NodeRx.traRx**, en este caso se produce un cuello de botella con lo cual los paquete que van llegando al buffer interno de **NodeRx** irán almacenándose continuadamente.
- Al igual que en el caso 1, si continuamos enviado paquetes, el buffer interno se irá llenando gradualmente.
- Cuando se supere la cota del buffer interno, nuestro protocolo detectará esto, por lo cual se enviará un paquete desde **NodeRx** hacia **queue1** para que este lo envía a **NodeTx** indicando que se pare la transmisión.
- Análogamente a lo anterior **NodeRx** envía constantemente paquetes a **queue1**, esto paquetes son enviados a **NodeTx** indicando que siga transmitiendo.
- Mientras no se supere la cota del buffer interno de **NodeRx**, éste seguirá enviando mensajes a **queue1** para que siga la transmisión de paquetes.
- Una vez que el buffer interno deje de superar la cota, se enviará un paquete a **queue1** hacia **NodeTx** indicando la reanudación de la transmisión.
- Para el caso 2, el buffer interno de **NodeRx** será el responsable de enviar el paquete para que se deba reanudar la transmisión.

Entonces en resumidas cuentas, nuestro protocolo detecta cuando los buffers superan las cotas establecidas enviando un mensaje al emisor para que deje de enviar paquetes con lo cual los paquetes del emisor irán almacenándose en su buffer interno, cuando se detecte que todos los buffers ya no superan las cotas, entonces se envía nuevamente un paquete al emisor avisándole que puede restablecer el envió de los paquetes y así sucesivamente.
Lo curioso de esto es que mientras vaya transcurriendo el tiempo de la simulación, nuestro protocolo se establecerá en un valor fijo de paquete que se envía y paquetes almacenados en los buffers esto lo visualizaremos en la parte de resultados.

### Como llegamos a las ideas para la implementación de nuestro protocolo.

- En principio nos dimos cuenta que una manera de evitar la saturación de los buffers era justamente usar cotas que restringieran de alguna manera este problema.
- Una vez que establecimos cotas como principio constructor para nuestro algoritmo, lo siguiente fue pensar en relacionar los nodos del network para mantener información sobre los estados de los buffers y cómo poder controlar este problema.
- En base a estudiar los problemas mencionados anteriormente y el análisis de lo que teníamos en mente, llegamos a la idea de implementar mensajes de `ESPERA` al emisor para evitar seguir congestionando la red.

### Hipótesis de por qué creemos que va a funcionar.

- Tenemos cotas en lo buffers para evitar la saturación de los mismos.
- Cuando detectamos que algunos de los buffers supera su cota establecida, enviamos un mensaje/paquete al emisor para que detenga la transmisión.
- Una vez que los buffers dejan de superar su cota establecida, se envía un mensaje/paquete al emisor para que se reanude la transmisión de los paquetes.
- Al tener pocos paquetes de control no influye tanto en la utilidad de la red.

Todo esto evitaría que se pierdan paquetes por saturación de buffers

---

## Resultados:

<!--
- Una sección que detalla los resultados de los algoritmos diseñados con gráficos tanto para el caso 1 y caso 2.
- Conclusiones de los resultados.

Opcional: Si no llegamos directamente al algoritmo, se puede incluir algunas modificación que le fuimos haciendo al algoritmo para que funcionen mejor. (Es decir algunas variables globales, parámetros, que no cambia tanto código, etc.)
Importante: Implementar todo el algoritmo de nuevo no se considera modificación,  eso se escribe en otro paper.

(Aca podemos agregar el caso 3 nuestro y explicar porque esta bueno, etc.)


Agregar gráficos de carga ofrecida vs carga util explicada en el video:
https://www.youtube.com/watch?v=W8r8zSPjeAs&feature=youtu.be

El enunciado dice que hay que contestar las siguientes preguntas de la PARTE TAREA DE DISEÑO:
- ¿Cómo cree que se comporta su algoritmo de control de flujo y congestión?
- ¿Funciona para el caso de estudio 1 y 2 por igual? ¿Por qué?
-->

En esta parte explicaremos y observaremos mediante gráficos los datos obtenidos de nuestra simulación con el protocolo `UMBRAL && ESPERA`. Conservando los casos de estudios 1 y 2 anteriores, para poder comparar los distintos resultados usando nuestro protocolo en los distintos casos de estudio.

Para crear las gráficas de todo el informe que representen los diferentes resultados, usamos Python.

Como hicimos en la introducción, empecemos analizando las capacidades de los buffers para poder observar la pérdida de paquetes, lo cuál es el objetivo.
A continuación les mostramos las gráficas de los dos casos de estudio en donde podemos ver el funcionamiento de nuestro protocolo.

![Ocupación de buffers parte 2 - caso 1](./GRAFICAS/buffers-parte2-caso1.png){width=auto height=500} ![Ocupación de buffers parte 2 - caso 2](./GRAFICAS/buffers-parte2-caso2.png){width=auto height=500}

Comportamiento de los distintos componentes:

- **Tx -> (NodeTx)**: Podemos observar que en el buffer interno de **Tx**, a diferencia de no tener el protocolo (parte 1), como éste se irá llenando aproximadamente lineal a partir de cierto tiempo (más adelante lo analizamos), y podemos ver que el crecimiento no cesará, pero el buffer tiene suficiente tamaño para almacenar todos los paquetes que no son enviados, por lo cual no se descartarán paquetes.
  Notemos que el comportamiento del `NodeTx` es similar en ambos casos de estudio.
- **Nx -> (queue0)**: En esta parte podemos ver una diferencia entre los distintos casos. A primera vista podemos observar que en el **caso 1**, el almacenamiento del buffer se mantiene constante en `1`, representando que el buffer a medida que van llegando los paquetes los envía inmediatamente, logrando así un buen comportamiento de nuestro protocolo.
  Por otra parte, en el **caso 2** podemos ver como el almacenamiento aumenta linealmente hasta alcanzar la cota en un determinado tiempo. Al llegar a la cota, nuestro protocolo se encargaría de avisar al emisor que deje de enviar paquetes, para evitar la saturación del mismo. De esta forma vemos como se desatura (en relación a la cota), porque nuestro protocolo avisará al emisor que reanude la transmisión, y siguiendo este proceso iterativamente a lo largo de toda la simulación vemos como se estabiliza el almacenamiento del buffer cercano a la cota, produciendo así un efecto serrucho como se ve en la gráfica. Consiguiendo que en ninguno de los dos casos perdamos paquetes.
- **Rx -> (NodeRx)**: En el buffer interno de **Rx** podemos ver nuevamente entre ambas gráficas que difieren según el caso. Notar algo particular en relación al comportamiento del buffer **Nx -> (queue0)**, éste en el **caso 1** se comporta similarmente (casi igual) al Rx del **caso 2**. Análogamente el **Rx** del **caso 1** se comporta similarmente al **Nx** del **caso 2**. Es decir, en otras palabras se comportan "igual" pero intercambiados.  
  El por qué pasa esto es debido a las distintas tasas de transferencia en los casos de estudio. Veamos en profundidad:
  - El cuello de botella en el **caso 1** se encuentra en el enlace interno del nodo **Rx**, por ello este buffer interno será el primero en llegar a la cota.
  - El cuello de botella en el **caso 2** se encuentra en el enlace desde **Nx -> queue0** hacia el nodo **Rx -> (NodeRx)**, de esta forma el buffer **queue0** llegará primero a la cota.

Cabe destacar que si observamos muy detenidamente los gráficos, en resumidas cuenta, estos explican detalladamente el comportamiento de nuestro protocolo. Es decir, que las distintas partes de los gráfico están diciendo algo muy importante sobre cómo es que funciona el protocolo. Por ejemplo como mencionamos antes, la parte del gráfico del nodo **Nx** del segundo caso de estudio después de un determinado tiempo se establece la cota del buffer **Nx** en un almacenamiento de aproximadamente 160 paquetes y luego se mantiene el almacenamiento en un rango de **`150--170`** paquetes durante toda la simulación restante. Esto sucede debido a que nuestro protocolo cada vez que el buffer supera su cota establecida lo detecta y le dice al emisor que pare, una vez que el buffer deja de superar su cota, el emisor comienza nuevamente a retransmitir.

Otro punto importante a mencionar es la parte del gráfico del nodo **Tx caso 1** que a partir de los **`25seg`** el crecimiento del almacenamiento del buffer es lineal. Entonces observando detenidamente el gráfico de **Nx caso 1** vemos que justamente el almacenamiento del buffer se estabiliza justo a los **`25seg`**, este comportamiento no es casualidad, ya que se produce cuando el buffer supera su cota establecida, que para el **caso 1** la cota del buffer es de **`100`** paquetes, por lo cual una vez que se supere la cota, el nodo **Tx** dejará de enviar paquetes hacia el nodo **Nx** almacenando dichos paquetes en su buffer interno, que es justamente esto lo que podemos ver en las gráficas. El crecimiento del almacenamiento del buffer es lineal a partir de la primera saturación **(se pasa de la cota establecida)** del buffer.

Como vimos en la introducción, el principal problema que tenia la red es que los buffers llegaban a su máxima capacidad y se empezaban a descartar paquetes. Veamos que pasa con esos buffers con el protocolo funcionando con una gráfica:

<!--
FIXME: Actualizar el gráfico usando los nombres mejorados
-->

![paquetes salientes del sistema](./GRAFICAS/Pks-Salientes-P2-C1.png){width=auto height=250}

Como podemos ver tenemos cuatro variables, las analicemos:

- Pkt Consumidos: Esta representa los paquetes que efectivamente llegaron a su destino, es decir que llegaron a Rx.Sink correctamente.
- Pkt Descartados: Ninguno de los buffers (Rx.Queue, Tx.Queue y Nx.Queue0) descarta paquetes debido a que nuestro protocolo evita la saturación de los buffers.

Concluimos de ésta gráfica que no se descartan paquetes en ningún momento de la simulación, pero esto no significa que se dejen de generar. Como en nuestros casos de estudio se consumen los paquetes a un menor ritmo, éstos se almacenarán en algún buffer de la red, que se llenará sin saturarse, como anteriormente lo vimos en las gráficas que representaban la ocupación de los buffers con el protocolo funcionando, específicamente sucede en el **NodoTx.Queue**. Veamos que está pasando con estos paquetes.

<!-- Entonces, al generarse paquetes mas rápido de los que se consumen, se guardan en algún buffer dentro de la red. Por lo tanto cada vez habra mas paquetes dentro del sistema, y eso lo podemos ver en la siguiente grafica:
![cantidad de paquetes generados vs los paquetes en el sistema](./GRAFICAS/Pks-Dentro-P1-C1.png){width=auto height=250}

Como podemos ver cada vez hay mas paquetes dentro del sistema. Esto significa que cada vez mas, los paquetes generados tardaran mas en consumirse debido al aumento de la cola de paquetes en los buffers. Al tiempo que tarda un paquete entre que se genera y se consume lo llamamos delay. y como podemos ver en la siguiente grafica, aumenta de forma proporcional.
![Delay parte 1 - caso 1](./GRAFICAS/Delay-P1-C1.png){width=auto height=250} -->

Primero, introducimos el término "Tiempo de vida del paquete", es decir el tiempo que le toma a un paquete desde que se genera hasta que se consume. Otra forma de llamarlo es el "Delay del paquete". Antes que nada veamos que pasaba con esto en la red de la parte 1:

![Delay parte 1 - caso 1](./GRAFICAS/Delay-P1-C1.png){width=auto height=250}

> Obs: Ésta gráfica es igual para ambos casos de estudio, por lo que no se muestra la otra.

Vemos como el **delay** llega a una especie de cota, en donde todos los paquetes tienen un tiempo de vida "máximo". Esto es debido a que los paquetes no se están almacenando en ningún lugar sino que se descartan.

<!--
FIXME: Agregar el otro gráfico que ayuda a entender el delay
-->

Ahora veamos que pasa con el **delay** en la parte 2:

![Delay parte 2 - caso 1](./GRAFICAS/Delay-P2-C1.png){width=auto height=250}

> Obs: Ésta gráfica es igual para ambos casos de estudio, por lo que no se muestra la otra.

Vemos como no se ve ninguna cota en término del **delay**. Este aumenta aproximadamente lineal a lo largo de toda la simulación. Se debe a que los paquetes se generan a un mayor ritmo del que se consumen durante la simulación. Generando así que los paquetes se vayan almacenando en algún buffer (Tx.queue) cada vez por más tiempo ya que se generan más rápido de lo que se consumen.
(Sería interesante ver un gráfico en donde se llegue a saturar el buffer al aumentar el generationInterval o dejar correr la simulación más tiempo)

//////////////////////////////////////////////////////////////////////////////

Las gráficas de **Carga ofrecida vs carga útil** se hicieron cambiando para cada parte y caso solamente el intervalo de tiempo de la generación de paquetes:

```cpp
Network.nodeTx.gen.generationInterval=exponential(0.1)
```

Con los siguientes valores: `0.05`, `0.1`, `0.15`, `0.2`, `0.3`, `0.4`, `0.8`, `1.6`, `3.2`
Donde podemos notar lo siguiente:

- Claramente la carga ofrecida aumentara de forma inversamente proporcional al aumento del generationInterval.
- La carga util y carga ofrecida es igual al inicio, osea en valores chicos.
- Mientras crece la carga ofrecida, la carga util lo hará a menor ritmo. Esto es gracias al aumento de la complejidad del sistema requiriendo que en vez de enviarse paquetes, se envíen feedback para la administración del sistema.
- La carga util crece hasta cierto punto. Este limite se debe a la capacidad del sistema.
<!-- - Al llegar a la cima, la carga util ira empeorando (tendera a la baja) por el costo de los feedback. En nuestro caso no son muy costosos por lo que apenas se notaran. -->

Las gráficas de todos ambas partes y casos son iguales. De la siguiente forma:

![Carga util vs Carga ofrecida](./GRAFICAS/CUtil-vs-COfrecida-P1-C1.png){width=auto height=250}

Para una mejor visualización de los datos hicimos la misma gráfica pero de `Carga util vs log(Carga ofrecida)`.
Esta es una mejor forma para visualizar datos que son cada vez mas dispersos, como en nuestro caso. Ademas ayuda a ver el crecimiento relativo y no absoluto como en la anterior gráfica.

![Carga util vs log(Carga ofrecida)](./GRAFICAS/CUtil-vs-logCOfrecida-P1-C1.png){width=auto height=250}
El gráfico se hizo aplicándole a cada dato del eje x logaritmo natural para una mejor visualización de la tendencia de los datos.

<!--
FIXME: Gráficas pendientes por explicar:
  - Carga ofrecida vs carga util.
-->

<!--
Las gráficas de `delay` indican el tiempo que le toma a cada paquete consumirse desde que se creo.
En la `parte 2`, la gráfica esta en contante crecimiento en ambos casos de igual forma, esto es asi gracias a que los paquetes se generan a un mayor ritmo del que se consumen y se van almacenando en la red. Notar que la red tiene una capacidad finita por lo que esto seria hasta el limite de su capacidad <!- FIXME: no se si es correcta esta hipótesis/observación -> (seria interesante ver un gráfico donde pase, al aumentar el generationInterval o dejar correr la simulación mas tiempo), en nuestro caso no llegamos al limite de la capacidad de nuestra red, por lo que no se pierden paquetes.

![Delay](./GRAFICAS/Delay-P2-C1.png){width=auto height=250}

Podemos ver en la siguiente gráfica de la `cantidad de paquetes generados vs los paquetes en el sistema` que se mantiene nuestra hipótesis, los paquetes se van almacenando en la red provocando nuevamente una tendencia al alza de la cantidad de paquetes en el sistema. Los paquetes en el sistema son la cantidad de paquetes generados menos los descartados (ninguno en este caso) y los paquetes consumidos.

![cantidad de paquetes generados vs los paquetes en el sistema](./GRAFICAS/Pks-Dentro-P2-C1.png){width=auto height=250}

En la siguiente gráfica de los `paquetes salientes del sistema` podremos ver la consumición de los paquetes:

![paquetes salientes del sistema](./GRAFICAS/Pks-Salientes-P2-C1.png){width=auto height=250}

Recordar que estas son las variables que se resta a la cantidad de paquetes generados para obtener la cantidad de paquetes aun dentro del sistema.
En la `parte 1`, podemos ver como el `delay` crece de la misma forma hasta que se "estabiliza" llegado un punto.

![Delay](./GRAFICAS/Delay-P1-C1.png){width=auto height=250}

A su vez ocurre algo similar con la `cantidad de paquetes generados vs los paquetes en el sistema`.

![cantidad de paquetes generados vs los paquetes en el sistema](./GRAFICAS/Pks-Dentro-P1-C1.png){width=auto height=250}

La causa de todo esto la podemos ver claramente en el siguiente gráfico de los `paquetes salientes del sistema`.

![paquetes salientes del sistema](./GRAFICAS/Pks-Salientes-P1-C1.png){width=auto height=250}

Esta gráfica si cambia entre ambos casos, pero su influencia es igual.

![paquetes salientes del sistema](./GRAFICAS/Pks-Salientes-P1-C2.png){width=auto height=250}

La causa de todo esto es que la red alcanzo su capacidad maxima y empezó a descartar paquetes causando esta "estabilidad" en los anteriores gráficos.
Notar que mas que estabilidad, se alcanzo un techo en el sistema. Se alcanzo el limite de su capacidad.

Podemos concluir que al introducirse paquetes dentro de una red mas rápido de lo que pueden llegar a consumirse, los paquetes deberán ser almacenados dentro de la red. Llegado ese punto, ante una constante generación de paquetes superior a una constante consumición de los mismos, podemos decir que cada vez serán mas los paquetes almacenados en la red.
A su vez, la red tiene una capacidad finita y ante un constante crecimiento de la cantidad de paquetes almacenados en la red, esta llegara a un limite y deberá empezar a descartarlos. Al dropear los paquetes, estos salen de la red. Por lo tanto llegados a ese punto, la cantidad de paquetes en el sistema se estabilizara y también la cantidad de paquetes en los buffers ocasionando que el delay de los paquetes también se estabilice.
-->

## Discusiones:

<!--
Una sección con los logros, limitaciones y posibles mejoras de nuestro algoritmo propuesto.
-->

Como logro en nuestro protocolo de control de flujo y congestion tenemos que para redes con tamaños de buffers suficientemente grandes y tasas de transferencias no tan alejadas, resuelve por completo la perdida de paquetes, además al trabajar con colas en buffers incluso llegan en orden.
Por otra parte es interesante notar que al ejecutar el protocolo al principio no conoce mucho las red, pero a medida que pasa el tiempo consigue enviar la mayor tasa de transferencia posible sin perdida de paquetes y mantenerse en un rango aceptable en la transmisión de paquetes.
Incluso los paquetes de control que viajan por la ruta de datos, al ser tan pequeños (1Byte) y enviados la menor cantidad de veces posible no influye casi nada en la carga útil. Conseguimos un algoritmo que resuelve tanto el problema de control de flujo y congestión de forma efectiva si se dan las condiciones.

Veamos las limitaciones, notar que al trabajar con cotas en porcentajes, si los tamaños de los buffers disminuyen, puede haber perdida de paquetes. Nuestro protocolo aprovecha ese porcentaje restante de paquetes que saturan los buffers utilizando ese tiempo para controlar la red y dejar de enviar paquetes, si estos son muy pocos no se va enterar el nodoTx a tiempo y se van a perder paquetes que luego no retransmitimos en nuestro protocolo.

Por otra parte tenemos inconvenientes si las tasas de transferencia difieren mucho entre sí, imaginemos un caso en particular para visualizar la problemática. Especifiquemos tasas de transferencia:

- NodeTx.queue ----- (100Mbps) ----> queue0
- queue0 ----- (1Mbps) ----> NodeRx.queue
- NodeRx.queue ----- (0.1Mbps) ----> Sink

Ahora especifiquemos los tamaños de buffers y sus cotas:

- queue0: 200pkt, cota=%80 => 160pkt.
- NodeRx.queue: 200pkt, cota=%50 => 100pkt.

En esta situación tenemos que el buffer de queue0 se va a llenar enseguida supongamos 199pkt y va a avisar que no se envíen paquetes, por lo tanto va a empezar a enviar todos su paquetes a NodeRx.queue, pero éste algún paquete tenia en su buffer, como envía tan lento le pueden llegar todos los de queue0 y en ese punto NodeRx.queue va a descartar paquetes. Esto sucede porque en la red hay más paquetes que los que pueden entrar en un solo buffer.

De esta forma introducimos una posible mejora al protocolo, además de garantizar que nunca se superen las cotas. Revisar también que en la red nunca haya más paquetes que los que pueden entrar en un buffer (ignorando el nodeTX). Esto se podría llevar a cabo con mensaje de control que informe al siguiente nodo cuántos paquetes se tienen actualmente en la cola, luego cuando se recibe esa información, sumarla a los paquetes que se tienen en ese nodo, y si la suma da mayor a la capacidad del buffer avisar que no se envíen mas paquetes.

---

## Referencias:

<!--
- Todas las referencias que usamos en el trabajo. LIBROS, PAPERS, WEB, ETC.
(Nosotros usamos el manual de Omnet++ y quizás algo más ...).

Si agregamos imágenes de Tanembaun para explicar algo de flujo y congestion, también se debe referenciar.
-->

- Andrew S. Tanenbaum, David J. Wetherall, Redes de Computadoras (5ta edición 2011), Pearson.
- Omnet++ Simulation Manual, (OMNeT++ version 6.0.3, 2020).

---

## Anexo: Inteligencia Artificial

En este anexo contamos como utilizamos las IA para desarrollo del estudio de redes.

Consideramos que podemos separar en dos partes como utilizamos la IA:

- **Consultar dudas conceptuales**.
- **Complemento en la escritura de código**.

### Consultar dudas conceptuales

Para este tipo de consultas utilizamos principalmente **Chat Gpt 3.5**, consideramos que estas son dudas rápidas que se pueden resolver con una respuesta corta y precisa. Si vemos que el tema es muy complejo, recurrimos a **Stack Overflow** o búsquedas en general por **Google**, somos cocientes que la respuesta puede ser mas lenta pero es mas precisa, las IA a veces no responden lo que uno espera.

### Complemento en la escritura de código

Para esto tenemos **Github Copilot**, nos ayuda a completar código que es repetitivo o sencillo, no dejamos que nos complete todo porque siempre sale mal y terminamos borrando lo que nos sugiere. Pero para cosas como completar un `for` o un `if` nos ayuda mucho. Notamos un incremento en nuestra velocidad de escritura de código y consideramos que esto es muy bueno, por eso la seguimos utilizando. Por otra parte nos planteamos si esto nos perjudica en algo y encontramos una gran desventaja de usarlo, "nos acostumbramos", esto puede ser malo si de un dia para el otro no lo tenemos más, quizá nos cueste más escribir código. Por ello cada tanto la desactivamos y escribimos código a mano para no perder la costumbre.

---
