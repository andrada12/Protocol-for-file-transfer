#ifndef LIB
#define LIB

typedef struct __attribute__(( packed )){
    int len;
    char payload[1400];
} msg;

void init(char* remote, int remote_port);
void set_local_port(int port);
void set_remote(char* ip, int port);
int send_message(const msg* m);
int recv_message(msg* r);
msg* receive_message_timeout(int timeout); //timeout in milliseconds
unsigned short crc16_ccitt(const void *buf, int len);


/*  Concatneaza doi bytes pentru a intoarce un short.
Functie folosita pentru a afla crc-ul din cei doi bytes ai campului CHECK  */

unsigned short refacere(char prima_parte, char a_doua_parte){
	unsigned short s_refacut = prima_parte;
	s_refacut = s_refacut << 8;
	unsigned char c = (unsigned char)a_doua_parte;
	s_refacut += c;

	return s_refacut;
}
/*   Imparte un short in doi bytes, intocandu-i intru-un vector de tip char
Functie folosita la impartirea crc-ului iniial in cei doi bytes ai campului CHECK  */
void impartire_pe_bytes(unsigned short s, char *parti){
	parti[1] = s;	
	s = s >> 8;
	parti[0] = s;
}

/*   Copiaza doua siruri caracter cu caracter   */

void copy(char *destinatie, char *sursa){
	sursa = "\0";
	int i;
	int lungime = strlen(sursa);
	for(i = 0; i< lungime; i++)
		destinatie[i] = sursa[i];
}


#define SOH 0x01

/*   Functii utilizate la construirea diferitelor tipuri de pachete   */

void fill_send_init(msg *t, char SEQ){
    //fill SOH
    t->payload[0] = SOH;
    //fill LEN
    t->payload[1] = 16 ; 
    //fill SEQ
    t->payload[2] = SEQ;
    //fill TYPE
    t->payload[3] = 'S';
    //fill DATA - 11 bytes
        //MAXL
        t->payload[4] = 250;
        //TIME
        t->payload[5] = 5;
        //NPAD 
        t->payload[6] = 0x00;
        //PADC
        t->payload[7] = 0x00;
        //EOL
        t->payload[8] = 0x0D;
        int i;
        for (i = 9; i<=14; i++)
            t->payload[i] = 0x00;

    //fill CHECK
    unsigned short crc = crc16_ccitt(t->payload, 15);
    char CHECK[2];
    impartire_pe_bytes(crc,CHECK);    

    t->payload[15] = CHECK[0];
    t->payload[16] = CHECK[1];

    //fill MARK
    t->payload[17] = 0x0D;

    t->len = 18;

}


void fill_file_header(msg *t, char *file_name, char SEQ){
    //fill SOH
    t->payload[0] = SOH;
    //fill LEN
    t->payload[1] = 5 + strlen(file_name) ; 
    //fill SEQ
    t->payload[2] = SEQ;
    //fill TYPE
    t->payload[3] = 'F';
    //fill DATA 
        int i;
        for(i = 0; i< strlen(file_name); i++){
        	t->payload[4+i] = file_name[i];
        }
        i = 4 + strlen(file_name);
    //fill CHECK
    unsigned short crc = crc16_ccitt(t->payload, strlen(file_name)+4);
    char CHECK[2];
    impartire_pe_bytes(crc,CHECK);    

    t->payload[i] = CHECK[0]; i++;
    t->payload[i] = CHECK[1]; i++;

    //fill MARK
    t->payload[i] = 0x0D;
    i++;
    t->len = i;

}

void fill_data(msg *t, char *data, char SEQ, int bytes){
    //fill SOH
    t->payload[0] = SOH;
    //fill LEN
    t->payload[1] = 5 + bytes ; 
    //fill SEQ
    t->payload[2] = SEQ;
    //fill TYPE
    t->payload[3] = 'D';
    //fill DATA - 11 bytes
        int i;
        for(i = 0; i< bytes; i++){
        	t->payload[4+i] =  data[i];
        }
        i = 4 + bytes;
    //fill CHECK
    unsigned short crc = crc16_ccitt(t->payload, bytes + 4);
    char CHECK[2];
    impartire_pe_bytes(crc,CHECK);    

    t->payload[i] = CHECK[0]; i++;
    t->payload[i] = CHECK[1]; i++;

    //fill MARK
    t->payload[i] = 0x0D;
    i++;
    t->len = bytes+7;

}

void fill_EOF(msg *t, char 	SEQ){
	 //fill SOH
    t->payload[0] = SOH;
    //fill LEN
    t->payload[1] = 5; 
    //fill SEQ
    t->payload[2] = SEQ;
    //fill TYPE
    t->payload[3] = 'Z';
    //DATA - null        
    //CHECK 
    unsigned short crc = crc16_ccitt(t->payload, 4);
    char CHECK[2];
    impartire_pe_bytes(crc,CHECK);    

    t->payload[4] = CHECK[0];
    t->payload[5] = CHECK[1];

    //fill MARK
    t->payload[6] = 0x0D;

    t->len = 7;
}

void fill_EOT(msg *t, char SEQ){
    //fill SOH
    t->payload[0] = SOH;
    //fill LEN
    t->payload[1] = 5; 
    //fill SEQ
    t->payload[2] = SEQ;
    //fill TYPE
    t->payload[3] = 'B';
    //DATA - null        
    //CHECK 
    unsigned short crc = crc16_ccitt(t->payload,4);
    char CHECK[2];
    impartire_pe_bytes(crc,CHECK);    

    t->payload[4] = CHECK[0];
    t->payload[5] = CHECK[1];

    //fill MARK
    t->payload[6] = 0x0D;

    t->len = 7;

}

void fill_ACK(msg *t, char SEQ){
    //fill SOH
    t->payload[0] = SOH;
    //fill LEN
    t->payload[1] = 5; 
    //fill SEQ
    t->payload[2] = SEQ;
    //fill TYPE
    t->payload[3] = 'Y';
    //DATA - null        
    //CHECK 
    unsigned short crc = crc16_ccitt(t->payload, 4);
    char CHECK[2];
    impartire_pe_bytes(crc,CHECK);    

    t->payload[4] = CHECK[0];
    t->payload[5] = CHECK[1];

    //fill MARK
    t->payload[6] = 0x0D;

    t->len = 7;

}

void fill_ACK_Send_Init(msg *t, msg *t2){

	//fill SOH
    t->payload[0] = t2->payload[0];
    //fill LEN
    t->payload[1] = 16 ; 
    //fill SEQ
    t->payload[2] = t2->payload[2];
    //fill TYPE
    t->payload[3] = 'Y';
    //fill DATA - 11 bytes
        //MAXL
        t->payload[4] = t2->payload[4];
        //TIME
        t->payload[5] = t2->payload[5];
        //NPAD 
        t->payload[6] = t2->payload[6];
        //PADC
        t->payload[7] = t2->payload[7];
        //EOL
        t->payload[8] = t2->payload[8];
        int i;
        for (i = 9; i<=14; i++)
            t->payload[i] = t2->payload[i];

    //fill CHECK
    unsigned short crc = crc16_ccitt(t->payload, 15);
    char CHECK[2];
    impartire_pe_bytes(crc,CHECK);    

    t->payload[15] = CHECK[0];
    t->payload[16] = CHECK[1];

    //fill MARK
    t->payload[17] = 0x0D;

    t->len = 18;
}

void fill_NAK(msg *t, char 	SEQ){
	 //fill SOH
    t->payload[0] = SOH;
    //fill LEN
    t->payload[1] = 5; 
    //fill SEQ
    t->payload[2] = SEQ;
    //fill TYPE
    t->payload[3] = 'N';
    //DATA - null        
    //CHECK 
    unsigned short crc = crc16_ccitt(t->payload, 4);
    char CHECK[2];
    impartire_pe_bytes(crc,CHECK);    

    t->payload[4] = CHECK[0];
    t->payload[5] = CHECK[1];

    //fill MARK
    t->payload[6] = 0x0D;

    t->len = 7;
}

/*    Verifica daca mesajele au fost transmise corect, comparand crc-ul mesajului primit cu cel din campurile CHECK  */
int correct_recv_mes(msg *r){
    /*  Calculare crc actual    */
    unsigned short crc2 = crc16_ccitt(r->payload, r->len-3); 
    /*  Refacere CHECK  */
    char CHECK[2];
    CHECK[0] = r->payload[r->len-3];
    CHECK[1] = r->payload[r->len-2];

    /*  Formare un short din cei doi bytes care formeaza CHECK-ul   */
    unsigned short CHECK_refacut = refacere(CHECK[0], CHECK[1]);

    if (crc2 != CHECK_refacut)
        return 0;
    return 1;

}

/*   Functie folosita pentru formarea numelui fisierului in care scrie receiverul   */
void nume_recv_fis(char *recv_nume, char *nume){
    recv_nume[0] = 'r';
    recv_nume[1] = 'e';
    recv_nume[2] = 'c';
    recv_nume[3] = 'v';
    recv_nume[4] = '_';
    int i;
    for(i = 4; i<strlen(nume)-3; i++){
        recv_nume[1+i] = nume[i];
    }

}

/*     Functie folosita pentru extragerea datelor din pachetul de tip DATA     */
void extract_data(char *extr_data, char *payload, int len){
    int i;
    for(i = 4; i<len - 3; i++){
        extr_data[i-4] = payload[i];
    }
}

#endif

