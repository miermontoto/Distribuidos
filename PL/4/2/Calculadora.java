import java.rmi.Remote;
import java.rmi.RemoteException;

public interface Calculadora extends Remote {
    public int suma(int a, int b) throws RemoteException;
    public int resta(int a, int b) throws RemoteException;
    public int multiplicacion(int a, int b) throws RemoteException;
    public double division(int a, int b) throws RemoteException, DivisionPorCero;
}
