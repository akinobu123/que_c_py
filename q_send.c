#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int send_msg(mqd_t q, char *msg, int job_id)
{
	char tmp_str[128];

	if(job_id == -1) {
		sprintf(tmp_str ,"%s", msg);
	} else {
		sprintf(tmp_str ,"%s.%d", msg, job_id);
	}
	void *msg_pt = calloc(strlen(tmp_str) + 1, sizeof(char));
	strcpy(msg_pt, tmp_str);

	int ret = mq_send(q, msg_pt, strlen(msg_pt), 0);
	if(ret < 0) {
		printf("mq_send error : %s\n", strerror(errno));
		free(msg_pt);
		return ret;
	}
	printf("c -> py : %s\n", tmp_str);

	free(msg_pt);
	return ret;
}

int wait_msg(mqd_t q, char *msg, int job_id)
{
	char target_str[128];
	struct mq_attr attr;
	char buff[1024];
	ssize_t bytes_read;

	// make target string
	if(job_id == -1) {
		sprintf(target_str ,"%s", msg);
	} else {
		sprintf(target_str ,"%s.%d", msg, job_id);
	}

	// wait for msg + job_id
	while(1){
		memset(buff, 0x00, sizeof(buff));
		bytes_read = mq_receive(q, buff, sizeof(buff), NULL);
		if(bytes_read < 0) {
			printf("mq_receive error : %s\n", strerror(errno));
			return bytes_read;
		}
		printf("py -> c : %s\n", buff);
		if(strcmp(buff, target_str)==0) {
			break;
		}
	}

	return bytes_read;
}

int main()
{
	// queueを構築
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = 1024;
	attr.mq_curmsgs = 0;

	mode_t omask;
	omask = umask(0);
	mq_unlink("/salt_send");
	mq_unlink("/pix2pix_send");
	mqd_t q_salt = mq_open("/salt_send", O_RDWR | O_CREAT, 0777, &attr);
	if(q_salt < 0) {
		printf("mq_open(q_salt) error : %s\n", strerror(errno));
		return 0;
	}
	mqd_t q_p2p  = mq_open("/pix2pix_send", O_RDWR | O_CREAT, 0777, &attr);
	if(q_p2p < 0) {
		printf("mq_open(q_p2p) error : %s\n", strerror(errno));
		return 0;
	}
	umask(omask);

	// pix2pix起動（スタンバイ状態）
	system("python q_recv.py &");
	wait_msg(q_p2p, "stanby", -1);

	// １回目のpix2pix実行
	send_msg(q_salt, "start", 1);
	wait_msg(q_p2p, "end", 1);

	// ２回目のpix2pix実行
	send_msg(q_salt, "start", 2);
	wait_msg(q_p2p, "end", 2);

	// pix2pix終了
	send_msg(q_salt, "quit", -1);

	// queueの後始末
	mq_close(q_salt);
	mq_close(q_p2p);
	mq_unlink("/salt_send");
	mq_unlink("/pix2pix_send");
}

