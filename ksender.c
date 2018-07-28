#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib.h"


#define HOST "127.0.0.1"
#define PORT 10000
#define modulo 64


//Definire campuri pachet
#define SOH 0x01
#define MARK 0x0D

#define TIME 5000
#define MAXL 250

char SEQ = 0;


int main(int argc, char** argv) {
    msg t;


    init(HOST, PORT);


    sprintf(t.payload, "Hello World of PC");
    t.len = strlen(t.payload);
    send_message(&t);


    msg *y = receive_message_timeout(5000);
    if (y == NULL) {
        perror("receive error");
    } else {
        printf("[%s] Got reply with payload: %s\n", argv[0], y->payload);
    }

    /*      Trimitere SEND INIT     */

    msg send_init;
    fill_send_init(&send_init,SEQ);
    int repetare_trimitere = 0;
    printf("[%d] S->R\n", SEQ );
    send_message(&send_init);

    while(1){
        if(repetare_trimitere == 3){
            perror("receive error Send-Init ");
            return -1;
        }
        msg *x = receive_message_timeout(TIME);
        if(x == NULL){
            repetare_trimitere++;
            printf("[%d] S->R\n", SEQ );
            send_message(&send_init);
        }
        else{
            if( x->payload[3] == 'Y' ){ 
                SEQ=(SEQ+1)%modulo;
                break;
            }
            else{
                SEQ=(SEQ+1)%modulo;
                fill_send_init(&send_init,SEQ);
                printf("[%d] S->R\n", SEQ ); 
                send_message(&send_init);
            }
        }
    }
    

    /*       Incepe trimiterea fisierelor       */

    int i;
    for(i =1; i<argc; i++){

        /*       Trimitere FILE HEADER        */

        msg nume_fisier;
        fill_file_header(&nume_fisier, argv[i], SEQ);
        repetare_trimitere = 0;
        printf("[%d] S->R\n", SEQ );
        send_message(&nume_fisier);

        while(1){
        	if(repetare_trimitere == 3){
        		perror("receive error nume ");
        		printf("%d\n", repetare_trimitere );
        		return -1;
        	}
        	msg *x = receive_message_timeout(TIME);
        	if(x == NULL){
                printf("[%d] S->R\n", SEQ );
        		repetare_trimitere++;
                send_message(&nume_fisier);
        	}
        	else{
        		if( x->payload[3] == 'Y' ){ 
        			printf("Am primit bine in sender FILE_HEADER. Nr retrim: %d\n", repetare_trimitere );
                    SEQ=(SEQ+1)%modulo;
        			break;
        		}
        		else{
                    SEQ=(SEQ+1)%modulo;
                    fill_file_header(&nume_fisier, argv[i], SEQ); 
                    printf("[%d] S->R\n", SEQ );
        			send_message(&nume_fisier);
        		}
        	}
        }
      
      	/*      Trimitere DATA       */

      	char date_fis[250];
      	FILE *f = fopen(argv[i], "rb+");
      	int bytes;
        while( (bytes = fread(date_fis, 1, 250, f) ) > 0 ){
        	msg data;
        	fill_data(&data, date_fis,SEQ,bytes);
        	int repetare_trimitere1 = 0;
            printf("[%d] S->R\n", SEQ );
        	send_message(&data);

        	while(1){
    	    	if(repetare_trimitere1 == 3){
    	    		perror("receive error date");
    	    		printf("%d\n", repetare_trimitere1 );
    	    		return -1;
    	    	}
    	    	msg *x = receive_message_timeout(TIME);
    	    	if(x == NULL){
    	    		repetare_trimitere1++;
                    printf("[%d] S->R\n", SEQ );
                    send_message(&data);
    	    	}
    	    	else{
    	    		if( x->payload[3] == 'Y' ){ 
    	    			printf("Am primit bine date_fisier. Nr retrim: %d\n", repetare_trimitere1 );
                        SEQ=(SEQ+1)%modulo;
    	    			break;
    	    		}
    	    		else{ 
                        SEQ=(SEQ+1)%modulo;
                        fill_data(&data, date_fis,SEQ,bytes);
                        printf("[%d] S->R\n", SEQ );
    	    			send_message(&data);
    	    		}
    	    	}
        	}
        }

        fclose(f);

        /*      Trimitere END OF FILE    */
        msg sf_fis;
        fill_EOF(&sf_fis, SEQ);
        int repetare_trimitere1 = 0;
        printf("[%d] S->R\n", SEQ );
        send_message(&sf_fis);

        	while(1){
    	    	if(repetare_trimitere1 == 3){
    	    		perror("receive error EOF");
    	    		printf("%d\n", repetare_trimitere1 );
    	    		return -1;
    	    	}
    	    	msg *x = receive_message_timeout(TIME);
    	    	if(x == NULL){
    	    		repetare_trimitere1++;
                    printf("[%d] S->R\n", SEQ );
                    send_message(&sf_fis);
    	    	}
    	    	else{
    	    		if( x->payload[3] == 'Y' && correct_recv_mes(x) == 1  ){
    	    			printf("Am primit bine EOF. Nr retrim: %d\n", repetare_trimitere1 );
                        SEQ=(SEQ+1)%modulo;
    	    			break;
    	    		}
    	    		else{
                        SEQ=(SEQ+1)%modulo;
                        fill_EOF(&sf_fis, SEQ);
                        printf("[%d] S->R\n", SEQ );
    	    			send_message(&sf_fis);
    	    		}
    	    	}
        	}
}

    /*       Trimitere END OF TRANSMISION        */

    msg EOT;
    fill_EOT(&EOT, SEQ);
    repetare_trimitere = 0;
    printf("[%d] S->R\n", SEQ );
    send_message(&EOT);

    while(1){
        if(repetare_trimitere == 3){
            perror("receive error EOT ");
            printf("%d\n", repetare_trimitere );
            return -1;
        }
        msg *x = receive_message_timeout(TIME);
        if(x == NULL){
            repetare_trimitere++;
            printf("[%d] S->R\n", SEQ );
            send_message(&EOT);
        }
        else{
            if( x->payload[3] == 'Y' ){ 
                printf("Am primit bine EOT. Nr retrim: %d\n", repetare_trimitere );
                SEQ=(SEQ+1)%modulo;
                break;
            }
            else{ 
                SEQ=(SEQ+1)%modulo;
                fill_EOT(&EOT, SEQ);
                printf("[%d] S->R\n", SEQ );
                send_message(&EOT);
            }
        }
    }
    



    return 0;
}
