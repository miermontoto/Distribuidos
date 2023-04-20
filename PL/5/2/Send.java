import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;

public class Send {

  private final static String QUEUE_NAME = "pugafran";

  //private static String getMessage(String[] strings)
  //{
  //   if (strings.length < 1)
  //      return "Hello World!";
  //}
  private static String joinStrings(String[] strings, String delimiter)
  {
     int length = strings.length;
     if (length == 0) return "";
     StringBuilder words = new StringBuilder(strings[0]);
     for (int i = 1; i < length; i++) {
        words.append(delimiter).append(strings[i]);
     }
     return words.toString();
  }
  public static void main(String[] argv) throws Exception {
    ConnectionFactory factory = new ConnectionFactory();
    factory.setHost("localhost");
    Connection connection = factory.newConnection();
    Channel channel = connection.createChannel();

    channel.queueDeclare(QUEUE_NAME, false, false, false, null);

    // Obtener el mensaje a enviar de la línea de comandos
    String message = joinStrings(argv, " ");

    // Enviarlo
    channel.basicPublish("", QUEUE_NAME, null, message.getBytes("UTF-8"));
    System.out.println(" [x] Sent '" + message + "'");

    channel.close();
    connection.close();
  }
}
