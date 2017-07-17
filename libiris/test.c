#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "iid.h"

#define LENGTH	15235

struct iris_custom_image_header icih_r[] = {
	[0] = {
		.image_number = 1,
		.image_quality = 0x48,
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
struct iris_custom_image_header icih_l[] = {
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
	.image_headers = icih_l,
};
	
struct iris_custom_biometric_subtype_header right = {
	.eye_position = 0x01,
	.num_images = 1,
	.image_headers = icih_r,
};
	
struct iris_custom_data ich = {
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
	
#if 1	//for 2011
struct iris_custom_representation_header icrh[] = {
	[0] = {
	//.representation_length = ,
	//.capture_data = 0x123456789987654321,
	.capture_device_tech_id = 0x0A,
	.capture_device_vendor_id = 0x0B0C,
	.capture_device_type_id = 0x0D0E,
	//.quality_block
	.representation_num = 1,
	.eye_label = 1,
	.image_type = 2,
	//uint8_t		image_format;
	.image_property_bitfield = 0x04,
	.image_width = 640,
	.image_height = 480,
	//.bit_depth;
	.range = 180,
	.rotation_angle = 30,
	.rotation_uncertainty = 50,
	.iris_centre_minX = 100,
	.iris_centre_maxX = 150,
	.iris_centre_minY = 100,
	.iris_centre_maxY = 150,
	.iris_diameter_min = 100,
	.iris_diameter_max = 150,
	//.image_length;
	//.image_data
	},
	[1] = {
	//.representation_length = ,
	//.capture_data = ,
	.capture_device_tech_id = 0x0A,
	.capture_device_vendor_id = 0x0B0C,
	.capture_device_type_id = 0x0D0E,
	//.quality_block
	.representation_num = 2,
	.eye_label = 2,
	.image_type = 2,
	//uint8_t		image_format;
	.image_property_bitfield = 0x04,
	.image_width = 640,
	.image_height = 480,
	//.bit_depth;
	.range = 180,
	.rotation_angle = 30,
	.rotation_uncertainty = 50,
	.iris_centre_minX = 100,
	.iris_centre_maxX = 150,
	.iris_centre_minY = 100,
	.iris_centre_maxY = 150,
	.iris_diameter_min = 100,
	.iris_diameter_max = 150,
	//.image_length;
	//.image_data
	},
};

struct iris_custom_data_2011 icd2011 = {
	.format_version = 0x30323000,
	.num_representations = 2,
	.cert_flag = 0x00,
	.num_eyes = 1,
	.icrh_image = icrh,
};
#endif	//end for 2011

int test2005()
{
	unsigned char buf[LENGTH];
	unsigned char* image[4];
	unsigned int imageSize[1];
	unsigned char *data = NULL;
	int len = 0;
	unsigned int bufLen = 0;
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
	fw = fopen("TEST_2005.iso","ab+");
	if(fp == NULL)
	{
		perror("fopen TEST_2005.iso failed\n");
		return -1;
	}
	fseek(fp,59,SEEK_SET);
	while(/*!feof(fp)*/(count = fread(buf,LENGTH,1,fp)) > 0)
	{
		//printf("fread count=%d\n",count);
	}
	/*image[0] = buf;
	image[1] = buf;
	image[2] = buf;
	image[3] = buf;*/
	icih_r[0].image_data = buf;
	icih_l[0].image_data = buf;
	
	//get buffer length
	len = getRecordLen2005(LENGTH * 1,1,1,0);
	
	//malloc buffer
	data = (char*)malloc(len);
	if(data == NULL)
	{
		printf("malloc failed\n");
		return -1;
	}else{
		memset(data,0,len);
	}
	
	ich.record_length = len;
	for(i = 0;i < 16;i++)
		ich.device_unique_id[i] = i + 1;
	
	//init bdb
	//INIT_BDB(&bdb,data,len);
	
	icih_r[0].image_length = LENGTH;
	icih_l[0].image_length = LENGTH;
	
	capsuleISO2005(&ich,data,&bufLen);
	
	/*for(i = 0;i < bufLen;i++)
	{
		if(i % 16 == 0)
			printf("\n");
		printf("0x%02x ",data[i]);
	}*/
	printf("\n");
	//write buffer to TEST.iso
	fwrite(data,len,1,fw);
	
	return 0;
}

int test2011()
{
	unsigned char buf[LENGTH];
	unsigned char* image[4];
	unsigned int imageSize[1];
	unsigned char *data = NULL;
	unsigned char qual_block_0[11] = {0x02,0x02,0x03,0x04,0x05,0x06,0x05,0x04,0x03,0x02,0x01};
	unsigned char qual_block_1 = 0x00;
	int len = 0;
	unsigned int bufLen = 0;
	int count = 0;
	int i = 0;
	
	struct tm* pNow;
	time_t tt;
	
	BDB bdb;
	
	FILE *fw = NULL;
	FILE *fp = NULL;
	fp = fopen("TEST_IIR.iso","rb");
	if(fp == NULL)
	{
		perror("open TEST_IIR.iso error");
		return -1;
	}
	fw = fopen("TEST_2011.iso","ab+");
	if(fp == NULL)
	{
		perror("fopen TEST_2011.iso failed\n");
		return -1;
	}
	fseek(fp,59,SEEK_SET);
	while(/*!feof(fp)*/(count = fread(buf,LENGTH,1,fp)) > 0)
	{
		//printf("fread count=%d\n",count);
	}
	
	len = getRecordLen2011(LENGTH * 2,2,2);
	printf("len = %d\n",len);
	//malloc buffer
	data = (char*)malloc(len);
	if(data == NULL)
	{
		printf("malloc failed\n");
		return -1;
	}else{
		memset(data,0,len);
	}
	
	icd2011.record_length = len;
	
	tt = time(NULL);
	pNow = localtime(&tt);
	icd2011.icrh_image[0].quality_block = qual_block_0;
	icd2011.icrh_image[0].image_length = LENGTH;
	icd2011.icrh_image[0].image_data = buf;
	icd2011.icrh_image[0].capture_data = pNow;
	
	tt = time(NULL);
	pNow = localtime(&tt);
	icd2011.icrh_image[1].quality_block = &qual_block_1;
	icd2011.icrh_image[1].image_length = LENGTH;
	icd2011.icrh_image[1].image_data = buf;
	icd2011.icrh_image[1].capture_data = pNow;
	
	capsuleISO2011(&icd2011,data,&bufLen);
	
	for(i = 0;i < bufLen;i++)
	{
		if(i % 16 == 0)
			printf("\n");
		printf("0x%02x ",data[i]);
	}
	printf("\n");
	//write buffer to TEST.iso
	fwrite(data,len,1,fw);
	return 0;
}
	
int main(void)
{
	
	//test2005();
	test2011();
	return 0;
}
