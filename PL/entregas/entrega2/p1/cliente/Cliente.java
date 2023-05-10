package cliente;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

// Imports necesarios para RabbitMQ
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;

public class Cliente {
    private static final String NOMBRE_COLA = "JuanFranciscoMM";

    // Función main por la que arranca el cliente
    public static void main(String[] argv) throws Exception {
        // =================================================
        // Obtener argumentos de línea de comandos
        if (argv.length < 1) {
            System.out.println("Uso: cliente fichero_eventos");
            System.exit(1);
        }

        // =================================================
        // Parte principal, toda dentro de un try para capturar cualquier excepción
        try {
            // Conectar con Rabbit para poder enviar peticiones a la cola
            ConnectionFactory factory = new ConnectionFactory();
            Connection connection = factory.newConnection();
            Channel channel = connection.createChannel();
            channel.queueDeclare(NOMBRE_COLA, false, false, false, null);

            // Arrancar la función que lee eventos del fichero y los envía por rabbit
            enviar_eventos(channel, argv[0]);

            // Terminar
            System.out.println("Cliente finalizado");
            channel.close();
            connection.close();

            System.exit(0);
        } catch (Exception e) {
            // Cualquier excepción simplemente se imprime
            System.out.println("Error en Cliente" + e.getMessage());
            e.printStackTrace();
            System.exit(9);
        }
    }

    // =========================================================================
    // La función lee del fichero de entrada y envia las líneas leídas
    // como mensajes a través de la cola de mensajes
    // Requiere como parámetros:
    //
    //  - El canal de comunicación con RabbitMQ para enviar los mensajes
    //  - El nombre del fichero con los eventos a enviar
    //
    // Una vez finaliza de leer todos los mensajes y enviarlos a la cola, termina
    static void enviar_eventos(Channel channel, String fich_evt) throws IOException {
        BufferedReader br = new BufferedReader(new FileReader(fich_evt));
        try {
            // Leer todas las líneas del fichero y enviarlas como mensajes
            String linea;
            while ((linea = br.readLine()) != null) {
                channel.basicPublish("", NOMBRE_COLA, null, linea.getBytes(StandardCharsets.UTF_8));
            }
            br.close();
        } catch (Exception e) {
            br.close();
            throw e;
        }
    }
}
