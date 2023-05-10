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

if maxf > 10 or maxn > 8 or maxf < 1 or maxn < 1
	puts "Error: max_facilidad y max_nivel deben ser enteros entre 1 y 10"
	exit
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
	`./lanzar_cliente rand.txt`
	puts "\rCliente #{i} [OK] (#{Time.now - time}s)"
end

puts `./lanzar_estadis`
