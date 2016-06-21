/*
        A simple program that demonstrates how to program a magnetometer
	on the Raspberry Pi.
	http://ozzmaker.com/2014/12/01/compass1


    Copyright (C) 2014  Mark Williams

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
    MA 02111-1307, USA
*/
#include <stdint.h>
#include "LSM9DS0.h"
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <curl/curl.h>
#include <string.h>

int file;


#define magXmax 906
#define magYmax 1175
#define magZmax 1119
#define magXmin -1081
#define magYmin -1143
#define magZmin -400

void writeMagReg(uint8_t reg, uint8_t value);
void readBlock(uint8_t command, uint8_t size, uint8_t *data);
void readMAG(int * m);




int main(int argc, char *argv[])

{
	CURL *curl;
 	CURLcode res;
 	
    
    
	char filename[20];
	int magRaw[3];

	//Open the i2c bus
	sprintf(filename, "/dev/i2c-%d", 1);
	file = open(filename, O_RDWR);
	if (file<0) {
        	printf("Unable to open I2C bus!");
                exit(1);
	}


	//Select the magnetomoter
	if (ioctl(file, I2C_SLAVE, MAG_ADDRESS) < 0) {
                printf("Error: Could not select magnetometer\n");
        }


	//Enable the magnetometer
	writeMagReg( CTRL_REG5_XM, 0b11110000);   // Temp enable, M data rate = 50Hz
	writeMagReg( CTRL_REG6_XM, 0b01100000);   // +/-12gauss
	writeMagReg( CTRL_REG7_XM, 0b00000000);   // Continuous-conversion mode

	while(1)
	{

		time_t rawtime;
   		struct tm *info, *infoBack;
   		
   		char buffer[80];
   		char bufferBack[80];

   		time( &rawtime );

   		info = localtime( &rawtime );
   		strftime(buffer,80,"%Y-%m-%dT%H:%M:%S", info);

		readMAG(magRaw);

		magRaw[0]-= (magXmin + magXmax) /2 ;
		magRaw[1] -= (magYmin + magYmax) /2 ;
		magRaw[2] -= (magZmin + magZmax) /2 ;

		//printf("%s,%i,%i,%i\n", buffer, magRaw[0],magRaw[1],magRaw[2]);
		
		char sql[255];
		char str[100];
		
		strcpy (sql, "{\"stmt\":\"Insert into mag values (\'");
		
		strcat (sql, buffer);
		strcat (sql, "\',");
		sprintf(str,"%d",magRaw[0]);
		strcat (sql, str);
		strcat (sql, ",");
		sprintf(str, "%d",magRaw[1]);
		strcat (sql, str);
		strcat (sql, ",");
		sprintf(str, "%d",magRaw[2]);
		strcat (sql, str);
		strcat (sql, ")\"}");
        //printf (sql);
        
    	curl = curl_easy_init();
   		if(curl) {
    	curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.132:4200/_sql?pretty");
    	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sql);
    	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(sql));
    	FILE* devnull = fopen("nul", "w"); 
    	curl_easy_setopt(curl, CURLOPT_WRITEDATA, devnull);
 
    	res = curl_easy_perform(curl);
   	 	/* Check for errors */ 
    	if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
  
		//Only needed if the heading value does not increase when the magnetometer is rotated clockwise
		magRaw[1] = -magRaw[1];

		//Sleep for 0.25ms
		usleep(50);

	}

}



void writeMagReg(uint8_t reg, uint8_t value)
{
  int result = i2c_smbus_write_byte_data(file, reg, value);
    if (result == -1)
    {
        printf ("Failed to write byte to I2C Mag.");
        exit(1);
    }
}

void  readBlock(uint8_t command, uint8_t size, uint8_t *data)
{
    int result = i2c_smbus_read_i2c_block_data(file, command, size, data);
    if (result != size)
    {
       printf("Failed to read block from I2C.");
        exit(1);
    }
}

void readMAG(int  *m)
{
        uint8_t block[6];

        readBlock(0x80 | OUT_X_L_M, sizeof(block), block);

        *m = (int16_t)(block[0] | block[1] << 8);
        *(m+1) = (int16_t)(block[2] | block[3] << 8);
        *(m+2) = (int16_t)(block[4] | block[5] << 8);

}






