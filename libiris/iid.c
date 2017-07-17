#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "iid.h"

void capsuleIRH(struct iris_record_header* irh,struct iris_custom_data* icd);
void capsuleIIH(struct iris_image_header* iih,struct iris_custom_data* icd,unsigned int index,unsigned char right_or_left);
void capsuleIBSH(struct iris_biometric_subtype_header* ibsh,struct iris_custom_data* icd,unsigned char right_or_left);

void capsuleIGH(struct iris_general_header* igh,struct iris_custom_data_2011* icd);
//int capsuleIREH(struct iris_representation_header* ireh,ICRH* icrh,BDB *bdb);
int capsuleIREH(struct iris_representation_header* ireh,ICRH* icrh,unsigned char** quality_block);
void freeQB(unsigned char**quality_block);

#if 1	//for 2011
/*
*Function description：get iris general data length for 2011
*parameter：
*	imageSize：total size of iris image
*	numRepre   :	total num of representations
*	numQual	   :	total num of quality block
*return：
*		iris general data length
*/
int getRecordLen2011(unsigned int imageSize,unsigned int numRepre,unsigned int numQual)
{
	int len = 0;
	
	len += IID_GENERAL_HEADER_LENGTH;
	len += numRepre * IID_REPRESENTATION_HEADER_LENGTH;
	len += numQual * IID_QUALITY_BLOCK_LEN;
	len += imageSize;
	return len;
}

/*
*Function description：Pack iris records for 2011
*parameter：
*	icd(in)		:User Settings information
*	data(out)	:iso format for iris image data
*	iso_data_len(out)	:data len
*return:
*		0:success	-1:error
*/
int capsuleISO2011(struct iris_custom_data_2011* icd,unsigned char* data,int* iso_data_len)
{
	int i = 0;
	int j = 0;
	int ret = 0;
	int imageIndex = 0;
	struct iris_general_header igh;
	struct iris_representation_header ireh;
	unsigned char* quality_block = NULL;

	BDB bdb;
	
	INIT_BDB(&bdb,data,icd->record_length);
	*iso_data_len = icd->record_length;
	
	//init iris general header
	capsuleIGH(&igh,icd);
	OPUSH(&igh,sizeof(struct iris_general_header),&bdb);
	
	//init iris representations header and image
	for(i = 0;i < icd->num_representations;i++)
	{
		//capsuleIREH(&ireh,&icd->icrh_image[i],&bdb);
		ret = capsuleIREH(&ireh,&icd->icrh_image[i],&quality_block);
		if(ret < 0)
			goto err_out;
		
		OPUSH(&ireh,IID_REPRE_HEADER_LEN_BEFORE_QUALITY_BLOCK,&bdb);
		OPUSH(quality_block,quality_block[0] * IID_QUALITY_BLOCK_LEN + 1,&bdb);
		OPUSH(&(ireh.representation_num),IID_REPRE_HEADER_LEN_AFTER_QUALITY_BLOCK,&bdb);
		
		OPUSH(icd->icrh_image[i].image_data,icd->icrh_image[i].image_length,&bdb);

		freeQB(&quality_block);
	}
	
	return 0;	
err_out:
	printf("error\n");
	return -1;
}

int capsuleIREH(struct iris_representation_header* ireh,ICRH* icrh,unsigned char** quality_block)
{
	int qb_len = 1;
	int i = 0;
	int len = 0;
	uint8_t data[IID_CAPTURE_DATA_LEN] = {0};
	
	len = IID_REPRESENTATION_HEADER_LENGTH + ( icrh->quality_block[0] * IID_QUALITY_BLOCK_LEN) +  icrh->image_length;
	
	//representation length
	for(i = 0;i < IID_FOUR_BYTE;i++)
		ireh->representation_length[i] = (len & (0xFF000000 >> (i * 8))) >> (IID_FOUR_BYTE - i - 1) * 8;
	
	//capture data
	data[0] = ((icrh->capture_data->tm_year + 1900) & 0xFF00) >> 8;
	data[1] = (icrh->capture_data->tm_year + 1900) & 0x00FF;
	data[2] = (icrh->capture_data->tm_mon+1) & 0xFF;
	data[3] = icrh->capture_data->tm_mday & 0xFF;
	data[4] = icrh->capture_data->tm_hour & 0xFF;
	data[5] = icrh->capture_data->tm_min & 0xFF;
	data[6] = icrh->capture_data->tm_sec & 0xFF;
	data[7] = data[8] = 0xFF;
	for(i = 0;i < IID_CAPTURE_DATA_LEN;i++)
		ireh->capture_data[i] = data[i];
	
	//capture device tech id
	ireh->capture_device_tech_id =  icrh->capture_device_tech_id;
	
	//capture device vendor id
	for(i = 0;i < IID_TWO_BYTE;i++)
		ireh->capture_device_vendor_id[i] = (icrh->capture_device_vendor_id & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//capture device type id
	for(i = 0;i < IID_TWO_BYTE;i++)
		ireh->capture_device_type_id[i] = (icrh->capture_device_type_id & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//quality block
	qb_len += icrh->quality_block[0] * IID_QUALITY_BLOCK_LEN;
	
	*quality_block = malloc(sizeof(unsigned char) * qb_len);
	if(*quality_block == NULL)
	{
		printf("malloc failed\n");
		return -1;
	}else{
		memset(*quality_block,0,qb_len);
	}
	
	(*quality_block)[0] = icrh->quality_block[0];
	for(i = 0;i < qb_len - 1;i++)
		(*quality_block)[i + 1] = icrh->quality_block[i + 1];
	
	//representation num
	for(i = 0;i < IID_TWO_BYTE;i++)
		ireh->representation_num[i] = (icrh->representation_num & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//eye label
	ireh->eye_label = icrh->eye_label;
	
	//image type
	ireh->image_type  = icrh->image_type;
	
	//image format
	ireh->image_format = IMAGEFORMAT_MONO_JPEG2000_2011;
	
	//image property bitfield
	ireh->image_property_bitfield = icrh->image_property_bitfield;
	
	//image width
	for(i = 0;i < IID_TWO_BYTE;i++)
		ireh->image_width[i] = (icrh->image_width  & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//image height
	for(i = 0;i < IID_TWO_BYTE;i++)
		ireh->image_height[i] = (icrh->image_height & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//bit depth
	ireh->bit_depth = INTENSITY_DEPTH;
	
	//range
	for(i = 0;i < IID_TWO_BYTE;i++)
		ireh->range[i] = (icrh->range & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//rotation angle
	for(i = 0;i < IID_TWO_BYTE;i++)
		ireh->rotation_angle[i] = (icrh->rotation_angle & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//rotation uncertainty
	for(i = 0;i < IID_TWO_BYTE;i++)
		ireh->rotation_uncertainty[i] = (icrh->rotation_uncertainty & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//iris centre minX
	for(i = 0;i < IID_TWO_BYTE;i++)
		ireh->iris_centre_minX[i] = (icrh->iris_centre_minX & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//iris centre maxX
	for(i = 0;i < IID_TWO_BYTE;i++)
		ireh->iris_centre_maxX[i] = (icrh->iris_centre_maxX & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//iris_centre_minY
	for(i = 0;i < IID_TWO_BYTE;i++)
		ireh->iris_centre_minY[i] = (icrh->iris_centre_minY & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//iris centre maxY
	for(i = 0;i < IID_TWO_BYTE;i++)
		ireh->iris_centre_maxY[i] = (icrh->iris_centre_maxY & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//iris diameter min
	for(i = 0;i < IID_TWO_BYTE;i++)
		ireh->iris_diameter_min[i] = (icrh->iris_diameter_min & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//iris diameter max
	for(i = 0;i < IID_TWO_BYTE;i++)
		ireh->iris_diameter_max[i] = (icrh->iris_diameter_max & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//image length
	for(i = 0;i < IID_FOUR_BYTE;i++)
		ireh->image_length[i] = (icrh->image_length & (0xFF000000 >> (i * 8))) >> (IID_FOUR_BYTE - i - 1) * 8;
	
	return 0;
}

void freeQB(unsigned char**quality_block)
{
	if(*quality_block != NULL)
	{
		free(*quality_block);
		*quality_block = NULL;
	}
}

void capsuleIGH(struct iris_general_header* igh,struct iris_custom_data_2011* icd)
{
	int i = 0;
	
	//format id
	for(i = 0;i < IID_FORMAT_ID_LEN;i++)
		igh->format_id[i] = (IID_FORMAT_ID_HEX & (0xFF000000 >> (i * 8))) >> (IID_FORMAT_ID_LEN - i - 1) * 8;
	
	//format version number
	for(i = 0;i < IID_FORMAT_VERSION_LEN;i++)
	{
		igh->format_version[i] = (icd->format_version & (0xFF000000 >> (i * 8))) >> (IID_FORMAT_ID_LEN - i - 1) * 8;
	}
	//record length
	for(i = 0;i < IID_FOUR_BYTE;i++)
		igh->record_length[i] = (icd->record_length & (0xFF000000 >> (i * 8))) >> (IID_FOUR_BYTE - i - 1) * 8;
	
	///device id
	for(i = 0;i < IID_TWO_BYTE;i++)
		igh->num_representations[i] = (icd->num_representations & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//cert flag
	igh->cert_flag = icd->cert_flag;
	
	//num eyes
	igh->num_eyes = icd->num_eyes;
	
}
#endif 	//for 2011

#if 1	//for 2005
/*
*Function description：get iris record data length for 2005
*parameter：
*	imageSize：total size of iris image
*	numEye   :	num of eyes
*	numRight ：iris image num of right eye
*	numLeft  ：iris image num of left eye
*return：
*		iris record data length
*/
int getRecordLen2005(unsigned int imageSize,unsigned int numEye,unsigned int numRight,unsigned int numLeft)
{
	int len = 0;
	
	len += IID_RECORD_HEADER_LENGTH;
	len += numEye * IID_IBSH_LENGTH;
	len += numRight * IID_IIH_LENGTH;
	len += numLeft * IID_IIH_LENGTH;
	len += imageSize;
	return len;
}

/*
*Function description：Pack iris records
*parameter：
*	icd(in)		:User Settings information
*	data(out)	:iso format for iris image data
*	iso_data_len(out)	:data len
*return:
*		0:success	-1:error
*/
int capsuleISO2005(struct iris_custom_data* icd,unsigned char* data,int* iso_data_len)
{
	int i = 0;
	int j = 0;
	int imageIndex = 0;
	struct iris_record_header irh;
	struct iris_biometric_subtype_header ibsh;
	struct iris_image_header	iih;

	BDB bdb;
	
	INIT_BDB(&bdb,data,icd->record_length);
	*iso_data_len = icd->record_length;
	
	//init iris_record_header
	capsuleIRH(&irh,icd);
	//printInfo(&irh);
	OPUSH(&irh,sizeof(struct iris_record_header),&bdb);

	
#if 1	
	if(icd->ibsh_right != NULL)
	{
		capsuleIBSH(&ibsh,icd,EYE_RIGHT);
		OPUSH(&ibsh,IID_IBSH_LENGTH,&bdb);
		
		for(j = 0;j < icd->ibsh_right->num_images;j++)
		{
			capsuleIIH(&iih,icd,j,EYE_RIGHT);
			OPUSH(&iih,IID_IIH_LENGTH,&bdb);
			
			OPUSH(icd->ibsh_right->image_headers[j].image_data,icd->ibsh_right->image_headers[j].image_length,&bdb);
		}
	}
	
	if(icd->ibsh_left != NULL)
	{
		capsuleIBSH(&ibsh,icd,EYE_LEFT);
		OPUSH(&ibsh,IID_IBSH_LENGTH,&bdb);
		
		for(j = 0;j < icd->ibsh_left->num_images;j++)
		{
			capsuleIIH(&iih,icd,j,EYE_LEFT);
			OPUSH(&iih,IID_IIH_LENGTH,&bdb);
			
			OPUSH(icd->ibsh_left->image_headers[j].image_data,icd->ibsh_left->image_headers[j].image_length,&bdb);
		}
	}
#endif
	return 0;	
err_out:
	return -1;
}

void capsuleIIH(struct iris_image_header* iih,struct iris_custom_data* icd,unsigned int index,unsigned char right_or_left)
{
	int i = 0;
	#if 1
	if(right_or_left == EYE_RIGHT)	//right eye
	{
		//image number
		for(i = 0;i < IID_TWO_BYTE;i++)
			iih->image_number[i] = (icd->ibsh_right->image_headers[index].image_number & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
		
		//image quality
		iih->image_quality = icd->ibsh_right->image_headers[index].image_quality;
		
		//rotation angle
		for(i = 0;i < IID_TWO_BYTE;i++)
			iih->rotation_angle[i] = (icd->ibsh_right->image_headers[index].rotation_angle & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
		
		//rotation uncertainty
		for(i = 0;i < IID_TWO_BYTE;i++)
			iih->rotation_uncertainty[i] = (icd->ibsh_right->image_headers[index].rotation_uncertainty & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
		
		//image length
		for(i = 0;i < IID_FOUR_BYTE;i++)
			iih->image_length[i] = (icd->ibsh_right->image_headers[index].image_length & (0xFF000000 >> (i * 8))) >> (IID_FORMAT_ID_LEN - i - 1) * 8;
	}else if(right_or_left == EYE_LEFT)	//left eye
	{
		//image number
		for(i = 0;i < IID_TWO_BYTE;i++)
			iih->image_number[i] = (icd->ibsh_left->image_headers[index].image_number & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
		
		//image quality
		iih->image_quality = icd->ibsh_left->image_headers[index].image_quality;
		
		//rotation angle
		for(i = 0;i < IID_TWO_BYTE;i++)
			iih->rotation_angle[i] = (icd->ibsh_left->image_headers[index].rotation_angle & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
		
		//rotation uncertainty
		for(i = 0;i < IID_TWO_BYTE;i++)
			iih->rotation_uncertainty[i] = (icd->ibsh_left->image_headers[index].rotation_uncertainty & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
		
		//image length
		for(i = 0;i < IID_FOUR_BYTE;i++)
			iih->image_length[i] = (icd->ibsh_left->image_headers[index].image_length & (0xFF000000 >> (i * 8))) >> (IID_FORMAT_ID_LEN - i - 1) * 8;
	}
	#endif
}

void capsuleIBSH(struct iris_biometric_subtype_header* ibsh,struct iris_custom_data* icd,unsigned char right_or_left)
{
	int i = 0;
	#if 1
	if(right_or_left == EYE_RIGHT)	//right eye
	{
		ibsh->eye_position = icd->ibsh_right->eye_position;
		
		//number images
		for(i = 0;i < IID_TWO_BYTE;i++)
			ibsh->num_images[i] = (icd->ibsh_right->num_images & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	}else if(right_or_left == EYE_LEFT)	//left eye
	{
		ibsh->eye_position = icd->ibsh_left->eye_position;
		
		//number images
		for(i = 0;i < IID_TWO_BYTE;i++)
			ibsh->num_images[i] = (icd->ibsh_left->num_images & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	}
	#endif
}

void capsuleIRH(struct iris_record_header* irh,struct iris_custom_data* icd)
{
	int i = 0;
	
	//format id
	for(i = 0;i < IID_FORMAT_ID_LEN;i++)
		irh->format_id[i] = (IID_FORMAT_ID_HEX & (0xFF000000 >> (i * 8))) >> (IID_FORMAT_ID_LEN - i - 1) * 8;
	
	//format version number
	for(i = 0;i < IID_FORMAT_VERSION_LEN;i++)
	{
		irh->format_version[i] = (icd->format_version & (0xFF000000 >> (i * 8))) >> (IID_FORMAT_ID_LEN - i - 1) * 8;
	}
	//record length
	for(i = 0;i < IID_FOUR_BYTE;i++)
		irh->record_length[i] = (icd->record_length & (0xFF000000 >> (i * 8))) >> (IID_FOUR_BYTE - i - 1) * 8;
	
	///device id
	for(i = 0;i < IID_TWO_BYTE;i++)
		irh->capture_device_id[i] = (icd->capture_device_id & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//num eyes
	irh->num_eyes = icd->num_eyes;
	
	//record header length
	for(i = 0;i < IID_TWO_BYTE;i++)
		irh->record_header_length[i] = (IID_RECORD_HEADER_LENGTH  & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//image property bitfield
	for(i = 0;i < IID_TWO_BYTE;i++)
		irh->image_property_bitfield[i] = (icd->image_property_bitfield  & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//iris diameter
	for(i = 0;i < IID_TWO_BYTE;i++)
		irh->diameter[i] = (icd->diameter  & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//image format
	for(i = 0;i < IID_TWO_BYTE;i++)
		irh->image_format[i] = (IMAGEFORMAT_MONO_JPEG2000  & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//image width
	 for(i = 0;i < IID_TWO_BYTE;i++)
		irh->image_width[i] = (icd->image_width  & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//image height
	 for(i = 0;i < IID_TWO_BYTE;i++)
		irh->image_height[i] = (icd->image_height  & (0xFF00 >> (i * 8))) >> (IID_TWO_BYTE - i - 1) * 8;
	
	//intensity depth
	irh->intensity_depth = INTENSITY_DEPTH;
	
	//image transformation
	irh->image_transformation = icd->image_transformation;
	
	//device unique id
	for(i = 0;i < IID_DEVICE_UNIQUE_ID_LEN;i++)
		irh->device_unique_id[i] = icd->device_unique_id[i];
}
#endif	//for 2005





















