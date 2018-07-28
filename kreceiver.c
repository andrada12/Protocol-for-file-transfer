#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib.h"


#define HOST "127.0.0.1"
#define PORT 10001


#define MAXL 250 
#define TIME 5000
char MARK;
char EOL;



int main(int argc, char** argv) {
    msg r, t;

    init(HOST, PORT);


    if (recv_message(&r) < 0) {
        perror("Receive message");
        return -1;
    }
    printf("[%s] Got msg with payload: %s\n", argv[0], r.payload);

    
    unsigned short crc = crc16_ccitt(r.payload, r.len);
   
    sprintf(t.payload, "CRC(%s)=0x%04X", r.payload, crc);
    t.len = strlen(t.payload);
    send_message(&t);

    
    
    char SEQ = 0;

    /*		Primire SEND INIT		*/

    while(1){
    	msg *x = receive_message_timeout(4*TIME);
    	if ( x == NULL){
    		perror("Recv nu mai primeste nimic. Transmisia se va incheia\n");
    		return -1;
    	}
    	if( correct_recv_mes(x) ){
    		msg rasnpuns;
    		SEQ = x->payload[2];
    		fill_ACK(&rasnpuns,SEQ);
    		printf("[%d] R->S ACK\n", SEQ );
    		send_message(&rasnpuns);
    		break;
    	}
    	else{
    		msg rasnpuns;
    		SEQ = x->payload[2];
    		fill_NAK(&rasnpuns,SEQ);
    		printf("[%d] R->S ACK\n", SEQ );
    		send_message(&rasnpuns);
    	}
    }

   
    /*		Primire TOT 	*/

    
  
    FILE *f;

    while(1){

    	char recv_nume[100];

    	msg *x = receive_message_timeout(4*TIME);
    	if ( x == NULL){
    		printf("Acelasi mesaj nu a fost receptionat de 3 ori\n");
    		return 0;
    	}
    	else{
    		
    		if (correct_recv_mes(x)){
    			msg rasnpuns;
    			SEQ = x->payload[2];
    			fill_ACK(&rasnpuns, SEQ);
    			char recv_nume[100]="";
    			char extr_data[250]="";
    			switch(x->payload[3]){
    				case 'B' :
    					printf("[%d] R->S ACK\n", SEQ );
    					send_message(&rasnpuns);
    					return 0;
    					break;

    				case 'F' :
    					nume_recv_fis(recv_nume, x->payload);
	                	f = fopen(recv_nume, "wb+");
	                	printf("[%d] R->S ACK\n", SEQ );
	                	send_message(&rasnpuns);
	                	break;

	                case 'D' :
                    	extract_data(extr_data, x->payload, x->len);
                    	fwrite(extr_data, 1,x->len-7,f);
                    	printf("[%d] R->S ACK\n", SEQ );
                    	send_message(&rasnpuns);
                    	break;

                    case 'Z' :
                    	fclose(f);
                    	printf("[%d] R->S ACK\n", SEQ );
                    	send_message(&rasnpuns);
                    	break; 		
    			}

    		}
    		else{
    			msg rasnpuns;
    			SEQ = x->payload[2];
    			fill_NAK(&rasnpuns, SEQ);
    			printf("[%d] R->S NAK\n", SEQ );
    			send_message(&rasnpuns);
    			}
    		
    	}
    }

    	    

	return 0;
}
