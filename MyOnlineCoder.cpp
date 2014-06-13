//  The Coding Algorithm works by coding only the difference of the current value to it's previous value.
//  It breaks the timestamp into two parts,
//       Say we have timestamp val: 1364281200.078739
//       We break down into: num1 = 1364281200 & num2 = 078739
#include"config.h"
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include <sys/time.h>
#include<math.h>

class MyOnlineCoder
{
    public:
        MyOnlineCoder(){ prevNum=0; iter_count=0; total_bits=0; bitPtr=0; bytePtr=0; 
                                       memset(buf,0,BUF_SZ);
                                     }

        // read next timestamp from input file and return as char*
        char* getNextLine(FILE* fp);

        // convert timestamp into internal representaion and write to file
        void codeNextTimestamp(char* tstamp_,FILE* output_file_);

            // get binary length of the given number
            int get_bin_length(long num);

        // get avg bits
        double get_avg_bits() {return (total_bits*1.0)/(iter_count*1.0);}

        // get count
        int get_count() {return iter_count;}

        // convert char* timestamp to long long
        long long convert2ll(char *tstamp_);

        // write to buffer
        void write_to_buf(uint8_t data,bool incr,FILE* fp);

        // get current buffer
        uint8_t get_curr_byte();

            // set 1s for the bit length of l
            int set_1s_for(int l);

        // Write remaining bytes if any
        void write_rem(FILE *fp);   
    private:
        long long prevNum;
        int iter_count; 
        long long total_bits;


        int bitPtr,bytePtr;
        uint8_t buf[BUF_SZ];


};

// generate a sequence of l 1's
int MyOnlineCoder::set_1s_for(int l){
  int set = 1;
  for(int i=1;i<l;i++)
    set = set<<1 | 0b1;
  return set;
}

uint8_t MyOnlineCoder::get_curr_byte()
{
    return buf[bytePtr];
}

void MyOnlineCoder::write_to_buf(uint8_t data,bool incr,FILE *fp)
{
  buf[bytePtr] = data;

  if(incr==true)
    bytePtr++;
  if(bytePtr==BUF_SZ){
    fwrite(buf,BUF_SZ,1,fp);
    memset(buf,0,BUF_SZ);
    bytePtr=0;
  }
}


long long MyOnlineCoder::convert2ll(char *tstamp_){
  int i,j=0;
  long long num;
  for(i=0;i<strlen(tstamp_);j++)
  {
    if(tstamp_[j]=='.')
      continue;
    tstamp_[i++] = tstamp_[j];
  }
  sscanf(tstamp_,"%lld",&num);
  return num;
}

char* MyOnlineCoder::getNextLine(FILE* fp)
{
  char * line = NULL;
    size_t len = 0;
  ssize_t read;

    if(getline(&line, &len, fp)==-1)
        return NULL;

    return line;
}

void MyOnlineCoder::write_rem(FILE *fp)
{
        // Write remaining bytes if any!
    fwrite(buf,bytePtr,1,fp);
}


// get binary length of the number
int MyOnlineCoder::get_bin_length(long num){
  int count=0;
  while(num!=0){
    count++;
    num/=2;
  }
  return count;
}

// code the Next TimeStamp
void MyOnlineCoder::codeNextTimestamp(char* tstamp_,FILE* output_file_)
{
    char buffer[240];
    int no_of_bits;
    long long num;
    int l;
    num = convert2ll(tstamp_);

    // If it's the first timestamp, then it does no coding, i.e codes 
    // as 'num1.num2' (as it is!)
    // Would store prevNum1 = num1 & prevNum2 = num2
    if(prevNum==0){  // The First Time Stamp
        int bits;
        prevNum = num;
            fwrite(&prevNum, sizeof(prevNum),1, output_file_);  
        no_of_bits=get_bin_length(prevNum);
        // Write Data as it is for First TimeStamp
    }   
    else if(num==prevNum){
            int type = 0;
            int len = 0;
            unsigned int header = type<<7|len;
            if(bitPtr==0)
            {
                write_to_buf(header,true,output_file_);
            }
            else
            {
                uint8_t head1,head2;
                head1 = get_curr_byte();
                head1 |= header>>(bitPtr);
                    head2 = (header & set_1s_for(bitPtr))<<(8-bitPtr);
                write_to_buf(head1,true,output_file_);                
                write_to_buf(head2,false,output_file_);  
            }
                no_of_bits = 1*8;
    }
    else // num != prevNum
    {                               
        bool done;                  
        int diff,data;
        diff = num - prevNum;
        data = diff;
        prevNum = num;
        int type = 1;
        int len = get_bin_length(diff);
        int header = type<<7|len;
        int total_len = len+8;
        if(bitPtr==0)
        {
            write_to_buf(header,true,output_file_);
            done = false;
                while(!done){
                    uint8_t byte;
                    if(len<8){

                        byte = ((data & set_1s_for(len))<<(8-len));
                        write_to_buf(byte & 0xFF,false,output_file_);
                        bitPtr = len;
                        done = true;
                    }
                    else
                    {
                        byte = (data>>(len-8));
                        write_to_buf(byte,true,output_file_);
                        len = len-8;
                    }
            }
        }
        else
        {       // If bitPtr is not pointing to 0
                    uint8_t head1,head2;
                head1 = get_curr_byte();
                    head1 |= header>>(bitPtr);
                    head2 = (header & set_1s_for(bitPtr))<<(8-bitPtr);

                    write_to_buf(head1,true,output_file_); 
                    write_to_buf(head2,false,output_file_);

                done = false;
                while(!done){
                        if(len<8){
                            uint8_t byte,dat;
                            if(len<=(8-bitPtr)){
                                byte = data & set_1s_for(len);
                                dat = get_curr_byte();
                                dat |= byte<<(8-bitPtr-len);

                            if((bitPtr+len)%8==0)
                                    write_to_buf(dat & 0xFF,true,output_file_);
                            else
                                    write_to_buf(dat & 0xFF,false,output_file_);

                            bitPtr = (bitPtr+len)%8;
                            }
                            else
                            {   
                                byte = data & set_1s_for(len);
                                dat = get_curr_byte();

                            int shift=bitPtr;


                                dat |= byte>>(shift);
                                write_to_buf(dat,true,output_file_);
                                write_to_buf(((byte & set_1s_for(shift))<<(8-shift)),false,output_file_);
                            }
                            done = true;
                        }
                        else
                    {
                            uint8_t byte,dat;
                            byte = data>>(len-8);
                            dat = get_curr_byte();
                            dat |= byte>>(bitPtr);
                            write_to_buf(dat,true,output_file_);
                            write_to_buf((byte & set_1s_for(bitPtr))<<(8-bitPtr),false,output_file_);
                            len = len-8;
                    }
                }
        }
        no_of_bits = total_len;
    }
    total_bits+=no_of_bits;
    iter_count++;
}

int main(int argc, char **argv)
{
  if(argc<2 || argc>2){
    printf("Usage: %s <timestamp_file>\n",argv[0]);
    exit(-1);
   }

  FILE *fp,*out_fp,*stats;
  fp = fopen(argv[1],"r");
  if (fp == NULL) {
    printf("Failed to open %s for reading.\n",argv[1]);
    exit(-2);
  }

  out_fp = fopen("coded_timestamps","wb");  
  if (out_fp == NULL) {
    printf("Failed to open %s for writing.\n","coded_timestamps");
    exit(-3);
  }

  stats = fopen("statistics.txt","w");
  if (stats == NULL) {
    printf("Failed to open %s for writing.\n","statistics.txt");
    exit(-4);
  }

    MyOnlineCoder coder;
    char *tstamp;

    struct timeval startTime;
    struct timeval endTime;
    double tS,tE,time;
    double total_time=0;
    double t[451210];   
    long size=0;
    while((tstamp=coder.getNextLine(fp))){
        gettimeofday(&startTime, NULL);   // get the start time
            coder.codeNextTimestamp(tstamp,out_fp);
        gettimeofday(&endTime, NULL);  // get the end time
            tS = startTime.tv_sec*1000000 + (startTime.tv_usec);
            tE = endTime.tv_sec*1000000  + (endTime.tv_usec);
        time = tE - tS;
        t[size]=time;
        size++;
        total_time += time;
        free(tstamp);
    }
    coder.write_rem(out_fp);
    // Caluclation of Standard Deviation    
    double avg_time,mean,sum_of_diff,variance,std_dev;
    avg_time = mean = total_time/coder.get_count();

    for(int i=0;i<size;i++)
        sum_of_diff += (t[i]-mean)*(t[i]-mean);
 
    variance = sum_of_diff/(coder.get_count()-1);
    std_dev = sqrt(variance);

    fprintf(stats," ---- COMPRESSION STATISTICS --- ");
    fprintf(stats,"\nDegree of compression(Avg bits): %f",coder.get_avg_bits());
    fprintf(stats,"\nAvg time to compress in usecs: %.0lf",avg_time);
    fprintf(stats,"\nMean:  %.0lf",mean);
    fprintf(stats,"\nStandard Deviation:  %.0lf\n",std_dev);

    fclose(fp);
    fclose(out_fp);
    fclose(stats);
    return 0;
}