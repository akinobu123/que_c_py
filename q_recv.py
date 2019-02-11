import posix_ipc
import time

# ----------------- pix2pixを実行 ------------------
def exec_pix2pix():
	print("  py : pix2pix start")
	for i in range(3):
		print("  py : pix2pix executing")
		time.sleep(1)
	print("  py : pix2pix end")


# ----------------- 専用queueを作成しスタンバイモードへ ----------------
q_salt = posix_ipc.MessageQueue('/salt_send', posix_ipc.O_CREAT)
q_p2p  = posix_ipc.MessageQueue('/pix2pix_send', posix_ipc.O_CREAT)

# スタンバイモードへ
q_p2p.send('stanby')								# スタンバイ準備完了を返送
while True:
	msg = q_salt.receive()
	msg_str = msg[0].decode('utf-8')
	if msg_str == 'quit':
		break
	elif 'start' in msg_str:						# start.xx : pix2pix実行
		job_id = msg_str.split('.')[1]
		exec_pix2pix()
		q_p2p.send('end.{}'.format(job_id))			# 終了のシグナルを返送

print("py : python end")

