#include <ao/ao.h>
#include <mpg123.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#define BITS 8

char dirpathh[] = "/home/anwar/fpmount/";

unsigned char *buffer;
ao_device *dev;
mpg123_handle *mh;
int songIsOver = 0;
char song[99];
int isPause = 0;
char qSong[100][100];
int songIndex;
    
void cleanSong();
void input();

void *playSong(void *ptr){

    //char *songName = (char *)ptr;
    char songName[100];
        
    while(1){
        
        int a;
        while(wait(a) > 0);
        
        memset(songName,0,sizeof(songName));
        strcpy(songName,dirpathh);
        strcat(songName,song);

        if(strlen(song) > 0){
            
            songIsOver = 0;
            //cleanSong();

            size_t buffer_size;
            size_t done;
            int err;

            int driver;
            
            ao_sample_format format;
            int channels, encoding;
            long rate;

            FILE *songFile = fopen(songName,"r");
            if(songFile == NULL)
                continue;
            
            /*if(strlen(songName) < 4)
                exit(0);*/

            /* initializations */
            ao_initialize();
            driver = ao_default_driver_id();
            mpg123_init();
            mh = mpg123_new(NULL, &err);
            buffer_size = mpg123_outblock(mh);
            buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

            /* open the file and get the decoding format */
            mpg123_open(mh, songName);
            mpg123_getformat(mh, &rate, &channels, &encoding);

            /* set the output format and open the output device */
            format.bits = mpg123_encsize(encoding) * BITS;
            format.rate = rate;
            format.channels = channels;
            format.byte_format = AO_FMT_NATIVE;
            format.matrix = 0;
            dev = ao_open_live(driver, &format, NULL);

            /* decode and play */
            /*while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK){
                
                if(isPause == 0){
                    ao_play(dev, buffer, done);
                    //printf("not pause\n");
                }
                
                if(songIsOver){
                    break;
                }
            }*/

            while(1){
                if(isPause == 0){
                    if(mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK){
                        ao_play(dev, buffer, done);
                    }else break;
                }

                if(songIsOver){
                    break;
                }
            }

            cleanSong();
        }

    }

}

void listSong(){
    
    DIR *dp;
	struct dirent *de;

	dp = opendir(dirpathh);
	if (dp == NULL)
		return;

	while ((de = readdir(dp)) != NULL) {
        if(strcmp(de->d_name,"..") != 0 && strcmp(de->d_name,".") != 0)
		    printf("%s\n",de->d_name);
    }

    closedir(dp);
    
}

void initSong(){
    DIR *dp;
	struct dirent *de;

	dp = opendir(dirpathh);
	if (dp == NULL)
		return;

    int i = 0;

	while ((de = readdir(dp)) != NULL) {
        if(strcmp(de->d_name,"..") != 0 && strcmp(de->d_name,".") != 0){
            //store the song
            strcpy(qSong[i],de->d_name);
            //printf("%s\n",qSong[i]);
            i++;
        }
		    
    }

    closedir(dp);
}

int findSongIndex(char songName[100]){
    DIR *dp;
	struct dirent *de;

	dp = opendir(dirpathh);
	if (dp == NULL)
		printf("Error wrong path directory \n");

    int count=0;

	while ((de = readdir(dp)) != NULL) {
        if(strcmp(de->d_name,"..") != 0 && strcmp(de->d_name,".") != 0){
            if(strcmp(songName,de->d_name) == 0){
                return count;
            }
            count++;
        }
		    
    }

    closedir(dp);
    return -1;
}

int getCountSong(){
    DIR *dp;
	struct dirent *de;

	dp = opendir(dirpathh);
	if (dp == NULL)
		printf("Error wrong path directory \n");

    int count=0;

	while ((de = readdir(dp)) != NULL) {
        if(strcmp(de->d_name,"..") != 0 && strcmp(de->d_name,".") != 0){
            count++;
        }
    }

    closedir(dp);
    return count;
}

int main(int argc, char *argv[])
{
    initSong();
    pthread_t thread;

    if(pthread_create(&thread,NULL,playSong,NULL)){
        printf("failed\n");
    }

    input();
    
    
    return 0;
}

void input(){

    char fiture[99];  
    char pTemp[99];  

    while(1){

        printf("input fiture : ");
        memset(fiture,0,sizeof(fiture));
        memset(pTemp,0,sizeof(pTemp));
        
        //scanf("%s",fiture);
        gets(fiture);

        for(int i=0; i < 4; i++){
            pTemp[i] = fiture[i];
        }
            
        if(strcmp(fiture,"pause")==0){
            isPause = 1;
           
        }else if(strcmp(fiture,"next")==0){

            songIsOver = 1;
            memset(song,0,sizeof(song));
            songIndex++;
            if(songIndex < 0){
                songIndex = getCountSong();
            }else if(songIndex > getCountSong() - 1){
                songIndex = 0;
            }
            
            //printf("%d\n",getCountSong());
            strcpy(song,qSong[songIndex]);
            isPause = 0;
          
            
            
            
        }else if(strcmp(fiture,"prev")==0){

            songIsOver = 1;
            memset(song,0,sizeof(song));
            songIndex--;
            if(songIndex < 0){
                songIndex = getCountSong() - 1;
            }
            
            //printf("%d\n",songIndex);
            
            strcpy(song,qSong[songIndex]);
            isPause = 0;
            

        }else if(strcmp(fiture,"list lagu")==0){
            listSong();
        }else if(strcmp(pTemp,"play")==0 && isPause == 1 && strlen(fiture) < 5){

            isPause = 0;         

        }else if(strcmp(pTemp,"play")==0 && strlen(fiture) > 5){

            songIsOver = 1;
            memset(song,0,sizeof(song));
            //scanf("%s",song);
            
            for(int i=0; i+5 < strlen(fiture); i++){
                song[i] = fiture[i+5];
            }
            isPause = 0;
            
            songIndex = findSongIndex(song);


        }else{
            printf("Command not found !! \n");
        }

    }

}

void cleanSong(){

    /* clean up */
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();

}


