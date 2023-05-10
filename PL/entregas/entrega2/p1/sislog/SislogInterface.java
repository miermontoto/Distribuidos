package sislog;
import java.rmi.Remote;
import java.rmi.RemoteException;

/**
  Interfaz remoto del Sislog que ha de hacer público los metodos que pueden ser
  invocados desde un cliente RMI como Estadis.java
*/
public interface SislogInterface extends Remote {
    public int obtenerValorFacilidadNivel(int facilidad,int nivel) throws Remote Exception;
    public int obtenerNumeroFacilidades() throws RemoteException;
    public int obtenerNumeroNiveles() throws RemoteException;
    public String obtenerNombreFacilidad(int facilidad) throws RemoteException;
    public String obtenerNombreNivel(int nivel) throws RemoteException;
}
