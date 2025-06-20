/*
 * Copyright (c) 2018-2025 Nikola Kolev <koue@chaosophia.net>
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
parent(struct imsgbuf *imsgbuf)
{
	struct imsg	imsg;
	unsigned long   datalen;
	int		idata, n = 0, msgnum = 0;
	char		sdata[STRING_LENGTH];

	for (;msgnum < MSGNUM;) {
		switch (imsgbuf_read(imsgbuf)) {
		case -1:
			/* handle read error */
			break;
		case 0:
			/* handle closed connection */
			break;
		}

		for (;;) {
			if ((n = imsgbuf_get(imsgbuf, &imsg)) == -1) {
				/* handle read error */
				printf("%s: imsgbuf_get error\n", __func__);
				exit(1);
			}
			if (n == 0)	/* no more messages */
				break;

			datalen = imsg.hdr.len - IMSG_HEADER_SIZE;

			switch (imsg_get_type(&imsg)) {
			case IMSG_MSG_INT:
				if (imsg_get_data(&imsg, &idata, sizeof(idata)) == -1) {
					/* handle corrupt message */
					printf("%s: datalen error\n", __func__);
					break;
				}
				/* handle message received */
				memcpy(&idata, imsg.data, sizeof(idata));
				printf("%s: received: %d, size %ld\n", __func__, idata, datalen);
				msgnum++;
				break;
			case IMSG_MSG_STRING:
				if (imsg_get_data(&imsg, sdata, sizeof(sdata)) == -1) {
					/* handle corrupt message */
					printf("%s: datalen error\n", __func__);
					break;
				}
				/* handle message received */
				memcpy(sdata, imsg.data, sizeof(sdata));
				printf("%s: received: %s, size %ld\n", __func__, sdata, datalen);
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
child(struct imsgbuf *imsgbuf)
{
	int	idata;
	char	sdata[STRING_LENGTH];

	idata = DATA_INT;
	imsg_compose(imsgbuf, IMSG_MSG_INT, 0, 0, -1, &idata, sizeof(idata));
	if (imsgbuf_write(imsgbuf) == -1) {
		if (errno == EPIPE) {
			/* handle closed connection */
			return (-1);
		} else {
			/* handle write failure */
			return (-1);
		}
	}
	printf("%s: sent: %d, size %ld\n", __func__, idata, sizeof(idata));

	sleep(1);

	snprintf(sdata, sizeof(sdata), "%s", DATA_STRING);
	imsg_compose(imsgbuf, IMSG_MSG_STRING, 0, 0, -1, sdata, sizeof(sdata));
	if (imsgbuf_write(imsgbuf) == -1) {
		if (errno == EPIPE) {
			/* handle closed connection */
			return (-1);
		} else {
			/* handle write failure */
			return (-1);
		}
	}
	printf("%s: sent: %s, size %ld\n", __func__, sdata, sizeof(sdata));

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
		if (imsgbuf_init(&child_ibuf, imsg_fds[1]) == -1)
			err(1, NULL);
		if (child(&child_ibuf) == -1) {
			printf("%s: sending error\n", __func__);
		}
		return (0);
	}

	/* parent */
	close(imsg_fds[1]);
	if (imsgbuf_init(&parent_ibuf, imsg_fds[0]) == -1)
		err(1, NULL);
	if (parent(&parent_ibuf) == -1) {
		printf("%s: receiving error\n", __func__);
	}
	return (0);
}
