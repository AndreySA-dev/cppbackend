import argparse
import subprocess
import time
import random
import shlex


RANDOM_LIMIT = 1000
SEED = 123456789
random.seed(SEED)
perf_data_filename = "perf.data"
perf_data_stage2_filename = "server_perf.stage2.data"
perf_data_stage3_filename = "server_perf.stage3.data"
perf_data_stage4_filename = "server_perf.stage4.data"
out_svg_filename = "graph.svg"

AMMUNITION = [
	'localhost:8080/api/v1/maps/map1',
	'localhost:8080/api/v1/maps'
]

SHOOT_COUNT = 100
COOLDOWN = 0.1


def start_server():
	parser = argparse.ArgumentParser()
	parser.add_argument('server', type=str)
	return parser.parse_args().server


def run(command, output=None):
	process = subprocess.Popen(shlex.split(command), stdout=output, stderr=subprocess.DEVNULL)
	return process


def stop(process, wait=False):
	if process.poll() is None and wait:
		process.wait()
	process.terminate()


def shoot(ammo):
	hit = run('curl ' + ammo, output=subprocess.DEVNULL)
	time.sleep(COOLDOWN)
	stop(hit, wait=True)


def make_shots():
	for _ in range(SHOOT_COUNT):
		ammo_number = random.randrange(RANDOM_LIMIT) % len(AMMUNITION)
		shoot(AMMUNITION[ammo_number])
	print('Shooting complete')


if __name__ == "__main__" :

	server = run(start_server())
	server_pid = server.pid
	print(f"Server started with pid={server_pid}")
	print(f"Start profiler and shooter...")
	profiler = run(f"perf record -g -p {server_pid} -o {perf_data_filename}")
	make_shots()
	stop(profiler)
	stop(server)
	profiler.wait()
	server.wait()
	print(f"Profiler and shooter stopped...")
	
	print(f"Perf script start...")
	perf_script = subprocess.Popen(shlex.split(f"perf script"), stdout=subprocess.PIPE)

	print(f"stackcollapse start...")
	collapser = subprocess.Popen(shlex.split(f"perl ./FlameGraph/stackcollapse-perf.pl"), stdout=subprocess.PIPE, stdin=perf_script.stdout)

	print(f"flamegraph start...")
	with open(out_svg_filename, "w") as out_file:
		subprocess.run(shlex.split(f"perl ./FlameGraph/flamegraph.pl"), stdin=collapser.stdout, stdout=out_file)

	perf_script.wait()
	collapser.wait()
	print('Job done')
	pass
