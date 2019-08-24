#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <modbus/modbus.h>
#include <curl/curl.h>

int main (void) {
    char USB_SERIAL[10], HOST[100], API_KEY[100], url[500],  val[100];
    char s[150];// = NULL;
    modbus_t *ctx;
    uint16_t tab_reg[64];
    int rc;

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
if(curl) {
curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);
curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);
}

while(1){

 rc = modbus_read_input_registers(ctx, 0, 10, tab_reg);
if (rc == -1) {
    fprintf(stderr, "%s\n", modbus_strerror(errno));
    //return -1;
}
if (rc != -1) {

sprintf(s, "{\"current\": %f, \"power\": %f, \"voltage\": %f, \"frequency\": %f, \"energy\": %d, \"powerfactor\": %f}", tab_reg[1]*0.001, tab_reg[3]*0.1, tab_reg[0]*0.1, tab_reg[7]*0.1, tab_reg[5], tab_reg[8]*0.0);

if(curl) {
char *output = curl_easy_escape(curl, s, strlen(s));
sprintf(url, "%s/emoncms/input/post?node=emontx&fulljson=%s&apikey=%s", HOST, output, API_KEY);
if(output) {
	curl_free(output);
}

curl_easy_setopt(curl, CURLOPT_URL, url);
curl_easy_perform(curl);
}
}
sleep(1);
}
curl_easy_cleanup(curl);
curl_global_cleanup();
modbus_close(ctx);
modbus_free(ctx);
    return 0;
}
