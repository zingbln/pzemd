#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
//#include <pthread.h>

#include <modbus/modbus.h>
#include <curl/curl.h>

//#define NUMT 1

/*static void *bufferupload(int *buffer, int *i) {
    int k;
    for ( k=0; k=i;  k++) {
        sprintf(s, "{\"current\": %f, \"power\": %f, \"voltage\": %f, \"frequency\": %f, \"energy\": %d, \"powerfactor\": %f}", buff[k][1]*0.001, buff[k][2]*0.1, buff[k][3]*0.1, buff[k][4]*0.1, buff[k][5], buff[k][6]*0.0);
        if(curl) {
        char *output = curl_easy_escape(curl, s1, strlen(s1));
        sprintf(url, "%s/emoncms/input/post?node=emontx&bulk.json?data==%s&apikey=%s", HOST, output, API_KEY);
        if(output) {
	    curl_free(output);
        }
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_perform(curl);
       }
    }
}
*/

int main (void) {
    char USB_SERIAL[10], HOST[100], API_KEY[100], url[8000],  val[100];
    char s[7000], p[7000];// = NULL;
    modbus_t *ctx;
    uint16_t tab_reg[64];
    int rc;
    int i,k;
    long response_code;
    int buff[30][7];
    //pthread_t tid[NUMT]

FILE* ptr = fopen("pzemd.cfg","r"); 
if (ptr==NULL) { 
    printf("no such file pzemd.cfg.\n"); 
    return 0; 
    } 

if (fscanf(ptr, " USBSERIAL=%s ", val) == 1) {
    strcpy(USB_SERIAL, val); 
} else {
     printf("Please configure your USB Serial Port in pzemd.cfg\n"); 
    return 0; 
} 
if (fscanf(ptr, " HOST=%s", val) == 1)  {
    strcpy(HOST, val); 
} else {
     printf("Please configure your Emoncms Host in pzemd.cfg\n"); 
    return 0; 
}
if (fscanf(ptr, " APIKEY=%s", val) == 1) {
    strcpy(API_KEY, val); 
}else {
     printf("Please configure your Emoncms Api Key in pzemd.cfg\n"); 
    return 0; 
}
fclose(ptr);

 ctx = modbus_new_rtu(USB_SERIAL, 9600, 'N', 8, 1);
if (modbus_connect(ctx) == -1) {
	fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
	modbus_free(ctx);
	return -1;
	}	

rc = modbus_set_slave(ctx, 0x01);
    if (rc == -1) {
      fprintf(stderr, "Set slave error: %s\n", modbus_strerror(errno));
      return -1;
    } else {
      fprintf(stderr, "Slave set to 0x01\n");
    }

curl_global_init(CURL_GLOBAL_ALL);
CURL *curl = curl_easy_init();
CURLcode res;
curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);
curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);
i=0;
//int (*buff)[3600] = malloc(sizeof(int[7][3600]));
while(1){

 rc = modbus_read_input_registers(ctx, 0, 10, tab_reg);
if (rc == -1) {
    fprintf(stderr, "%s\n", modbus_strerror(errno));
    //return -1;
}
if (rc != -1) {

/*sprintf(s, "{\"current\": %f, \"power\": %f, \"voltage\": %f, \"frequency\": %f, \"energy\": %d, \"powerfactor\": %f}", tab_reg[1]*0.001, tab_reg[3]*0.1, tab_reg[0]*0.1, tab_reg[7]*0.1, tab_reg[5], tab_reg[8]*0.0);

if(curl) {
char *output = curl_easy_escape(curl, s, strlen(s));
sprintf(url, "%s/emoncms/input/post?node=emontx&fulljson=%s&apikey=%s", HOST, output, API_KEY);
if(output) {
	curl_free(output);
}
curl_easy_setopt(curl, CURLOPT_URL, url);
curl_easy_perform(curl);

}*/
//if (i > 100) {
//    int (*buff)[i] = realloc(buff, sizeof(int[7][i]));
//}

buff[i][0] = (int)time(NULL);
buff[i][1] = tab_reg[1];
buff[i][2] = tab_reg[3]; 
buff[i][3] = tab_reg[0]; 
buff[i][4] = tab_reg[7]; 
buff[i][5] = tab_reg[5];
buff[i][6] = tab_reg[8];
printf("%d %d %d %d %d %d \n",buff[i][0],  buff[i][1], buff[i][2], buff[i][3], buff[i][4], buff[i][5] );
//printf("%d \n" , sizeof(int[7][i]));

i++;
/*if (i==1) {
    sprintf(s, "{\"current\": %f, \"power\": %f, \"voltage\": %f, \"frequency\": %f, \"energy\": %d, \"powerfactor\": %f}", tab_reg[1]*0.001, tab_reg[3]*0.1, tab_reg[0]*0.1, tab_reg[7]*0.1, tab_reg[5], tab_reg[8]*0.0);
    if(curl) {
        char *output = curl_easy_escape(curl, s, strlen(s));
        sprintf(url, "%s/emoncms/input/post?node=emontx&fulljson=%s&apikey=%s", HOST, output, API_KEY);
        if(output) {
	        curl_free(output);
        }
        res = curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_perform(curl);
        }
        if(res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        printf("%ld \n", response_code);
        if(response_code == 200) {
            i=0;
        }
        }
}
*/
if (i>=2) {
    if(curl) {
        strcpy(s, "[");
    //https://e.com/input/bulk.json?data=[[0,1,{"power":10, "voltage": 240}],[10,1,{"power":11, "voltage": 240.2}]]
    for ( k=0; k<=i-1;  k++) {
        //printf("%d, %d, %d, %d, %d, %d",buff[k][1],  buff[k][2], buff[k][3], buff[k][4], buff[k][5], buff[k][6] );
        sprintf(p, "[%d, \"emontx\",{\"current\":%f},{\"power\":%f},{\"voltage\":%f},{\"frequency\":%f},{\"energy\":%d},{\"powerfactor\":%f}]", buff[k][0]-buff[i-1][0], buff[k][1]*0.001, buff[k][2]*0.1, buff[k][3]*0.1, buff[k][4]*0.1, buff[k][5], buff[k][6]*0.0);   

    strcat(s,p);
    if (k<i-1){strcat(s, ",");}
    }
    strcat(s, "]");
    printf("%s", s);
        char *output = curl_easy_escape(curl, s, strlen(s));
        sprintf(url, "%s/emoncms/input/bulk.json?data=%s&&time=%d&apikey=%s", HOST,  output, buff[i-1][0], API_KEY);
        if(output) {
	    curl_free(output);
        }
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_perform(curl);
        //usleep(200000);
       

        if(res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        printf("%ld \n", response_code);
        if(response_code == 200) {
            i=0;
        }

}
}
}
}
sleep(1);
}
free(buff);
curl_easy_cleanup(curl);
curl_global_cleanup();
modbus_close(ctx);
modbus_free(ctx);
    return 0;
}
