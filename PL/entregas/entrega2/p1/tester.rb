# Archivo que comprueba todas las entradas de todas las partes
# del programa.

# Orden de ejecución:
# 1. lanzar_registro
# 2. lanzar_sislog <num_facilidades> <num_niveles> <tam_cola> <num_workers>
# 3. ruby tester.rb <num_facilidades> <num_niveles> <num_clientes> <num_eventos>

# num_facilidades máximo: 10
# num_niveles máximo: 8
# num_workers máximo: 10000

# Se han de comprobar todos los parámetros de entrada
# Se ha de comprobar que el servicio `rabbitmq-server` está activo

require "colorize"


def lc
	return "java -cp clases:clases/rabbitmq-client.jar -Djava.security.policy=policy sislog.Sislog"
end

def le
	result = `java -cp clases:clases/rabbitmq-client.jar -Djava.security.policy=policy cliente.Estadis`
	if $?.exitstatus != 0 then
		print "FAIL".red
		puts " (estadis)"
		abort
	end
	result.split("\n").last.split(" ").last.to_i
end

def lc_run(args, err)
	t = Thread.new {system("#{lc} #{args} &>/dev/null")}

	# Después de 1 décima, comprobar si el hilo sigue activo y matarlo si es así
	sleep(0.1)
	if t.alive? then
		print "FAIL".red
		puts " (#{err})"
		abort
	end
end

system("killall rmiregistry 2>/dev/null")

# Comprobar que el fichero "policy" existe
print "policy\t\t"
if !File.exist? "policy" then
	puts "FAIL".red
	abort
end
puts "OK".green

print "rabbitmq-server\t"
rabbitmq = `systemctl status rabbitmq-server`
if !rabbitmq.include? "running" then
	system("sudo systemctl start rabbitmq-server")
	if $?.exitstatus != 0 then
		puts "FAIL".red
		abort
	end
end
puts "OK".green

print "compilar\t"
msg = `./compilar 2>/dev/null | wc -l`
if msg.to_i != 6 then
	puts "FAIL".red
	abort
end
puts "OK".green
001
print "lanzar_registro\t"
system("CLASSPATH=clases rmiregistry & 2>/dev/null")
puts $?.exitstatus == 0 ? "OK".green : "FAIL".red

print "lanzar_sislog\t"

lc_run("", "sin argumentos")
lc_run("10 8 9", "3 argumentos")
lc_run("10 8 9 10 10", "5 argumentos")
lc_run("11 8 10 10", "num_facilidades > 10")
lc_run("10 9 10 10", "num_niveles > 8")
lc_run("10 8 0 10", "tam_cola = 0")
lc_run("10 8 10 0", "num_workers = 0")
#lc_run("10 8 10 10001", "num_workers > 10000")

puts "OK".green
system("killall java &>/dev/null")

print "lanzar_cliente\t"
t = Thread.new { system("#{lc} 10 8 10 10 &>/dev/null") }
sleep(1)
system("java -cp clases:clases/rabbitmq-client.jar -Djava.security.policy=policy cliente.Cliente eventos.txt &>/dev/null")
if $?.exitstatus != 0 then
	puts "FAIL".red
	abort
end
puts "OK".green

print "lanzar_estadis\t"
result = le
if result != 7 then
	print "FAIL".red
	puts " (#{result})"
	abort
end
puts "OK".green
t.kill

if !File.exist?("generator.rb") then; exit; end
print "heavy test\t"
t = Thread.new { system("#{lc} 10 8 100 100 &>/dev/null") }
sleep(1)
result = `ruby generator.rb 10 8 10`.split("\n").last.split(" ").last.to_i
if result != 50000 then
	print "FAIL".red
	puts " (#{result})"
	abort
end
