# receive

import socket
import hashlib
import struct
import os
import tqdm

HOST = '192.168.31.89'
PORT = 1080
BUFFER_SIZE = 1024
HEAD_STRUCT = '128sIq32s'
info_size = struct.calcsize(HEAD_STRUCT)
COD = 'utf-8'


def requrest_filelist(dirname):
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = (HOST, PORT)
    client_socket.connect(server_address)
    send_data = dirname.encode(COD)

    client_socket.send(send_data)

    recved_size = 0

    with open("cache.txt", 'wb') as fw:
        while True:
            recv_file = client_socket.recv(BUFFER_SIZE)
            if len(recv_file) <= 0:
                break
            else:
                fw.write(recv_file)


def make_dirs(directory):
    path, filename = os.path.split(directory)
    dirs = path.split('\\')
    dir = ''
    for i in dirs:
        dir += i
        dir += '/'
        if not os.path.exists(dir):
            os.mkdir(dir)

def download_file(url, output):
    make_dirs(output)
    print(output)
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = (HOST, PORT)
    client_socket.connect(server_address)
    send_data = url.encode(COD)
    client_socket.send(send_data)

    recved_size = 0

    with open(output, 'wb') as fw:
        while True:
            recv_file = client_socket.recv(BUFFER_SIZE)
            if len(recv_file) <= 0:
                break
            else:
                fw.write(recv_file)


if __name__ == '__main__':
    path_planning = 'F:/1211/3DGLueGuider'
    dirname = "F:\\1126"
    dirname = path_planning
    requrest_filelist(dirname)

    with open("cache.txt") as f:
        for line in tqdm.tqdm(f.readlines(), 'downloading...'):
            if line.find('.vs') != -1:
                continue
            if line.find('x64') != -1:
                continue
            if line.find('.git') != -1:
                continue
            line = line.strip()
            url = line
            output = f'downloads{line[len(dirname):]}'
            download_file(url, output)
