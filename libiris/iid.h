#ifndef _IID_H
#define _IID_H

/*
*********************************************
*	Iris Reacord Data Format(2005)
*********************************************
*	
*	+-------------------------------+
*	|	iris record header(45 byte) |
*	+-------------------------------+
*	Array[2]{
*		+--------------------------------------------+
*		|	iris biometric subtype header (3 bytes)  |
*		+--------------------------------------------+
*		List{
*			+------------------------------+
*			|	iris image header(11byte)  |
*			+------------------------------+
*			+------------------------------+
*			|		iris image data	       |
*			+------------------------------+
*		}
*	}
*	
*
*/
/*
*********************************************
*	Iris Reacord Data Format(2011)
*********************************************
*	
*	+-------------------------------+
*	|	iris general header(16 byte)|
*	+-------------------------------+
*	List{
*		+--------------------------------------------+
*		|	  iris representation header (52 bytes)  |
*		+--------------------------------------------+
*		+--------------------------------------------+
*		|			iris image data	 ( >= 1 byte)    |
*		+--------------------------------------------+
*	}
*	
*
*/

typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned int	uint32_t;

#define IID_FORMAT_ID				"IIR"
#define IID_FORMAT_ID_HEX				0x49495200
#define IID_FORMAT_ID_LEN			4

#define IID_FORMAT_VERSION_LEN		4

#define IID_RECORD_LENGTH_INDEX		4
#define IID_DEVIDE_ID_LEN			2
#define IID_TWO_BYTE	2
#define IID_FOUR_BYTE	4

#define IID_DEVICE_UNIQUE_ID_LEN		16
#define IMAGEFORMAT_MONO_JPEG2000		0x14
#define IMAGEFORMAT_MONO_JPEG2000_2011	0x0A
#define INTENSITY_DEPTH			0x08

#define EYE_UNDEF	0x00
#define EYE_RIGHT	0x01
#define EYE_LEFT	0x02

#define IID_RECORD_HEADER_LENGTH		45
#define IID_IBSH_LENGTH				3
#define IID_IIH_LENGTH				11

#define IID_GENERAL_HEADER_LENGTH		16
#define IID_REPRESENTATION_HEADER_LENGTH	52
#define IID_REPRE_HEADER_LEN_BEFORE_QUALITY_BLOCK	18
#define IID_REPRE_HEADER_LEN_AFTER_QUALITY_BLOCK	33

#define IID_CAPTURE_DATA_LEN		9

//quality block=quality score(1 byte)+algoritham(2 byte)+algoritham(2 byte).
#define IID_QUALITY_BLOCK_LEN		5

#define OPUSH(ptr, size, bdb)						\
	do {								\
		if (((bdb)->bdb_current + size) > (bdb)->bdb_end)	\
			goto err_out;					\
		(void)memcpy((bdb)->bdb_current, ptr, size);		\
		(bdb)->bdb_current += size;				\
	} while (0)

		
#define INIT_BDB(bdb, ptr, size) do {					\
	(bdb)->bdb_size = size;						\
	(bdb)->bdb_start = (bdb)->bdb_current = ptr;			\
	(bdb)->bdb_end = ptr + size;					\
} while (0)

//buffer for iris record data
struct biometric_data_buffer {
	uint32_t		bdb_size;	// Max size of the buffer
	uint8_t			*bdb_start;	// Beginning read/write location
	uint8_t			*bdb_end;	// Last read/write location
	uint8_t			*bdb_current;	// Current read/write location
};
typedef struct biometric_data_buffer BDB;

#if 1	//iris reocrd data for 2011
//iris general header
struct iris_general_header {
	char			format_id[IID_FORMAT_ID_LEN];
	char			format_version[IID_FORMAT_VERSION_LEN];
	uint8_t			record_length[IID_FOUR_BYTE];
	uint8_t			num_representations[IID_TWO_BYTE];
	uint8_t			cert_flag;
	uint8_t			num_eyes;
};
typedef struct iris_general_header IGH;

//iris representation header
struct iris_representation_header {
	uint8_t		representation_length[IID_FOUR_BYTE];
	uint8_t		capture_data[IID_CAPTURE_DATA_LEN];
	uint8_t		capture_device_tech_id;
	uint8_t		capture_device_vendor_id[IID_TWO_BYTE];
	uint8_t		capture_device_type_id[IID_TWO_BYTE];
	uint8_t		*quality_block;
	uint8_t		representation_num[IID_TWO_BYTE];
	uint8_t		eye_label;
	uint8_t		image_type;
	uint8_t		image_format;
	uint8_t		image_property_bitfield;
	uint8_t		image_width[IID_TWO_BYTE];
	uint8_t		image_height[IID_TWO_BYTE];
	uint8_t		bit_depth;
	uint8_t		range[IID_TWO_BYTE];
	uint8_t		rotation_angle[IID_TWO_BYTE];
	uint8_t		rotation_uncertainty[IID_TWO_BYTE];
	uint8_t		iris_centre_minX[IID_TWO_BYTE];
	uint8_t		iris_centre_maxX[IID_TWO_BYTE];
	uint8_t		iris_centre_minY[IID_TWO_BYTE];
	uint8_t		iris_centre_maxY[IID_TWO_BYTE];
	uint8_t		iris_diameter_min[IID_TWO_BYTE];
	uint8_t		iris_diameter_max[IID_TWO_BYTE];
	uint8_t		image_length[IID_FOUR_BYTE];
};
typedef struct iris_representation_header IREH;

struct iris_custom_representation_header {
	uint32_t		representation_length;
	struct tm*		capture_data;
	uint8_t			capture_device_tech_id;
	uint16_t		capture_device_vendor_id;
	uint16_t		capture_device_type_id;
	uint8_t			*quality_block;
	uint16_t		representation_num;
	uint8_t			eye_label;
	uint8_t			image_type;
	//uint8_t		image_format;
	uint8_t			image_property_bitfield;
	uint16_t		image_width;
	uint16_t		image_height;
	uint8_t			bit_depth;
	uint16_t		range;
	uint16_t		rotation_angle;
	uint16_t		rotation_uncertainty;
	uint16_t		iris_centre_minX;
	uint16_t		iris_centre_maxX;
	uint16_t		iris_centre_minY;
	uint16_t		iris_centre_maxY;
	uint16_t		iris_diameter_min;
	uint16_t		iris_diameter_max;
	uint32_t		image_length;
	uint8_t			*image_data;
};
typedef struct iris_custom_representation_header	ICRH;

struct iris_custom_data_2011 {
	uint32_t			format_version;
	uint32_t			record_length;
	uint16_t			num_representations;
	uint8_t				cert_flag;
	uint8_t				num_eyes;
	ICRH*	icrh_image;
};
#endif	//iris reocrd data for 2011

#if 1	//iris record data for 2005
//iris record header
struct iris_record_header {
	char			format_id[IID_FORMAT_ID_LEN];
	char			format_version[IID_FORMAT_VERSION_LEN];
	uint8_t			record_length[IID_FOUR_BYTE];
	uint8_t			capture_device_id[IID_TWO_BYTE];
	uint8_t			num_eyes;
	uint8_t			record_header_length[IID_TWO_BYTE];
	uint8_t			image_property_bitfield[IID_TWO_BYTE];
	uint8_t			diameter[IID_TWO_BYTE];
	uint8_t			image_format[IID_TWO_BYTE];
	uint8_t			image_width[IID_TWO_BYTE];
	uint8_t			image_height[IID_TWO_BYTE];
	uint8_t			intensity_depth;
	uint8_t			image_transformation;
	char			device_unique_id[IID_DEVICE_UNIQUE_ID_LEN];
};
typedef struct iris_record_header IRH;

//iris image header
struct iris_image_header {
	uint8_t				image_number[IID_TWO_BYTE];
	uint8_t					image_quality;
	uint8_t				rotation_angle[IID_TWO_BYTE];
	uint8_t				rotation_uncertainty[IID_TWO_BYTE];
	uint8_t				image_length[IID_FOUR_BYTE];
	//uint8_t					*image_data;
};
typedef struct iris_image_header IIH;

//iris biometric subtype header
struct iris_biometric_subtype_header {
	uint8_t					eye_position;
	uint8_t				num_images[IID_TWO_BYTE];
	struct iris_image_header*	image_headers;
};
typedef struct iris_biometric_subtype_header IBSH;


struct iris_custom_image_header {
	uint16_t				image_number;
	uint8_t					image_quality;
	uint16_t				rotation_angle;
	uint16_t				rotation_uncertainty;
	uint32_t				image_length;
	uint8_t					*image_data;
};
typedef struct iris_custom_image_header ICIH;

struct iris_custom_biometric_subtype_header {
	uint8_t					eye_position;
	uint16_t				num_images;
	struct iris_custom_image_header*	image_headers;
};
typedef struct iris_custom_biometric_subtype_header ICBSH;

struct iris_custom_data {
	uint32_t 			format_version;
	uint32_t		record_length;
	uint16_t		capture_device_id;
	uint8_t			num_eyes;
	//uint16_t		record_header_length;
	uint16_t		image_property_bitfield;
	uint16_t		diameter;
	//uint16_t		image_format;
	uint16_t		image_width;
	uint16_t		image_height;
	//uint8_t			intensity_depth;
	uint8_t			image_transformation;
	char			device_unique_id[IID_DEVICE_UNIQUE_ID_LEN];

	/*uint16_t		num_right_eye_iris_image;
	IIH*	iih_right;
	uint16_t		num_left_eye_iris_image;
	IIH*	iih_left;*/
	ICBSH*	ibsh_right;
	ICBSH*	ibsh_left;
};
#endif	//iris record data for 2005
/*
*Function description：Pack iris records for 2005
*parameter：
*	icd(in)		:User Settings information
*	data(out)	:iso format for iris image data
*	iso_data_len(out)	:data len
*return:
*		0:success	-1:error
*/
int capsuleISO2005(struct iris_custom_data* icd,unsigned char* data,int* iso_data_len);
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
int getRecordLen2005(unsigned int imageSize,unsigned int numEye,unsigned int numRight,unsigned int numLeft);

/*
*Function description：get iris general data length for 2011
*parameter：
*	imageSize：total size of iris image
*	numRepre   :	total num of representations
*	numQual	   :	total num of quality block
*return：
*		iris general data length
*/
int getRecordLen2011(unsigned int imageSize,unsigned int numRepre,unsigned int numQual);

/*
*Function description：Pack iris records for 2011
*parameter：
*	icd(in)		:User Settings information
*	data(out)	:iso format for iris image data
*	iso_data_len(out)	:data len
*return:
*		0:success	-1:error
*/
int capsuleISO2011(struct iris_custom_data_2011* icd,unsigned char* data,int* iso_data_len);
#endif 	/* _IID_H */
