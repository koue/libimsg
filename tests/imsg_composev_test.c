/*
 * Copyright (c) 2018-2026 Nikola Kolev <koue@chaosophia.net>
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

#include <assert.h>
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

#define DATA_INT	24
#define DATA_STRING	"This is my message"
#define MSGNUM		1

enum imsg_type {
	IMSG_MSG_IOV
};

#define IOV_NUM		2

int
parent(struct imsgbuf *imsgbuf)
{
	struct imsg	imsg;
	uint32_t	idata, n, msgnum = 0;
	char		*sdata;

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

			msgnum++;

			switch (imsg_get_type(&imsg)) {
			case IMSG_MSG_IOV:
				memcpy(&idata, imsg.data, sizeof(idata));
				sdata = (char *)imsg.data + sizeof(idata);
				assert(idata == DATA_INT);
				assert(strcmp(sdata, DATA_STRING) == 0);
				printf("%s: received: %d, %s\n", __func__,
				    idata, sdata);
				break;
			default:
				printf("%s: wrong type.\n", __func__);
				break;
			}
			imsg_free(&imsg);
		}
	}
	printf("%s: receiving done.\n", __func__);
	imsgbuf_clear(imsgbuf);
	return (0);
}


int
child(struct imsgbuf *imsgbuf)
{
	struct iovec	iov[IOV_NUM];

	int	idata;
	char	*sdata;

	idata = DATA_INT;
	sdata = DATA_STRING;

	iov[0].iov_base = &idata;
	iov[0].iov_len = sizeof(idata);
	iov[1].iov_base = sdata;
	iov[1].iov_len = strlen(sdata) + 1;

	imsg_composev(imsgbuf, IMSG_MSG_IOV, 0, 0, -1, iov, IOV_NUM);

	if (imsgbuf_write(imsgbuf) == -1) {
		if (errno == EPIPE) {
			/* handle closed connection */
			return (-1);
		} else {
			/* handle write failure */
			return (-1);
		}
	}
	printf("%s: sending done.\n", __func__);
	imsgbuf_clear(imsgbuf);
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
