#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iid.h"

#define LENGTH	15235

struct iris_custom_image_header icih[] = {
	[0] = {
		.image_number = 1,
		.image_quality = 0x40,
		.rotation_angle = 0xFFFF,
		.rotation_uncertainty = 0xFFFF,
	},
	[1] = {
		.image_number = 2,
		.image_quality = 0x48,
		.rotation_angle = 0xFFFF,
		.rotation_uncertainty = 0xFFFF,
	}
};
struct iris_custom_biometric_subtype_header left = {
	.eye_position = 0x02,
	.num_images = 1,
	.image_headers = icih,
};
	
struct iris_custom_biometric_subtype_header right = {
	.eye_position = 0x01,
	.num_images = 1,
	.image_headers = icih,
};
	
struct iris_custom_data irh = {
	.format_version = 0x30323000,
	.capture_device_id = 0x1111,
	.num_eyes = 2,
	.image_property_bitfield = 0x1234,
	.diameter = 16,
	.image_width = 640,
	.image_height = 480,
	.image_transformation = 0x01,
	//.device_unique_id = 0x0123456789ABCDEF,
	.ibsh_right = &right,
	.ibsh_left = &left,
};
	
int main(void)
{
	unsigned char buf[LENGTH];
	unsigned char* image[4];
	unsigned int imageSize[1];
	char *data = NULL;
	int len = 0;
	int count = 0;
	int i = 0;
	
	BDB bdb;
	
	FILE *fw = NULL;
	FILE *fp = NULL;
	fp = fopen("TEST_IIR.iso","rb");
	if(fp == NULL)
	{
		perror("open TEST_IIR.iso error");
		return -1;
	}
	fw = fopen("TEST.iso","ab+");
	if(fp == NULL)
	{
		perror("fopen TEST.iso failed\n");
		return -1;
	}
	fseek(fp,59,SEEK_SET);
	while(/*!feof(fp)*/(count = fread(buf,LENGTH,1,fp)) > 0)
	{
		printf("fread count=%d\n",count);
	}
	image[0] = buf;
	image[1] = buf;
	image[2] = buf;
	image[3] = buf;
	
	//get buffer length
	len = getRecordLen(LENGTH * 2,2,1,1);
	
	//malloc buffer
	data = (char*)malloc(len);
	if(data == NULL)
	{
		printf("malloc failed\n");
		return -1;
	}else{
		memset(data,0,len);
	}
	
	irh.record_length = len;
	for(i = 0;i < 16;i++)
		irh.device_unique_id[i] = i + 1;
	
	//init bdb
	INIT_BDB(&bdb,data,len);
	
	icih[0].image_length = LENGTH;
	icih[1].image_length = LENGTH;
	
	capsuleISO(&irh,image,&bdb);
	
	printf("len = %d\n",len);
	/*for(i = 0;i < len;i++)
	{
		if(i % 16 == 0)
			printf("\n");
		printf("0x%02x ",bdb.bdb_start[i]);
	}*/
	//write buffer to TEST.iso
	fwrite(bdb.bdb_start,len,1,fw);
	
	return 0;
}