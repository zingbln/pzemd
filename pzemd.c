#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <modbus/modbus.h>
#include <jansson.h>
#include <curl/curl.h>

int main (void) {
    char USB_SERIAL[10];
    char HOST[100];
    char API_KEY[100];
    char url[500];
    char* s = NULL;
    json_t *config;
    json_error_t error;
    json_t *root = json_object();
    modbus_t *ctx;
    uint16_t tab_reg[64];
    int rc;
    int i;
    CURL *curl_handle;
    CURLcode res;


config = json_load_file("config.json", 0, &error);
    if(!config)
        fprintf(stderr, "no config file found\n");
    if(error.line != -1)
        fprintf(stderr, "json_load_file returned an invalid line number %s\n", error.line);

strcpy(USB_SERIAL,  json_string_value(json_object_get(config, "USB_SERIAL"))); 
strcpy(HOST,  json_string_value(json_object_get(config, "HOST")));
strcpy(API_KEY,  json_string_value(json_object_get(config, "API_KEY"))); 

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


while(1){
rc = modbus_read_input_registers(ctx, 0, 10, tab_reg);
if (rc == -1) {
    fprintf(stderr, "%s\n", modbus_strerror(errno));
    //return -1;
}
if (rc != -1) {
//for (i=0; i < rc; i++) {
//    printf("reg[%d]=%d (0x%X)\n", i, tab_reg[i], tab_reg[i]);
//}

json_object_set_new( root, "current", json_real(tab_reg[1]*0.001));
json_object_set_new( root, "power", json_real(tab_reg[3]*0.1));
json_object_set_new( root, "voltage", json_real(tab_reg[0]*0.1));
json_object_set_new( root, "frequency", json_real(tab_reg[7]*0.1));
json_object_set_new( root, "energy", json_real(tab_reg[5]));
json_object_set_new( root, "powerfactor", json_real(tab_reg[8]*0.01));
		  
s = json_dumps(root, 0);

//puts(s);

strcpy(url, HOST);
strcat(url, "/emoncms/input/post?node=emontx&fulljson=");
strcat(url, s);
strcat(url, "&apikey=");
strcat(url, API_KEY);

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	res = curl_easy_perform(curl_handle);
	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();
}
//json_decref(root);
//s = NULL;
sleep(1);
}

modbus_close(ctx);
modbus_free(ctx);

    return 0;
}
