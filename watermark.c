#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#pragma pack(1) // to align the structs in 1 byte


typedef struct BMPHeader {
    char format[2];         // 2 bytes
    int fileSize;           // 4 bytes
    char reserved[4];       // 4 bytes
    int offset;             // 4 bytes
} BMPHeader;

typedef struct DIBHeader {
    int dibSize;            // 4 bytes
    int width;              // 4 bytes
    int height;             // 4 bytes
    short colorPlanes;      // 2 bytes
    short bitsPerPixels;    // 2 bytes
} DIBHeader;

typedef unsigned char byte;

typedef struct RGB {
    byte red;
    byte green;
    byte blue;

} RGB;

int help(int argc, char *argv[]);
char *letterToMorse(char letter);




int main(int argc, char *argv[]) {

     if(help(argc,argv) == 1) return 0;
  
int messageInd=-1,dateInd=-1,colorInd=-1, positionInd=-1; // indicator to know at which place each option is mentioned, if not mentioned => -1

for(int i=0; i<argc; i++) {
  if(strcmp(argv[i],"-text")==0) messageInd=i;
  else if(strcmp(argv[i],"-date")==0) dateInd=i;
  else if(strcmp(argv[i],"-color")==0) colorInd=i;
  else if(strcmp(argv[i],"-pos")==0) positionInd=i;
}

if(messageInd==-1 && dateInd==-1) {
  printf("nothing will be written to your file: mention date or text option\n");
  printf("if you need help with correct syntax, type: \n\t ./watermark -h\n");
  return 1;
}

if(colorInd==-1) {
  printf("-color is not optional, you must add it.\n");
   printf("if you need help with correct syntax, type: \n\t ./watermark -h\n");
   return 1;
}


// translating given string color to hexadecimal

char color[7];
strcpy(color,argv[colorInd+1]); // copying given color into string
color[6]='\0';

char componentR[3],componentG[3],componentB[3]; // divide string into 3 parts ( R G B components)

componentR[0]=color[0];
componentR[1]=color[1];
componentR[2]='\0';
componentG[0]=color[2];
componentG[1]=color[3];
componentG[2]='\0';
componentB[0]=color[4];
componentB[1]=color[5];
componentB[2]='\0';

RGB rgb; // converting string to byte for each component, saving into RGB struct
rgb.red=(byte)strtol(componentR,NULL,16);
rgb.green=(byte)strtol(componentG,NULL,16);
rgb.blue=(byte)strtol(componentB,NULL,16); // number base 16

// ---------------------------------------------------------------------------

//  POSITIONS 
int posX, posY;

if(positionInd==-1) { // -pos option is not mentioned
  posX=0;
  posY=0;
}

else { // position is given in format x,y
char *position=(char*)malloc(strlen(argv[positionInd+1]));
strcpy(position,argv[positionInd+1]);
char pos1[4], pos2[4]; // to store 2 position components
int j,k,l; 

// dividing position string into 2 positions

for (int i=0; i<strlen(position); i++) {
    if(position[i]==',') {
        for (j=0; j<i; j++) {
            pos1[j]=position[j];
        }
        pos1[j]='\0';
        for (k=i+1,l=0; k<strlen(position); k++, l++) {
            pos2[l]=position[k];
        }
        pos2[l]='\0';
        break;
    }
}
 posX=(int)strtol(pos1,NULL,10); //convert string to int
 posY=(int)strtol(pos2,NULL,10); // base 10
}

// ------------------------------------------------

// MESSSAGE FROM USER 
char message[100]; // message of user

if(messageInd!=-1) {
    strcpy(message,argv[messageInd+1]);

//converting string to lower case (because in Morse's alphabet there is no difference between them)

    for (int i=0; i<strlen(message); i++) {
        if(message[i]>=65 && message[i]<=90) {
           // printf("%d\n",message[i]);
            message[i]=message[i]+32; // converting to lower case
        }
    }

} 
// ------------------------------------------------

// DATE 
    
    char date[15];
    int i;
    char c;
    if(dateInd==-1) {
      strcpy(date,"");
    }

    else {
    system("date +%d-%m-%Y > date.txt"); // redirecting the answer of system call to file

    FILE *fpdate=fopen("date.txt","r");

    for (i=0; (c=fgetc(fpdate))!=EOF; i++ ) { // saving date in string
        date[i]=c;
    }
        date[i]='\0'; 

    fclose(fpdate);
    } 
  
 // ------------------------------------------------------------
  // creating one text (from date/ message of user) to hide in file 
  char *text;
  if(messageInd==-1) text= (char *)malloc(sizeof(date)+1);
  else if(dateInd==-1) text=(char *)malloc(sizeof(message)+1);
  else if (messageInd!=-1 && dateInd!=-1) text=(char *)malloc(sizeof(message)+sizeof(date)+2); // for \0 and for space

    int m=0;

    if(messageInd!=-1) {
    for (m=0; m<strlen(message); m++) {
        text[m]=message[m];
    }
    text[m]=' ';
    m++;
    }
    
    if(dateInd!=-1) {
    for (int i=0; i<strlen(date); i++, m++) {
        if(date[i]=='-') text[m]=' ';
        else text[m]=date[i];
    }
    }

// --------------------------------------------------------------------

// ------------------- ENCODING IN MORSE ------------------------------

char codeinmorse[1000];

FILE *fpcodeinmorse=fopen("codeinmorse.txt","w");

for( int i=0; i<strlen(text); i++) {
    fputs(letterToMorse(text[i]),fpcodeinmorse); // writing ready code into file
}

fclose(fpcodeinmorse); // close for write mode
fpcodeinmorse=fopen("codeinmorse.txt","r"); //open in read mode

 for (int i=0; (c=fgetc(fpcodeinmorse))!=EOF; i++ ) { //copying morse code from file to string
        codeinmorse[i]=c;
    }
    
int z=(int)strlen(codeinmorse);
codeinmorse[z]='\0';
fclose(fpcodeinmorse);

//   --------------------------------------------------------------

// Work with old file
    char *filename=argv[1]; // sign file name - old
    FILE *fporiginal;
    fporiginal=fopen(filename,"rb"); // in binary mode

    if(fporiginal==NULL) {
        fputs("Error opening a file", stderr);
        return 1;
    }

    // calculate the size of old file
    fseek(fporiginal, 0, SEEK_END);
    int originalFileSize=ftell(fporiginal);
    rewind(fporiginal); // fseek(fporiginal,0L, SEEK_SET)


// create a buffer to store all bytes of file
    byte *buffer=(byte *)malloc(originalFileSize+1); // '\0'

// check for error
    if(buffer==NULL) {
        fputs("Error arranging a memory", stderr);
        return 1;
    }
// copy everything from original file to buffer
    fread(buffer,sizeof(byte), originalFileSize, fporiginal);
    fclose(fporiginal);

// we are done with original file

// -----------------------------------------------------------

BMPHeader bmph;
DIBHeader dibh;

memmove(&bmph, buffer, sizeof(BMPHeader)); // copying blocks of memory from buffer to bmp header
memmove(&dibh,buffer+sizeof(BMPHeader), sizeof(DIBHeader)); // copying blocks of memory from buffer to dib header

// --------------------------------

// creating new (modified) file


int sumofHeaders= bmph.offset;
int myposition= (dibh.height-posY-1)*dibh.width*3+posX*3;
int byteposition=sumofHeaders+myposition;


// hiding morse code in buffer ( changing the pixels )
for(int i=0; i<strlen(codeinmorse); i++) {

    if(codeinmorse[i]=='.') { // colors are in reversed order : B G R
        buffer[byteposition]=rgb.blue; // blue
        buffer[byteposition+1]=rgb.green; // green 
        buffer[byteposition+2]=rgb.red; // red

    }
    if ((byteposition/3 - (dibh.width-1))%dibh.width == 0 ) { // at the end of line, we need new line
  
        byteposition=byteposition- 3*(2*dibh.width-1); // jumping back in case of row ends
    } 
    else {
        byteposition=byteposition+3;
    }
}

  
if (strcmp(argv[argc-2],"-o")==0) {
  char *newfilename=(char*)malloc(50*sizeof(char));
     newfilename =argv[argc-1]; // modsign file name - new
    FILE *fpmodified;
    fpmodified=fopen(newfilename,"wb"); // in binary mode

    if(fpmodified==NULL) {
        fputs("Error opening a file", stderr);
        return 1;
    }

    fwrite(buffer, bmph.offset, 1, fpmodified); // copied 54 bytes to newfile ( headers )
  fwrite(buffer+bmph.offset, bmph.fileSize-bmph.offset, 1, fpmodified); // write the rest to the file

    
}

else {
  fwrite(buffer, bmph.offset, 1, stdout);
  fwrite(buffer+bmph.offset,bmph.fileSize-bmph.offset, 1, stdout); // write the rest to the file

}

char o_input[50];


       if (strcmp(argv[argc-1],"-special") != 0) { // if i enter the file with -special option => no need to enter this if
         if (strcmp(argv[argc-2],"-o") != 0) {  // if -o option is not mentioned, take stdin from user
        fgets(o_input,50,stdin);
        
        char *command=(char *)malloc(100*sizeof(char)); // command to be executed by system

        for ( i=0; i<argc; i++) {
          command=strcat(command,argv[i]);

          command=strcat(command," ");

        }
        command=strcat(command,"-special ");
        command=strcat(command,o_input);
        system(command);
    }
       }
    return 0;
}

int help(int argc, char *argv[]) {
    if (argc==1  || strcmp(argv[1], "-h") == 0) {
        printf("\nCorrect Syntax to use the Program is\n\n\t./watermark sign.bmp -text yourtext -date -color yourcolor -pos x,y -o yourfile.bmp\n");
        printf("\n EXAMPLE:\n");
        printf("\t./watermark sign.bmp -text Hello -date -color FFFFFE -pos 10,20 -o modsign.bmp\n");
        printf("\n ! Note that the options -text, -date, -pos are optional, you can skip any of them, but not date/text at the same time\n");
        printf("\n ! Note that the option -color is not optional, you can't skip it\n\n");

        return 1;
    }
    return 0;
}


// MORSE  CODEEEEE -----------------------------------------------
char *letterToMorse(char letter) {
switch (letter) { 
  case 'a': 
    return ". ...   "; 
  case 'b': 
    return "... . . .   "; 
  case 'c': 
    return "... . ... .   "; 
  case 'd': 
    return "... . .   "; 
  case 'e': 
    return ".   "; 
  case 'f': 
    return ". . ... .   "; 
  case 'g': 
    return "... ... .   "; 
  case 'h': 
    return ". . . .   "; 
  case 'i': 
    return ". .   "; 
  case 'j': 
    return ". ... ... ...   "; 
  case 'k': 
    return "... . ...   "; 
  case 'l': 
    return ". ... . .   "; 
  case 'm': 
    return "... ...   "; 
  case 'n': 
    return "... .   "; 
  case 'o': 
    return "... ... ...   "; 
  case 'p': 
    return ". ... ... .   "; 
  case 'q': 
    return "... ... . ...   "; 
  case 'r': 
    return ". ... .   "; 
  case 's': 
    return ". . .   "; 
  case 't': 
    return "...   "; 
  case 'u': 
    return ". . ...   "; 
  case 'v': 
    return ". . . ...   "; 
  case 'w': 
    return ". ... ...   "; 
  case 'x': 
    return "... . . ...   "; 
  case 'y': 
    return "... . ... ...   "; 
  case 'z': 
    return "... ... . .   ";

  case '1': 
    return ". ... ... ... ...   ";
      case '2': 
    return ". . ... ... ...   ";
     case '3': 
    return ". . . ... ...   ";
     case '4': 
    return ". . . . ...   ";
     case '5': 
    return ". . . . .   ";
     case '6': 
    return "... . . . .   ";
     case '7': 
    return "... ... . . .   ";
     case '8': 
    return "... ... ... . .   ";
     case '9': 
    return "... ... ... ... .   ";
     case '0': 
    return "... ... ... ... ...   ";
    case ' ':
    return "  " ; // 2 spaces because there is 3 spaces after each letter, so by aadding 2 spaces we obtain 5 spaces
    default :
    return "";
              }
}
