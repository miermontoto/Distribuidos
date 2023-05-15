require "faker"

# Script que genera un archivo de texto siguiendo el formato
# especificado en el enunciado de la práctica. El objetivo es
# generar eventos aleatorios en un archivo de texto y utilizar
# el programa "cliente" para enviarlos al servidor rpc.

# Ejecución:
# $ gem install faker
# $ ruby tester.rb <max_facilidad> <max_nivel> [num_eventos] [num_threads]

# La estructura de un evento es la siguiente:
# <facilidad>:<nivel>:<mensaje>
# Donde la facilidad y nivel son enteros y el mensaje es una cadena.
#
# MAXFACILITIES = 10
# MAXLEVELS = 8
#
# Ejemplo:
# 1:3:Prueba

def generate_event(max_facilidad = 10, max_nivel = 8)
	facilidad = rand(0..max_facilidad - 1)
	nivel = rand(0..max_nivel - 1)
	mensaje = Faker::Lorem.sentence
	return "#{facilidad}:#{nivel}:#{mensaje}"
end

def generate_file(filename, n, max_facilidad, max_nivel)
	File.open(filename, "w") do |f|
		n.times do
			f.puts(generate_event(max_facilidad, max_nivel))
		end
	end
end


if ARGV.length < 3 or ARGV.length > 4
	puts "Usage: ruby tester.rb <max_facilidad> <max_nivel> <num_clientes> [num_eventos]"
	exit
end

maxf = ARGV[0].to_i
maxn = ARGV[1].to_i
clientes = ARGV[2].to_i
eventos = ARGV.length == 4 ? ARGV[3].to_i : 10000

if maxf > 10 or maxn > 8 or maxf < 1 or maxn < 11
end

if eventos < 1 or clientes < 1
	puts "Error: num_eventos y clientes deben ser enteros mayores que 0"
	exit
end

file = "rand.txt"

for i in 1..clientes
	time = Time.now
	print "Cliente #{i} [  ]"
	generate_file(file, eventos, maxf, maxn)
	`java -cp clases:clases/rabbitmq-client.jar -Djava.security.policy=policy cliente.Cliente rand.txt`
	puts "\rCliente #{i} [OK] (#{Time.now - time}s)"
	while(`sudo rabbitmqctl list_queues`.split("\n").last.split(" ").last.to_i > 1) do
		sleep(0.001)
	end
	sleep(0.5)
end

puts `./lanzar_estadis`
