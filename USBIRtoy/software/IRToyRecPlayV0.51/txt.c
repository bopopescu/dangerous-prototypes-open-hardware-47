#include "txt.h"

extern int modem;
extern int verbose;
void IRtxtrecord( char *param_fname)
{
     int i,flag;
  unsigned long absolute=0;

  unsigned long size=0;

  int fcounter;
  char inkey;
 // char *param_delay=NULL;

  char fnameseq[255],fnameseq_txt[255];
  FILE *fp, *fp_txt=NULL;
  int res;
  uint16_t txt_buffer[1];
   char s[4];
      //check filename if exist
        printf(" Entering TXT conversion Mode \n");
        fcounter=0;
        inkey=0;
        while (1) {
            sprintf(fnameseq,"%s_%03d.bin",param_fname,fcounter);
            fp=fopen(fnameseq,"rb");
            if (fp==NULL) {
                if (fcounter > 0)
                    printf(" No more file(s). \n\n");
                else
                    printf(" File does not exits. \n");
               break;
            }

            sprintf(fnameseq_txt,"%s_%03d.txt",param_fname,fcounter);
            fp_txt=fopen(fnameseq_txt,"w");
            if (fp_txt==NULL) {
                   printf(" Error: Cannot create txt file: %s. \n",fnameseq_txt);
               break;
            }

            printf("\n Creating txt file: %s \n",fnameseq_txt);
            size=0;
            absolute=0;
            char temp[4];
            flag=0;
             while(!feof(fp)) {
                if ((res=fread(&txt_buffer,sizeof(uint16_t),sizeof(txt_buffer),fp)) > 0) {
                      for(i=0;i<res;i++) {
                       sprintf(temp,"%04X",(uint16_t) txt_buffer[i]);
                       sprintf(s,"%c%c%c%c",temp[2],temp[3],temp[0],temp[1]);
                       if (verbose==TRUE)
                            printf("%s ",s);
                       fprintf(fp_txt,"%s ", s);
                   }
                }

               }

           printf(" .. Done.\n");
           fclose(fp);
           fclose(fp_txt);
           fcounter++;
        }
}

void IRtxtplay(	char *param_fname,int fd,char *param_delay)
{

	BOOL no_data_yet=TRUE, file_created=FALSE;
	int res;
	int fcounter;
    char buffer[8*1024] = {0};  //   buffer
    char fnameseq[255];
	FILE *fp=NULL;
	int i;
    file_created=FALSE;
	no_data_yet=TRUE;
	char inkey;
    char temp[4];
    char *token;
    char hex1[2];
    char hex2[2];
    uint8_t buf[16*1024];

        //check filename if exist
        printf(" Entering Player Mode \n");
        fcounter=0;
        inkey=0;
        int delay = atoi(param_delay) ;
        int firstfile = 0;
        while (1) {
            sprintf(fnameseq,"%s_%03d.txt",param_fname,fcounter);
            fp=fopen(fnameseq,"rt");
            if (fp==NULL) {
                if (fcounter > 0)
                    printf(" No more file(s). \n");
                else
                    printf(" File does not exits. \n");

               break;
            }
            if (delay< 0){
				printf(" Press a key to start playing %s or X to exit \n",fnameseq);
				while (1) {
				  if(kbhit()) {
#ifdef _WIN32
					  inkey=getch();
#else
					  inkey = fgetc(stdin);
#endif

					 if ((inkey=='x') || (inkey=='X')) {
						 break;
					 }
					 else
						break;

				  }

				}
				if (inkey=='x'|| inkey=='X'){
					  break;
				}
			}
            if ((delay > 0) && (firstfile++ > 0)){
                if (verbose)
                    printf("....delay is %d miliseconds.\n", atoi(param_delay));
#ifdef _WIN32
                Sleep(atoi(param_delay));           //auto play. Do not wait for keyboard input, just wait the specified time (miliseconds)
#else
   //             sleep(atoi(param_delay));           //auto play. Do not wait for keyboard input, just wait the specified time (miliseconds)
#endif
            }

            printf("\n Playing file: %s\n",fnameseq);

            int comsresult = 0;
            for (i=0;i<5;i++) {
            //send 5x, just to make sure it exit the sump mode too
            serial_write( fd, "\x00", 1);
            }

            serial_write( fd, "S", 1);
            res= serial_read(fd, buffer, sizeof(buffer));
            serial_write( fd, "\x03", 1);

            printf(" Sending IRCodes...\n");
            int c=0;
            while(!feof(fp)) {
               if ((res=fread(&buffer,sizeof(unsigned char),sizeof(buffer),fp)) > 0) {

                    token = strtok (buffer," ");
                    while (token != NULL)
                      {
                        strcpy(temp,token);
                        //convert hex string into real hex, by fair
                        sprintf(hex1,"%c%c",temp[0],temp[1]);
                        i = (uint8_t) strtoul(hex1, NULL, 16);
                        buf[c++]=i;
                        sprintf(hex2,"%c%c",temp[2],temp[3]);
                        i = (uint8_t) strtoul(hex2, NULL, 16);
                        buf[c++]=i;

                        if (verbose==TRUE)
                            printf("%02X%02X ",buf[c-2],buf[c-1]);

                        token = strtok (NULL, " ");
                      }
                     comsresult = serial_write( fd, (char *) buf, c);
                     c=0;

#ifdef _WIN32
   // temporary disabled to alow to pass here.. in win64 param_delay with -1 seems to wait forever. --Need to confirm
       //        Sleep(atoi(param_delay));           //auto play. Do not wait for keyboard input, just wait the specified time (miliseconds)
#else
             sleep(atoi(param_delay));           //auto play. Do not wait for keyboard input, just wait the specified time (miliseconds)
#endif

               }
           }
           printf("\n");
           printf(" End of file reached: %s \n",fnameseq);
           fclose(fp);
           fcounter++;
        }

}