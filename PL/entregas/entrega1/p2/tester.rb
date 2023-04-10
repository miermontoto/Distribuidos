require "faker"

# Script que genera un archivo de texto siguiendo el formato
# especificado en el enunciado de la práctica. El objetivo es
# generar eventos aleatorios en un archivo de texto y utilizar
# el programa "cliente" para enviarlos al servidor rpc.

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


if ARGV.length != 2 and ARGV.length != 3 and ARGV.length != 4
	puts "Usage: ruby tester.rb <max_facilidad> <max_nivel> [num_eventos] [num_threads]"
	exit
end

maxf = ARGV[0].to_i
maxn = ARGV[1].to_i
eventos = ARGV.length >= 3 ? ARGV[2].to_i : 10000
threads = ARGV.length == 4 ? ARGV[3].to_i : rand(1..10)
if maxf > 10 or maxn > 8 or maxf < 1 or maxn < 1
	puts "Error: max_facilidad y max_nivel deben ser enteros entre 1 y 10"
	exit
end

if eventos < 1 or threads < 1
	puts "Error: num_eventos y num_threads deben ser enteros mayores que 0"
	exit
end

print "Generando eventos... [  ]"
time = Time.now
file = "rand.txt"
generate_file(file, eventos, maxf, maxn)
puts "\rGenerando eventos... [OK] (#{Time.now - time}s)"

time = Time.now
print "Enviando eventos...  [  ]"
test = `./cliente #{threads} 127.0.0.1 #{file}`
puts "\rEnviando eventos...  [OK] (#{Time.now - time}s)"

enviados = 0
recibidos = 0
errores = 0
test.each_line do |line|
	if line.include? "envia"
		enviados += 1
	elsif line.include? "recibe"
		recibidos += 1
		if line.include? "Caso: 1"
			errores += 1
		end
	end
end

puts "S: #{enviados}, R: #{recibidos} | %: #{(recibidos.to_f / eventos.to_f * 100).round(2)}, E: #{errores}"
