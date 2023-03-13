make clean
make all -B

echo "Validación de sislog"
echo "--------------------"

sislog &>/dev/null || echo "Prueba sin parámetros"
sislog asd t 5 5 5 &>/dev/null || echo "Prueba con puerto incorrecto"
sislog -1 t 5 5 5 &>/dev/null || echo "Prueba con puerto inválido (<0)"
sislog 1000 t 5 5 5 &>/dev/null || echo "Prueba con puerto inválido (<1024)"
sislog 66666 t 5 5 5 &>/dev/null || echo "Prueba con puerto inválido (>65535)"
sislog 7890 test 5 5 5 &>/dev/null || echo "Prueba con con protocolo inválido (t-est)"
sislog 7890 p 5 5 5 &>/dev/null || echo "Prueba con con protocolo incorrecto (p)"
sislog 7890 t 0 5 5 &>/dev/null || echo "Prueba con tamaño de cola inválido"
sislog 7890 t 5 0 5 &>/dev/null || echo "Prueba con número de hilos_aten inválido"
sislog 7890 t 5 5 0 &>/dev/null || echo "Prueba con número de hilos_work inválido"
sislog 7890 t a b c &>/dev/null || echo "Prueba con parámetros inválidos"

echo
echo "Validación de cliente"
echo "---------------------"

cliente &>/dev/null || echo "Prueba sin parámetros"
cliente 127 7890 t 5 eventos.txt &>/dev/null || echo "Prueba con IP incorrecta"
cliente 10.0.0.1 t 5 eventos.txt &>/dev/null || echo "Prueba con IP inexistente"
cliente 127.0.0.1 asd t 5 eventos.txt &>/dev/null || echo "Prueba con puerto incorrecto"
cliente 127.0.0.1 1000 t 5 eventos.txt &>/dev/null || echo "Prueba con puerto inválido (<1024)"
cliente 127.0.0.1 66666 t 5 eventos.txt &>/dev/null || echo "Prueba con puerto inválido (>65535)"
cliente 127.0.0.1 7890 test 5 eventos.txt &>/dev/null || echo "Prueba con protocolo inválido (t-est)"
cliente 127.0.0.1 7890 p 5 eventos.txt &>/dev/null || echo "Prueba con protocolo incorrecto (p)"
cliente 127.0.0.1 7890 t 0 eventos.txt &>/dev/null || echo "Prueba con número de hilos inválido"
cliente 127.0.0.1 7890 t 5 rubennmg.tpg &>/dev/null || echo "Prueba con archivo de eventos inexistente"
cliente 127.0.0.1 7890 t 5 cola.h &>/dev/null || echo "Prueba con archivo de eventos incorrecto"
