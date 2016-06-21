#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <curl/curl.h>
#include <string.h>


int main(int argc, char *argv[])

{
	CURL *curl;
 	CURLcode res;

	char sql[255];
	char str[100];
		
	strcpy (sql, "{\"stmt\":\"delete from mag where magTime < current_timestamp - 100000\"}");
	//printf (sql);
        
    	curl = curl_easy_init();
   		if(curl) {
    	curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.132:4200/_sql?pretty");
    	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sql);
    	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(sql));
    	FILE* devnull = fopen("nul", "w"); 
    	//curl_easy_setopt(curl, CURLOPT_WRITEDATA, devnull);
 
    	res = curl_easy_perform(curl);
   	 	/* Check for errors */ 
    	if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }

}