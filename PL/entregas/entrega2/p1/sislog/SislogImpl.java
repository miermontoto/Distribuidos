package sislog;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.concurrent.ArrayBlockingQueue;

/*
La clase SislogImpl implementa el interfaz SislogInterface:

obtenerValorFacilidadNivel(): que será invocado desde un cliente RMI
    para obtener el número de eventos de un determinado nivel correspondientes
    a una determinada facilidad

obtenerNumeroFacilidades(): devuelve el numero de facilidades
    con los cuales puede trabajar el sislog.

obtenerNumeroNiveles(): devuelve el numero de niveles de severidad
    con los cuales puede trabajar el sislog.

obtenerNombreFacilidad(): obtiene el nombre de la facilidad
    cuyo id le pasamos como argumento.

obtenerNombreNivel(): obtiene el nobre del nivel (severidad)
    cuyo id le pasamos como argumento.

*/

public class SislogImpl extends UnicastRemoteObject implements SislogInterface {
    private ContabilidadEventos accountev;  // Objeto que registra la contabilidad de los eventos recibidos
    private String[] fac_names;             // Nombres de facilidades
    private String[] level_names;           // Nombres de niveles

    public SislogImpl(ContabilidadEventos accountev, String [] fac_names, String[] level_names) throws RemoteException {
        super();
        this.accountev = accountev;
        this.fac_names = fac_names;
        this.level_names = level_names;
    }

    @Override
    public int obtenerValorFacilidadNivel(int facilidad, int nivel) throws RemoteException {
        return accountev.obtenerValorFacilidadNivel(facilidad, nivel);
    }

    @Override
    public int obtenerNumeroFacilidades() throws RemoteException {
        return accountev.obtenerNumeroFacilidades();
    }

    @Override
    public int obtenerNumeroNiveles() throws RemoteException {
        return accountev.obtenerNumeroNiveles();
    }

    @Override
    public String obtenerNombreFacilidad(int facilidad) throws RemoteException {
        try {
            return fac_names[facilidad];
        } catch (ArrayIndexOutOfBoundsException e) {
            throw new RemoteException("Facilidad no encontrada");
        }
    }

    @Override
    public String obtenerNombreNivel(int nivel) throws RemoteException {
        try {
            return level_names[nivel];
        } catch (ArrayIndexOutOfBoundsException e) {
            throw new RemoteException("Nivel no encontrado");
        }
    }

    @Override
    public boolean equals(Object o) {
        if (o instanceof SislogImpl) {
            SislogImpl other = (SislogImpl) o;
            return accountev.equals(other.accountev);
        }
        return false;
    }

    @Override
    public int hashCode() {
        return accountev.hashCode();
    }
}
