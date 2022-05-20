/*
 * friendlist.c - a web-based friend-graph manager
 *
 * Based on:
 *  tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *      GET method to serve static and dynamic content.
 *   Tiny Web server
 *   Dave O'Hallaron
 *   Carnegie Mellon University
 *
 * Scott Crowley (u1178178)
 * CS 4400 - Assignment 6
 * 27 April 2020
 */
#include "csapp.h"
#include "dictionary.h"
#include "more_string.h"

static void doit(int fd);
static void* just_doit(void* connfdp);
static dictionary_t* read_requesthdrs(rio_t* rp);
static void read_postquery(rio_t* rp, dictionary_t* headers, dictionary_t* d);
static void clienterror(int fd, char* cause, char* errnum, char* shortmsg, char* longmsg);
//static void print_stringdictionary(dictionary_t* d);
static void serve_request(int fd, char* body);

static void befriend(int fd, dictionary_t* query);
static void unfriend(int fd, dictionary_t* query);
static void introduce(int fd, dictionary_t* query);
static void get_friends(int fd, dictionary_t* query);

dictionary_t* friend_dict;
pthread_mutex_t lock;


int main(int argc, char** argv) {
	int listenfd, connfd;
	char hostname[MAXLINE], port[MAXLINE];
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;

	/* Check command line args */
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}

	listenfd = Open_listenfd(argv[1]);
	friend_dict = make_dictionary(COMPARE_CASE_SENS, free);
	pthread_mutex_init(&lock, NULL);

	/* Don't kill the server if there's an error, because
	   we want to survive errors due to a client. But we
	   do want to report errors. */
	exit_on_error(0);

	/* Also, don't stop on broken connections: */
	Signal(SIGPIPE, SIG_IGN);

	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
		if (connfd >= 0) {
			Getnameinfo((SA*)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
			//printf("Accepted connection from (%s, %s)\n", hostname, port);

			int* connfd_p  = malloc(sizeof(int));
			*connfd_p = connfd;
			pthread_t thread;
			pthread_create(&thread, NULL, just_doit, connfd_p);
			pthread_detach(thread);
		}
	}
}


/*
 * just_doit - simple driver for doit function below
 */
void* just_doit(void* connfd_p) {
	int connfd = *((int*)connfd_p);
	free(connfd_p);
	doit(connfd);
	close(connfd);
	return NULL;
}


/*
 * doit - handles one HTTP request/response transaction
 */
void doit(int fd) {
	char buf[MAXLINE], * method, * uri, * version;
	rio_t rio;
	dictionary_t* headers, * query;

	/* Read request line and headers */
	Rio_readinitb(&rio, fd);
	if (Rio_readlineb(&rio, buf, MAXLINE) <= 0)
		return;
	//printf("%s", buf);

	if (!parse_request_line(buf, &method, &uri, &version)) {
		clienterror(fd, method, "400", "Bad Request", "Friendlist did not recognize the request");
	}
	else {
		if (strcasecmp(version, "HTTP/1.0") && strcasecmp(version, "HTTP/1.1")) {
			clienterror(fd, version, "501", "Not Implemented", "Friendlist does not implement that version");
		}
		else if (strcasecmp(method, "GET") && strcasecmp(method, "POST")) {
			clienterror(fd, method, "501", "Not Implemented", "Friendlist does not implement that method");
		}
		else {
			headers = read_requesthdrs(&rio);

			/* Parse all query arguments into a dictionary */
			query = make_dictionary(COMPARE_CASE_SENS, free);
			parse_uriquery(uri, query);
			if (!strcasecmp(method, "POST"))
				read_postquery(&rio, headers, query);

			if (starts_with("/friends", uri)) {
				pthread_mutex_lock(&lock);
				get_friends(fd, query);
				pthread_mutex_unlock(&lock);
			}
			else if (starts_with("/befriend", uri)) {
				pthread_mutex_lock(&lock);
				befriend(fd, query);
				pthread_mutex_unlock(&lock);
			}
			else if (starts_with("/unfriend", uri)) {
				pthread_mutex_lock(&lock);
				unfriend(fd, query);
				pthread_mutex_unlock(&lock);
			}
			else if (starts_with("/introduce", uri)) {
				introduce(fd, query);
			}

			/* Clean up */
			free_dictionary(query);
			free_dictionary(headers);
		}

		/* Clean up status line */
		free(method);
		free(uri);
		free(version);
	}
}


/*
 * read_requesthdrs - reads HTTP request headers
 */
dictionary_t* read_requesthdrs(rio_t* rp) {
	char buf[MAXLINE];
	dictionary_t* dict = make_dictionary(COMPARE_CASE_INSENS, free);

	Rio_readlineb(rp, buf, MAXLINE);
	//printf("%s", buf);
	while (strcmp(buf, "\r\n")) {
		Rio_readlineb(rp, buf, MAXLINE);
		//printf("%s", buf);
		parse_header_line(buf, dict);
	}

	return dict;
}


/*
 * read_postquery - reads HTTP POST request
 */
void read_postquery(rio_t* rp, dictionary_t* headers, dictionary_t* dest) {
	char* len_str, * type, * buffer;
	int len;

	len_str = dictionary_get(headers, "Content-Length");
	len = (len_str ? atoi(len_str) : 0);

	type = dictionary_get(headers, "Content-Type");

	buffer = malloc(len + 1);
	Rio_readnb(rp, buffer, len);
	buffer[len] = 0;

	if (!strcasecmp(type, "application/x-www-form-urlencoded")) {
		parse_query(buffer, dest);
	}

	free(buffer);
}


/*
 * ok_header - creates HTTP 200 OK response header
 */
static char* ok_header(size_t len, const char* content_type) {
	char* len_str, * header;

	header = append_strings("HTTP/1.0 200 OK\r\n",
							"Server: Friendlist Web Server\r\n",
							"Connection: close\r\n",
							"Content-length: ", len_str = to_string(len), "\r\n",
							"Content-type: ", content_type, "\r\n\r\n",
							NULL);

	free(len_str);

	return header;
}


/*
 * get_friends - handles '/friends?user=‹user›' request
 */
static void get_friends(int fd, dictionary_t* query) {
	
	if (dictionary_count(query) != 1)
		clienterror(fd, "GET", "400", "Bad Request", "<user> field is required");

	char* user = dictionary_get(query, "user");

	if (user == NULL)
		clienterror(fd, "GET", "400", "Bad Request", "<user> field was null");

	dictionary_t* users_friends = dictionary_get(friend_dict, user);

	char* body;

	if (users_friends == NULL) {
		body = "";
		serve_request(fd, body);
	}
	else {
		const char** friends_arr = dictionary_keys(users_friends);

		body = join_strings(friends_arr, '\n');
		serve_request(fd, body);
	}
}


/*
 * befriend - handles '/befriend?user=‹user›&friends=‹friends›' request
 */
static void befriend(int fd, dictionary_t* query) {

	if (query == NULL) {
		clienterror(fd, "POST", "400", "Bad Request", "query was null");
		return;
	}

	if (dictionary_count(query) != 2) {
		clienterror(fd, "POST", "400", "Bad Request", "query requires two fields: <user> & <friends>");
		return;
	}

	char* user = dictionary_get(query, "user");

	if (user == NULL) {
		dictionary_t* new_user = make_dictionary(COMPARE_CASE_SENS, free);
		dictionary_set(friend_dict, user, new_user);
	}

	dictionary_t* users_friends = dictionary_get(friend_dict, user);

	if (users_friends == NULL) {
		users_friends = make_dictionary(COMPARE_CASE_SENS, free);
		dictionary_set(friend_dict, user, users_friends);
	}
	
	char** friends_arr = split_string((char*)dictionary_get(query, "friends"), '\n');

	int idx;
	for (idx = 0; friends_arr[idx] != NULL; ++idx) {
		if (strcmp(friends_arr[idx], user) == 0)
			continue;

		dictionary_t* user_dict = dictionary_get(friend_dict, user);

		if (user_dict == NULL) {
			user_dict = make_dictionary(COMPARE_CASE_SENS, free);
			dictionary_set(friend_dict, user, user_dict);
		}

		if (dictionary_get(user_dict, friends_arr[idx]) == NULL)
			dictionary_set(user_dict, friends_arr[idx], NULL);

		dictionary_t* next_friend = dictionary_get(friend_dict, friends_arr[idx]);

		if (next_friend == NULL) {
			next_friend = make_dictionary(COMPARE_CASE_SENS, free);
			dictionary_set(friend_dict, friends_arr[idx], next_friend);
		}
		
		if (dictionary_get(next_friend, user) == NULL)
			dictionary_set(next_friend, user, NULL);
	}
	
	const char** users_friends_arr = dictionary_keys(users_friends);

	char* body = join_strings(users_friends_arr, '\n');

	serve_request(fd, body);
}


/*
 * unfriend - handles '/unfriend?user=‹user›&friends=‹friends›' request
 */
static void unfriend(int fd, dictionary_t* query) {

	if (query == NULL) {
		clienterror(fd, "POST", "400", "Bad Request", "query was null");
		return;
	}

	if (dictionary_count(query) != 2) {
		clienterror(fd, "POST", "400", "Bad Request", "query requires two fields: <user> & <friends>");
		return;
	}

	char* user = dictionary_get(query, "user");

	if (user == NULL) {
		clienterror(fd, "POST", "400", "Bad Request", "<user> field was null");
		return;
	}

	dictionary_t* users_friends = dictionary_get(friend_dict, user);

	if (users_friends == NULL) {
		clienterror(fd, "POST", "400", "Bad Request", "<user> field was invalid");
		return;
	}

	char** remove_arr = split_string((char*)dictionary_get(query, "friends"), '\n');

	if (remove_arr == NULL) {
		clienterror(fd, "GET", "400", "Bad Request", "<friends> field was null");
		return;
	}
	
	int idx;
	for (idx = 0; remove_arr[idx] != NULL; ++idx) {	
		dictionary_remove(users_friends, remove_arr[idx]);
		
		dictionary_t* removed_friend = dictionary_get(friend_dict, remove_arr[idx]);

		if (removed_friend != NULL)
			dictionary_remove(removed_friend, user);
	}
	
	const char** users_friends_arr = dictionary_keys(users_friends);

	char* body = join_strings(users_friends_arr, '\n');
	
	serve_request(fd, body);
}


/*
 * introduce - handles '/introduce?user=‹user›&friend=‹friend›&host=‹host›&port=‹port›' request
 */
static void introduce(int fd, dictionary_t* query) {

	if (dictionary_count(query) != 4) {
		clienterror(fd, "POST", "400", "Bad Request", "query requires four fields: <user>, <friends>, <host>, & <port>");
		return;
	}

	char* user = dictionary_get(query, "user");
	char* friend = dictionary_get(query, "friend");
	char* host = dictionary_get(query, "host");
	char* port = dictionary_get(query, "port");

	if (user == NULL || friend == NULL || host == NULL || port == NULL) {
		clienterror(fd, "POST", "400", "Bad Request", "one or more field(s) was null");
		return;
	}
	
	int connfd = Open_clientfd(host, port);
	char buffer[MAXBUF];
	sprintf(buffer, "GET /friends?user=%s HTTP/1.1\r\n\r\n", query_encode(friend));
	Rio_writen(connfd, buffer, strlen(buffer));
	shutdown(connfd, SHUT_WR);

	rio_t rio;
	char buf[MAXLINE];
	Rio_readinitb(&rio, connfd);
	if (Rio_readlineb(&rio, buf, MAXLINE) <= 0)
		clienterror(fd, "POST", "400", "Bad Request", "target server error");

	char* status;
	char* version;
	char* desc;

	if (!parse_status_line(buf, &version, &status, &desc))
		clienterror(fd, "GET", "400", "Bad Request", "target server error");

	else {
		if (strcasecmp(version, "HTTP/1.0") && strcasecmp(version, "HTTP/1.1"))
			clienterror(fd, version, "501", "Not Implemented", "version not implemented");

		else if (strcasecmp(status, "200") && strcasecmp(desc, "OK"))
			clienterror(fd, status, "501", "Not Implemented", "did not receive 200 OK response from target server");
		
		else {
			dictionary_t* headers = read_requesthdrs(&rio);

			char* len_str = dictionary_get(headers, "Content-length");
			int len = (len_str ? atoi(len_str) : 0);

			char buff[len];

			if (len <= 0)
				clienterror(fd, "GET", "400", "Bad Request", "target server did not provide any friends");

			else {
				Rio_readnb(&rio, buff, len);
				buff[len] = 0;

				pthread_mutex_lock(&lock);
				dictionary_t* users_friends = dictionary_get(friend_dict, user);

				if (users_friends == NULL) {
					users_friends = make_dictionary(COMPARE_CASE_SENS, NULL);
					dictionary_set(friend_dict, user, users_friends);
				}

				char** friends_arr = split_string(buff, '\n');

				int idx;
				for (idx = 0; friends_arr[idx] != NULL; ++idx) {
					if (strcmp(friends_arr[idx], user) == 0)
						continue;

					dictionary_t* user_dict = dictionary_get(friend_dict, user);

					if (user_dict == NULL) {
						user_dict = make_dictionary(COMPARE_CASE_SENS, free);
						dictionary_set(friend_dict, user, user_dict);
					}
					
					if (dictionary_get(user_dict, friends_arr[idx]) == NULL)
						dictionary_set(user_dict, friends_arr[idx], NULL);

					dictionary_t* next_friend = dictionary_get(friend_dict, friends_arr[idx]);

					if (next_friend == NULL) {
						next_friend = make_dictionary(COMPARE_CASE_SENS, free);
						dictionary_set(friend_dict, friends_arr[idx], next_friend);
					}
					
					if (dictionary_get(next_friend, user) == NULL)
						dictionary_set(next_friend, user, NULL);

					free(friends_arr[idx]);
				}
				free(friends_arr);

				const char** users_friends_arr = dictionary_keys(users_friends);
				pthread_mutex_unlock(&lock);

				char* body = join_strings(users_friends_arr, '\n');
				serve_request(fd, body);
				free(body);
			}
		}
	}
	free(version);
	free(status);
	free(desc);
	close(connfd);
}


/*
 * serve_request - sends server response to client
 */
static void serve_request(int fd, char* body) {
	size_t len = strlen(body);
	char* header;

	/* Send response headers to client */
	header = ok_header(len, "text/html; charset=utf-8");
	Rio_writen(fd, header, strlen(header));
	//printf("Response headers:\n");
	//printf("%s", header);

	free(header);

	/* Send response body to client */
	Rio_writen(fd, body, len);
}

/*
 * clienterror - returns an error message to the client
 */
void clienterror(int fd, char* cause, char* errnum, char* shortmsg, char* longmsg) {
	size_t len;
	char* header;
	char* body;
	char* len_str;

	body = append_strings("<html><title>Friendlist Error</title>",
						  "<body bgcolor=""ffffff"">\r\n",
						  errnum, " ", shortmsg,
						  "<p>", longmsg, ": ", cause,
						  "<hr><em>Friendlist Server</em>\r\n", NULL);
	len = strlen(body);

	/* Print the HTTP response */
	header = append_strings("HTTP/1.0 ", errnum, " ", shortmsg, "\r\n",
							"Content-type: text/html; charset=utf-8\r\n",
							"Content-length: ", len_str = to_string(len), "\r\n\r\n", NULL);
	free(len_str);

	Rio_writen(fd, header, strlen(header));
	Rio_writen(fd, body, len);

	free(header);
	free(body);
}


/*
 * print_stringdictionary - prints a dictionary to console for debugging
 */
//static void print_stringdictionary(dictionary_t* d) {
//	int i, count;
//
//	count = dictionary_count(d);
//	for (i = 0; i < count; i++) {
//		printf("%s=%s\n",
//			dictionary_key(d, i),
//			(const char*)dictionary_value(d, i));
//	}
//	printf("\n");
//}
