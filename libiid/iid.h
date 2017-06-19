#ifndef _IID_H
#define _IID_H

/*
*********************************************
*	Iris Reacord Data Format
*********************************************
*	
*	+-------------------------------+
*	|	iris record header(45byte)  |
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
#define INTENSITY_DEPTH			0x08

#define EYE_UNDEF	0x00
#define EYE_RIGHT	0x01
#define EYE_LEFT	0x02

#define IID_RECORD_HEADER_LENGTH		45
#define IID_IBSH_LENGTH				3
#define IID_IIH_LENGTH				11

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

/*
*Function description：Pack iris records
*parameter：
*	icd(in)		:User Settings information
*	data(out)	:iso format for iris image data
*	iso_data_len(out)	:data len
*return:
*		0:success	-1:error
*/
int capsuleISO(struct iris_custom_data* icd,unsigned char* data,int* iso_data_len);
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
int getRecordLen(unsigned int imageSize,unsigned int numEye,unsigned int numRight,unsigned int numLeft);
#endif 	/* _IID_H */
