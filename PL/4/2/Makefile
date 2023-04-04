BIN=/usr/lib/jvm/java-1.8.0/bin/

compile:
		$(BIN)javac Calculadora.java
		$(BIN)javac DivisionPorCero.java
		$(BIN)javac CalculadoraImpl.java
		$(BIN)javac Servidor.java
		$(BIN)javac Cliente.java

rmiregistry:
		CLASSPATH=. $(BIN)rmiregistry &

run_server:
		$(BIN)java -Djava.security.policy=policy Servidor
		CalculadoraServer registrada

run_client:
		$(BIN)java -Djava.security.policy=policy Cliente
