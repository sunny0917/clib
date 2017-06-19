#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "iid.h"

void capsuleIRH(struct iris_record_header* irh,struct iris_custom_data* icd);
void capsuleIIH(struct iris_image_header* iih,struct iris_custom_data* icd,unsigned int index,unsigned char right_or_left);
void capsuleIBSH(struct iris_biometric_subtype_header* ibsh,struct iris_custom_data* icd,unsigned char right_or_left);

/*
*Function description：Pack iris records
*parameter：
*	icd(in)		:User Settings information
*	data(out)	:iso format for iris image data
*	iso_data_len(out)	:data len
*return:
*		0:success	-1:error
*/
int getRecordLen(unsigned int imageSize,unsigned int numEye,unsigned int numRight,unsigned int numLeft)
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
*Function description：get iris record data length
*parameter：
*	imageSize：total size of iris image
*	numEye   :	num of eyes
*	numRight ：iris image num of right eye
*	numLeft  ：iris image num of left eye
*return：
*		iris record data length
*/
int capsuleISO(struct iris_custom_data* icd,unsigned char* data,int* iso_data_len)
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






















