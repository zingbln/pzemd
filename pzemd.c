#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
//#include <pthread.h>

#include <modbus/modbus.h>
#include <curl/curl.h>
#include <dirent.h>

//#define NUMT 
//struct BuffStruct parse_tmp(char *filename);
int parse_tmp(char *filename, int buff[30][7]);

int file_select(const struct dirent *entry);
char* bulk_upload(int buff[][7], int j);
struct MemoryStruct chunk;

char HOST[100], API_KEY[100];

struct MemoryStruct {
  char *memory;
  size_t size;
};

/*struct BuffStruct {
        int buff[30][7];
        int len;
};*/

static size_t

WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
  
  mem->memory = ptr;
  
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}
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

//struct BuffStruct parse_tmp(char *filename){
int parse_tmp(char *filename, int buff[30][7]){
    FILE *file;
    char line[100];
    char seps[] = ",";
    char* val;
    //int csv[30][7];
    int i = 0;
    int j = 0;
    int var;

    file = fopen(filename, "r");
    
    if (file != NULL) {
        while(fgets(line,100,file) != NULL) {
            //line[strlen(line)-1]=',';
            //printf("%s\n", line);
            val = strtok (line, seps);
            while (val != NULL) {
                sscanf (val, "%d", &var);
                buff[j][i++] = var;
                val = strtok (NULL, seps);
        }
            i=0;
            j++;
    }
    fclose(file);
    }
    //printf("%d\n", len);
    //for (i=0; i<30; i++) {printf("%d %d\n", csv[i][0], csv[i][6]);}
    //struct BuffStruct r = {{{ **csv, len }}};
    return j;

    
}

char* bulk_upload(int buff[][7], int j){
char url[8000], s[7000], p[7000], *result;// = NULL;
int k;

CURL *curl = curl_easy_init();
if (curl) {
CURLcode res;


strcpy(s, "[");
for ( k=0; k<=j;  k++) {
        //printf("%d, %d, %d, %d, %d, %d",buff[k][1],  buff[k][2], buff[k][3], buff[k][4], buff[k][5], buff[k][6] );
    sprintf(p, "[%d, \"emontx\",{\"current\":%.2f},{\"power\":%.2f},{\"voltage\":%.2f},{\"frequency\":%.2f},{\"energy\":%.0f},{\"powerfactor\":%.2f}]", buff[k][0]-buff[j][0], buff[k][1]*0.0001, buff[k][2]*0.01, buff[k][3]*0.01, buff[k][4]*0.01, buff[k][5]*0.1, buff[k][6]*0.001);   


strcat(s,p);
if (k<j){strcat(s, ",");}
}
strcat(s, "]");
printf("%s\n", s);
char *output = curl_easy_escape(curl, s, strlen(s));
sprintf(url, "%s/emoncms/input/bulk.json?data=%s&&time=%d&apikey=%s", HOST,  output, buff[j][0], API_KEY);
if(output) {
	curl_free(output);
    }
curl_easy_setopt(curl, CURLOPT_URL, url);
curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
res = curl_easy_perform(curl);
if(res == CURLE_OK) {
    result =  chunk.memory;
}
else {
    result = "false";
}
curl_easy_cleanup(curl);
}
return result;
}

int file_select(const struct dirent *entry)
{
    int len = strlen(entry->d_name ) ;
    if(len < 4) { return 0;}
    return strcmp(entry->d_name+len-4,".tmp") == 0;

}

int main (void) {
    char USB_SERIAL[10], url[8000],  val[100], filename[12], *result[100];
    char s[7000]; //p[7000];// = NULL;
    modbus_t *ctx;
    uint16_t tab_reg[64];
    int rc;
    int i,j,k,m,n;
    long response_code;
    int buff[30][7];
    int buff10[30][7]; 
    //pthread_t tid[NUMT]
    FILE *tmp;
    struct dirent **namelist;
    

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

n = scandir(".", &namelist, file_select, alphasort);
if (n > 0) {
    for (k=0; k<n; k++) {
        printf("%d %s\n", k,  namelist[k]->d_name);
        j = parse_tmp(namelist[k]->d_name, buff);
        //struct BuffStruct t = parse_tmp(namelist[k]->d_name);
        //printf("%d\n", j);
        //for (k=0; k<30; k++) {printf("%d\n", buff[k][0]);}
        *result = bulk_upload(buff, j-1);
        /*if (strcmp(result, "ok") == 0) {
            remove(namelist[k]->d_name);    
        }*/
        free(namelist[k]);
        }
    free(namelist);
}


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
j=0;
m=0;
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

/*buff[i][0] = (int)time(NULL);
buff[i][1] = tab_reg[1];
buff[i][2] = tab_reg[3]; 
buff[i][3] = tab_reg[0]; 
buff[i][4] = tab_reg[7]; 
buff[i][5] = tab_reg[5];
buff[i][6] = tab_reg[8];*/

buff10[j][0] = (int)time(NULL);
buff10[j][1] += tab_reg[1]; 
buff10[j][2] += tab_reg[3]; 
buff10[j][3] += tab_reg[0]; 
buff10[j][4] += tab_reg[7];
buff10[j][5] += tab_reg[5];
buff10[j][6] += tab_reg[8];

//printf("%d %d %d %d %d %d %d\n",buff[i][0],  buff[i][1], buff[i][2], buff[i][3], buff[i][4], buff[i][5], buff[i][6] );
i++;
//printf("%.0f %.2f %.2f %.2f %.2f %.2f %.2f\n",buff10[j][0],  buff10[j][1], buff10[j][2], buff10[j][3], buff10[j][4], buff10[j][5], buff10[j][6] );

//printf("%d \n" , sizeof(int[7][i]));
if (i==10) { 
   /* buff10[j][1] = buff10[j][1]*0.1;
    buff10[j][2] = buff10[j][2]*0.1;
    buff10[j][3] = buff10[j][3]*0.1; 
    buff10[j][4] = buff10[j][4]*0.1;
    buff10[j][5] = buff10[j][5]*0.1;
    buff10[j][6] = buff10[j][6]*0.1;*/

    printf("%d %d %d %d %d %d %d\n",buff10[j][0],  buff10[j][1], buff10[j][2], buff10[j][3], buff10[j][4], buff10[j][5], buff10[j][6] );

    j++;
    i=0;

    chunk.memory = malloc(1);
    chunk.size = 0;

if (j==31 && m==0) {
    sprintf(s, "{\"current\": %.2f, \"power\": %.2f, \"voltage\": %.2f, \"frequency\": %.2f, \"energy\": %.0f, \"powerfactor\": %.2f}", buff10[j-1][1] *0.0001, buff10[j-1][2] *0.01, buff10[j-1][3] *0.01, buff10[j-1][4] *0.01, buff10[j-1][5]*0.1 , buff10[j-1][6] *0.001);
    if(curl) {
        char *output = curl_easy_escape(curl, s, strlen(s));
        sprintf(url, "%s/emoncms/input/post?node=emontx&fulljson=%s&apikey=%s", HOST, output, API_KEY);
        if(output) {
	        curl_free(output);
        }
        curl_easy_setopt(curl, CURLOPT_URL, url);
        res = curl_easy_perform(curl);
        }
        if(res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        printf("%ld \n", response_code);
        if(response_code == 200) {
            j=0;
        }
        }
}

if (j>1 && m==0) {
    *result = bulk_upload(buff10, j-1);
    printf("§%s\n", *result);
    if (strcmp(*result, "ok") == 0) {
            j=0;
            memset(buff10, 0, sizeof buff10);
    }

/*    if(curl) {
        strcpy(s, "[");
    //https://e.com/input/bulk.json?data=[[0,1,{"power":10, "voltage": 240}],[10,1,{"power":11, "voltage": 240.2}]]
    for ( k=0; k<=j-1;  k++) {
        //printf("%d, %d, %d, %d, %d, %d",buff[k][1],  buff[k][2], buff[k][3], buff[k][4], buff[k][5], buff[k][6] );
        sprintf(p, "[%d, \"emontx\",{\"current\":%.2f},{\"power\":%.2f},{\"voltage\":%.2f},{\"frequency\":%.2f},{\"energy\":%.0f},{\"powerfactor\":%.2f}]", buff10[k][0]-buff10[j-1][0], buff10[k][1]*0.0001, buff10[k][2]*0.01, buff10[k][3]*0.01, buff10[k][4]*0.01, buff10[k][5]*0.1, buff10[k][6]*0.001);   

    strcat(s,p);
    if (k<j-1){strcat(s, ",");}
    }
    strcat(s, "]");
    printf("%s\n", s);
        char *output = curl_easy_escape(curl, s, strlen(s));
        sprintf(url, "%s/emoncms/input/bulk.json?data=%s&&time=%d&apikey=%s", HOST,  output, buff10[j-1][0], API_KEY);
        if(output) {
	    curl_free(output);
        }
        curl_easy_setopt(curl, CURLOPT_URL, url);
        res = curl_easy_perform(curl);
        //usleep(200000);
       

        if(res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        printf("%ld \n", response_code);
        if(response_code == 200) {
            j=0;
            memset(buff10, 0, sizeof buff10);
        }

}
}*/
}


if (j==30 && m==0) {

    sprintf(filename, "%d.tmp", buff10[0][0]);
    tmp = fopen(filename, "w+"); 
    if (tmp != NULL) {
    for(k=0;k<j-1;k++){
         fprintf(tmp, "%d,%d,%d,%d,%d,%d,%d\n", buff10[k][0], buff10[k][1], buff10[k][2], buff10[k][3], buff10[k][4], buff10[k][5], buff10[k][6]);  
    }
    memset(buff10, 0, sizeof buff10);
    fclose(tmp);
    j=0;
    m++;
    }
}

if (m>0) {
    if (j==1) {
        sprintf(filename, "%d.tmp", buff10[0][0]);
        tmp = fopen(filename, "w+"); 
    }
    if (tmp != NULL) {
        fprintf(tmp, "%d,%d,%d,%d,%d,%d,%d\n", buff10[j-1][0], buff10[j-1][1], buff10[j-1][2], buff10[j-1][3], buff10[j-1][4], buff10[j-1][5], buff10[j-1][6]);
        fflush(tmp);
        if (j==30) {
        fclose(tmp);
        }
    memset(buff10, 0, sizeof buff10);
    j=0;
    m++;
    }
}


}
}
sleep(1);
}
free(buff);
free(buff10);
free(chunk.memory);
curl_easy_cleanup(curl);
curl_global_cleanup();
modbus_close(ctx);
modbus_free(ctx);
    return 0;
}


