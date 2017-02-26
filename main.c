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

size_t	handle_token_response(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	json_t	*json_obj;
	json_t	*token_obj;
	char	*token_str;

	json_obj = json_loadb(ptr, size * nmemb, JSON_DECODE_ANY, NULL);
	token_obj = json_object_get(json_obj, "access_token");
	token_str = (char *)json_string_value(token_obj);
	ft_memmove(userdata, token_str, 64);
	free(json_obj);
	free(token_obj);
	free(token_str);
	return (size * nmemb);
}

char	*get_token(CURL *curl, char *uid, char *secret)
{
	char		*token;
	CURLcode	res;

	/* Malloc space for token */
	if (!(token = (char *)ft_memalloc(65)))
		return (0);
	/* get a curl handle */
	curl = curl_easy_init();
	if(curl) {
		/* First set the URL that is about to receive our POST. This URL can
		   just as well be a https:// URL if that is what should receive the
		   data. */
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.intra.42.fr/oauth/token");
		char *params = (char *)ft_memalloc(184);
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

size_t	handle_response(char *ptr, size_t size, size_t nmemb, void **userdata)
{
	void	*delete_me;
	char	*c_str;

	delete_me = *userdata;
	c_str = ft_strnew(size * nmemb);
	ft_memmove(c_str, ptr, size * nmemb);
	*userdata = ft_strjoin(*userdata, c_str);
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
		/* Parse Response */
		json_obj = json_loadb(json_response, ft_strlen(json_response), JSON_DECODE_ANY, NULL);
		session_obj = json_array_get(json_obj, 0);
		loc_obj = json_object_get(session_obj, "host");
		loc_str = (char *)json_string_value(loc_obj);
		/* Malloc location data */
		if (!(location = (char *)ft_memalloc(ft_strlen(loc_str) + 1)))
			return (0);
		ft_strcpy(location, loc_str);
		/* Check for errors */
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		/* always cleanup */
		free(url);
		free(loc_str);
		free(loc_obj);
		free(json_obj);
		free(session_obj);
		free(json_response);
		curl_easy_cleanup(curl);
	}
	return (location);
}

int		main(int argc, char** argv)
{
	int		fd;
	char	*uid;
	char	*secret;
	char	*token;
	char	*location;
	char	*output;
	CURL	*curl;

	curl_global_init(CURL_GLOBAL_ALL);
	if ((fd = open("creds", O_RDONLY)) == -1)
	{
		ft_putstr("bad credentials!\n");
		exit(1);
	}
	get_next_line(fd, &uid);
	get_next_line(fd, &secret);
	token = get_token(curl, uid, secret);
	output = (char *)ft_memalloc(500);
	location = get_user_location(curl, token, argv[1]);
	sprintf(output, "%s is located at %s\n", argv[1], location);
	write(1, output, ft_strlen(output));
	curl_global_cleanup();
	free(uid);
	free(token);
	free(output);
	free(secret);
	free(location);
	return (0);
}
