//  The Decoding Algorith works by adding the differences, starting with the First timestamp which was stored
//  as it is.
//      Say we have coded_timestamp val:
//          First Coded Timestamp:  1364281200.78739
//          Second Coded Timestamp: 32.672875

//      The decoded_timestamp will be:
//          First Coded Timestamp:  1364281200.078739   (Obtained by stuffing zeros so that num2 length is equal to 6)
//          Second Coded Timestamp: 1364281232.672875   (Obtained by adding 32 to First timestamp num1 value)
#include"config.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/time.h>
#include<math.h>


class MyOnlineDecoder
{
	public:

		MyOnlineDecoder(){count=0; total_time=0; size=0; bitPtr=0; bytePtr=0; memset(buf,0,BUF_SZ); }
		// parse input file and decode - this function should
		// incrementally decode and write every new timestamp
		// to output file
		void parseAndDecode(FILE* input_file_,FILE* output_file_);
		
		// get count		
		int get_count(){return count;}
		
		// get avg time
		double get_avg_time(){return total_time/get_count();}

		// get mean
		double get_mean(){return total_time/get_count(); }

		// get standard deviation
		double get_std_dev();

		// get next number
		long long getNextNum(FILE* fp);

		int getCodeType(FILE* fp);

		// convert long long to char*
		char* convert2str(long long num);

		// Read from buf
		void read_frm_buf(uint8_t *data,bool incr,FILE* fp);

    // set 1s for the bit length of l
    int set_1s_for(int l);


	private:
		int count;
    long long prevNum;
		int num2_len;
	  struct timeval startTime;
  	struct timeval endTime;
  	double tS,tE,time;
  	double total_time;

		double t[SIZE];
  	long size;

    int bitPtr,bytePtr;
    uint8_t buf[BUF_SZ];

};

// generate a sequence of l 1's
int MyOnlineDecoder::set_1s_for(int l){
  int set = 1;
  for(int i=1;i<l;i++)
    set = set<<1 | 0b1;
  return set;
}

void MyOnlineDecoder::read_frm_buf(uint8_t *data,bool incr,FILE* fp){
  if(incr)
    *data = buf[bytePtr++];
  else
    *data = buf[bytePtr];

  if(bytePtr==BUF_SZ){
    memset(buf,0,BUF_SZ);
    fread(buf,BUF_SZ,1,fp);
    bytePtr = 0;
  }
}

char* MyOnlineDecoder::convert2str(long long num){
  char t;
  static char ts[18];
  int cnt=16;
  while(num!=0){
    if(cnt == 10)
    {
      ts[cnt]='.';
      cnt--;
    }
    t = num%10;
    ts[cnt]=t+48;
    num=num/10;
    cnt--;
  }
  return ts;
}

double MyOnlineDecoder::get_std_dev(){
	// Caluclation of Standard Deviation  
	double avg_time,mean,sum_of_diff,variance,std_dev;
  avg_time = mean = total_time/get_count();

  for(int i=0;i<size;i++)
    sum_of_diff += (t[i]-mean)*(t[i]-mean);

  variance = sum_of_diff/(get_count()-1);
  std_dev = sqrt(variance);

	return std_dev;
}

// get the next coded num from the binary file
long long MyOnlineDecoder::getNextNum(FILE* fp){
	long long x;
	int ret;
	ret = fread(&x, sizeof(x), 1, fp);
	if(ret == 0)
		return -1;
	return x;
}

int MyOnlineDecoder::getCodeType(FILE* fp){
	uint8_t header=0;
	if(bitPtr==0)
		read_frm_buf(&header,true,fp);
	else{
    uint8_t head1,head2;
    read_frm_buf(&head1,true,fp);
    read_frm_buf(&head2,false,fp);
	  header = head1<<bitPtr | head2>>(8-bitPtr);
	}
  return header;
}

// Parse & Decode the encoded file
void MyOnlineDecoder::parseAndDecode(FILE* input_file_,FILE* output_file_){
	fpos_t position;
	int first=1;
	long long num;
	int n;
	int l1,l2;
	int num1,num2;
	int x;
  while(1){ 
			if(count>=SIZE)
				break;
			count++;
			gettimeofday(&startTime, NULL);   // get the start time
			char *stuffed;
			// If it's the first coded timestamp, then it does no decoding, i.e decodes 
			// as 'num1.num2' (as it is)
			// Would store prevNum1 = num1 & prevNum2 = num2
			if(first){ // The First Coded Time Stamp
        prevNum = getNextNum(input_file_);
        fprintf(output_file_,"%s",convert2str(prevNum));
        first = 0;
				fread(buf,BUF_SZ,1,input_file_);
			}  
			else { 
				x = getCodeType(input_file_);
				int type,len;
		    type = x>>7;
		    len = x & 0x7F;
				if(type==0){ 
        	fprintf(output_file_,"%s",convert2str(prevNum));
				} 
				else if(type==1){ 
					int diff;
			    unsigned int data=0;
			    bool done;
					if(bitPtr==0)
					{ 
						done = false;
				    while(!done){ 
							uint8_t byte=0;
				      if(len<8){ 
								read_frm_buf(&byte,false,input_file_);
				        data |= byte>>(8-len);
				        bitPtr = len;
				        done = true;
				      } 
				      else
	     				{ 		
								read_frm_buf(&byte,true,input_file_);
		 			      data |= byte<<(len-8);
  	      			len = len-8;
    	  			} 
    				}	
					} 
					else 
					{	data = 0; 
						done = false;
						while(!done){ 
				     if(len<8){
			         	uint8_t byte,dat;
			         	if(len<=(8-bitPtr)){

									if((bitPtr+len)%8==0)
			            	read_frm_buf(&dat,true,input_file_);
									else
			            	read_frm_buf(&dat,false,input_file_);
	
		            	byte = dat>>(8-bitPtr-len) & set_1s_for(len);
		            	bitPtr = (bitPtr+len)%8;
		            	data |= byte;
		          	}
		          	else
		          	{
		           	 	uint8_t byte1,byte2;
		            	read_frm_buf(&byte1,true,input_file_);
		            	read_frm_buf(&byte2,false,input_file_);

									int shift = (bitPtr);

		            	byte = (byte1 & set_1s_for(8-shift))<<(shift) | byte2>>(8-shift);
		            	data |= byte;
		          	}
		          	done = true;
    			   	}
							else
							{ 
			          uint8_t byte1,byte2,byte;
			          read_frm_buf(&byte1,true,input_file_);
			          read_frm_buf(&byte2,false,input_file_);
			          byte = byte1<<(bitPtr) | byte2>>(8-bitPtr);
			          data |= byte<<(len-8);
			          len = len-8;
							} 
						} 
					} 
					diff = data;
					prevNum = prevNum + diff;
        	fprintf(output_file_,"%s",convert2str(prevNum));
				} 	
			} 
			fprintf(output_file_,"\n");
			gettimeofday(&endTime, NULL);  // get the end time
		  tS = startTime.tv_sec*1000000 + (startTime.tv_usec);
	    tE = endTime.tv_sec*1000000  + (endTime.tv_usec);
  	  time = tE - tS;
			t[size]=time;
			size++;
  	  total_time += time;
		} 
}

int main(int argc, char **argv)
{
  if(argc<2 || argc>2){
    printf("Usage: %s <coded_timestamp_file>\n",argv[0]);
    exit(-1);
   }

  FILE *fp,*out_fp,*stats;
  fp = fopen(argv[1],"rb");
  if (fp == NULL) {
    printf("Failed to open %s for reading.\n",argv[1]);
    exit(0);
  }

  out_fp = fopen("decoded_timestamps","w");
  if (out_fp == NULL) {
    printf("Failed to open %s for writing.\n","decoded_timestamps");
    exit(0);
  }

  stats = fopen("statistics.txt","a");
  if (stats == NULL) {
    printf("Failed to open %s for writing.\n","statistics.txt");
    exit(0);
  }



	MyOnlineDecoder decoder;	
	decoder.parseAndDecode(fp,out_fp);

  fprintf(stats,"\n ---- DECOMPRESSION STATISTICS --- ");
	fprintf(stats,"\nAvg time to decompress in usecs: %.0lf",decoder.get_avg_time());
  fprintf(stats,"\nMean:  %.0lf",decoder.get_mean());
  fprintf(stats,"\nStandard Deviation:  %.0lf\n",decoder.get_std_dev());

	fclose(fp);
	fclose(out_fp);
	fclose(stats);
	return 0;	
}
