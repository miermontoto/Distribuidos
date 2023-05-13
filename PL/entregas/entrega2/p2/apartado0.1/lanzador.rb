# filosofo <id_filosofo> <num_filosofos> <ip_siguiente> <puerto_siguiente> \
# <puerto_local> <delay_conexion>
# ./filodist 0 5 127.0.0.1 40001 40000 2 &
# ./filodist 1 5 127.0.0.1 40002 40001 2 &
# ./filodist 2 5 127.0.0.1 40003 40002 2 &
# ./filodist 3 5 127.0.0.1 40004 40003 2 &
# ./filodist 4 5 127.0.0.1 40000 40004 2 &

class Filodist
	def initialize(id, num, port, prev, delay)
		system("./filodist #{id} #{num} 127.0.0.1 #{prev} #{port} #{delay}")
	end
end



if ARGV.length != 2 && ARGV.length != 1 then
	puts "Uso: ruby lanzador.rb <num_filosofos> [delay]"
	exit
end

system("killall filodist 2> /dev/null")
system("make all > /dev/null")
num = ARGV[0].to_i
delay = ARGV.length == 2 ? ARGV[1].to_i : 5
used_ports = `netstat -tulpn 2> /dev/null | grep tcp | awk '{print $4}' | awk -F: '{print $2}'`.split("\n").map(&:to_i)
start = rand(10000..60000)
while (start..start+num).any? { |port| used_ports.include?(port) }
	start = rand(10000..60000)
end

threads = []
for i in 0..num-2
	threads << Thread.new {Filodist.new(i, num, start+i, start+i+1, delay)}
	sleep(0.005) # Importante, para que no se solapen los hilos.
end
threads << Thread.new {Filodist.new(num-1, num, start+num-1, start, delay)}
threads.each { |t| t.join }
