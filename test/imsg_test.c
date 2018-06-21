/*
 * Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <unistd.h>

#include <imsg.h>

#include "imsg_test.h"

#define DATA_INT	42
#define DATA_STRING	"This is my message"
#define STRING_LENGTH	128
#define MSGNUM		2

enum imsg_type {
	IMSG_MSG_INT,
	IMSG_MSG_STRING
};

int
parent(struct imsgbuf *ibuf)
{
	struct imsg	imsg;
	unsigned long   datalen;
	int		idata, n = 0, msgnum = 0;
	char		sdata[STRING_LENGTH];

	for (; msgnum < MSGNUM; n = imsg_read(ibuf)) {
		if (n == -1 && errno != EAGAIN) {
			/* handle socket error */
			printf("%s: imsg_read error\n", __func__);
			exit(1);
		}
		for(;;) {
			if ((n = imsg_get(ibuf, &imsg)) == -1) {
				/* handle read error */
				printf("%s: imsg_get error\n", __func__);
				break;
			}
			if (n == 0) {
				break;
			}
			datalen = imsg.hdr.len - IMSG_HEADER_SIZE;

			switch (imsg.hdr.type) {
			case IMSG_MSG_INT:
				if (datalen < sizeof(idata)) {
					/* handle corrupt message */
					printf("%s: datalen error\n", __func__);
					break;
				}
				memcpy(&idata, imsg.data, sizeof(idata));
					/* handle message received */
				printf("%s: received: %d, size %ld\n", __func__,
								idata, datalen);
				/* increase message number */
				msgnum++;
				break;
			case IMSG_MSG_STRING:
				if (datalen < sizeof(sdata)) {
					/* handle corrupt message */
					printf("%s: datalen error\n", __func__);
					break;
				}
				memcpy(sdata, imsg.data, sizeof(sdata));
				printf("%s: received: %s, size %ld\n", __func__,
								sdata, datalen);
				/* increase message number */
				msgnum++;
				break;
			}

			imsg_free(&imsg);
		}
	}
	printf("%s: receiving done.\n", __func__);
	return (0);
}


int
child(struct imsgbuf *ibuf)
{
	int	idata;
	char	sdata[STRING_LENGTH];

	idata = DATA_INT;
	imsg_compose(ibuf, IMSG_MSG_INT, 0, 0, -1, &idata, sizeof(idata));
	if (msgbuf_write(&ibuf->w) <= 0 && errno != EAGAIN)
		return (-1);
	else
		printf("%s: sent: %d, size %ld\n", __func__, idata,
								sizeof(idata));

	sleep(1);

	snprintf(sdata, sizeof(sdata), "%s", DATA_STRING);
	imsg_compose(ibuf, IMSG_MSG_STRING, 0, 0, -1, sdata, sizeof(sdata));
	if (msgbuf_write(&ibuf->w) <= 0 && errno != EAGAIN)
		return (-1);
	else
		printf("%s: sent: %s, size %ld\n", __func__, sdata,
								sizeof(sdata));

	printf("%s: sending done.\n", __func__);
	return (0);
}

int main(void)
{
	struct imsgbuf	parent_ibuf, child_ibuf;
	int		imsg_fds[2];

	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, imsg_fds) == -1)
		err(1, "socketpair");

	switch (fork()) {
	case -1:
		err(1, "fork");
	case 0:
		/* child */
		close(imsg_fds[0]);
		imsg_init(&child_ibuf, imsg_fds[1]);
		if (child(&child_ibuf) == -1) {
			printf("%s: sending error\n", __func__);
		}
		return (0);
	}

	/* parent */
	close(imsg_fds[1]);
	imsg_init(&parent_ibuf, imsg_fds[0]);
	if (parent(&parent_ibuf) == -1) {
		printf("%s: receiving error\n", __func__);
	}
	return (0);
}

