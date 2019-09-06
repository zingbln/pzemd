#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
//#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>

#include <modbus/modbus.h>
#include <curl/curl.h>
#include <dirent.h>

//#define NUMT 
//#define RUNNING_DIR "/home/marbel/pzemd"

void daemonize(void);
void upload_tmp(void);
int file_select(const struct dirent *entry);
char* bulk_upload(int buff[][7], int j, CURL *curl);
char* upload(char string[8000], CURL *curl);
void signal_handler(int sig);

struct MemoryStruct chunk;

char HOST[20],PATH[20], API_KEY[100];

struct MemoryStruct {
  char *memory;
  size_t size;
};


static size_t

WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    /* out of memory! */ 
    //printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
  
  mem->memory = ptr;
  
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

void upload_tmp(void){
    FILE *file;
    struct dirent **namelist;
    char line[100], *result[100];
    char seps[] = ",";
    char* val;
    int buff[30][7];
    int i = 0;
    int j = 0;
    int n = 0;
    int k = 0;
    int var;

    CURL *curl = curl_easy_init();
    n = scandir(".", &namelist, file_select, alphasort);
    if (n > 0) {
        for (k=0; k<n; k++) {
            printf("%d %s\n", k,  namelist[k]->d_name);
            //j = parse_tmp(namelist[k]->d_name, buff);
            file = fopen(namelist[k]->d_name, "r");
    
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
        if (curl) {
            *result = bulk_upload(buff, j-1, curl);
            //printf("§%s\n", *result);
            if (strcmp(*result, "ok") == 0) {
                remove(namelist[k]->d_name);    
           }
        }
        j=0;
        free(namelist[k]);
        }
    free(namelist);
    }
    curl_easy_cleanup(curl);
}

char* upload(char string[8000], CURL *curl ){
    char url[8000], *result;
    chunk.memory = malloc(1);
    chunk.size = 0;
    
    CURLcode res;
    char *output = curl_easy_escape(curl, string, strlen(string));
    sprintf(url, "%s/%s/input/post?node=emontx&fulljson=%s&apikey=%s", HOST, PATH, output, API_KEY);
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
               
return result;

}

char* bulk_upload(int buff[][7], int j, CURL *curl){
char url[8000], s[7000], p[7000], *result;// = NULL;
int k;

    chunk.memory = malloc(1);
    chunk.size = 0;

    CURLcode res;

    strcpy(s, "[");
    for ( k=0; k<=j;  k++) {
         //printf("%d, %d, %d, %d, %d, %d",buff[k][1],  buff[k][2], buff[k][3], buff[k][4], buff[k][5], buff[k][6] );
        sprintf(p, "[%d, \"emontx\",{\"current\":%.4f},{\"power\":%.3f},{\"voltage\":%.2f},{\"frequency\":%.2f},{\"energy\":%.0f},{\"powerfactor\":%.2f}]", buff[k][0]-buff[j][0], buff[k][1]*0.0001, buff[k][2]*0.01, buff[k][3]*0.01, buff[k][4]*0.01, buff[k][5]*0.1, buff[k][6]*0.001);   

        strcat(s,p);
        if (k<j){strcat(s, ",");}
    }
    strcat(s, "]");
    //printf("%s\n", s);
    char *output = curl_easy_escape(curl, s, strlen(s));
    sprintf(url, "%s/%s/input/bulk.json?data=%s&&time=%d&apikey=%s", HOST, PATH,  output, buff[j][0], API_KEY);
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

return result;
}

int file_select(const struct dirent *entry)
{
    int len = strlen(entry->d_name ) ;
    if(len < 4) { return 0;}
    return strcmp(entry->d_name+len-4,".tmp") == 0;

}

int main (void) {
    char str[10], USB_SERIAL[10],  val[100], filename[12], *result[100], s[7000];
    modbus_t *ctx;
    uint16_t tab_reg[64];
    int rc;
    int pid_fd,i=0,j=0,k,m=0;
    //long response_code;
    //int buff[30][7];
    int buff10[30][7]; 
    //pthread_t tid[NUMT]
    FILE *tmp;
    //struct dirent **namelist;
    openlog ("pzemd", LOG_PID, LOG_DAEMON);

    pid_fd = open("pzemd.lock", O_RDWR|O_CREAT, 0640);
    if (pid_fd < 0) {
        syslog (LOG_ERR, "Can't open lockfile");
        exit(0);
    }
    if (lockf(pid_fd, F_TLOCK, 0) < 0) {
        syslog (LOG_ERR, "Can't lock lockfile *");
        exit(0);
    }
     /* Get current PID */
    sprintf(str, "%d\n", getpid());
    /* Write PID to lockfile */
    write(pid_fd, str, strlen(str));

FILE* ptr = fopen("pzemd.cfg","r"); 
if (ptr==NULL) { 
    //printf("no such file pzemd.cfg.\n"); 
    syslog (LOG_ERR, "Config: No such file pzem.cfg");
    return 0; 
    } 

if (fscanf(ptr, " USBSERIAL=%s ", val) == 1) {
    strcpy(USB_SERIAL, val); 
} else {
    syslog (LOG_ERR, "Config: No USB Serial configured");
     //printf("Please configure your USB Serial Port in pzemd.cfg\n"); 
    return 0; 
} 
if (fscanf(ptr, " HOST=%s", val) == 1)  {
    strcpy(HOST, val); 
} else {
    syslog (LOG_ERR, "Config: No EmonCMS Host configured");
     //printf("Please configure your Emoncms Host in pzemd.cfg\n"); 
    return 0; 
}
if (fscanf(ptr, " EMONPATH=%s", val) == 1)  {
    strcpy(PATH, val); 
} else {
    syslog (LOG_ERR, "Config: No EmonCMS Path configured");
     //printf("Please configure your Emoncms Path in pzemd.cfg\n"); 
    return 0; 
}
if (fscanf(ptr, " APIKEY=%s", val) == 1) {
    strcpy(API_KEY, val); 
}else {
    syslog (LOG_ERR, "Config: No EmonCMS Api Key configured");
     //printf("Please configure your Emoncms Api Key in pzemd.cfg\n"); 
    return 0; 
}
fclose(ptr);

ctx = modbus_new_rtu(USB_SERIAL, 9600, 'N', 8, 1);
if (modbus_connect(ctx) == -1) {
	//fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    syslog (LOG_ERR, "Modbus: Connection failed");
	modbus_free(ctx);
	return -1;
	}	

rc = modbus_set_slave(ctx, 0x01);
    if (rc == -1) {
      //fprintf(stderr, "Set slave error: %s\n", modbus_strerror(errno));
    syslog (LOG_ERR, "Modbus: Set Slave Error");
      return -1;
    } /*else {
      fprintf(stderr, "Slave set to 0x01\n");
    }*/

curl_global_init(CURL_GLOBAL_ALL);
upload_tmp();
CURL *curl = curl_easy_init();
if (curl) {
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);

    syslog (LOG_NOTICE, "started.");
    while(1){

    rc = modbus_read_input_registers(ctx, 0, 10, tab_reg);
    if (rc == -1) {
        //fprintf(stderr, "%s\n", modbus_strerror(errno));
        syslog (LOG_ERR, "Modbus: %s", modbus_strerror(errno));
        return -1;
    }
    if (rc != -1) {

        buff10[j][0] = (int)time(NULL);
        buff10[j][1] += tab_reg[1]; 
        buff10[j][2] += tab_reg[3]; 
        buff10[j][3] += tab_reg[0]; 
        buff10[j][4] += tab_reg[7];
        buff10[j][5] += tab_reg[5];
        buff10[j][6] += tab_reg[8];

        i++;

        if (i==10) { 
            j++;
            i=0;

            if (j==1 && m==0) {
                sprintf(s, "{\"current\": %.4f, \"power\": %.3f, \"voltage\": %.2f, \"frequency\": %.2f, \"energy\": %.0f, \"powerfactor\": %.2f}", buff10[j-1][1] *0.0001, buff10[j-1][2] *0.01, buff10[j-1][3] *0.01, buff10[j-1][4] *0.01, buff10[j-1][5]*0.1 , buff10[j-1][6] *0.001);
                *result = upload(s, curl);
                //printf("§%s\n", *result);
                if (strcmp(*result, "{\"success\": true}") == 0) {
                    j=0;
                    memset(buff10, 0, sizeof buff10);
                }

            }

        if (j>1 && m==0) {
            *result = bulk_upload(buff10, j-1, curl);
            //printf("§%s\n", *result);
            if (strcmp(*result, "ok") == 0) {
                j=0;
                memset(buff10, 0, sizeof buff10);
            }

        }


        if (j==30 && m==0) {
            syslog (LOG_WARNING, "libcurl: EmonCMS Host not reachable");
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
                    memset(buff10, 0, sizeof buff10);
                    j=0;
                    strcpy(s, "{}");
                    *result = upload(s, curl);
                    //printf("§%s\n", *result);
                    if (strcmp(*result, "{\"success\": true}") == 0) {
                        upload_tmp();
                        m=0;
                    }
                    else {
                        syslog (LOG_WARNING, "libcurl: EmonCMS Host not reachable");
                        m++;
                    }
                }
            }
        }

    }
}
sleep(1);
}
}
//free(buff);
//free(buff10);
free(chunk.memory);
curl_easy_cleanup(curl);
curl_global_cleanup();
modbus_close(ctx);
modbus_free(ctx);
syslog (LOG_NOTICE, "terminated.");
closelog();
    return 0;
}


