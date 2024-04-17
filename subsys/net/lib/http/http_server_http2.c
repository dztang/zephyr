/*
 * Copyright (c) 2023, Emna Rekik
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/http/service.h>

LOG_MODULE_DECLARE(net_http_server, CONFIG_NET_HTTP_SERVER_LOG_LEVEL);

#include "headers/server_functions.h"

static const char content_404[] = {
#ifdef INCLUDE_HTML_CONTENT
#include "not_found_page.html.gz.inc"
#endif
};

static bool settings_ack_flag(unsigned char flags)
{
	return (flags & HTTP_SERVER_FLAG_SETTINGS_ACK) != 0;
}

#if 0
/* Disabled for now to avoid warning, as temporarily not used. */
static bool end_headers_flag(unsigned char flags)
{
	return (flags & HTTP_SERVER_FLAG_END_HEADERS) != 0;
}
#endif

static bool end_stream_flag(unsigned char flags)
{
	return (flags & HTTP_SERVER_FLAG_END_STREAM) != 0;
}

static void print_http_frames(struct http_client_ctx *client)
{
#if defined(PRINT_COLOR)
	const char *bold = "\033[1m";
	const char *reset = "\033[0m";
	const char *green = "\033[32m";
	const char *blue = "\033[34m";
#else
	const char *bold = "";
	const char *reset = "";
	const char *green = "";
	const char *blue = "";
#endif

	struct http_frame *frame = &client->current_frame;
	int payload_received_length;

	LOG_DBG("%s=====================================%s", green, reset);
	LOG_DBG("%sReceived %s Frame :%s", bold, get_frame_type_name(frame->type), reset);
	LOG_DBG("  %sLength:%s %u", blue, reset, frame->length);
	LOG_DBG("  %sType:%s %u (%s)", blue, reset, frame->type, get_frame_type_name(frame->type));
	LOG_DBG("  %sFlags:%s %u", blue, reset, frame->flags);
	LOG_DBG("  %sStream Identifier:%s %u", blue, reset, frame->stream_identifier);

	if (client->data_len > frame->length) {
		payload_received_length = frame->length;
	} else {
		payload_received_length = client->data_len;
	}

	LOG_HEXDUMP_DBG(frame->payload, payload_received_length, "Payload");
	LOG_DBG("%s=====================================%s", green, reset);
}

static struct http_stream_ctx *find_http_stream_context(
			struct http_client_ctx *client, uint32_t stream_id)
{
	ARRAY_FOR_EACH(client->streams, i) {
		if (client->streams[i].stream_id == stream_id) {
			return &client->streams[i];
		}
	}

	return NULL;
}

static struct http_stream_ctx *allocate_http_stream_context(
			struct http_client_ctx *client, uint32_t stream_id)
{
	ARRAY_FOR_EACH(client->streams, i) {
		if (client->streams[i].stream_state == HTTP_SERVER_STREAM_IDLE) {
			client->streams[i].stream_id = stream_id;
			client->streams[i].stream_state = HTTP_SERVER_STREAM_OPEN;
			return &client->streams[i];
		}
	}

	return NULL;
}

static int add_header_field(struct http_client_ctx *client, uint8_t **buf,
			    size_t *buflen, const char *name, const char *value)
{
	int ret;

	client->header_field.name = name;
	client->header_field.name_len = strlen(name);
	client->header_field.value = value;
	client->header_field.value_len = strlen(value);

	ret = http_hpack_encode_header(*buf, *buflen, &client->header_field);
	if (ret < 0) {
		return ret;
	}

	*buf += ret;
	*buflen -= ret;

	return 0;
}

static void encode_frame_header(uint8_t *buf, uint32_t payload_len,
				enum http_frame_type frame_type,
				uint8_t flags, uint32_t stream_id)
{
	sys_put_be24(payload_len, &buf[HTTP_SERVER_FRAME_LENGTH_OFFSET]);
	buf[HTTP_SERVER_FRAME_TYPE_OFFSET] = frame_type;
	buf[HTTP_SERVER_FRAME_FLAGS_OFFSET] = flags;
	sys_put_be32(stream_id, &buf[HTTP_SERVER_FRAME_STREAM_ID_OFFSET]);
}

static int send_headers_frame(struct http_client_ctx *client,
			      enum http_status status, uint32_t stream_id,
			      const char *content_encoding)
{
	uint8_t headers_frame[64];
	uint8_t status_str[4];
	uint8_t *buf = headers_frame + HTTP_SERVER_FRAME_HEADER_SIZE;
	size_t buflen = sizeof(headers_frame) - HTTP_SERVER_FRAME_HEADER_SIZE;
	size_t payload_len;
	int ret;

	ret = snprintf(status_str, sizeof(status_str), "%d", status);
	if (ret > sizeof(status_str) - 1) {
		return -EINVAL;
	}

	ret = add_header_field(client, &buf, &buflen, ":status", status_str);
	if (ret < 0) {
		return ret;
	}

	if (content_encoding != NULL) {
		ret = add_header_field(client, &buf, &buflen, "content-encoding",
				       "gzip");
		if (ret < 0) {
			return ret;
		}
	}

	payload_len = sizeof(headers_frame) - buflen - HTTP_SERVER_FRAME_HEADER_SIZE;

	encode_frame_header(headers_frame, payload_len, HTTP_SERVER_HEADERS_FRAME,
			    HTTP_SERVER_FLAG_END_HEADERS, stream_id);

	ret = http_server_sendall(client->fd, headers_frame,
				  payload_len + HTTP_SERVER_FRAME_HEADER_SIZE);
	if (ret < 0) {
		LOG_DBG("Cannot write to socket (%d)", ret);
		return ret;
	}

	return 0;
}

static int send_data_frame(int socket_fd, const char *payload, size_t length,
			   uint32_t stream_id, uint8_t flags)
{
	uint8_t frame_header[HTTP_SERVER_FRAME_HEADER_SIZE];
	int ret;

	encode_frame_header(frame_header, length, HTTP_SERVER_DATA_FRAME,
			    end_stream_flag(flags) ?
			    HTTP_SERVER_FLAG_END_STREAM : 0,
			    stream_id);

	ret = http_server_sendall(socket_fd, frame_header, sizeof(frame_header));
	if (ret < 0) {
		LOG_DBG("Cannot write to socket (%d)", ret);
	} else {
		if (payload != NULL && length > 0) {
			ret = http_server_sendall(socket_fd, payload, length);
			if (ret < 0) {
				LOG_DBG("Cannot write to socket (%d)", ret);
			}
		}
	}

	return ret;
}

int send_settings_frame(struct http_client_ctx *client, bool ack)
{
	uint8_t settings_frame[HTTP_SERVER_FRAME_HEADER_SIZE +
			       2 * sizeof(struct http_settings_field)];
	struct http_settings_field *setting;
	size_t len;
	int ret;

	if (ack) {
		encode_frame_header(settings_frame, 0,
				    HTTP_SERVER_SETTINGS_FRAME,
				    HTTP_SERVER_FLAG_SETTINGS_ACK, 0);
		len = HTTP_SERVER_FRAME_HEADER_SIZE;
	} else {
		encode_frame_header(settings_frame,
				    2 * sizeof(struct http_settings_field),
				    HTTP_SERVER_SETTINGS_FRAME, 0, 0);

		setting = (struct http_settings_field *)
			(settings_frame + HTTP_SERVER_FRAME_HEADER_SIZE);
		UNALIGNED_PUT(htons(HTTP_SETTINGS_HEADER_TABLE_SIZE),
			      &setting->id);
		UNALIGNED_PUT(0, &setting->value);

		setting++;
		UNALIGNED_PUT(htons(HTTP_SETTINGS_MAX_CONCURRENT_STREAMS),
			      &setting->id);
		UNALIGNED_PUT(htonl(CONFIG_HTTP_SERVER_MAX_STREAMS),
			      &setting->value);

		len = HTTP_SERVER_FRAME_HEADER_SIZE +
		      2 * sizeof(struct http_settings_field);
	}

	ret = http_server_sendall(client->fd, settings_frame, len);
	if (ret < 0) {
		LOG_DBG("Cannot write to socket (%d)", ret);
		return ret;
	}

	return 0;
}

static int send_http2_404(struct http_client_ctx *client,
			  struct http_frame *frame)
{
	int ret;

	ret = send_headers_frame(client, HTTP_404_NOT_FOUND,
				 frame->stream_identifier, NULL);
	if (ret < 0) {
		LOG_DBG("Cannot write to socket (%d)", ret);
		return ret;
	}

	ret = send_data_frame(client->fd, content_404, sizeof(content_404),
			      frame->stream_identifier,
			      HTTP_SERVER_FLAG_END_STREAM);
	if (ret < 0) {
		LOG_DBG("Cannot write to socket (%d)", ret);
	}

	return ret;
}

static int handle_http2_static_resource(
	struct http_resource_detail_static *static_detail,
	struct http_frame *frame, struct http_client_ctx *client)
{
	const char *content_200;
	size_t content_len;
	int ret;

	if (!(static_detail->common.bitmask_of_supported_http_methods & BIT(HTTP_GET))) {
		return -ENOTSUP;
	}

	content_200 = static_detail->static_data;
	content_len = static_detail->static_data_len;

	ret = send_headers_frame(client, HTTP_200_OK, frame->stream_identifier,
				 static_detail->common.content_encoding);
	if (ret < 0) {
		LOG_DBG("Cannot write to socket (%d)", ret);
		goto out;
	}

	ret = send_data_frame(client->fd, content_200, content_len,
			      frame->stream_identifier, 0);
	if (ret < 0) {
		LOG_DBG("Cannot write to socket (%d)", ret);
		goto out;
	}

	ret = send_data_frame(client->fd, NULL, 0,
			      frame->stream_identifier,
			      HTTP_SERVER_FLAG_END_STREAM);
	if (ret < 0) {
		LOG_DBG("Cannot send last frame (%d)", ret);
		goto out;
	}

out:
	return ret;
}

static int dynamic_get_req_v2(struct http_resource_detail_dynamic *dynamic_detail,
			      struct http_client_ctx *client)
{
	struct http_frame *frame = &client->current_frame;
	int ret, remaining, offset = dynamic_detail->common.path_len;
	char *ptr;

	ret = send_headers_frame(client, HTTP_200_OK, frame->stream_identifier,
				 dynamic_detail->common.content_encoding);
	if (ret < 0) {
		LOG_DBG("Cannot write to socket (%d)", ret);
		return ret;
	}

	remaining = strlen(&client->url_buffer[dynamic_detail->common.path_len]);

	/* Pass URL to the client */
	while (1) {
		int copy_len, send_len;

		ptr = &client->url_buffer[offset];
		copy_len = MIN(remaining, dynamic_detail->data_buffer_len);

		memcpy(dynamic_detail->data_buffer, ptr, copy_len);

again:
		send_len = dynamic_detail->cb(client,
					      dynamic_detail->data_buffer,
					      copy_len,
					      dynamic_detail->user_data);
		if (send_len > 0) {
			ret = send_data_frame(client->fd,
					      dynamic_detail->data_buffer,
					      send_len,
					      frame->stream_identifier,
					      0);
			if (ret < 0) {
				break;
			}

			offset += copy_len;
			remaining -= copy_len;

			/* If we have passed all the data to the application,
			 * then just pass empty buffer to it.
			 */
			if (remaining == 0) {
				copy_len = 0;
				goto again;
			}

			continue;
		}

		ret = send_data_frame(client->fd, NULL, 0,
				      frame->stream_identifier,
				      HTTP_SERVER_FLAG_END_STREAM);
		if (ret < 0) {
			LOG_DBG("Cannot send last frame (%d)", ret);
		}

		break;
	}

	return ret;
}

static int dynamic_post_req_v2(struct http_resource_detail_dynamic *dynamic_detail,
			       struct http_client_ctx *client)
{
	struct http_frame *frame = &client->current_frame;
	int ret, tmp, remaining;

	if (dynamic_detail == NULL) {
		return -ENOENT;
	}

	ret = send_headers_frame(client, HTTP_200_OK, frame->stream_identifier,
				 dynamic_detail->common.content_encoding);
	if (ret < 0) {
		LOG_DBG("Cannot write to socket (%d)", ret);
		return ret;
	}

	remaining = client->content_len;

	while (1) {
		int copy_len, send_len;

		/* Read all the user data and pass it to application. After
		 * passing all the data, if application returns 0, it means
		 * that there is no more data to send to client.
		 */

		copy_len = MIN(remaining, dynamic_detail->data_buffer_len);
		copy_len = MIN(copy_len, client->data_len);

		if (copy_len > 0) {
			memcpy(dynamic_detail->data_buffer, client->cursor, copy_len);
			remaining -= copy_len;
			client->cursor += copy_len;
			client->data_len -= copy_len;
		} else {
			remaining = 0;
		}

		send_len = dynamic_detail->cb(client,
					      dynamic_detail->data_buffer,
					      copy_len,
					      dynamic_detail->user_data);
		if (send_len > 0) {
			ret = send_data_frame(client->fd,
					      dynamic_detail->data_buffer,
					      send_len,
					      frame->stream_identifier,
					      0);
			if (ret < 0) {
				break;
			}
		} else if (send_len == 0) {
			/* If we do not have anything to send to application,
			 * then just stop.
			 */

			/* Application did not get all the data */
			if (remaining > 0) {
				continue;
			}

			ret = 0;
			break;
		}
	}

	tmp = send_data_frame(client->fd, NULL, 0,
			      frame->stream_identifier,
			      HTTP_SERVER_FLAG_END_STREAM);
	if (tmp < 0) {
		LOG_DBG("Cannot send last frame (%d)", tmp);
		ret = tmp;
	}

	return ret;
}

static int handle_http2_dynamic_resource(
	struct http_resource_detail_dynamic *dynamic_detail,
	struct http_frame *frame, struct http_client_ctx *client)
{
	uint32_t user_method;

	if (dynamic_detail->cb == NULL) {
		return -ESRCH;
	}

	user_method = dynamic_detail->common.bitmask_of_supported_http_methods;

	if (!(BIT(client->method) & user_method)) {
		return -ENOPROTOOPT;
	}

	switch (client->method) {
	case HTTP_GET:
		if (user_method & BIT(HTTP_GET)) {
			return dynamic_get_req_v2(dynamic_detail, client);
		}

		goto not_supported;

	case HTTP_POST:
		/* The data will come in DATA frames. Remember the detail ptr
		 * which needs to be known when passing data to application.
		 */
		if (user_method & BIT(HTTP_POST)) {
			client->current_detail =
				(struct http_resource_detail *)dynamic_detail;
			break;
		}

		goto not_supported;

not_supported:
	default:
		LOG_DBG("HTTP method %s (%d) not supported.",
			http_method_str(client->method),
			client->method);

		return -ENOTSUP;
	}

	return 0;
}

int enter_http2_request(struct http_client_ctx *client)
{
	int ret;

	client->server_state = HTTP_SERVER_FRAME_HEADER_STATE;
	client->data_len -= sizeof(HTTP2_PREFACE) - 1;
	client->cursor += sizeof(HTTP2_PREFACE) - 1;

	/* HTTP/2 client preface received, send server preface
	 * (settings frame).
	 */
	if (!client->preface_sent) {
		ret = send_settings_frame(client, false);
		if (ret < 0) {
			return ret;
		}

		client->preface_sent = true;
	}

	return 0;
}

static int enter_http_frame_settings_state(struct http_client_ctx *client)
{
	client->server_state = HTTP_SERVER_FRAME_SETTINGS_STATE;

	return 0;
}

static int enter_http_frame_data_state(struct http_server_ctx *server,
				       struct http_client_ctx *client)
{
	struct http_frame *frame = &client->current_frame;
	struct http_stream_ctx *stream;

	stream = find_http_stream_context(client, frame->stream_identifier);
	if (!stream) {
		LOG_DBG("|| stream ID ||  %d", frame->stream_identifier);

		stream = allocate_http_stream_context(client, frame->stream_identifier);
		if (!stream) {
			LOG_DBG("No available stream slots. Connection closed.");

			return -ENOMEM;
		}
	}

	client->server_state = HTTP_SERVER_FRAME_DATA_STATE;

	return 0;
}

static int enter_http_frame_headers_state(struct http_server_ctx *server,
					  struct http_client_ctx *client)
{
	struct http_frame *frame = &client->current_frame;
	struct http_stream_ctx *stream;

	stream = find_http_stream_context(client, frame->stream_identifier);
	if (!stream) {
		LOG_DBG("|| stream ID ||  %d", frame->stream_identifier);

		stream = allocate_http_stream_context(client, frame->stream_identifier);
		if (!stream) {
			LOG_DBG("No available stream slots. Connection closed.");

			return -ENOMEM;
		}
	}

	client->server_state = HTTP_SERVER_FRAME_HEADERS_STATE;

	return 0;
}

static int enter_http_frame_continuation_state(struct http_client_ctx *client)
{
	client->server_state = HTTP_SERVER_FRAME_CONTINUATION_STATE;

	return 0;
}

static int enter_http_frame_window_update_state(struct http_client_ctx *client)
{
	client->server_state = HTTP_SERVER_FRAME_WINDOW_UPDATE_STATE;

	return 0;
}

static int enter_http_frame_priority_state(struct http_client_ctx *client)
{
	client->server_state = HTTP_SERVER_FRAME_PRIORITY_STATE;

	return 0;
}

static int enter_http_frame_rst_stream_state(struct http_server_ctx *server,
					     struct http_client_ctx *client)
{
	client->server_state = HTTP_SERVER_FRAME_RST_STREAM_STATE;

	return 0;
}

static int enter_http_frame_goaway_state(struct http_server_ctx *server,
					 struct http_client_ctx *client)
{
	client->server_state = HTTP_SERVER_FRAME_GOAWAY_STATE;

	return 0;
}

int handle_http_frame_header(struct http_server_ctx *server,
			     struct http_client_ctx *client)
{
	int bytes_consumed;
	int parse_result;

	LOG_DBG("HTTP_SERVER_FRAME_HEADER");

	parse_result = parse_http_frame_header(client);
	if (parse_result == 0) {
		return -EAGAIN;
	} else if (parse_result < 0) {
		return parse_result;
	}

	bytes_consumed = HTTP_SERVER_FRAME_HEADER_SIZE;

	client->cursor += bytes_consumed;
	client->data_len -= bytes_consumed;

	switch (client->current_frame.type) {
	case HTTP_SERVER_DATA_FRAME:
		return enter_http_frame_data_state(server, client);
	case HTTP_SERVER_HEADERS_FRAME:
		return enter_http_frame_headers_state(server, client);
	case HTTP_SERVER_CONTINUATION_FRAME:
		return enter_http_frame_continuation_state(client);
	case HTTP_SERVER_SETTINGS_FRAME:
		return enter_http_frame_settings_state(client);
	case HTTP_SERVER_WINDOW_UPDATE_FRAME:
		return enter_http_frame_window_update_state(client);
	case HTTP_SERVER_RST_STREAM_FRAME:
		return enter_http_frame_rst_stream_state(server, client);
	case HTTP_SERVER_GOAWAY_FRAME:
		return enter_http_frame_goaway_state(server, client);
	case HTTP_SERVER_PRIORITY_FRAME:
		return enter_http_frame_priority_state(client);
	default:
		return enter_http_done_state(server, client);
	}

	return 0;
}

/* This feature is theoretically obsoleted in RFC9113, but curl for instance
 * still uses it, so implement as described in RFC7540.
 */
int handle_http1_to_http2_upgrade(struct http_server_ctx *server,
				  struct http_client_ctx *client)
{
	static const char switching_protocols[] =
		"HTTP/1.1 101 Switching Protocols\r\n"
		"Connection: Upgrade\r\n"
		"Upgrade: h2c\r\n"
		"\r\n";
	struct http_frame frame = {
		/* The HTTP/1.1 request that is sent prior to upgrade is
		 * assigned a stream identifier of 1.
		 */
		.stream_identifier = 1
	};
	struct http_resource_detail *detail;
	int path_len;
	int ret;

	ret = http_server_sendall(client->fd, switching_protocols,
				  sizeof(switching_protocols) - 1);
	if (ret < 0) {
		goto error;
	}

	/* The first HTTP/2 frame sent by the server MUST be a server connection
	 * preface.
	 */
	ret = send_settings_frame(client, false);
	if (ret < 0) {
		goto error;
	}

	client->preface_sent = true;

	detail = get_resource_detail(client->url_buffer, &path_len);
	if (detail != NULL) {
		detail->path_len = path_len;

		if (detail->type == HTTP_RESOURCE_TYPE_STATIC) {
			ret = handle_http2_static_resource(
				(struct http_resource_detail_static *)detail,
				&frame, client);
			if (ret < 0) {
				goto error;
			}
		}

		/* TODO: implement POST when using upgrade method */
	} else {
		ret = send_http2_404(client, &frame);
		if (ret < 0) {
			goto error;
		}
	}

	client->server_state = HTTP_SERVER_PREFACE_STATE;
	client->cursor += client->data_len;
	client->data_len = 0;

	return 0;

error:
	return ret;
}

int handle_http_frame_data(struct http_client_ctx *client)
{
	struct http_frame *frame = &client->current_frame;
	int bytes_consumed;
	int ret;

	if (client->data_len < frame->length) {
		return -EAGAIN;
	}

	LOG_DBG("HTTP_SERVER_FRAME_DATA_STATE");

	print_http_frames(client);

	bytes_consumed = client->current_frame.length;

	if (client->current_detail == NULL) {
		/* There is no handler */
		LOG_DBG("No dynamic handler found.");
		(void)send_http2_404(client, frame);
		return -ENOENT;
	}

	ret = dynamic_post_req_v2(
		(struct http_resource_detail_dynamic *)client->current_detail,
		client);
	if (ret < 0 && ret == -ENOENT) {
		ret = send_http2_404(client, frame);
	}

	return ret;
}

int handle_http_frame_headers(struct http_client_ctx *client)
{
	struct http_frame *frame = &client->current_frame;
	struct http_resource_detail *detail;
	int ret, path_len;

	LOG_DBG("HTTP_SERVER_FRAME_HEADERS");

	print_http_frames(client);

	/* TODO This is wrong, we can't expect full frame within the buffer */
	if (client->data_len < frame->length) {
		return -EAGAIN;
	}

	while (frame->length > 0) {
		struct http_hpack_header_buf *header = &client->header_field;

		ret = http_hpack_decode_header(client->cursor, client->data_len,
					       header);
		if (ret == -EAGAIN) {
			goto out;
		}

		if (ret <= 0) {
			return -EBADMSG;
		}

		if (ret > client->current_frame.length) {
			LOG_ERR("Protocol error, frame length exceeded");
			return -EBADMSG;
		}

		client->current_frame.length -= ret;
		client->cursor += ret;
		client->data_len -= ret;

		LOG_DBG("Parsed header: %.*s %.*s", (int)header->name_len,
			header->name, (int)header->value_len, header->value);

		/* For now, we're only interested in method and URL. */
		if (header->name_len == (sizeof(":method") - 1) &&
		    memcmp(header->name, ":method", header->name_len) == 0) {
			/* TODO Improve string to method conversion */
			if (header->value_len == (sizeof("GET") - 1) &&
			    memcmp(header->value, "GET", header->value_len) == 0) {
				client->method = HTTP_GET;
			} else if (header->value_len == (sizeof("POST") - 1) &&
				   memcmp(header->value, "POST", header->value_len) == 0) {
				client->method = HTTP_POST;
			} else {
				/* Unknown method */
				return -EBADMSG;
			}

		} else if (header->name_len == (sizeof(":path") - 1) &&
			   memcmp(header->name, ":path", header->name_len) == 0) {
			if (header->value_len > sizeof(client->url_buffer) - 1) {
				/* URL too long to handle */
				return -ENOBUFS;
			}

			memcpy(client->url_buffer, header->value, header->value_len);
			client->url_buffer[header->value_len] = '\0';

		} else if (header->name_len == (sizeof("content-type") - 1) &&
			   memcmp(header->name, "content-type", header->name_len) == 0) {
			if (header->value_len > sizeof(client->content_type) - 1) {
				/* URL too long to handle */
				return -ENOBUFS;
			}

			memcpy(client->content_type, header->value, header->value_len);
			client->content_type[header->value_len] = '\0';

		} else if (header->name_len == (sizeof("content-length") - 1) &&
			   memcmp(header->name, "content-length", header->name_len) == 0) {
			char len_str[16] = { 0 };
			char *endptr;
			unsigned long len;

			memcpy(len_str, header->value, MIN(sizeof(len_str), header->value_len));
			len_str[sizeof(len_str) - 1] = '\0';

			len = strtoul(len_str, &endptr, 10);
			if (*endptr != '\0') {
				return -EINVAL;
			}

			client->content_len = (size_t)len;
		} else {
			/* Just ignore for now. */
			LOG_DBG("Ignoring field %.*s", (int)header->name_len, header->name);
		}
	}

	/* TODO This should be done only after headers frame is complete, i. e.
	 * current_frame.length == 0.
	 */
	detail = get_resource_detail(client->url_buffer, &path_len);
	if (detail != NULL) {
		detail->path_len = path_len;

		if (detail->type == HTTP_RESOURCE_TYPE_STATIC) {
			ret = handle_http2_static_resource(
				(struct http_resource_detail_static *)detail,
				frame, client);
			if (ret < 0) {
				return ret;
			}
		} else if (detail->type == HTTP_RESOURCE_TYPE_DYNAMIC) {
			ret = handle_http2_dynamic_resource(
				(struct http_resource_detail_dynamic *)detail,
				frame, client);
			if (ret < 0) {
				return ret;
			}
		}

	} else {
		ret = send_http2_404(client, frame);
		if (ret < 0) {
			return ret;
		}
	}

	client->server_state = HTTP_SERVER_FRAME_HEADER_STATE;

out:
	return 0;
}

int handle_http_frame_priority(struct http_client_ctx *client)
{
	struct http_frame *frame = &client->current_frame;
	int bytes_consumed;

	LOG_DBG("HTTP_SERVER_FRAME_PRIORITY_STATE");

	print_http_frames(client);

	if (client->data_len < frame->length) {
		return -EAGAIN;
	}

	bytes_consumed = client->current_frame.length;
	client->data_len -= bytes_consumed;
	client->cursor += bytes_consumed;

	client->server_state = HTTP_SERVER_FRAME_HEADER_STATE;

	return 0;
}

int handle_http_frame_rst_frame(struct http_server_ctx *server, struct http_client_ctx *client)
{
	struct http_frame *frame = &client->current_frame;
	int bytes_consumed;

	LOG_DBG("FRAME_RST_STREAM");

	print_http_frames(client);

	if (client->data_len < frame->length) {
		return -EAGAIN;
	}

	bytes_consumed = client->current_frame.length;
	client->data_len -= bytes_consumed;
	client->cursor += bytes_consumed;

	client->server_state = HTTP_SERVER_FRAME_HEADER_STATE;

	return 0;
}

int handle_http_frame_settings(struct http_client_ctx *client)
{
	struct http_frame *frame = &client->current_frame;
	int bytes_consumed;

	LOG_DBG("HTTP_SERVER_FRAME_SETTINGS");

	print_http_frames(client);

	if (client->data_len < frame->length) {
		return -EAGAIN;
	}

	bytes_consumed = client->current_frame.length;
	client->data_len -= bytes_consumed;
	client->cursor += bytes_consumed;

	if (!settings_ack_flag(frame->flags)) {
		int ret;

		ret = send_settings_frame(client, true);
		if (ret < 0) {
			LOG_DBG("Cannot write to socket (%d)", ret);
			return ret;
		}
	}

	client->server_state = HTTP_SERVER_FRAME_HEADER_STATE;

	return 0;
}

int handle_http_frame_goaway(struct http_server_ctx *server, struct http_client_ctx *client)
{
	struct http_frame *frame = &client->current_frame;
	int bytes_consumed;

	LOG_DBG("HTTP_SERVER_FRAME_GOAWAY");

	print_http_frames(client);

	if (client->data_len < frame->length) {
		return -EAGAIN;
	}

	bytes_consumed = client->current_frame.length;
	client->data_len -= bytes_consumed;
	client->cursor += bytes_consumed;

	enter_http_done_state(server, client);

	return 0;
}

int handle_http_frame_window_update(struct http_client_ctx *client)
{
	struct http_frame *frame = &client->current_frame;
	int bytes_consumed;

	LOG_DBG("HTTP_SERVER_FRAME_WINDOW_UPDATE");

	print_http_frames(client);

	/* TODO Implement flow control, for now just ignore. */

	if (client->data_len < frame->length) {
		return -EAGAIN;
	}

	bytes_consumed = client->current_frame.length;
	client->data_len -= bytes_consumed;
	client->cursor += bytes_consumed;

	client->server_state = HTTP_SERVER_FRAME_HEADER_STATE;

	return 0;
}

int handle_http_frame_continuation(struct http_client_ctx *client)
{
	LOG_DBG("HTTP_SERVER_FRAME_CONTINUATION_STATE");
	client->server_state = HTTP_SERVER_FRAME_HEADERS_STATE;

	return 0;
}

const char *get_frame_type_name(enum http_frame_type type)
{
	switch (type) {
	case HTTP_SERVER_DATA_FRAME:
		return "DATA";
	case HTTP_SERVER_HEADERS_FRAME:
		return "HEADERS";
	case HTTP_SERVER_PRIORITY_FRAME:
		return "PRIORITY";
	case HTTP_SERVER_RST_STREAM_FRAME:
		return "RST_STREAM";
	case HTTP_SERVER_SETTINGS_FRAME:
		return "SETTINGS";
	case HTTP_SERVER_PUSH_PROMISE_FRAME:
		return "PUSH_PROMISE";
	case HTTP_SERVER_PING_FRAME:
		return "PING";
	case HTTP_SERVER_GOAWAY_FRAME:
		return "GOAWAY";
	case HTTP_SERVER_WINDOW_UPDATE_FRAME:
		return "WINDOW_UPDATE";
	case HTTP_SERVER_CONTINUATION_FRAME:
		return "CONTINUATION";
	default:
		return "UNKNOWN";
	}
}

int parse_http_frame_header(struct http_client_ctx *client)
{
	unsigned char *buffer = client->cursor;
	unsigned long buffer_len = client->data_len;
	struct http_frame *frame = &client->current_frame;

	frame->length = 0;
	frame->stream_identifier = 0;

	if (buffer_len < HTTP_SERVER_FRAME_HEADER_SIZE) {
		return 0;
	}

	frame->length = (buffer[HTTP_SERVER_FRAME_LENGTH_OFFSET] << 16) |
			(buffer[HTTP_SERVER_FRAME_LENGTH_OFFSET + 1] << 8) |
			buffer[HTTP_SERVER_FRAME_LENGTH_OFFSET + 2];
	frame->type = buffer[HTTP_SERVER_FRAME_TYPE_OFFSET];
	frame->flags = buffer[HTTP_SERVER_FRAME_FLAGS_OFFSET];
	frame->stream_identifier = (buffer[HTTP_SERVER_FRAME_STREAM_ID_OFFSET] << 24) |
				   (buffer[HTTP_SERVER_FRAME_STREAM_ID_OFFSET + 1] << 16) |
				   (buffer[HTTP_SERVER_FRAME_STREAM_ID_OFFSET + 2] << 8) |
				   buffer[HTTP_SERVER_FRAME_STREAM_ID_OFFSET + 3];
	frame->stream_identifier &= 0x7FFFFFFF;
	frame->payload = buffer + HTTP_SERVER_FRAME_HEADER_SIZE;

	LOG_DBG("Frame len %d type 0x%02x flags 0x%02x id %d",
		frame->length, frame->type, frame->flags, frame->stream_identifier);

	return 1;
}
