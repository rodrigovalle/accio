#!/usr/bin/env python3
import sys, os
import socket
import filecmp
import subprocess
import time

chunk_size = 2045
delay = 0.01
sizelist = [500, 1048576, 104857600, 104729] # throw in a prime number, why not

srv_port = '3000'
srv_host = 'localhost'
srv_dir = 'save/'

inf_dir = 'infiles/'
infile = inf_dir + 'infile{}.txt'

subprocess.run(['rm', '-rf', srv_dir])
subprocess.run(['rm', '-rf', inf_dir])
os.mkdir(srv_dir)
os.mkdir(inf_dir)

subprocess.run(['pkill', 'server'])
server = subprocess.Popen(['./server', srv_port, srv_dir])

def create_file(name, size):
  with open(name, 'wb') as f:
    f.write(os.urandom(size))

time.sleep(1)

for i, size in enumerate(sizelist[:2], start=1):
  create_file(infile.format(i), size)

  with open(infile.format(i), 'rb') as f:
      s = socket.create_connection((srv_host, srv_port))
      b = f.read(chunk_size)
      while b:
          time.sleep(delay)
          s.send(b)
          b = f.read(chunk_size)
      s.close()

  time.sleep(0.1)

  if filecmp.cmp(infile.format(i), 'save/{}.file'.format(i), shallow=False):
    print('{}B file transfer successful'.format(size))
  else:
    print('file transfer failed')

server.kill()
