import json
import posix_ipc
import time
import argparse

def wait_for_end(q_p2p, job_id):
	while True:
		msg = q_p2p.receive()
		msg_str = msg[0].decode('utf-8')
		if 'end' in msg_str:
			r_job_id = msg_str.split('.')[1]
			if r_job_id == job_id:
				print("py -> c : end.xx")
				break

q_salt_name = '/salt_send'
q_p2p_name  = '/pix2pix_send'
q_salt = posix_ipc.MessageQueue(q_salt_name, posix_ipc.O_CREAT)
q_p2p  = posix_ipc.MessageQueue(q_p2p_name, posix_ipc.O_CREAT)

print("c -> py : start.xx")
q_salt.send('start.1')
wait_for_end(q_p2p, '1')

print("c -> py : start.xx")
q_salt.send('start.2')
wait_for_end(q_p2p, '2')

print("c -> py : quit")
q_salt.send("quit")

