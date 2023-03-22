package servidor;

import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import interfaz.Calculadora;

public class CalculadoraImpl extends UnicastRemoteObject implements Calculadora {

    public CalculadoraImpl() throws RemoteException {
        super();
    }

    @Override
    public int suma(int a, int b) throws RemoteException {return a + b;}

    @Override
    public int resta(int a, int b) throws RemoteException {return a - b;}
}
