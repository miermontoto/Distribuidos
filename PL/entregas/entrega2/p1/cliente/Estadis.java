package cliente;

import java.rmi.Naming;

// Imports necesarios para invocar via RMI métodos del sislog
import sislog.SislogInterface;

public class Estadis {
    // Funcion main por la que arranca
    public static void main(String[] argv) throws Exception {
        int nfils, ncols, suma, n, total;   // Variables para las estadísticas
        SislogInterface evtserv;            // Objeto remoto para invocar métodos RMI

        // =================================================
        // Instanciar SecurityManager necesario para RMI
        if (System.getSecurityManager() == null) {
            System.setSecurityManager(new SecurityManager());
        }

        // =================================================
        // Parte principal, toda dentro de un try para capturar cualquier excepción
        try {
            // Obtener por RMI el número de niveles (ncols) y facilidades (nfils)
            evtserv = (SislogInterface) Naming.lookup("Sislog");
            nfils = evtserv.obtenerNumeroFacilidades();
            ncols = evtserv.obtenerNumeroNiveles();

            System.out.println("**************************  RECUENTO EVENTOS  ********************************");
            System.out.print("\t");

            // Imprimir, separados por \t, los nombres de las columnas (nombres de niveles
            // que se obtienen por RMI)
            for(int i = 0; i < ncols; i++) {
                System.out.printf("%s\t", evtserv.obtenerNombreNivel(i));
            }

            System.out.println("TOTAL");
            for (int i = 0; i < nfils; i++) {
                // Para cada fila se imprime primero el nombre de la facilidad (obtenido por RMI)
                // con un \t al final
                System.out.printf("%s\t", evtserv.obtenerNombreFacilidad(i));
                suma = 0;

                // Seguidamente se itera por cada columna (nivel) y se obtiene por RMI el valor del
                // contador correspodiente a la facilidad y nivel actual (fila y columna)
                for (int j = 0; j < ncols; j++) {
                    n = evtserv.obtenerValorFacilidadNivel(j, i);

                    System.out.print(n+"\t");
                    suma += n;
                }
                System.out.println(suma);
            }
            // Una vez impresas todas las filas, se imprime una fila final con los totales
            // Estos hay que computarlos por columnas, para lo que de nuevo se
            // harán llamadas RMI para obtener los valores de cada contador
            System.out.print("TOTALES\t");
            total = 0;
            for (int j = 0; j < ncols; j++) {
                suma = 0; // Para computar el total por columnas
                for (int i = 0; i < nfils; i++) {
                    suma += evtserv.obtenerValorFacilidadNivel(j, i);
                }
                System.out.print(suma+"\t");
                total += suma;
            }
            System.out.println(total);
        } catch (Exception e) {
            // Cualquier excepción simplemente se imprime
            System.out.println("Error en Estadis" + e.getMessage());
            e.printStackTrace();
        }
    }
}
