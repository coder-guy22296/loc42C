/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/02/25 15:42:44 by cyildiri          #+#    #+#             */
/*   Updated: 2017/02/25 15:42:45 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <unistd.h>
#include "libft.h"
#include <fcntl.h>
#include "curl/curl.h"
#include "jansson.h"
#include <pthread.h>

size_t	handle_token_response(char *ptr, size_t size, size_t nmemb, void *token)
{
	json_t	*json_obj;
	json_t	*token_obj;
	char	*token_str;

	json_obj = json_loadb(ptr, size * nmemb, JSON_DECODE_ANY, NULL);
	token_obj = json_object_get(json_obj, "access_token");
	token_str = (char *)json_string_value(token_obj);
	ft_memmove(token, token_str, 64);
	free(json_obj);
	free(token_obj);
	free(token_str);
	return (size * nmemb);
}

char	*get_token(CURL *curl, char *uid, char *secret)
{
	char		*params;
	char		*token;
	CURLcode	res;

	/* Malloc space for token */
	token = (char *)ft_memalloc(65);
	params = (char *)ft_memalloc(184);
	if (!token || !params)
		return (0);
	/* get a curl handle */
	curl = curl_easy_init();
	if(curl) {
		/* First set the URL that is about to receive our POST. This URL can
		   just as well be a https:// URL if that is what should receive the
		   data. */
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.intra.42.fr/oauth/token");
		sprintf(params, "grant_type=client_credentials&client_id=%s&client_secret=%s", uid, secret);
		/* Now specify the POST data */
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);
		/* Don't Verify Cert */
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		/* Set the response callback */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_token_response);
		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)token);
		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		/* always cleanup */
		free(params);
		curl_easy_cleanup(curl);
	}
	return (token);
}

size_t	handle_response(char *ptr, size_t size, size_t nmemb, void **response)
{
	void	*delete_me;
	char	*c_str;

	delete_me = *response;
	c_str = ft_strnew(size * nmemb);
	ft_memmove(c_str, ptr, size * nmemb);
	*response = ft_strjoin(*response, c_str);
	free(delete_me);
	free(c_str);
	return (size * nmemb);
}

char	*get_user_location(CURL *curl, char *token, char *username)
{
	char		*json_response;
	json_t		*json_obj;
	json_t		*session_obj;
	json_t		*loc_obj;
	char		*loc_str;
	char		*location;
	CURLcode	res;

	/* Malloc empty string for response to be appended */
	if (!(json_response = (char *)ft_memalloc(1)))
		return (0);
	/* get a curl handle */
	curl = curl_easy_init();
	if (curl)
	{
		/* Assemble GET URL */
		char *url = (char *)ft_memalloc(500);
		sprintf(url, "https://api.intra.42.fr/v2/users/%s/locations?access_token=%s", username, token);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		/* Don't Verify Cert */
    	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		/* Set the response callback */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_response);
		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&json_response);
		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		/* Parse Response */
		json_obj = json_loadb(json_response, ft_strlen(json_response), JSON_DECODE_ANY, NULL);
		session_obj = json_array_get(json_obj, 0);
		if (json_is_null(json_object_get(session_obj, "end_at")))
		{
			loc_obj = json_object_get(session_obj, "host");
			loc_str = (char *)json_string_value(loc_obj);
			/* Malloc location data */
			if (!(location = ft_strdup(loc_str)))
				return (0);
			free(loc_str);
			free(loc_obj);
		}
		else
			location = 0;
		/* always cleanup */
		free(url);
		free(json_obj);
		free(session_obj);
		free(json_response);
		curl_easy_cleanup(curl);
	}
	return (location);
}

void	load_credentials(char *filename, char **uid, char **secret)
{
	int		fd;
	if ((fd = open(filename, O_RDONLY)) == -1)
	{
		ft_putstr("bad credentials!\n");
		exit(1);
	}
	get_next_line(fd, uid);
	get_next_line(fd, secret);
	close(fd);
}

void	load_user_list(char *filename, t_list **users, int *usr_cnt)
{
	int		fd;
	char 	*line;

	*usr_cnt = 0;
	if ((fd = open(filename, O_RDONLY)) == -1)
	{
		ft_putstr("bad users file!\n");
		exit(1);
	}
	while(get_next_line(fd, &line) == 1)
	{
		ft_lstaddend(users, ft_lstnew(ft_strdup(line), ft_strlen(line) + 1));
//		ft_putstr("user: ");
//		ft_putstr(line);
//		ft_putstr(" len: ");
//		ft_putnbr(ft_strlen(line));
//		ft_putstr("\n");
		//ft_bzero(line, ft_strlen(line));
		free(line);
		(*usr_cnt)++;
	}
	close(fd);
}

typedef struct	s_thread_data
{
	char *token;
	char *username;
}				t_thread_data;

void *thread_job(void *vargp)
{
	t_thread_data *params;
	char	*location;
	char	*output;
	CURL	*curl;
	size_t	out_len;

	params = (t_thread_data *)vargp;
	location = get_user_location(curl, params->token, params->username);
	if (location)
	{
		out_len = ft_strlen(params->username) + ft_strlen(location) + 17;
		output = (char *)ft_memalloc(out_len);
		sprintf(output, "%s is located at %s\n", params->username, location);
		write(1, output, out_len - 1);
	}
	else
	{
		ft_putchar('*');
		ft_putstr(params->username);
		ft_putstr("'s location is not availible\n");
	}
	free(output);
	free(location);
	free(params->username);
	return NULL;
}

int		main(int argc, char** argv)
{
	int		usr_cnt;
	char	*uid;
	char	*secret;
	char	*token;
	t_list	*users;
	t_list	*head;
	CURL	*curl;
	pthread_t *tids;
	t_thread_data *params;
	int i;

	curl_global_init(CURL_GLOBAL_ALL);
	load_credentials("creds", &uid, &secret);
	token = get_token(curl, uid, secret);
	ft_bzero(secret, ft_strlen(secret));
	free(uid);
	free(secret);
	load_user_list(argv[1], &users, &usr_cnt);
	tids = (pthread_t *)ft_memalloc(sizeof(pthread_t) * usr_cnt);
	params = (t_thread_data *)ft_memalloc(sizeof(t_thread_data) * usr_cnt);
	if (!tids || !params)
		return (0);
	ft_putstr("============================================================\n");
	head = users;
	i = 0;
	while (head)
	{
		params[i].token = token;
		params[i].username = (char *)head->content;
		if(pthread_create(&tids[i], NULL, thread_job, (void *)&params[i]))
		{
			ft_putstr("thread failed!\n");
			exit(1);
		}
		i++;
		head = head->next;
	}
	i = 0;
	while (i < usr_cnt)
		pthread_join(tids[i++], NULL);
	ft_putstr("============================================================\n");
	curl_global_cleanup();
	free(token);
	return (0);
}
