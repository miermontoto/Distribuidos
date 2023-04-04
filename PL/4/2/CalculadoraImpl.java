import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;

public class CalculadoraImpl extends UnicastRemoteObject implements Calculadora {

    public CalculadoraImpl() throws RemoteException {
        super();
    }

    @Override
    public int suma(int a, int b) throws RemoteException {return a + b;}

    @Override
    public int resta(int a, int b) throws RemoteException {return a - b;}

    @Override
    public int multiplicacion(int a, int b) throws RemoteException {return a * b;}

    @Override
    public double division(int a, int b) throws RemoteException, DivisionPorCero {
        if (b == 0) {
            throw new DivisionPorCero("No se puede dividir por cero");
        }
        return (double) a / b;
    }
}
